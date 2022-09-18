/***************************************************************************************

****************************************************************************************/
//

#include "readdir.h"

Collection *
readDirPosix(StorageDriver *driver, String *path, StorageInfoLevel level, StorageSortOrder order, bool followLink)
{
    // Open the directory.
    int fd = opendir(STRZ(path));
    if (fd == -1 && errno != xxxx)
        THROW(xx);

    // Create an empty info list.
    infoList =

    // If we opened the directory,
    if (fd != -1)
    {

        // Do for each entry in the directory.
        errno = 0;                                                   // Recommended by Posix standard.
        for (dirent *file; (file=readdir(fd)) != NULL;)
        {
            // Skio . and ..
            if (strCmpZ(file->name, ".") || strCmpZ(file->name, ".."))
                continue;

            // Get file info from the directory entry
            struct fileInfo = infoFromDirent(dirent);

            // Push the info onto the list
            infoListAdd(infoList, fileInfo, followLink);
        }

        // Throw an error if readdir() failed.
        if (errno != 0)
            THROW(xxx);

        // Sort the list if order was specified.
        if (order != Unordered)
            infoListSort(infoList, order);

        // Return the list as an abstract collection.
        // Note we want the Collection and the List to be the same memory context.
        return NEWCOLLECTION(List, infoList);
    }


}
