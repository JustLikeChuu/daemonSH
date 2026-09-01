/*
 * rc_parser.c
 *
 * Classifies one line of .cseshellrc as empty, a PATH= directive, or a
 * startup command to run. See rc_parser.h for the full contract.
 */

#include "libs/rc_parser.h"

#include <ctype.h>
#include <string.h>

rc_line_type_t classify_rc_line(const char *line, const char **value)
{
    // Skip leading whitespace without modifying the input buffer
    const char *p = line;
    while (*p != '\0' && isspace((unsigned char)*p))
        p++;

    // Nothing left after trimming: blank or whitespace-only line
    if (*p == '\0')
    {
        *value = NULL; // Go to the address value holds and write NULL there
        return RC_LINE_EMPTY;
    }

    // "PATH=" must appear literally right at the start of the trimmed line
    // "PATHETIC" doesn't match here since strncmp checks the '=' too
    static const char PATH_PREFIX[] = "PATH=";
    size_t prefix_len = strlen(PATH_PREFIX);
    if (strncmp(p, PATH_PREFIX, prefix_len) == 0)
    {
        // Since p points at the P in PATH=/usr/bin:/bin, adding 5 moves the pointer 5 bytes forward
        // Lands at /usr/bin:/bin
        *value = p + prefix_len;
        return RC_LINE_PATH;
    }

    // Anything else, trimmed, is a command to run at shell startup
    *value = p;
    return RC_LINE_COMMAND;
}
