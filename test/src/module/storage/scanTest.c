/***********************************************************************************************************************************
Test Posix/CIFS Storage
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
    "dir0/dir1/dir2/dir3/dir4/dir5/dir6/dir7/dir8/dir9/file9,dir0/dir1/dir2/dir3/dir4/dir5/dir6/dir7/dir8/dir9/,"
    "dir0/dir1/dir2/dir3/dir4/dir5/dir6/dir7/dir8/file8,dir0/dir1/dir2/dir3/dir4/dir5/dir6/dir7/dir8/,"
    "dir0/dir1/dir2/dir3/dir4/dir5/dir6/dir7/file7,dir0/dir1/dir2/dir3/dir4/dir5/dir6/dir7/,"
    "dir0/dir1/dir2/dir3/dir4/dir5/dir6/file6,dir0/dir1/dir2/dir3/dir4/dir5/dir6/,dir0/dir1/dir2/dir3/dir4/dir5/file5,"
    "dir0/dir1/dir2/dir3/dir4/dir5/,dir0/dir1/dir2/dir3/dir4/file4,dir0/dir1/dir2/dir3/dir4/,"
    "dir0/dir1/dir2/dir3/file3,dir0/dir1/dir2/dir3/,dir0/dir1/dir2/file2,dir0/dir1/dir2/,"
    "dir0/dir1/file1,dir0/dir1/,dir0/file0,dir0/";
const char *expectedDeepDesc =
    "dir0/,dir0/file0,dir0/dir1/,dir0/dir1/file1,dir0/dir1/dir2/,dir0/dir1/dir2/file2,dir0/dir1/dir2/dir3/,"
    "dir0/dir1/dir2/dir3/file3,dir0/dir1/dir2/dir3/dir4/,dir0/dir1/dir2/dir3/dir4/file4,dir0/dir1/dir2/dir3/dir4/dir5/,"
    "dir0/dir1/dir2/dir3/dir4/dir5/file5,dir0/dir1/dir2/dir3/dir4/dir5/dir6/,dir0/dir1/dir2/dir3/dir4/dir5/dir6/file6,"
    "dir0/dir1/dir2/dir3/dir4/dir5/dir6/dir7/,dir0/dir1/dir2/dir3/dir4/dir5/dir6/dir7/file7,"
    "dir0/dir1/dir2/dir3/dir4/dir5/dir6/dir7/dir8/,dir0/dir1/dir2/dir3/dir4/dir5/dir6/dir7/dir8/file8,"
    "dir0/dir1/dir2/dir3/dir4/dir5/dir6/dir7/dir8/dir9/,dir0/dir1/dir2/dir3/dir4/dir5/dir6/dir7/dir8/dir9/file9";
 

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
        TEST_RESULT_VOID(strPathJoin(NULL, NULL), "strPathJoin(NULL,NULL) is NULL");
    }

    FUNCTION_HARNESS_RETURN_VOID();
}




String *storageInfoToLog(StorageInfo *this);

void testScan(Storage *storage, const char *path, const char *testTitle, const char *expected, StorageScanParams param)
{
    MEM_CONTEXT_TEMP_BEGIN()

    // Set default info level and sort order.
    if (param.level == storageInfoLevelDefault)
        param.level = storageInfoLevelType;
    if (param.sortOrder == sortOrderNone)
        param.sortOrder = sortOrderAsc;

    Collection *actualList = storageScan(storage, strNewZ(path), param);
    ASSERT(actualList != NULL);

    String *actual = strNew();
    StorageInfo *info;
    FOREACH(info, Collection, actualList)
        if (!strEmpty(actual))
            strCatZ(actual, ",");
        strCat(actual, storageInfoToLog(info));
    ENDFOREACH;

    TEST_RESULT_STR(actual, strNewZ(expected), testTitle);

    MEM_CONTEXT_TEMP_END();
}

/***********************************************************************************************************************************
Format the storage info into a usable form for debugging and display.
TODO: add details according to level.
***********************************************************************************************************************************/
String *storageInfoToLog(StorageInfo *this)
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

#ifdef XXXX
// -------------------------------------------------------------------------------------------------------------------------
        TEST_TITLE("path missing");

        TEST_ERROR_FMT(
                StorageScanP(storageTest, STRDEF(BOGUS_STR), .errorOnMissing = true), PathMissingError,
                STORAGE_ERROR_LIST_INFO_MISSING, TEST_PATH "/BOGUS");

        TEST_RESULT_PTR(storageNewItrP(storageTest, STRDEF(BOGUS_STR), .nullOnMissing = true), NULL, "ignore missing dir");

        // -------------------------------------------------------------------------------------------------------------------------
        TEST_TITLE("path with only dot");

        storagePathCreateP(storageTest, STRDEF("pg"), .mode = 0766);

        TEST_STORAGE_LIST(
                storageTest, "pg",
                "./ {u=" TEST_USER ", g=" TEST_GROUP ", m=0766}\n",
                .level = storageInfoLevelDetail, .includeDot = true);

        // -------------------------------------------------------------------------------------------------------------------------
        TEST_TITLE("path with file, link, pipe");

#ifdef TEST_CONTAINER_REQUIRED
        storagePathCreateP(storageTest, STRDEF("pg/.include"), .mode = 0755);
    HRN_SYSTEM("sudo chown 77777:77777 " TEST_PATH "/pg/.include");
#endif // TEST_CONTAINER_REQUIRED

        storagePutP(
                storageNewWriteP(storageTest, STRDEF("pg/file"), .modeFile = 0660, .timeModified = 1656433838), BUFSTRDEF("TESTDATA"));

        HRN_SYSTEM("ln -s ../file " TEST_PATH "/pg/link");
        HRN_SYSTEM("mkfifo -m 777 " TEST_PATH "/pg/pipe");

        TEST_STORAGE_LIST(
                storageTest, "pg",
                "./ {u=" TEST_USER ", g=" TEST_GROUP ", m=0766}\n"
#ifdef TEST_CONTAINER_REQUIRED
                ".include/ {u=77777, g=77777, m=0755}\n"
#endif // TEST_CONTAINER_REQUIRED
                "file {s=8, t=1656433838, u=" TEST_USER ", g=" TEST_GROUP ", m=0660}\n"
                "link> {d=../file, u=" TEST_USER ", g=" TEST_GROUP "}\n"
                "pipe*\n",
                .level = storageInfoLevelDetail, .includeDot = true);

#ifdef TEST_CONTAINER_REQUIRED
        HRN_SYSTEM("sudo rmdir " TEST_PATH "/pg/.include");
#endif // TEST_CONTAINER_REQUIRED
 // -------------------------------------------------------------------------------------------------------------------------
        TEST_TITLE("path - recurse desc");

        storagePathCreateP(storageTest, STRDEF("pg/path"), .mode = 0700);
        storagePutP(
                storageNewWriteP(storageTest, STRDEF("pg/path/file"), .modeFile = 0600, .timeModified = 1656434296),
        BUFSTRDEF("TESTDATA"));

        TEST_STORAGE_LIST(
                storageTest, "pg",
                "pipe*\n"
                "path/file {s=8, t=1656434296}\n"
                "path/\n"
                "link> {d=../file}\n"
                "file {s=8, t=1656433838}\n"
                "./\n",
                .level = storageInfoLevelBasic, .includeDot = true, .sortOrder = sortOrderDesc);

        // -------------------------------------------------------------------------------------------------------------------------
        TEST_TITLE("path - recurse asc");

        // Create a path with a subpath that will always be last to make sure lists are not freed too early in the iterator
        storagePathCreateP(storageTest, STRDEF("pg/zzz/yyy"), .mode = 0700);

        TEST_STORAGE_LIST(
                storageTest, "pg",
                "./\n"
                "file {s=8, t=1656433838}\n"
                "link> {d=../file}\n"
                "path/\n"
                "path/file {s=8, t=1656434296}\n"
                "pipe*\n"
                "zzz/\n"
                "zzz/yyy/\n",
                .level = storageInfoLevelBasic, .includeDot = true);

        // -------------------------------------------------------------------------------------------------------------------------
        TEST_TITLE("path basic info - recurse");

        //storageTest->pub.interface.feature ^= 1 << storageFeatureInfoDetail;

        TEST_STORAGE_LIST(
                storageTest, "pg",
                "zzz/yyy/\n"
                "zzz/\n"
                "pipe*\n"
                "path/file {s=8, t=1656434296}\n"
                "path/\n"
                "link> {d=../file}\n"
                "file {s=8, t=1656433838}\n"
                "./\n",
                .levelForce = true, .includeDot = true, .sortOrder = sortOrderDesc);

        // storageTest->pub.interface.feature ^= 1 << storageFeatureInfoDetail;

        // -------------------------------------------------------------------------------------------------------------------------
        TEST_TITLE("empty path - filter");

        storagePathCreateP(storageTest, STRDEF("pg/empty"), .mode = 0700);

        TEST_STORAGE_LIST(
                storageTest, "pg",
                "empty/\n",
                .level = storageInfoLevelType, .expression = "^empty");

        // -------------------------------------------------------------------------------------------------------------------------
        TEST_TITLE("filter in subpath during recursion");

        TEST_STORAGE_LIST(
                storageTest, "pg",
                "path/file {s=8, t=1656434296}\n",
                .level = storageInfoLevelBasic, .expression = "\\/file$");
            }
}
#endif // XXXX
