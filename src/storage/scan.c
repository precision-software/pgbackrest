/******
 *
 */

#include "build.auto.h"
#include "storage.h"
#include "storage/iterator.h"
#include "common/debug.h"
#include "common/log.h"
#include "storage/scan.h"

typedef struct StorageScan
{
    Storage *driver;
    String *path;
    StorageScanParams param;
} StorageScan;

typedef struct StorageScanItr
{
    StorageScan *scan;
    StorageIterator *iterator;
    StorageInfo info;
} StorageScanItr;

#define CAMEL_StorageScan storageScan

StorageScanItr *
storageScanItrNew(StorageScan *scan)
{
    StorageScanParams *param = &scan->param;

    StorageScanItr *this;

    OBJ_NEW_BEGIN(StorageScanItr, .childQty=MEM_CONTEXT_QTY_MAX, .allocQty=MEM_CONTEXT_QTY_MAX);

        // Get an iterator for the storage.
        this = OBJ_NEW_ALLOC();

        StorageIterator *iterator =
            storageItrNew(
                scan->driver,
                scan->path,
                param->level,
                param->errorOnMissing,
                param->nullOnMissing,
                param->recursive,
                param->sortOrder,
                param->expression
            );

        *this = (StorageScanItr) {.scan=scan, .iterator=iterator};
    OBJ_NEW_END();

    return this;
}

StorageInfo *
storageScanItrNext(StorageScanItr *this)
{
    // If the iterator wasn't created and an error wasn't thrown earlier, then treat it as empty.
    if (this->iterator == NULL || !storageItrMore(this->iterator))
        return NULL;
    else
    {
        this->info = storageItrNext(this->iterator);
        return &this->info;
    }
}

Collection *storageScan(Storage *driver, String *path, struct StorageScanParams param)
{
    StorageScan *this;
    OBJ_NEW_BEGIN(StorageScan)
        this = OBJ_NEW_ALLOC();
        *this = (StorageScan) {.path=path, .driver=driver, .param=param};
    OBJ_NEW_END();

    return collectionNew(StorageScan, this);
}
