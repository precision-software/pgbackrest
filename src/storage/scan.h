//
#ifndef STORAGE_SCAN_H
#define STORAGE_SCAN_H

#include "common/regExp.h"

#include "common/type/collection.h"
#include "common/type/list.h"
#include "common/type/string.h"
#include "storage/list.h"
#include "storage/storage.h"

/*******************************************************************************************************************************
Read the contents of a directory (and subdirectories if .recurse is set), returning a lazy collection of Info for each file.
***********************************************************************************************************************************/
#define storageScanP(this, path, ...)                                                                                  \
    storageScan(this, path, (StorageScanParams){VAR_PARAM_INIT, __VA_ARGS__})

typedef struct StorageScanParams
{
    VAR_PARAM_HEADER;
    StorageInfoLevel level;                                         // Level of detail about each file
    bool followLink;                                                // Should we follow links or report their contents
    SortOrder sortOrder;                                            // Order sort the files.
    bool recursive;                                                 // Should we scan subdirectories?
    // Below are not implemented yet.
    String *expression;                                             // A regular expression to match files we want to keep.
    bool errorOnMissing;                                            // Throw an error if a file is missing by the time we scan it.
    bool nullOnMissing;                                             // Return NULL if a file is missing
} StorageScanParams;

Collection *
storageScan(Storage *this, String *path, struct StorageScanParams param);

#endif //STORAGE_SCAN_H
