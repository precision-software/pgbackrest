/***********************************************************************************************************************************
Define the interface for an abstract, iterable Collection and provide "syntactic sugar" for scanning across collections.
For more information on how to create an "Iterable" collection, see the "collection.c" file.
These interfaces are inspired by Rust's Collection and Iterator traits.

= List Interface
The simplest iteration is through Lists, where a List is any type which implements the Size() and Get() methods.

To be considered a List, the type must provide
     unsigned listTypeSize(list);
     ItemType listTypeGet(list, index);
     #define CAMEL_ListType  listType

== foreach looping through Lists.
ListTypes can use the light weight "foreach" macro to scan through a list.
     String *str;
     foreach(String, str, StringList, strLst)
        doSomething(*str)

= Collection Interface
A Collection is the next level of complexity from List. Collections are iterable and can be scanned from start to finish.
Each collection type has a companion type CollectionTypeItr which holds the current state of the scan.

To be considered a "Collection", the type must provide the following:
  typedef CollectionTypeItr;                                        // Definition of the companion iterator type.
  CollectionTypeItr *iterator = collectionTypeItrNew(CollectionType *collection)       // Create an iterator from the collection.
  ItemType *item = collectionTypeItrNext(iterator)                  // Point to the next item in the collection, NULL when done.
  #define CAMEL_CollectionType collectionType                       // A macro converting the TypeName to camelCase.

For convenience, any ListType can be extended to be a CollectionType.
    DEFINE_LIST_COLLECTION(ItemType, ListType)

== FOREACH looping through Collections.
Any collection which implements the above interface can be scanned using the FOREACH "syntactic sugar".
For example, to scan a list:
    ItemType *item;
    FOREACH(item, ItemList, listOfItems)
        doSomething(*item);
    ENDFOREACH;

Unlike List iteration, FOREACH iteration manages memory contexts as it iterates.
 1) "Prior" context is the context which exists outside the loop.
 2) "Loop Control" context contains the iterator itself and is destroyed when the loop exits. It is hidden from the loop body.
 3) "Loop" context is a temporary context which is created and destroyed with every loop iteration.

(Note: actual implementation differs; the "Loop" context is periodically reset rather than created/destroyed.)

= Abstract Collections.
This file also defines an abstract Collection, which is a wrapper around any iterable collection.
Once wrapped, the different types of collections can be passed around and the users don't need to know the details of how
the underlying collection is implemented.

    // Construct an abstract Collection from a collection of information about files.
    Collection *fileCollection = collectionNew(ListType, list);

    // Iterate through the abstract Collection just like any other collection.
    ItemType *item;
    FOREACH(item, Collection, fileCollection)
        doSomething(*item)
    ENDFOREACH;
***********************************************************************************************************************************/
#ifndef COMMON_TYPE_COLLECTION_H
#define COMMON_TYPE_COLLECTION_H

#include <stdbool.h>
#include "common/assert.h"
#include "common/type/object.h"
#include "common/type/string.h"

// A Polymorphic interface for Iterable collections.
typedef struct Collection Collection;

/***********************************************************************************************************************************
Construct an abstract Collection from a concrete collection (eg. List)
Use a macro front end so we can support polymorphic collections.
Note we depend on casting between compatible function pointers where return value and arguments must also be compatible.
    (void *  and struct * are compatible)
***********************************************************************************************************************************/
#define collectionNew(SubType, subCollection)                                                                                      \
    collectionNewHelper(                                                                                                           \
        subCollection,                                              /* The collection we are wrapping */                           \
        (void *(*)(void*))METHOD(SubType,ItrNew),                   /* Get an iterator to the collection  */                       \
        (void *(*)(void*))METHOD(SubType,ItrNext)                   /* Get next item using the iterator  */                        \
    )

// Helper to construct an abstract collection.
Collection *collectionNewHelper(void *subCollection, void *(*newItr)(void*), void *(*next)(void*));

// Define the methods so an abstract Collection implements the Collection Interface.
typedef struct CollectionItr CollectionItr;
CollectionItr *collectionItrNew(Collection *collection);
void *collectionItrNext(CollectionItr *this);
#define CAMEL_Collection collection

/***********************************************************************************************************************************
Syntactic sugar to make iteration look like C++ or Python.
FOREACH and ENDFOREACH are block macros, so the overall pair must be terminated with a semicolon.
TODO: Consider rewriting with direct calls to memContext routines.
***********************************************************************************************************************************/
#define FOREACH(item, CollectionType, collection)                                                                                  \
    BEGIN                                                                                                                          \
        item = NULL;                                                                                                               \
        CollectionType *FOREACH_collection = (collection);  /* Be sure to evaluate "collection" only once. */                      \
        if (FOREACH_collection != NULL) /* Convenience feature - treat a NULL collection as an empty one. */                       \
        {                                                                                                                          \
            MEM_CONTEXT_TEMP_BEGIN()  /* Memory context for loop control */                                                        \
                CollectionType##Itr *FOREACH_itr = METHOD(CollectionType,ItrNew)(FOREACH_collection);                              \
                MEM_CONTEXT_PRIOR_BEGIN()  /* Hide the loop control memory context */                                              \
                    MEM_CONTEXT_TEMP_RESET_BEGIN()   /* Memory context for loop body. Optimized to reset every N iterations */     \
                        while ( (item = METHOD(CollectionType,ItrNext)(FOREACH_itr)) != NULL)                               \
                        {
#define ENDFOREACH                                                                                                                 \
                            MEM_CONTEXT_TEMP_RESET(FOREACH_RESET_COUNT);                                                           \
                        }                                                                                                          \
                   MEM_CONTEXT_TEMP_END();                                                                                         \
               MEM_CONTEXT_PRIOR_END();                                                                                            \
           MEM_CONTEXT_TEMP_END();                                                                                                 \
        }                                                                                                                          \
    END

#define FOREACH_RESET_COUNT  1000    /* Redefine as needed. */

/***********************************************************************************************************************************
A much simpler iteration macro for List classes which do their own memory context management.
where a List class implements the Size() and Next() methods.
   ItemType *item;
   foreach(ItemType, item, ListType, list)
       doSomething(*item);
***********************************************************************************************************************************/
#define foreach(ItemType, itemP, ListType, list)                                                                                   \
    for (struct {unsigned int idx; ListType *lst; ItemType item;}_iter = {0, list};                                                \
        (itemP = (_iter.idx < METHOD(ListType,Size)(_iter.lst))                                                                    \
            ? (_iter.item=METHOD(ListType,Get)(_iter.lst, _iter.idx++),&_iter.item)                                                \
            : NULL);                                                                                                               \
        )

/***********************************************************************************************************************************
Create the Collection interface for ListType, where ListType is any class which implements Size() and Next() methods.
Note "ItemType" must match the type returned by listTypeGet().
Experimental - generally prefer to avoid complex macros.
***********************************************************************************************************************************/
#define DEFINE_LIST_COLLECTION(ItemType, ListType)                                                                                 \
    typedef struct ListType##Itr                                                                                                   \
    {                                                                                                                              \
        unsigned int idx;                                                                                                          \
        ListType *list;                                                                                                            \
        ItemType item;                                                                                                             \
    } ListType##Itr;                                                                                                               \
                                                                                                                                   \
    FN_INLINE_ALWAYS ListType##Itr *                                                                                               \
    METHOD(ListType,ItrNew)(ListType *list)                                                                                        \
    {                                                                                                                              \
        ListType##Itr *this;                                                                                                       \
        OBJ_NEW_BEGIN(ListType##Itr)                                                                                               \
            this = OBJ_NEW_ALLOC();                                                                                                \
            *this = (ListType##Itr) {.idx=0, .list=list};                                                                          \
        OBJ_NEW_END();                                                                                                             \
        return this;                                                                                                               \
    }                                                                                                                              \
                                                                                                                                   \
    FN_INLINE_ALWAYS ItemType *                                                                                                    \
    METHOD(ListType,ItrNext)(ListType##Itr *this)                                                                                  \
    {                                                                                                                              \
        ItemType *item = (this->idx >= METHOD(ListType,Size)(this->list))                                                          \
            ? NULL                                                                                                                 \
            : (this->item = METHOD(ListType,Get)(this->list, this->idx++), &this->item);                                           \
        return item;                                                                                                               \
    }

// Syntactic sugar to get the full method name from the object type and the short method name.
// Used to generate method names for abstract interfaces.
//    eg.   METHOD(List,Get) --> listGet
// Note it takes an (obscure) second level of indirection and a "CAMEL_Type" macro to make this work.
#define METHOD(type, method)  JOIN(CAMEL(type), method)
#define JOIN(a,b) JOIN_AGAIN(a,b)
#define JOIN_AGAIN(a, b) a ## b

// We can't really convert to camelCase, but we can invoke the symbol CAMEL_<TypeName> to get it.
// Each of the collection types must provide a CAMEL_* macro to provide the type name in camelCase.
// In the case of iterators, we append Itr to the collection type, so we don't have to create CAMEL_*Itr.
// Note we don't have to be strict about camelCase. For example, "CAMEL_StringList" could be defined as "strLst",
// in which case METHOD(StringList, Move) would become strLstMove.
#define CAMEL(type) CAMEL_##type

// Universal block macros. Belong in a different file, but for the moment this is the only file which uses them.
#define BEGIN do {
#define END   } while (0)


/***********************************************************************************************************************************
Macros for function logging.
***********************************************************************************************************************************/
#define FUNCTION_LOG_COLLECTION_TYPE                                                                                               \
    Collection *
#define FUNCTION_LOG_COLLECTION_FORMAT(value, buffer, bufferSize)                                                                  \
    FUNCTION_LOG_STRING_OBJECT_FORMAT(value, collectionToLog, buffer, bufferSize)
String *collectionToLog(const Collection *this);

#define FUNCTION_LOG_COLLECTION_ITR_TYPE                                                                                           \
    CollectionItr *
#define FUNCTION_LOG_COLLECTION_ITR_FORMAT(value, buffer, bufferSize)                                                                  \
   FUNCTION_LOG_STRING_OBJECT_FORMAT(value, collectionItrToLog, buffer, bufferSize)
String *collectionItrToLog(const CollectionItr *this);


#endif //COMMON_TYPE_COLLECTION_H
