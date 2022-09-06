/***********************************************************************************************************************************
Describe file.
***********************************************************************************************************************************/
#ifndef PGBACKREST_READDIR_H
#define PGBACKREST_READDIR_H

// A collection representing all the files in a Posix directory.
//; This is a "lazy" iterable, meaning the file info is generated "on the fly".
//  Does not provide reliable iteration for remote file systems,
//  so it should be buffered before being passed to upper layers.
typedef struct ReadDirRaw
{
    String *path;
    StorageLevel level;
    bool followLinks;
} ReadDirRaw;

Inline ReadDirRaw *
newReadDirRaw(String *path, StorageLevel level, bool followLinks)
{
    return &(ReadDirRaw) {.path = path, .level = level, .followLinks = followLinks};
}


typedef struct ReadDirRawItr
{
    ReadDirRaw *readDir;
    Dir *dir;
    int errno;
} ReadDirRawItr;

INLINE void
newItrReadDirRaw(ReadDirRawItr *this, ReadDirRaw *readDir)
{
    this->readDir = readDir;
    this->errno = 0;
    this->dir = opendir(readDir->path);
    if (this->dir == NULL && errno != XXXX)
        this->errno = errno
}


INLINE StorageInfo *
nextReadDirRawItr(ReadDirRawItr *this)
{
    if (this->dir == NULL || this->errno != 0)
        return NULL;

    // Get the next entry. If error, save the error code and stop iterating.
    errno = 0;                                                      // As recommended by Posix standard for readdir()
    dirent *entry = readdir(this->dir);
    if (entry == NULL) {
        this->errno = errno;
        return NULL;
    }

    // Fetch extended info about the file, setting errno and stopping iteration if problems.
    StorageInfo info = direntToInfo(this, entry);
    if (this->errno != 0)
        return NULL;

}

INLINE void
destruct(ReadDirRawItr *this)
{
    if (this->dir != NULL)
        closedir(this->dir);  // Could generate error.

    // Check for errors.
    if (this->errno != 0) {

        // when opening the directory or getting extended file information.
        if (this->dir == NULL)
            THROW();
        else
            THROW();
        }
}


#endif //PGBACKREST_READDIR_H
