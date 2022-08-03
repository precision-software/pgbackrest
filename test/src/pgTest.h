//
// Created by micro on 8/2/2022.
//
#include <stdio.h>
#include <stdlib.h>
#include "common/harnessTest.h"
#include "common/harnessDebug.h"
#include "common/memContext.h"
#define TEST_PATH "."  // Better as a -DTEST_PATH=...??

static void testRun(void);


/***********************************************************************************************************************************
main - run the tests
***********************************************************************************************************************************/
int
main(int argListSize, const char *argList[])
{
    // Basic sanity test on input parameters
    if (argListSize == 0 || argList[0] == NULL)
    {
        fprintf(stderr, "at least one argument expected");
        fflush(stderr);
        exit(25);
    }

    // Initialize stack trace for the harness
    FUNCTION_HARNESS_INIT(argList[0]);

    FUNCTION_HARNESS_BEGIN();
    FUNCTION_HARNESS_PARAM(INT, argListSize);
    FUNCTION_HARNESS_PARAM(CHARPY, argList);
    FUNCTION_HARNESS_END();

    int result = 0;

    // Run the tests
    MEM_CONTEXT_TEMP_BEGIN()
    testRun();
    MEM_CONTEXT_TEMP_END();



    printf("\nTESTS COMPLETED SUCCESSFULLY\n");
    fflush(stdout);

    FUNCTION_HARNESS_RETURN(int, 0);
}


