/***********************************************************************************************************************************
Test Scanning directories for files.

Verifies the StorageScan wrapper is working correctly. The StorageIterator tests go into more detail.
***********************************************************************************************************************************/
#include "build.auto.h"

#include "common/io/io.h"
#include "common/type/string.h"
#include "common/time.h"
#include "storage/read.h"
#include "storage/write.h"
#include "storage/posix/storage.h"
#include "storage/helper.h"

#include "common/harnessConfig.h"
#include "common/harnessFork.h"
#include "common/harnessStorage.h"

#include "storage/scan.h"

#ifndef TEST_PATH
#define TEST_PATH "/tmp/pgtest"
#endif

// Forward references.
#define testScanP(this, path, title, expect, ...) \
    testScan(this, path, title, expect, (StorageScanParams){VAR_PARAM_INIT, __VA_ARGS__})
void testScan(Storage *storage, const char *path, const char *testTitle, const char *expectStr, StorageScanParams param);
void createFiles(Storage *driver);

const unsigned int testWide = 10;
const char *expectedWide =
        "file000000,file000001,file000002,file000003,file000004,file000005,file000006,file000007,file000008,file000009";
const char *expectedWideDesc =
        "file000009,file000008,file000007,file000006,file000005,file000004,file000003,file000002,file000001,file000000";

const unsigned int testDeep = 10;
const char *expectedDeep =
    "dir0/,dir0/dir1/,dir0/dir1/dir2/,dir0/dir1/dir2/dir3/,dir0/dir1/dir2/dir3/dir4/,dir0/dir1/dir2/dir3/dir4/dir5/,"
    "dir0/dir1/dir2/dir3/dir4/dir5/dir6/,dir0/dir1/dir2/dir3/dir4/dir5/dir6/dir7/,"
    "dir0/dir1/dir2/dir3/dir4/dir5/dir6/dir7/dir8/,dir0/dir1/dir2/dir3/dir4/dir5/dir6/dir7/dir8/dir9/,"
    "dir0/dir1/dir2/dir3/dir4/dir5/dir6/dir7/dir8/dir9/file9,dir0/dir1/dir2/dir3/dir4/dir5/dir6/dir7/dir8/file8,"
    "dir0/dir1/dir2/dir3/dir4/dir5/dir6/dir7/file7,dir0/dir1/dir2/dir3/dir4/dir5/dir6/file6,dir0/dir1/dir2/dir3/dir4/dir5/file5,"
    "dir0/dir1/dir2/dir3/dir4/file4,dir0/dir1/dir2/dir3/file3,dir0/dir1/dir2/file2,dir0/dir1/file1,dir0/file0";
const char *expectedDeepDesc =
    "dir0/file0,dir0/dir1/file1,dir0/dir1/dir2/file2,dir0/dir1/dir2/dir3/file3,dir0/dir1/dir2/dir3/dir4/file4,"
    "dir0/dir1/dir2/dir3/dir4/dir5/file5,dir0/dir1/dir2/dir3/dir4/dir5/dir6/file6,dir0/dir1/dir2/dir3/dir4/dir5/dir6/dir7/file7,"
    "dir0/dir1/dir2/dir3/dir4/dir5/dir6/dir7/dir8/file8,dir0/dir1/dir2/dir3/dir4/dir5/dir6/dir7/dir8/dir9/file9,"
    "dir0/dir1/dir2/dir3/dir4/dir5/dir6/dir7/dir8/dir9/,dir0/dir1/dir2/dir3/dir4/dir5/dir6/dir7/dir8/,"
    "dir0/dir1/dir2/dir3/dir4/dir5/dir6/dir7/,dir0/dir1/dir2/dir3/dir4/dir5/dir6/,dir0/dir1/dir2/dir3/dir4/dir5/,"
    "dir0/dir1/dir2/dir3/dir4/,dir0/dir1/dir2/dir3/,dir0/dir1/dir2/,dir0/dir1/,dir0/";
 

/***********************************************************************************************************************************
Test Run
***********************************************************************************************************************************/
static void
testRun(void)
{
    FUNCTION_HARNESS_VOID();

    // Get a Posix driver to use for testing.
    Storage *storageTest = storagePosixNewP(TEST_PATH_STR, .write = true);

    // Create the files used in the test.  (under pg/)
    createFiles(storageTest);

    // *****************************************************************************************************************************

    if (testBegin("StorageScan()"))
    {

        TEST_TITLE("Non-recursive scans.");
        StorageScanParams param = (StorageScanParams) {.recursive=false};
        testScan(storageTest, TEST_PATH "/pg/emptyDir", "empty directory", "", param);
        testScan(storageTest, TEST_PATH "/pg/one_of_each", "one of each", "dir/,file,link>,pipe*", param);
        testScan(storageTest, TEST_PATH "/pg/wideDir", "scan a wide directory", expectedWide, param);
        testScan(storageTest, TEST_PATH "/pg/deepDir", "scan a deep directory", "dir0/", param);

        TEST_TITLE("Recursive scans.");
        param.recursive = true;
        testScan(storageTest, TEST_PATH "/pg/emptyDir", "empty directory", "", param);
        testScan(storageTest, TEST_PATH "/pg/one_of_each", "one of each", "dir/,file,link>,pipe*", param);
        testScan(storageTest, TEST_PATH "/pg/wideDir", "scan a wide directory", expectedWide, param);
        testScan(storageTest, TEST_PATH "/pg/deepDir", "scan a deep directory", expectedDeep, param);

        TEST_TITLE("Descending Order");
        param.sortOrder = sortOrderDesc;
        testScan(storageTest, TEST_PATH "/pg/emptyDir", "empty directory", "", param);
        testScan(storageTest, TEST_PATH "/pg/one_of_each", "one of each", "pipe*,link>,file,dir/", param);
        testScan(storageTest, TEST_PATH "/pg/wideDir", "scan a wide directory", expectedWideDesc, param);
        testScan(storageTest, TEST_PATH "/pg/deepDir", "scan a deep directory", expectedDeepDesc, param);

        TEST_TITLE("With Regular Expression");
        param.sortOrder = sortOrderAsc; param.expression=strNewZ("^.*/link.*$");
        testScan(storageTest, TEST_PATH "/pg", "search for link", "badLink/link>,one_of_each/link>", param);
        param.expression = strNewZ("BADMATCH");
        testScan(storageTest, TEST_PATH "/pg", "nothing matches pattern", "", param);

        testScan(storageTest, TEST_PATH "/pg/MISSING", "missing directory", "", param);
        param.nullOnMissing=true;  // For coverage.
        testScan(storageTest, TEST_PATH "/pb/MISSING", "missing directory - NULL", "", param);
    }

    FUNCTION_HARNESS_RETURN_VOID();
}




String *storageInfoToLog(StorageInfo *this);

/***********************************************************************************************************************************
Scan the files and verify we scanned the correct files in the correct order.
***********************************************************************************************************************************/
void testScan(Storage *storage, const char *path, const char *testTitle, const char *expected, StorageScanParams param)
{
    MEM_CONTEXT_TEMP_BEGIN()

    // Set default info level and sort order.
    if (param.level == storageInfoLevelDefault)
        param.level = storageInfoLevelType;
    if (param.sortOrder == sortOrderNone)
        param.sortOrder = sortOrderAsc;

    // Define the scan and get a collection of files.
    Collection *actualList = storageScan(storage, strNewZ(path), param);
    ASSERT(actualList != NULL);

    // Join the file info into a comma separated string.
    String *actual = strNew();
    StorageInfo *info;
    FOREACH(info, Collection, actualList)
        if (!strEmpty(actual))
            strCatZ(actual, ",");
        strCat(actual, storageInfoToLog(info));
    ENDFOREACH;

    // Verify we go the expected string.
    TEST_RESULT_STR(actual, strNewZ(expected), testTitle);

    MEM_CONTEXT_TEMP_END();
}

/***********************************************************************************************************************************
Format the storage info into a usable form for debugging and display.
TODO: add details according to level. Might be useful for log formatting. Similar code in storage iterator test.
***********************************************************************************************************************************/
String *
storageInfoToLog(StorageInfo *this)
{
    ASSERT(this != NULL);
    String *str;
    switch (this->type)
    {
        case storageTypeFile:
            str = strNewFmt("%s", strZ(this->name));
            break;

        case storageTypeLink:
            str = strNewFmt("%s>", strZ(this->name));
            break;

        case storageTypePath:
            str = strNewFmt("%s/", strZ(this->name));
            break;

        case storageTypeSpecial:
            str = strNewFmt("%s*", strZ(this->name));
            break;

        default:
            ASSERT_FMT("File %s has unexpected type: %d", strZ(this->name), (int)this->type); //{uncovered - Unexpected value}
    }

    return str;
}

/***********************************************************************************************************************************
Create a hierarchy of test files.
     pg/
       one_of_each/     contains one example of each type of file
       emptyDir/
       wideDir/         contains a bunch of files but no subdirectories
       deepDir/         deeply nested subdirectories, each holding one data file.
       badLink/link     an invalid symbolic link
 **********************************************************************************************************************************/
void createFiles(Storage *storage)
{
    (void)storage;  // Not used.

// Create a file hierarchy for testing the scanning code.
    HRN_SYSTEM("rm -rf " TEST_PATH "/pg");
    HRN_SYSTEM("mkdir -p " TEST_PATH "/pg");

// Start by creating a subdirectory with one of every type of entry.
    HRN_SYSTEM("mkdir -p " TEST_PATH "/pg/one_of_each");
    HRN_SYSTEM("ln -s ../file " TEST_PATH "/pg/one_of_each/link");
    HRN_SYSTEM("mkfifo -m 777 " TEST_PATH "/pg/one_of_each/pipe");
    HRN_SYSTEM("echo 'Hello World!' > " TEST_PATH "/pg/one_of_each/file");
    HRN_SYSTEM("mkdir -p " TEST_PATH "/pg/one_of_each/dir");

// An empty directory
    HRN_SYSTEM("mkdir -p " TEST_PATH "/pg/emptyDir");

// A directory with lots of files.
    HRN_SYSTEM("mkdir -p " TEST_PATH "/pg/wideDir");
    for (int idx = 0; idx < 10; idx++)  // bump it up to 100K and check for memory leaks.
        HRN_SYSTEM_FMT("echo %d > " TEST_PATH "/pg/wideDir/file%06d", idx, idx);

// A very deep subdirectories.
    String *path = strNew();
    strCatZ(path, TEST_PATH "/pg/deepDir");
    for (int idx = 0; idx < 10; idx++)
    {
        path = strCatFmt(path, "/dir%d", idx);
        HRN_SYSTEM_FMT("mkdir -p %s; echo %d > %s/file%d", strZ(path), idx, strZ(path), idx);
    }

    // A directory with a bad link
    HRN_SYSTEM("mkdir -p " TEST_PATH "/pg/badLink");
    HRN_SYSTEM("ln -s ../BOGUS " TEST_PATH "/pg/badlink/link");
}
