/***********************************************************************************************************************************


***********************************************************************************************************************************/

#include "directoryItr.h"



/**********************************************************************************************************************************/
StorageInfo storageItrNext(StorageIterator *const this)
{
    FUNCTION_TEST_BEGIN();
    FUNCTION_TEST_PARAM(STORAGE_ITERATOR, this);
    FUNCTION_TEST_END();

    ASSERT(this != NULL);
    ASSERT(!this->returnedNext);

    this->returnedNext = true;

    FUNCTION_TEST_RETURN(STORAGE_INFO, this->infoNext);
}