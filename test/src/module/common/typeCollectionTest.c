/*********

**********/
#include "common/type/collection.h"
#include "common/type/list.h"


/***********************************************************************************************************************************
Create a test collection which returns a range of unsigned integers 0..n
***********************************************************************************************************************************/
typedef struct Range
{
    unsigned int size;
} Range;
#define CAMEL_Range range
Range* rangeNew(unsigned int size)
{
    Range *this;
    OBJ_NEW_BEGIN(Range);
        {
            this = OBJ_NEW_ALLOC();
            *this = (Range){.size=size};
        }
    OBJ_NEW_END();
    return this;
}
unsigned int rangeGet(Range *this, unsigned int idx) {(void)this; return idx;}
unsigned int rangeSize(Range *this) {return this->size;}
DEFINE_LIST_COLLECTION(unsigned, Range);

#define rangeItrNew callbackItrNew                                  // invoke callbackItrNew rather than lstItrNew
int eventCount;                                                     // Counter is incremented by the callback.
RangeItr *callbackItrNew(Range *range);                             // Forward reference.

/***********************************************************************************************************************************
Test Run
***********************************************************************************************************************************/
static void
testRun(void)
{
    FUNCTION_HARNESS_VOID();

    const int testMax = 100;
    unsigned int *item;
    Range *emptyRange;
    Range *longRange;
    Collection *emptyCollection;
    Collection *longCollection;
    unsigned int count;


    if (testBegin("setup collections"))
    {
        // Create an empty Range
        emptyRange = rangeNew(0);
        longRange = rangeNew(100);
        emptyCollection = collectionNew(Range, emptyRange);
        longCollection = collectionNew(Range, longRange);
    }

    /******************************************************************************************************************************/
    if (testBegin("foreach Range Iteration"))
    {
        // Scan the empty Range.
        foreach(unsigned, item, Range, emptyRange)
            ASSERT_MSG("iterating through an empty Range");// {uncoverable - this statement should not be reached}

        ASSERT(item == NULL);
        TEST_RESULT_VOID((void)0, "scan empty Range");

        // Scan the bigger Range.
        count = 0;
        foreach(unsigned, item, Range, longRange)
        {
            ASSERT(*item == count);
            count++;
        }
        ASSERT(item == NULL);
        TEST_RESULT_INT(count, testMax, "non-empty Range");
    }

    if (testBegin("FOREACH Range iteration"))
    {
        // Scan the empty Range,
        FOREACH(item, Range, emptyRange)
            ASSERT_MSG("iterating through an empty container");     // {uncoverable - this statement should not be reached}
        ENDFOREACH;

        ASSERT(item == NULL);
        TEST_RESULT_VOID((void)0, "empty Range ");

        // Scan the longer Range, inside a collection.
        count = 0;
        FOREACH(item, Range, longRange)
            ASSERT(*item == count);
            count++;
        ENDFOREACH;

        ASSERT(item == NULL);

        TEST_RESULT_INT(count, testMax, "non-empty Range inside Collection");

        // Try to get next() item of an empty Range.
        RangeItr *itr = rangeItrNew(emptyRange);
        ASSERT(rangeItrNext(itr) == NULL);
        TEST_RESULT_PTR(rangeItrNext(itr), NULL, "iterate beyond end of empty Range");
        objFree(itr);

        // Similar, but this time with items in the Range.
        itr = rangeItrNew(longRange);  // A second iterator in parallel
        FOREACH(item, Range, longRange)
            ASSERT(*rangeItrNext(itr) == *item);
        ENDFOREACH;

        ASSERT(item == NULL);
        TEST_RESULT_PTR(rangeItrNext(itr), NULL, "iterate beyond end of Range");
        objFree(itr);
    }

    if (testBegin("FOREACH Collection iteration"))
    {
        // Scan the empty Range, inside a collection
        FOREACH(item, Collection, emptyCollection)
            ASSERT_MSG("iterating through an empty container");     // {uncoverable - this statement should not be reached}
        ENDFOREACH;

        ASSERT(item == NULL);
        TEST_RESULT_VOID((void)0, "empty Range inside Collection");

        // Scan the longer Range, inside a collection.
        count = 0;
        FOREACH(item, Collection, longCollection)
            ASSERT(*item == count);
            count++;
        ENDFOREACH;

        ASSERT(item
        == NULL);
        TEST_RESULT_INT(count, testMax, "non-empty Range inside Collection");

        // Try to get next() item of an empty Range.
        CollectionItr *itr = collectionItrNew(emptyCollection);
        ASSERT(collectionItrNext(itr) == NULL);
        TEST_RESULT_PTR(collectionItrNext(itr), NULL, "iterate beyond end of empty Range");
        objFree(itr);

        // Similar, but this time with items in the Range.
        itr = collectionItrNew(longCollection);  // A second iterator in parallel
        FOREACH(item, Range, longRange)
            ASSERT(*(unsigned int*)collectionItrNext(itr) == *item);
        ENDFOREACH;

        ASSERT(item == NULL);
        TEST_RESULT_PTR(collectionItrNext(itr), NULL, "iterate beyond end of Range");
        objFree(itr);

        // Try to create a Collection from NULL Range.
        TEST_ERROR((void) collectionNew(Range, NULL), AssertError, "assertion 'subCollection != NULL' failed");

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
        // Verify the destructor gets called on a Range with no exceptions.
        eventCount = 0;
        FOREACH(item, Range, longRange)
        ENDFOREACH;

        TEST_RESULT_INT(eventCount, 1, "destructor invoked after loop ends");

        // Throw an exception within a loop and verify the destructor gets called.
        eventCount = 0;
        TRY_BEGIN()
            FOREACH(item, Range, longRange)
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
Wrap a Range iterator in a context which includes a callback destructor.
***********************************************************************************************************************************/
#undef rangeItrNew
RangeItr *callbackItrNew(Range *range)
{
    RangeItr *this = NULL;
    OBJ_NEW_BEGIN(RangeItr, .callbackQty=1, .childQty=1)
        {
            this = rangeItrNew(range);
            memContextCallbackSet(memContextCurrent(), destructor, this);
        }
    OBJ_NEW_END();

    return this;
}
