/***************************************************************************************

****************************************************************************************/
//

#include "readdir.h"




ReadDirRawItr *
readDirRawNewItr(ReadDirRaw *container)
{
    FUNCTION_TEST_BEGIN();
        FUNCTION_TEST_PARAM(CONTAINER, container);
    FUNCTION_TEST_END();

    // Open the posix directory for reading.
    int fd = opendir(STRZ(container->path));
    if (fd == -1)
        if (errno == XXXX) THROW(   );

    ReadDirRawItr this = NULL;
    OBJ_NEW_BEGIN(ReadDirRawItr)
    {


        this = OBJ_NEW_ALLOC();
        *this = (ReadDirRawItr)()
    }
    OBJ_NEW_END();



}
