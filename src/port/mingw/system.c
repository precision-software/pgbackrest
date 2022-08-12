#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
static const char EOS = '\0';

/***********************************************************************************************************************************
Safely append a string to a string buffer.
   Will not overwrite the buffer, but (ret) == bufend indicates buffer overflowed.
   Designed so a sequence of appends only needs one final check to detect overflow.
   @param str string to append
   @param bp pointer to last EOL character copied into string buffer.
   @param bufEnd end of the string buffer (buffer+size)
   @return pointer to where the next append would start.
*/
char *strAppend(char *bp, char const *bufEnd, const char *str) {

    // Copy characters from the string into the buffer while the buffer has room.
    for (; bp < bufEnd; bp++,str++) {
        *bp = *str;

        // Stop early at end of string.
        if (*bp == EOS)
            break;
    }

    return bp;
}


/***********************************************************************************************************************************
Quote a string and append it to the string buffer.
   Intended to be used for single quoting a shell command,  eg.  sh -c ' command '
   Shell quoting is somewhat unpredictable, but this covers the common cases.
 * @param str the unquoted shell command string
 * @param bp next location in the string buffer
 * @param bufEnd last location of the string buffer (buffer+size)
 * @return pointer to where the next append would start.
 */
char *strQuoteAppend(char *bp, char const *bufEnd, const char *str) {

    // Start with a quote
    bp = strAppend(bp, bufEnd, "\"");

    // Copy string characters into the buffer while buffer has room
    for (; bp < bufEnd; bp++,str++) {

        // Depending on the string character, copy an escape sequence into the buffer.
        //    At end of this statement, bp will point to where next character would be placed.
        if (*str == '\\')   bp = strAppend(bp, bufEnd, "\\\\");  // 2 character escape
        else if (*str == '\"')  bp = strAppend(bp, bufEnd, "\"");  // 1 character escape
        else ;

        // Copy the character itself into the buffer.
        *bp = *str;

        // Exit loop when the end of string (EOS) is copied into the buffer.
        if (*bp == EOS)
            break;
    }

    // End with a final quote.
    bp = strAppend(bp, bufEnd, "\"");

    // If we haven't overflowed, we should be pointing the EOS character.
    //assert(*bp == EOS || bp == bufEnd);
    return bp;
}

/**********************************************************************************************************************************
 * Safely format a string and append it to the buffer.
 */
char *strAppendFmt(char *bp, char const * bufEnd, const char const *format, ...) {MISSING;}

#define debug printf
/**********************************************************************************************************************************
Execute a Posix style shell command. Quoting can be tricky, so things could break for unusual or complex shell commands.
    Note this implementation requires Msys2 be installed at the default C:/msys64 location.

This version of system() handles:
   system("meson setup build")               // Run a simple program with arguments.
   system("echo *")                          // Wildcards match.
   system("echo '*'")                        // single quote disables wildcard.
   system("diff file1  file2 > file.diff")  // I/O redirection.
 */
int pgsystem(const char *cmd) {
#undef system

    // Which shell are we using? We could pick it up from the runtime path, but msys2 will be more reproducible..
    const char const *shell = "c:\\msys64/usr/bin/bash";

    // Allocate a temp buffer to build up the command. TODO: use malloc?
    char buf[10240];
    char const *bufEnd = buf + sizeof(buf);

    // Create a command line "shell -c <quoted cmd>" by appending multiple strings.
    char *bp = buf;
    bp = strAppend(bp, bufEnd, shell);
    bp = strAppend(bp, bufEnd, " -c ");
    bp = strQuoteAppend(bp, bufEnd, cmd);

    debug("executing command system(%s)\n", buf);
    assert (bp < bufEnd);

    // Run the command.
    return system(buf);
}

