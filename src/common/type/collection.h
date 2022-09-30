/***********************************************************************************************************************************
Define the interface for an abstract, iterable Collection and provide "syntactic sugar" for scanning across collections.
For more information on how to create an "Iterable" collection, see the "collection.c" file.
These interfaces are inspired by Rust's Collection and Iterator traits.

= Interface Definition
Each iterable collection "CollectionType" must provide the following:
  typedef CollectionType                                            // A structure defining the iterable collection
  typedef CollectionTypeItr                                         // A structure for iterating through the collection
  iterator = collectionTypeItrNew(CollectionType *collection)       // Create an iterator from the collection.
  itemPtr = collectionTypeItrNext(iterator)                         // Point to the next item in the collection, NULL when done.
  #define CAMEL_CollectionType collectionType                       // A macro converting the TypeName to lower camelCase.

= FOREACH looping
Any collection type which implements the above interface can be scanned using the FOREACH "syntactic sugar".
For example, to scan a list:
    ItemType *item;
    FOREACH(item, List, list)
        doSomething(*item);
    ENDFOREACH;

= Abstract Collections
This file also defines an abstract Collection type, which is a wrapper around any iterable collection.
Once wrapped, the different types of collections can be passed around and the users don't need to know the details of how
the underlying collections are implemented.

    // Construct an abstract Collection which wraps the List.
    Collection *collection = collectionNew(List, list);

    // Iterate through the abstract Collection just like any other collection.
    ItemType *item;
    FOREACH(item, Collection, collection)
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
    collectionNewHelper(                                                                                                                 \
        subCollection,                                              /* The collection we are wrapping */                           \
        (void *(*)(void*))METHOD(SubType, ItrNew),                  /* Get an iterator to the collection  */                       \
        (void *(*)(void*))METHOD(SubType, ItrNext)                  /* Get next item using the iterator  */                        \
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
                CollectionType##Itr *FOREACH_itr = METHOD(CollectionType, ItrNew)(FOREACH_collection);                             \
                MEM_CONTEXT_PRIOR_BEGIN()  /* Hide the loop control memory context */                                              \
                    MEM_CONTEXT_TEMP_RESET_BEGIN()   /* Memory context for loop body. Optimized to reset every N iterations */     \
                        while ( (item = METHOD(CollectionType,ItrNext)(FOREACH_itr)) != NULL)                                      \
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
A much simpler iteration macro for cases where memory contexts don't matter. (eg. simple scanning of a list)
   ItemType *item;
   foreach(item, List, list)
       doSomething(*item);
***********************************************************************************************************************************/
#define foreach(item, CollectionType, collection) \
    for (CollectionType##Itr _itr = METHOD(CollectionType,ItrNew)(collection); (item = METHOD(CollectionType,ItrNext)(_itr));)

// Syntactic sugar to get the function name from the object type and the short method name.
// Used to generate method names for abstract interfaces.
//    eg.   METHOD(List, Get) --> listGet
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
