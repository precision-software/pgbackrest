/***********************************************************************************************************************************
Directory Iterators scan through the files contained in a storage directory path.

There are essentially three kinds of directory iterators:
   DirectoryItrRaw, an iterator through a single directory. Raw iterators are the
     most efficient iterators for each type of storage. A raw iterator has a regular expression for filtering files,
     along with a sort order (possibly "none") which is native to the storage and cannot be changed.
   DirectoryIteratorBuffered, an iterator through a single directory which takes a "snopshot" of the directory.
     It has a sort order which can be set when the iterator is created.
   DirectoryIteratorRecursive, a recursive, iterator created with a sortOrder and a regular expression to filter
     out files and subdirectories. This iterator is built on top of the other two iterators.

Each storage driver is able to create a non-recursive directory iterator. The recursive iterator will usually be built on top
 of the non-recursive iterators, but for transitioning, the storage driver may return its own implementation of a recursive
 iterator.

These iterators are intended to be efficient in their use of resources.  In particular,
  - they allocate full size buffers only when necessary to sort or take a snapshot.
  - they sort data only when the sort order differs from the natural order provided by the storage driver.

When iterating through a directory, the following "completeness" constraint must be enforced.
     Unless a file was removed from or added to the directory after a directory scan is started,
     then that file must show up in the directory scan.

These are opaque classes. Once created, DirectoryIterators are only accessed through the following public interface:
    StorageInfo *StorageItrNext(this)
    bool StorageItrHasMore(this)
***********************************************************************************************************************************/
#ifndef PGBACKREST_DIRECTORYITR_H
#define PGBACKREST_DIRECTORYITR_H



// Superclass for iterating through a directory.  TODO: Move to .c file?
struct DirectoryItr
{
    void *driver;                                                   // Storage driver
    const String *path;                                             // Path to iterate
    StorageInfoLevel level;                                         // Level of detail for each file
    SortOrder sortOrder;                                            // The existing sort order.
};

StorageInfo *StorageItrNext(void *this);
bool StorageItrHasMore(void *this);



struct DirectoryItrBuffered
{
    struct DirectoryItr parent;                                     // Info from our DirectoryItr parent class. TODO: not sure about this approach.

    unsigned int listSize;                                          // Number of entries in the list.
    unsigned int nextIdx;                                           // Index to the next entry to be returned.
    bool returnedNext;                                              // Next info was returned. Reset by querying "hasMore".

    StorageList *list;                                              // Sorted list of info for each entry in the directory.
};


#endif //PGBACKREST_DIRECTORYITR_H
