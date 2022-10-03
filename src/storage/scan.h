//
// Created by John Morris on 9/27/22.
//

#ifndef STORAGE_S3_NEWITERATOR_H
#define STORAGE_S3_NEWITERATOR_H

#include "common/regExp.h"

#include "common/type/collection.h"
#include "common/type/list.h"
#include "common/type/string.h"
#include "storage/list.h"
#include "storage/storage.h"

/*******************************************************************************************************************************
Read the contents of a single directory.
 @param this the storage driver
 @param path the path to the directory
 @param .level how detailed should the retVal be
 @param .sortOrder desired sort order
 @param .followLink should we follow links or report them as links.
 @return Collection of file retVal.

This method will eventually be an interface which replaces storageInterfaceList (or upgrades it?)
***********************************************************************************************************************************/
#define storageScanP(this, path, ...)                                                                                  \
    storageScan(this, path, (StorageScanParams){VAR_PARAM_INIT, __VA_ARGS__})

typedef struct StorageScanParams
{
    int dummy;
    StorageInfoLevel level;
    bool followLink;
    SortOrder sortOrder;
    String *expression;
    bool recursive;
} StorageScanParams;

Collection *
storageScan(Storage *this, String *path, struct StorageScanParams param);

#endif //STORAGE_S3_NEWITERATOR_H
