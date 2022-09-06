/***********************************************************************************************************************************

***********************************************************************************************************************************/

#include "readDir.h"


/***********************************************************************************************************************************
A collection representing all the files in a Posix directory.
This is a "lazy" iterable, meaning the file info is read from the operating system "on the fly".
ReadDir does not provide reliable iteration for remote file systems, so it should be copied to a StorageInfoList
before being passed to upper layers.
***********************************************************************************************************************************/
struct ReadDir
{
    String *path;                                                   // path to the directory to be read.
    StorageLevel level;                                             // level of detail to return for each file entry.
    bool followLinks;                                               // status of link, or status of file the link points to.
};

// Iterator for scanning through a directory using readdir().
struct ReadDirItr
{
    ReadDir *readDir;                                               // the directory we are scanning.
    DIR dir;                                                        // File handle to the open directory.
};

/***********************************************************************************************************************************
Define a collection of files contained within a directory.
* @param path the directory we wish to scan
* @param level what level of detail do we want for each file in directory.
* @param followLinks Do we want info about the link or about the file the link point to.
* @return an initialized ReadDir structure. (not a pointer, but the structure itself)
***********************************************************************************************************************************/
ReadDir
newReadDir(String *path, StorageLevel level, bool followLinks)
{
    return (ReadDir) {.path = path, .level = level, .followLinks = followLinks};
}


/***********************************************************************************************************************************
Construct an iterator for scanning a directory.
* @param this - allocated structure for us to initialize.
* @param readDir - the ReadDir collection representing the directory to scan.
*/
void
newReadDirItr(ReadDirItr *this, ReadDir *readDir)
{
    this->readDir = readDir;
    this->dir = opendir(readDir->path);
    if (this->dir == NULL && errno != XXXX)
        THROW()
}

/***********************************************************************************************************************************
Point to the next item in the collection, returning NULL if there are no more.
***********************************************************************************************************************************/
StorageInfo *
nextReadDirItr(ReadDirItr *this)
{
    if (this->dir == NULL)
        return NULL;

    // Get the next entry, checking for errors and EOF.
    errno = 0;                                                      // As recommended by Posix standard for readdir()
    dirent *entry = readdir(this->dir);
    if (entry == NULL && errno != 0)
        THROW();
    if (entry == NULL)
        return NULL;

    // Fetch extended info about the file, throwing exception if there are problems.
    this->info = direntToInfo(this, entry);

    // Return a pointer to information about the file.
    return *this->info;
}

/***********************************************************************************************************************************
destroy the iterator, closing the directory if it was successfully opened.
***********************************************************************************************************************************/
void
destructReadDirItr(ReadDirItr *this)
{
    if (this->dir != NULL)
        closedir(this->dir);  // Could generate error.
}

/***********************************************************************************************************************************
Given the limited info about a file we got from readdir(), gather additional info and convert to a StorageInfo structure.
***********************************************************************************************************************************/
void StorageInfo
direntToInfo(ReadDirItr *this, dirent *entry)
{

}

#endif //PGBACKREST_READDIR_H
