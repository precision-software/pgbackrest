/*********

**********/
#include "common/type/collection.h"

#include "common/type/list.h"

// Create the iterable Collection interface for a List.
#define CAMEL_List lst
DEFINE_COLLECTION(List, void)

/***********************************************************************************************************************************
Hack so the list iterator will call back when its memory is freed.
***********************************************************************************************************************************/
#define lstItrNew callbackItrNew                                    // invoke callbackItrNew rather than lstItrNew
int eventCount;                                                     // Counter is incremented by the callback.
ListItr *callbackItrNew(List *list);                                // Forward reference.

/***********************************************************************************************************************************
Test Run
***********************************************************************************************************************************/
static void
testRun(void)
{
    FUNCTION_HARNESS_VOID();

    const int testMax = 100;
    int *item;
    List *emptyList;
    List *longList;
    Collection *emptyCollection;
    Collection *longCollection;

    if (testBegin("setup collections"))
    {
        // Create an empty list
        emptyList = lstNewP(sizeof(int));

        // Create a long list
        longList = lstNewP(sizeof(int));
        for (int listIdx = 0; listIdx < testMax; listIdx++)
            lstAdd(longList, &listIdx);
        ASSERT(lstSize(longList) == (unsigned int) testMax);

        emptyCollection = collectionNew(List, emptyList);
        longCollection = collectionNew(List, longList);
    }

    /******************************************************************************************************************************/
    if (testBegin("foreach List Iteration"))
    {
        // Scan the empty list.
        foreach(item, List, emptyList)
            ASSERT_MSG("iterating through an empty list");// {uncoverable - this statement should not be reached}

        ASSERT(item == NULL);
        TEST_RESULT_VOID((void) 0, "scan empty list");

        // Scan the bigger list.
        int count = 0;
        foreach(item, List, longList)
        {
            ASSERT(*item == count);
            count++;
        }
        ASSERT(item == NULL);
        TEST_RESULT_INT(count, testMax, "non-empty List");
    }

    if (testBegin("FOREACH List iteration"))
    {
        // Scan the empty list,
        FOREACH(item, List, emptyList)
            ASSERT_MSG("iterating through an empty container");     // {uncoverable - this statement should not be reached}
        ENDFOREACH;

        ASSERT(item == NULL);
        TEST_RESULT_VOID((void)0, "empty list ");

        // Scan the longer list, inside a collection.
        int count = 0;
        FOREACH(item, List, longList)
            ASSERT(*item == count);
            count++;
        ENDFOREACH;

        ASSERT(item == NULL);
        TEST_RESULT_INT(count, testMax, "non-empty list inside Collection");

        // Try to get next() item of an empty list.
        ListItr *itr = lstItrNew(emptyList);
        ASSERT(lstItrNext(itr) == NULL);
        TEST_RESULT_PTR(lstItrNext(itr), NULL, "iterate beyond end of empty List");
        objFree(itr);

        // Similar, but this time with items in the list.
        itr = lstItrNew(longList);  // A second iterator in parallel
        FOREACH(item, List, longList)
            ASSERT(*(int *)lstItrNext(itr) == *item);
        ENDFOREACH;

        ASSERT(item == NULL);
        TEST_RESULT_PTR(lstItrNext(itr), NULL, "iterate beyond end of List");
        objFree(itr);
    }

    if (testBegin("FOREACH Collection iteration"))
    {
        // Scan the empty list, inside a collection
        FOREACH(item, Collection, emptyCollection)
            ASSERT_MSG("iterating through an empty container");     // {uncoverable - this statement should not be reached}
        ENDFOREACH;

        ASSERT(item == NULL);
        TEST_RESULT_VOID((void)0, "empty list inside Collection");

        // Scan the longer list, inside a collection.
        int count = 0;
        FOREACH(item, Collection, longCollection)
            ASSERT(*item == count);
            count++;
        ENDFOREACH;

        ASSERT(item == NULL);
        TEST_RESULT_INT(count, testMax, "non-empty list inside Collection");

        // Try to get next() item of an empty list.
        CollectionItr *itr = collectionItrNew(emptyCollection);
        ASSERT(collectionItrNext(itr) == NULL);
        TEST_RESULT_PTR(collectionItrNext(itr), NULL, "iterate beyond end of empty List");
        objFree(itr);

        // Similar, but this time with items in the list.
        itr = collectionItrNew(longCollection);  // A second iterator in parallel
        FOREACH(item, List, longList)
            ASSERT(*(int *) collectionItrNext(itr) == *item);
        ENDFOREACH;

        ASSERT(item == NULL);
        TEST_RESULT_PTR(collectionItrNext(itr), NULL, "iterate beyond end of List");
        objFree(itr);

        // Try to create a Collection from NULL list.
        TEST_ERROR((void) collectionNew(List, NULL), AssertError, "assertion 'subCollection != NULL' failed");

        // Create a Collection within a Collection and verify we can still iterate through it.
        Collection *superCollection = collectionNew(Collection, longCollection);
        count = 0;
        FOREACH(item, Collection, superCollection)
                                ASSERT(*item == count);
                                count++;
        ENDFOREACH;
        TEST_RESULT_INT(count, testMax, "Collection inside Collection");
    }

    if (testBegin("destructors invoked"))
    {
        // Verify the destructor gets called on a list with no exceptions.
        eventCount = 0;
        FOREACH(item, List, longList)
        ENDFOREACH;

        TEST_RESULT_INT(eventCount, 1, "destructor invoked after loop ends");

        // Throw an exception within a loop and verify the destructor gets called.
        eventCount = 0;
        TRY_BEGIN()
            FOREACH(item, List, longList)
                if (*item > testMax / 2)
                THROW(FormatError, "");  // Any non-fatal error.
            ENDFOREACH;
        CATCH(FormatError)
            eventCount++; // An extra increment to verify we catch the error.
        TRY_END();

        ASSERT(*item = testMax/2 + 1);
        TEST_RESULT_INT(eventCount, 2, "destructor invoked after exception");

        eventCount = 0;
        FOREACH(item, Collection, longCollection)
        ENDFOREACH;

        TEST_RESULT_INT(eventCount, 1, "destructor invoked after Collection ends");
    }

FUNCTION_HARNESS_RETURN_VOID();
}

void
destructor(void *this)
{
    extern int eventCount;
    eventCount++;
    (void)this;
}

/***********************************************************************************************************************************
Wrap a list iterator in a context which includes a callback destructor.
***********************************************************************************************************************************/
#undef lstItrNew
ListItr *callbackItrNew(List *list)
{
    ListItr *this = NULL;
    OBJ_NEW_BEGIN(ListItr, .callbackQty=1, .childQty=1)
        {
            this = lstItrNew(list);
            memContextCallbackSet(memContextCurrent(), destructor, this);
        }
    OBJ_NEW_END();

    return this;
}
