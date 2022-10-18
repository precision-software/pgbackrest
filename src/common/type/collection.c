/***********************************************************************************************************************************
TODO: this header repeats retVal in collection.h. Clean things up between the two files.
TODO: break out unit tests into a separate type-collection test.

 This file contains more detailed documentation on iterators. It is of interest when creating a new iterator, where
the comments in the header file are geared to using existing iterators.

This interface is inspired by the need to iterate through diverse types of storage directories.
It consists of two pieces.
 1) It defines methods which can efficiently scan through specific collections.
 2) It defines an abstract Collection which can scan through an underlying collection without knowing the details
    of the underlying collection.

= Collection Properties
Collections are iterable sets of items. While collections vary widely in their implementation, as far as wa are concerened,
they have two property in common:
    newItr(collection) creates an iterator object for scanning through that particular collection.
    ItrNext(iterator)  finds the next item in the collection, returning NULL if no more.

Compared to similar traits in Rust, a collection implements the "Iterable" interface and an iterator implements the
"Iterator" trait. The method names are different, but the functionality is essentially the same.

= Naming conventions for specific collections.  (eg "List")
By convention, Pgbackrest uses camelCase method names, where the type name is followed vy the "short" method name.
For example, the "get" method on a List is called "listGet". (actually, it is "lstGet, but let's ignore that for now)

The C preprocessor isn't able to generate camelCase names, so it requires a small "trick" when generating method names.
For each concrete "MyType" which is going to be abstracted, provide the following macro:
    #define CAMEL_MyType myType

Now the C preprocessor can generate full method names in camelCase.
    METHOD(MyType,Get)   -->   myTypeGet

= Goals
The immediate task is to create an interface for scanning through directories from different storage managers.
However, this task is an example of two more general issues - 1) scanning through abstract sets of abjects,
and even more generally 2) Implementing abstract interfaces automatically.

The goals are as follows:
 - define a uniform set of methods for scanning through various types of collections.
   These are "ItrNew()" to create an iterator, and "IterNext()" to advance to the next item.
 - create "syntactic sugar" to make it easy to use the new iterators.
   This are the "FOREACH"/"ENDFOREACH" macros.
 - implement an abstract "Collection", wrapping an underlying collection, which iterates the underlying collection.
   This is the collectionNew() macro.
 - maintain performance and memory comparable to scanning through the collections "by hand" (ie not using the interface)
   Partially met. The current implementation dynamically allocates iterators (which could be stack based),and they
   create temporary memory contexts to prevent memory leaks. These steps can reduce efficency, but they avoid some
   programming mistakes.

= Example: iterating through a List.
Here are some examples of how to iterate through a List.

== Without the iterator interface,
   for (int listIdx=0;  listIdx < lstSize(list); listIdx++) {
       ItemType *item = (ItemType *)lstGet(list, listIdx);
       doSomething(*item)
   }

== A Lightweight iteration.
   ItemType *item;
   foreach(item, List, list)
       do_something(*item)

== Using the more heavyweight interface, useful for all iterable collections and cleaning up after exceptions.
    ItemType *item;
    FOREACH(item, List, list)
        doSomething(*item);
    ENDFOREACH

== Performance and memory.
 - iterators are allocated dynamically, so there is some performance overhead.
 - iterators can be inlined, making the per-loop overhead very low.  (consider optimizing with -flto)
 - memory allocated inside A FOREACH loop will be freed as the loop progresses. (overhead creating temporary memory contexts)
 - iterators can be configured with callbacks (eg close a file), so it is not necessary to explicitly catch exceptions.

= Abstract Collection
An abstract "Collection" is a polymorphic wrapper around anything which iterates through items. Like abstract collections
in C++ or Rust, a Collection (and its iterator) depend on function pointers to the underlying methods.
Adding a level of indirection has a performance penalty, but the penalty is comparable to any C++ program using abstract
data types.

The following code shows how to construct an abstract Collection and iterate through it using "syntactic sugar" macros.

    // Construct an abstract Collection which wraps the List.
    Collection collection[] = NEWCONTAINER(List, list);

    // Iterate through the abstract Collection just like any other collection.
    FOREACH(ItemType, item, Collection, collection)
        doSomething(*item)
    ENDFOREACH

One new goal is to support a "generator" object, where a program loop is transformed into an iterator. This object
will allow scanning through diverse data structures, say XML documents, without creating intermediate Lists.
***********************************************************************************************************************************/
#include "build.auto.h"  // First include in all C files
#include "common/debug.h"
#include "common/assert.h"
#include "common/type/collection.h"

// Abstract collection of items which can be iterated.
// This is an opaque object with no public fields.
struct Collection
{
    void *subCollection;                                            // Pointer to the underlying collection.
    void *(*newItr)(void *collection);                              // Method to construct a new iterator.
    void *(*next)(void *this);                                      // Method to get a pointer to the next item. NULL if no more.
    //int itrSize;                                                  // Size to allocate for the underlying iterator. (not now)
};

// An iterator though an abstract collection.
typedef struct CollectionItr
{
    void *subIterator;                                              // An iterator to the underlying collection.
    void *(*next)(void *this);                                      // subIterator's "next" method
} CollectionItrPub;

/***********************************************************************************************************************************
Create a new abstract Collection from a specific collection (such as List).
 @param subCollection - the specific collection being presented as a Collection.
 @param newItr - the collections function for creating an iterator.
 @param next - the collection iterqtor method for selecting the next item.
 @return - abstract Collection containing the original one.
 TODO: create empty collection if passed NULL.
***********************************************************************************************************************************/
Collection *collectionNewHelper(void *subCollection, void *(*newItr)(void *), void *(*next)(void *))
{

        FUNCTION_TEST_BEGIN();
            FUNCTION_TEST_PARAM_P(VOID, subCollection);
            FUNCTION_TEST_PARAM(FUNCTIONP, newItr);
            FUNCTION_TEST_PARAM(FUNCTIONP, next);
        FUNCTION_TEST_END();
        ASSERT(subCollection != NULL);
        ASSERT(newItr != NULL);
        ASSERT(next != NULL);

        Collection *this;
        OBJ_NEW_BEGIN(Collection)
        {
            // Allocate memory for the Collection object.
            this = OBJ_NEW_ALLOC();

            // Fill in the fields including the jump table pointers and a new iterator to the subCollection.
            *this = (Collection) {
                .subCollection = subCollection,
                .next = next,
                .newItr = newItr,
            };
        }
        OBJ_NEW_END();

        FUNCTION_TEST_RETURN(COLLECTION, this);
}


/***********************************************************************************************************************************
Construct an iterator to scan through the abstract Collection.
***********************************************************************************************************************************/
CollectionItr *
collectionItrNew(Collection *collection)
{
    FUNCTION_TEST_BEGIN();
        FUNCTION_TEST_PARAM(COLLECTION, collection);
    FUNCTION_TEST_END();

    CollectionItr *this;
    OBJ_NEW_BEGIN(CollectionItr, .childQty=1)
    {
        // Allocate memory for the Collection object.
        this = OBJ_NEW_ALLOC();

        // Fill in the fields including the jump table pointers and the iterator to the subCollection.
        //  Note we allocate the subiterator inside our memory context, so it will be freed when the iterator is freed.
        *this = (CollectionItr) {
            .next = collection->next,
            .subIterator = collection->newItr(collection->subCollection),
        };
    }
    OBJ_NEW_END();

    FUNCTION_TEST_RETURN(COLLECTION_ITR, this);
}

/***********************************************************************************************************************************
Iterate to the next item in the Collection.
  As a future optimization, consider inlining with the -flto compiler option.
***********************************************************************************************************************************/
void *
collectionItrNext(CollectionItr *this)
{
    FUNCTION_TEST_BEGIN();
        FUNCTION_TEST_PARAM(COLLECTION_ITR, this);
    FUNCTION_TEST_END();

    // Advance the sub iterator to get the next item.
    void *item = this->next(this->subIterator);

    FUNCTION_TEST_RETURN_P(VOID, item);
}

// Display an abstract Collection. For now, just the address of the sub-collection.
String *collectionToLog(const Collection *this)
{
    return strNewFmt("Collection{.subCollection=%p}", this->subCollection);
}

String *collectionItrToLog(const CollectionItr *this)
{
    return strNewFmt("CollectionItr{.subIterator=%p}", this->subIterator);
}