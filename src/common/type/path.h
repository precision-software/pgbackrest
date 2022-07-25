//
// Created by micro on 7/24/2022.
//

#ifndef PGBACKREST_PATH_H
#define PGBACKREST_PATH_H

#include "common/type/string.h"

// Does the String represent an absolute path?
bool strPathIsAbsolute(const String *strPath);

// Make an absolute path from a possibly relative one.
String * strPathMakeAbsolute(String *strPath, String *cwd);

// Normalize a path.
String * strPathNormalize(String *strPath);

// Get the parent directory.
String * strPathParent(String *strPath);

// Get the base file name
String * strPathBase(String *strPath);


/**** Internal ****/
#include "common/type/stringList.h"

// Is this character a path separator?
bool isCharSeparator(char c);

// Default separator.
static const char normalCharSeparator = '/';


// A path is a list of path segment strings.
typedef StringList Path;

// Normalize a set of path segments.
void pathNormalize(Path *this);

// Split a string into path segments.
Path *pathSplit(const String *str);

// Join path segments into a string.
String *pathJoin(Path *this);

#endif //PGBACKREST_PATH_H
