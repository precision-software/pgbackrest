/***********************************************************************************************************************************
Test Regular Expression Handler
***********************************************************************************************************************************/

#include "common/harnessTest.h"

/***********************************************************************************************************************************
Test Run
***********************************************************************************************************************************/
static void
testRun(void)
{
    FUNCTION_HARNESS_VOID();

    // *****************************************************************************************************************************
    if (testBegin("regExpNew(), regExpMatch(), and regExpFree()"))
    {
        // Error message varies based on the libc version/OS
        char *messages[] = {
                "Unmatched [ or [^",                           // Older libc
                "Unmatched [, [^, [:, [., or [=",              // New libc
                "brackets ([ ]) not balanced",                 // MacOS
                "Missing ']'",                                 // MinGW
                };

        TEST_ERROR_MESSAGES(regExpNew(STRDEF("[[[")), FormatError, messages, sizeof(messages)/sizeof(*messages));


        char *badbrMessages[] = {
                "Invalid content of \\{\\}",
                "invalid repetition count(s)",
                "Invalid contents of {}"
        };

        TEST_ERROR_MESSAGES(regExpErrorCheck(REG_BADBR), FormatError, badbrMessages, sizeof(badbrMessages)/sizeof(*badbrMessages));


        // -------------------------------------------------------------------------------------------------------------------------
        TEST_TITLE("new regexp");

        RegExp *regExp = NULL;
        TEST_ASSIGN(regExp, regExpNew(STRDEF("^abc")), "new regexp");

        // -------------------------------------------------------------------------------------------------------------------------
        TEST_TITLE("regexp match");

        const String *string = STRDEF("abcdef");
        TEST_RESULT_BOOL(regExpMatch(regExp, string), true, "match regexp");
        TEST_RESULT_PTR(regExpMatchPtr(regExp), strZ(string), "check ptr");
        TEST_RESULT_UINT(regExpMatchSize(regExp), 3, "check size");
        TEST_RESULT_STR_Z(regExpMatchStr(regExp), "abc", "check str");

        // -------------------------------------------------------------------------------------------------------------------------
        TEST_TITLE("no regexp match");

        TEST_RESULT_BOOL(regExpMatch(regExp, STRDEF("bcdef")), false, "no match regexp");
        TEST_RESULT_PTR(regExpMatchPtr(regExp), NULL, "check ptr");
        TEST_RESULT_UINT(regExpMatchSize(regExp), 0, "check size");
        TEST_RESULT_STR(regExpMatchStr(regExp), NULL, "check str");

        // -------------------------------------------------------------------------------------------------------------------------
        TEST_TITLE("free regexp");

        TEST_RESULT_VOID(regExpFree(regExp), "free regexp");
    }

    // *****************************************************************************************************************************
    if (testBegin("regExpPrefix()"))
    {
        TEST_RESULT_STR(regExpPrefix(NULL), NULL, "null expression has no prefix");
        TEST_RESULT_STR(regExpPrefix(strNew()), NULL, "empty expression has no prefix");
        TEST_RESULT_STR(regExpPrefix(STRDEF("ABC")), NULL, "expression without begin anchor has no prefix");
        TEST_RESULT_STR(regExpPrefix(STRDEF("^.")), NULL, "expression with no regular character has no prefix");

        TEST_RESULT_STR_Z(regExpPrefix(STRDEF("^ABC$")), "ABC", "prefix stops at special character");
        TEST_RESULT_STR_Z(regExpPrefix(STRDEF("^ABC*")), "ABC", "prefix stops at special character");
        TEST_RESULT_STR_Z(regExpPrefix(STRDEF("^ABC+")), "ABC", "prefix stops at special character");
        TEST_RESULT_STR_Z(regExpPrefix(STRDEF("^ABC-")), "ABC", "prefix stops at special character");
        TEST_RESULT_STR_Z(regExpPrefix(STRDEF("^ABC?")), "ABC", "prefix stops at special character");
        TEST_RESULT_STR_Z(regExpPrefix(STRDEF("^ABC(")), "ABC", "prefix stops at special character");
        TEST_RESULT_STR_Z(regExpPrefix(STRDEF("^ABC[")), "ABC", "prefix stops at special character");
        TEST_RESULT_STR_Z(regExpPrefix(STRDEF("^ABC{")), "ABC", "prefix stops at special character");
        TEST_RESULT_STR_Z(regExpPrefix(STRDEF("^ABC ")), "ABC", "prefix stops at special character");
        TEST_RESULT_STR_Z(regExpPrefix(STRDEF("^ABC|")), "ABC", "prefix stops at special character");
        TEST_RESULT_STR_Z(regExpPrefix(STRDEF("^ABC\\")), "ABC", "prefix stops at special character");

        TEST_RESULT_STR_Z(regExpPrefix(STRDEF("^ABC^")), NULL, "no prefix when more than one begin anchor");
        TEST_RESULT_STR_Z(regExpPrefix(STRDEF("^ABC|^DEF")), NULL, "no prefix when more than one begin anchor");
        TEST_RESULT_STR_Z(regExpPrefix(STRDEF("^ABC[^DEF]")), "ABC", "prefix when ^ used for exclusion");
        TEST_RESULT_STR_Z(regExpPrefix(STRDEF("^ABC\\^DEF]")), "ABC", "prefix when ^ is escaped");

        TEST_RESULT_STR_Z(regExpPrefix(STRDEF("^ABCDEF")), "ABCDEF", "prefix is entire expression");
    }

    // *****************************************************************************************************************************
    if (testBegin("regExpMatchOne()"))
    {
        TEST_RESULT_BOOL(regExpMatchOne(STRDEF("^abc"), STRDEF("abcdef")), true, "match regexp");
        TEST_RESULT_BOOL(regExpMatchOne(STRDEF("^abc"), STRDEF("bcdef")), false, "no match regexp");
    }

    FUNCTION_HARNESS_RETURN_VOID();
}
