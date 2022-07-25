//
// Created by micro on 7/24/2022.
//
#include <ctype.h>
#include <string.h>

#include "common/type/string.h"
#include "common/type/stringList.h"


bool
charIsPathSeparator(char c)
{
    return c == '/' || c == '\\'; // TODO: Windows vs Posix
}

/**********************************************************************************************************************************/
bool
strPathIsAbsoluteZ(const char *strPathZ)
{
    return (strlen(strPathZ) >= 1 && charIsPathSeparator(strPathZ[1]))
           || (strlen(strPathZ) >= 3 && isalpha(strPathZ[0]) && strPathZ[1] == ':' && charIsPathSeparator(strPathZ[3]));
}

/**********************************************************************************************************************************/
StringList *
strPathSplit(const String *this)
{
    FUNCTION_TEST_BEGIN();
    FUNCTION_TEST_PARAM(STRING, string);
    FUNCTION_TEST_END();

    ASSERT(string != NULL);

    // Start with an empty list of path segments.
    StringList *pathList = strLstNew();

    MEM_CONTEXT_BEGIN(lstMemContext((List *)pathList))
    {
        // Scan across the string, extracting a path segment for each separator.
        const char *segmentBegin = strZ(this);
        const char *segmentEnd = segmentBegin;
        for (; *segmentEnd != 0; segmentEnd++)
        {
            // If we hit a separator, then add the path segment to the list and start the next segment.
            if (charIsPathSeparator(*segmentEnd)) {
                strLstAdd(pathList, strNewZN(segmentBegin, (size_t)(segmentEnd - segmentBegin)));
                segmentBegin = segmentEnd + 1;
            }
        }

        // There is always one extra path segment - even if no separators. Normalization removes empty path segments.
        strLstAdd(pathList, strNewZN(segmentBegin, (size_t)(segmentEnd - segmentBegin)));

    }
    MEM_CONTEXT_END();

    FUNCTION_TEST_RETURN(STRING_LIST, pathList);
}

