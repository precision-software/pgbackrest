/***********************************************************************************************************************************
Describe file.
***********************************************************************************************************************************/
#ifndef PGBACKREST_READDIR_H
#define PGBACKREST_READDIR_H

// A collection representing all the files in a Posix directory.
//; This is a "lazy" iterable, meaning the file info is generated "on the fly".
//  Does not provide reliable iteration for remote file systems, so it should be buffered before being passed to upper layers.
struct ReadDir;
struct ReadDirItr;
typedef struct ReadDir ReadDir;
typedef struct ReadDirItr ReadDirItr;

// Create a collection for scanning a posix directory using readdir();
ReadDir newReadDir(String *path, StorageLevel level, bool followLinks);

// Define Iterator methods on the ReadDir collection.
void newReadDirItr(ReadDirItr *this, ReadDirRaw *readDir);
StorageInfo *nextReadDirItr(ReadDirItr *this);
destructReadDirItr(ReadDirItr *this);


#endif //PGBACKREST_READDIR_H
