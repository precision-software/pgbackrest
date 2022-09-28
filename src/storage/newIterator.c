//
// Created by John Morris on 9/27/22.
//


#include "storage/newIterator.h"
#include "storage/storage.h"
#include "storage/list.h"
#include "common/type/string.h"
#include "common/type/object.h"



/****Belongs in string.h  or a path manipulation library.  ******/
static String *
strPathJoin(String *path1, String *path2)
{
    return strNewFmt("%s/%s", strZ(path1), strZ(path2));
}

/*** belongs in storage/list.h. Enables iterating through StorageList **********/

// Belongs in storage/list.h
typedef struct StorageListItr
{
    StorageList *list;
    unsigned int idx;
    StorageInfo currentInfo;
} StorageListItr;

StorageListItr *
storageLstItrNew(StorageList *list)
{
    StorageListItr *this;
    OBJ_NEW_BEGIN(StorageListItr)
        this = OBJ_NEW_ALLOC();
        *this = (StorageListItr) {.list=list, .idx = 0};
    OBJ_NEW_END();
}

StorageInfo *
storageLstItrNext(StorageListItr *this)
{
    if (this->idx >= storageLstSize(this->list))
        return NULL;

    // Fetch the next file info into local, stable storage.
    this->currentInfo = storageLstGet(this->list, this->idx++);

    // return a pointer to the stable file info.
    return &this->currentInfo;
}

#define CAMEL_StorageList  storageLst


/***********************************************************************************************************************************
Read the contents of a single directory.
 * @param this the storage driver
 * @param path the path to the directory
 * @param level how detailed should the info bi
 * @param sortOrder desired sort order
 * @param followLinks should we follow links or report them as links.
 * @return Collection of file info.

This will eventually be an interface which replaces storageInterfaceList.
**********************************************************************************************************************************/
Collection *
storageDirRead(Storage *this, String *path, StorageInfoLevel level, SortOrder sortOrder, bool followLinks)
{
    StorageList *list = storageInterfaceListP(this, path, level);
    if (sortOrder != sortOrderNone)
        storageLstSort(list, sortOrder);
    Collection *collection = NEWCOLLECTION(StorageList, list);
    return collection;
}


/***********************************************************************************************************************************
Represents a set of files from a single directory.
***********************************************************************************************************************************/
typedef struct Directory
{
    Storage *driver;                                                // The relevant storage driver
    String *path;                                                   // Path to the relevant directory
    String *filter;        // Actually, save the compiled expression                                         // Regular expression to filter by file name
    StorageInfoLevel level;                                         // Level of information details to provide for each file.
    SortOrder sortOrder;                                            // The desired order of the files.
} Directory;



/***********************************************************************************************************************************
Define a single directory file set.
***********************************************************************************************************************************/
Directory *
directoryNew(Storage *driver, String *path, String *subPath, String *filter, StorageInfoLevel level, SortOrder sortOrder)
{
    Directory *this;
    OBJ_NEW_BEGIN(Directory)
        this = OBJ_NEW_ALLOC();
        *this = (Directory) {.driver=driver, .path=strPathJoin(path, subPath), .filter=recompile(filter), .level=level, .sortOrder=sortOrder};
    OBJ_NEW_END();
}



typedef struct DirectoryItr
{
    Directory *dir;
    CollectionItr *itr;
} DirectoryItr;

DirectoryItr *directoryItrNew(Directory *dir)
{
    DirectoryItr *this;
    OBJ_NEW_BEGIN(DirectoryItr)
        this = OBJ_NEW_ALLOC();
        Collection *files = storageListP(this, dir->path, .level=dir->level, .sortOrder=dir->sortOrder);
        *this = (DirectoryItr) {.dir=dir, .itr=collectionItrNew(files)};
    OBJ_NEW_END();

    return this;
}

typedef struct RecursiveDirectoryItr
{
    String *path;
    Directory Itr *iterator;
    StorageInfo dirInfo;
} DirectoryItr;

DirectoryItr *directoryItrNew(Directory *dir)

}




typedef Directory RecursiveDirectory;

RecursiveDirectory *recursiveDirectoryNew(Directory *dir)
{
    return (RecursiveDirectory *)dir;
}

typedef struct RecursiveDirectoryItr
{
    Directory *dir;                                                 // Information about the root directory collection.
    List *directories;                                              // A stack of iterators for scanning each of the paths.
    MemContext *context;                                            // The memory context used for the iterator.
} RecursiveDirectoryItr;


/***********************************************************************************************************************************
Construct an iterator for recursively scanning the given directory.
***********************************************************************************************************************************/
RecursiveDirectoryItr *recursiveDirectoryItrNew(Directory *dir)
{
    // Create empty stacks of paths and iterators.
    StringList *paths = strLstNew();
    List *iterators = lstNewP(sizeof(CollectionItr *));

    // Push the first directory path and its corresponding iterator onto the empty stacks.
    Collection *files = storageListP(dir->driver, dir->path, .level=dir->level);
    lstAdd(iterators, collectionItrNew(files));
    strLstAdd(paths, dir->path);

    // Create the recursive iterator which is prepared to examine the first file in the root directory.
    *this = (RecursiveDirectoryItr) {.dir = dir, .paths = paths, .iterators=iterators};
    return this;
}

/***********************************************************************************************************************************
Get the next item from the current directory, popping or pushing directories as appropriate.
***********************************************************************************************************************************/
StorageInfo *recursiveDirectoryItrNext(RecursiveDirectoryItr *this)
{
    StorageInfo *info = NULL;

    // Keep moving forward while there are more files to examine
    while (lstSize(this->directories) > 0)
    {
        // Get the top of stack path and iterator.
        DirectoryState *state = lstGetLast(this->directories);

        // Get the next file info from the iterator.
        info = collectionItrNext(state->iterator);

        // CASE: finished scanning current directory.
        if (info == NULL)
        {
            // Pop the directory from the stacks.
            lstRemoveLast(this->directories);

            // if post-order, then output directory info now.
            if (this->dir->sortOrder != sortOrderAsc)  // TODO: which direction?
            {
                info = &state->info;
                break;
            }

        // CASE: encountered a directory.
        else if (info->type == storageTypePath)
        {
            // Get the directory contents from the driver.
            String *newPath = strPathJoin(path, info->name); // TODO: create this function.
            Collection *newFiles = storageListP(this->dir->driver, newPath); // NULL?
            StorageInfo newInfo = *info;

            // Push info about the new directory onto the stack.
            DirectoryState state = (DirectoryState){
                .info=newInfo,
                .iterator=collectionItrNew(newFiles),
                .path=newPath};
            lstAdd(this->directories, state);

            // If we are pre-order, then output the directory info now, before we scan its subfiles.
            if (this->dir->sortOrder != sortOrderAsc)
                break;
        }

        // OTHERWISE: file or link. Just return the file info.
        else
            break;
    }

    // Return a pointer to the file info.
    return info;
}

typedef Directory RecursiveDirectory;

typedef struct RecursiveDirectoryItr
{
    List *stack;
} RecursiveDirectoryItr;

RecursiveDirectory *recursiveDirectoryNew(String *path, String *filter, StorageInfoLevel level, SortOrder sortOrder)
{
    OBJ_NEW_BEGIN(RecursiveDirectory, .childQty=MEM_CONTEXT_QTY_MAX)


}

typedef struct RecursiveDirectoryItr
{
    RecursiveDirectory *dir;
    CollectionItr *itr;
} RecursiveDirectoryItr;

RecursiveDirectoryItr *recursiveDirectoryNewItr(RecursiveDirectory *dir)
{
    RecursiveDirectoryItr *this;
    OBJ_NEW_BEGIN(RecursiveDirectoryItr, .childQty=MEM_CONTEXT_QTY_MAX);
        Collection *files = storageInterfaceListP(this, dir->path, dir->level);
        this = OBJ_NEW_ALLOC();
        *this = (RecursiveDirectoryItr) {.dir = dir, .itr=collectionItrNew(files)};
    OBJ_NEW_END();

}

StorageInfo *recursiveDirectoryItrNext(RecursiveDirectoryItr *this)
{

}
