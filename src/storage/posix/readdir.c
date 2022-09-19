/***************************************************************************************

****************************************************************************************/
//

#include "readdir.h"

Collection *
readDirPosix(StorageDriver *driver, String *path, StorageInfoLevel level, StorageSortOrder order, bool followLink)
{
    FUNCTION_LOG_BEGIN();
        FUNCTION_LOG_PARAM(STORAGE_POSIX, this);
        FUNCTION_LOG_PARAM(STRING, path;
        FUNCTION_LOG_PARAM(ENUM, level);
        FUNCTION_LOG_PARAM(ENUM, order);
        FUNCTION_LOG_PARAM(BOOL, followLink)
    FUNCTION_LOG_END();

    // Get the list of files.
    StorageList *infoList = storagePosixList();

    // Order them as requested.
    if (order != Unsorted)
        lstSort(infoList, order);

    // Return the list as an abstract collection.
    // Note we want the list to be deleted when the collection is deleted
    Collection *collection = NEWCOLLECTION(List, infoList));
    FUNCTION_LOG_RETURN(COLLECTION, collection);
}
