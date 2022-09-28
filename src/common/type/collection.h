/***********************************************************************************************************************************
Define the interface for an abstract, iterable Collection and provide "syntactic sugar" for scanning across collections which
implement the "IterNew()" method. For information on how to create an "Iterable" collection, see the "Iterator.c" file.
These interfaces are inspired by Rust's Collection and Iterable traits.

Using "syntactic sugar" to scan a list,
    ItemType *item;
    FOREACH(item, List, list)
        doSomething(*item);
    ENDFOREACH;

This file also defines an abstract Collection type, which is a wrapper around any iterable collection.
Once wrapped, the Collection can be passed around and the users don't need to know the details of what type
of collection is inside.

    // Construct an abstract Collection which wraps the List.
    Collection *collection = NEWCOLLECTION(List, list);

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
#define NEWCOLLECTION(SubType, subCollection)                                                                                      \
    collectionNew(                                                                                                                 \
        subCollection,                                              /* The collection we are wrapping */                           \
        (void *(*)(void*))METHOD(SubType, ItrNew),                  /* Get an iterator to the collection  */                       \
        (void *(*)(void*))METHOD(SubType, ItrNext)                  /* Get next item using the iterator  */                        \
    )

// Helper to construct an abstract collection.
Collection *collectionNew(void *subCollection, void *(*newItr)(void*), void *(*next)(void*));

// Iterator to scan an abstract Collecton.
typedef struct CollectionItr CollectionItr;
#define CAMEL_Collection collection
CollectionItr *collectionItrNew(Collection *collection);
void *collectionItrNext(CollectionItr *this);

/***********************************************************************************************************************************
Syntactic sugar to make iteration look like C++ or Python.
Note FOREACH and ENDFOREACH are block macros, so the overall pair must be terminated with a semicolon.

There are three memory contexts used in FOREACH looping.
1) The original outer context used to collect results from the loop.
   It can be accessed with memContextPrior() or MEM_CONTEXT_PRIOR_BEGIN().
2) The loop control context which holds the iterator (and the collection if the collection arg is a constructor function).
   This context is invisible to the loop body and should not be accessed.
3) The loop body context which (logically) is created and destroyed with every loop iteration.
   As an optimization, the loop body context is created once and reset every n iterations.

This arrangement creates a somewhat complex nesting of memory contexts, but it provides automatic cleanup of memory
and should simplify code inside the loop as well as in the iterators.
Consider rewriting with direct calls to memContext routines.
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

// A much simpler iteration macro for cases where memory contexts don't matter.
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
