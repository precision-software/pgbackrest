/***********************************************************************************************************************************
Recursive scan of a directory.
***********************************************************************************************************************************/
#include "build.auto.h"

#include "storage/scan.h"
#include "storage/storage.h"
#include "storage/list.h"
#include "storage/info.h"
#include "common/type/collection.h"
#include "common/type/string.h"
#include "common/type/object.h"


/****Belongs in string.h  or a path manipulation library.  ******/

/***********************************************************************************************************************************

 * @param path
 * @param subPath
***********************************************************************************************************************************/
static String *
strPathJoin(const String *path, const String *subPath)
{
    String *joined;
    if (path == NULL && subPath == NULL)
        joined = NULL;
    else if (path == NULL)
        joined = strDup(subPath);
    else if (subPath == NULL)
        joined = strDup(path);
    else
        joined = strNewFmt("%s/%s", strZ(path), strZ(subPath));

    return joined;
}

// Enable StorageList as a Collection.
#define CAMEL_StorageList storageLst
DEFINE_LIST_COLLECTION(StorageInfo, StorageList);

/***********************************************************************************************************************************
Add a directory path to the file name in an info structure, creating a new info structure.
***********************************************************************************************************************************/
StorageInfo *
storageInfoUpdatePath(StorageInfo *info, String *parentPath)
{
    StorageInfo *this;
    OBJ_NEW_BEGIN(StorageInfo, .childQty=MEM_CONTEXT_QTY_MAX, .allocQty=MEM_CONTEXT_QTY_MAX)

        // Copy everything except the name. TODO: This copy can be refined by looking at retVal level.
        this = OBJ_NEW_ALLOC();
        *this = *info;

        // Fill in the complete name by joining the directory path with the file name.
        this->name = strPathJoin(parentPath, info->name);
    OBJ_NEW_END();

    return this;
}

/***** Now we are doing our own thing ***************/

/***********************************************************************************************************************************
A "lazy-container" of files which will be generated by recursively scanning through a directory.
***********************************************************************************************************************************/
typedef struct RecursiveScan
{
    Storage *driver;                                                // The relevant storage driver
    String *path;                                                   // Path to the relevant directory
    StorageScanParams param;                                        // Parameters describing the scan.
    bool postOrder;
    RegExp *regEx;                                                  // Compiled version of param.expression.
} RecursiveScan;

// Enable use of FOREACH and containerNew().
#define CAMEL_RecursiveScan  recursiveScan

/***********************************************************************************************************************************
A "helper-iterator" to scan files contained in a single directory.
***********************************************************************************************************************************/
typedef struct SubdirItr
{
    CollectionItr *files;                                           // Iterator scanning each file in directory.
    StorageInfo *postponedInfo;                                     // Postorder directory output after the contents.

    // The following fields are all NULL for the root directory.
    struct SubdirItr *parent;                                       // Pointer to our parent directory iterator.
    String *path;                                                   // Path to the current directory, relative to start.
} SubdirItr;


/***********************************************************************************************************************************
"lazy iterator" to recursively scan through a directory and its subdirectories.
***********************************************************************************************************************************/
typedef struct RecursiveScanItr
{
    RecursiveScan *scan;                                            // Point to the scan we are doing.
    SubdirItr *current;                                             // the lowest directory of the subdirectory stack.
    StorageInfo *info;                                              // The file info we output last iteration.
    MemContext *ctx;                                                // Memory context of the iterator.
} RecursiveScanItr;

/***********************************************************************************************************************************
Construct a "lazy-container" of files which will be found by recursively scanning through the directory.
***********************************************************************************************************************************/
RecursiveScan *
recursiveScanNew(Storage *driver, String *path, StorageScanParams param)
{
    RecursiveScan *this;
    OBJ_NEW_BEGIN(RecursiveScan, .childQty=MEM_CONTEXT_QTY_MAX, .allocQty=MEM_CONTEXT_QTY_MAX)
        this = OBJ_NEW_ALLOC();
        *this = (RecursiveScan) {
            .driver = driver,
            .path = strDup(path),
            .param = param,
            .postOrder = param.sortOrder != sortOrderDesc,
            .regEx = (param.expression == NULL)? NULL: regExpNew(param.expression)
        };

        // We are recursive, but all our subdirectory queries will not be recursive.
        this->param.recursive = false;
    OBJ_NEW_END();

    return this;
}

/***********************************************************************************************************************************
Construct a helper "lazy-container" of files from a single subdirectory.
This constructor is used for recursively scanning through a directory hierarchy.
 @param rootItr - the overall recursive iterator.
 @param parent - parent of the current directory being scanned
 @param subdirInfo - file information about the subdirectory, including its name.
 @return - the subdirectory we are about to scan.
***********************************************************************************************************************************/
static SubdirItr *
subdirNew(RecursiveScanItr *rootItr, SubdirItr *parent, StorageInfo *info)
{
    // Point to the original scan definition.
    RecursiveScan *scan = rootItr->scan;

    SubdirItr *this;
    OBJ_NEW_BEGIN(SubdirItr, .childQty=MEM_CONTEXT_QTY_MAX, .allocQty=MEM_CONTEXT_QTY_MAX)

        // Get the path to this subdirectory, relative to the start of the scan.
        String *relPath = (parent != NULL) ? strPathJoin(parent->path, info->name) : NULL;

        // Fetch the non-recursive collection of files in this subdirectory. (We set .recursive=false earlier.)
        String *fullPath = strPathJoin(scan->path, relPath);
        Collection *files = storageScan(scan->driver, fullPath, scan->param);
        strFree(fullPath);  // Probably not necessary.

        // Create the subdirectory helper for iterating through them.
        this = OBJ_NEW_ALLOC();
        *this = (SubdirItr) {
            .files = collectionItrNew(files),
            .postponedInfo = NULL,
            .parent = parent,
            .path = relPath,
        };
    OBJ_NEW_END();

    return this;
}

/***********************************************************************************************************************************
Construct an iterator for recursively scanning through a directory tree
***********************************************************************************************************************************/
RecursiveScanItr *
recursiveScanItrNew(RecursiveScan *scan)
{
    RecursiveScanItr* this;
    OBJ_NEW_BEGIN(RecursiveScanItr, .childQty=MEM_CONTEXT_QTY_MAX, .allocQty=MEM_CONTEXT_QTY_MAX)
        this = OBJ_NEW_ALLOC();
        *this = (RecursiveScanItr) {
                .scan = scan,
                .info = NULL,
                .current = NULL,
                .ctx = memContextCurrent()
        };
        this->current = subdirNew(this, NULL, NULL),
    OBJ_NEW_END();

    return this;
}

static StorageInfo *
recursiveScanItrNext(RecursiveScanItr *this)
{
    MEM_CONTEXT_BEGIN(this->ctx);

        // Repeat until an item matches or we reach end.
        do
        {
            // Free up the results from previous iteration. TODO: Use truncate/append trick instead.
            if (this->info != NULL)
                objFree(this->info);
            this->info = NULL;

            // Repeat until we find an entry or there are none
            StorageInfo *info = NULL;                                       // The next file info we are considering.
            String *path = NULL;                                           // The corresponding path which contains the file
            while (this->current != NULL)
            {
                // Get the next file from the current directory.
                path = this->current->path;
                info = collectionItrNext(this->current->files);

                // CASE: finished scanning current directory.
                if (info == NULL)
                {
                    // Pop the current directory since it is now processed.
                    SubdirItr *parent = this->current->parent;
                    objFree(this->current);  // was allocated in loop control ctx, so it must be freed.
                    this->current = parent;

                    // If this is post order, then output the postponed directory and path.
                    if (this->current != NULL && this->scan->postOrder)
                    {
                        path = this->current->path;
                        info = this->current->postponedInfo;
                        break;
                    }
                }

                    // CASE: encountered a new subdirectory.
                else if (info->type == storageTypePath)
                {
                    // Keep track of the latest subdirectory info, so we can do post-order if requested.
                    this->current->postponedInfo = info;

                    // Start scanning the subdirectory. Will be freed when we finish scanning.
                    this->current = subdirNew(this, this->current, info);

                    // If pre-order then output the directory path and info now.
                    if (!this->scan->postOrder)
                        break;
                }

                    // OTHERWISE: file or link. Just output the file and path info if it matches the regular expression.
                else
                    break;
            }

            // At this point, we have info and path for the next file, and NULL if no more files.
            // For our return value, change the name to include the subpath from where we started scanning.
            if (info != NULL)
                this->info = storageInfoUpdatePath(info, path); // Reminder: Will be freed next iteration.

        // End "Repeat until an item matches or we reach end"
        } while (this->info && this->scan->regEx && !regExpMatch(this->scan->regEx, this->info->name));

    MEM_CONTEXT_END();

    // Return file info with augmented name, or NULL if done.
    return this->info;
}

/***********************************************************************************************************************************
Use the legacy List function to get a list of files from one directory.
***********************************************************************************************************************************/
static StorageList *
simpleScan(Storage *this, String *path, StorageScanParams param)
{
    // Get a list of file from the directory.
    StorageList *list = storageInterfaceListP(this, path, param.level);

    // If directory wasn't found, pretend it was empty.
    if (list == NULL)
        list = storageLstNew(param.level);

    // Sort the directory if requested.
    storageLstSort(list, param.sortOrder);

    return list;
}

Collection *
storageScan(Storage *this, String *path, struct StorageScanParams param)
{
    // Get the files - recursive or single directory.
    Collection *files = (param.recursive)
        ? collectionNew(RecursiveScan, recursiveScanNew(this, path, param))
        : collectionNew(StorageList, simpleScan(this, path, param));

    return files;
}
