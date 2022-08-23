/***********************************************************************************************************************************
Storage Iterator.

A storage iterator scans through the files contained in a storage directory.
There are essentially three kinds of storage iterators:
   - RawDirectoryIterator, an iterator through a single directory. Raw iterators are low level,
     most efficient iterators for each type of storage. A raw iterator has a sort order
     (possibly "none") which is fixed and cannot be changed.
   - BufferedDirectoryIterator, an iterator through a single directory which takes a "snopshot" of the directory.
     It has a sort order which can be changed through the "directoryItrSort(this, order)" method.
   - DirectoryIterator, an abstract super class which includes both raw and buffered iterators.
   - StorageIterator, a recursive, sortable iterator with a regular expression used to filter
     out files and subdirectories. It can be viewed as a super-class of DirectoryIterator.

Each type of storage has a method "DirectoryIterator" which returns a non-recursive
 iterator (raw or buffered). The recursive StorageIterator is built on top of the low level iterators.

These classes are intended to be efficient in their use of resources.  In particular,
  - they only allocate buffers when necessary to sort or take a snapshot.
  - they only sort data when the sort order differs from the natural order provided by the storage driver.

When iterating through a directory, the following (POSIX-like) constraint must be enforced:
     If a file is removed from or added to the directory after a directory scan is started,
     whether the scan returns an entry for that file is unspecified.

***********************************************************************************************************************************/
#ifndef STORAGE_ITERATOR_H
#define STORAGE_ITERATOR_H

/***********************************************************************************************************************************
The high level, recursive storage iterator.
***********************************************************************************************************************************/
typedef struct StorageIterator StorageIterator;

#include "common/type/string.h"
#include "storage/info.h"

// Create a recursive storage iterator.
StorageIterator *storageItrNew(
    void *driver, const String *path, StorageInfoLevel level, bool errorOnMissing, bool nullOnMissing, bool recurse,
    SortOrder sortOrder, const String *expression);


// Create an iterator through a single directory. Missing files will be flagged with a null value.
StorageIterator *directoryItrNew(void *driver, const String *path, StorageInfoLevel level);

/***********************************************************************************************************************************
StorageIterators implement the following interface methods:
***********************************************************************************************************************************/

// Is there more info to be retrieved from the iterator?
bool storageItrMore(StorageIterator *this);

// Move to a new parent mem context
__attribute__((always_inline)) static inline StorageIterator *
storageItrMove(StorageIterator *const this, MemContext *const parentNew)
{
    return objMove(this, parentNew);
}

// Get next info. An error will be thrown if there is no more data so use storageItrMore() to check. Note that StorageInfo pointer
// members (e.g. name) will be undefined after the next call to storageItrMore().
StorageInfo storageItrNext(StorageIterator *this);


/***********************************************************************************************************************************
DirectoryIterators implement additional methods.
***********************************************************************************************************************************/

// What is the sort order of the iterator?
SortOrder directoryItrSortOrder(DirectoryIterator *this);

// Can the iterator's sort order be changed?
bool directoryItrIsSortable(DirectoryIterator *this);

// Change the sort order of a buffered iterator.
void bufferedDirectoryItrSort(BufferedDirectoryIterator *this, SortOrder sortOrder);

// Create a buffered iterator from another iterator.  Note the original iterator may be read to completion.
BufferedDirectoryItr *bufferedDirectoryItrNew(DirectoryIterator *directoryItr);




/***********************************************************************************************************************************
Destructor
***********************************************************************************************************************************/
__attribute__((always_inline)) static inline void
storageItrFree(StorageIterator *const this)
{
    objFree(this);
}

/***********************************************************************************************************************************
Macros for function logging
***********************************************************************************************************************************/
String *storageItrToLog(const StorageIterator *this);

#define FUNCTION_LOG_STORAGE_ITERATOR_TYPE                                                                                         \
    StorageIterator *
#define FUNCTION_LOG_STORAGE_ITERATOR_FORMAT(value, buffer, bufferSize)                                                            \
    FUNCTION_LOG_STRING_OBJECT_FORMAT(value, storageItrToLog, buffer, bufferSize)

#endif
