/***********************************************************************************************************************************
Storage List
***********************************************************************************************************************************/
#ifndef STORAGE_LIST_H
#define STORAGE_LIST_H

/***********************************************************************************************************************************
Object type
***********************************************************************************************************************************/
typedef struct StorageList StorageList;

#include "common/type/string.h"
#include "storage/info.h"

/***********************************************************************************************************************************
Constructors
***********************************************************************************************************************************/
StorageList *storageLstNew(StorageInfoLevel level);

/***********************************************************************************************************************************
Getters/Setters
***********************************************************************************************************************************/
typedef struct StorageListPub
{
    List *list;                                                     // Storage list
    StorageInfoLevel level;                                         // Storage info level
} StorageListPub;

// Empty?
FN_INLINE_ALWAYS bool
storageLstEmpty(const StorageList *const this)
{
    return lstEmpty(THIS_PUB(StorageList)->list);
}

// Storage info level
FN_INLINE_ALWAYS StorageInfoLevel
storageLstLevel(const StorageList *const this)
{
    return THIS_PUB(StorageList)->level;
}

// List size
FN_INLINE_ALWAYS unsigned int
storageLstSize(const StorageList *const this)
{
    return lstSize(THIS_PUB(StorageList)->list);
}

// List size
FN_INLINE_ALWAYS void
storageLstSort(StorageList *const this, const SortOrder sortOrder)
{
    lstSort(THIS_PUB(StorageList)->list, sortOrder);
}

/***********************************************************************************************************************************
Functions
***********************************************************************************************************************************/
// Insert info
void storageLstInsert(StorageList *this, unsigned int idx, const StorageInfo *info);

// Add info
FN_INLINE_ALWAYS void
storageLstAdd(StorageList *const this, const StorageInfo *const info)
{
    storageLstInsert(this, storageLstSize(this), info);
}

// Get info. Note that StorageInfo pointer members (e.g. name) will be undefined after the next call to storageLstGet().
StorageInfo storageLstGet(StorageList *this, unsigned int idx);

// Move to a new parent mem context
FN_INLINE_ALWAYS StorageList *
storageLstMove(StorageList *const this, MemContext *const parentNew)
{
    return objMove(this, parentNew);
}

/***********************************************************************************************************************************
Destructor
***********************************************************************************************************************************/
FN_INLINE_ALWAYS void
storageLstFree(StorageList *const this)
{
    objFree(this);
}

/***********************************************************************************************************************************
Macros for function logging
***********************************************************************************************************************************/
String *storageLstToLog(const StorageList *this);

#define FUNCTION_LOG_STORAGE_LIST_TYPE                                                                                             \
    StorageList *
#define FUNCTION_LOG_STORAGE_LIST_FORMAT(value, buffer, bufferSize)                                                                \
    FUNCTION_LOG_STRING_OBJECT_FORMAT(value, storageLstToLog, buffer, bufferSize)

#endif
