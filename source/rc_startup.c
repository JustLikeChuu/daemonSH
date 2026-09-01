#include "rc_startup.h"
#include "shell.h"
#include "libs/rc_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PATH_ENV 4096

// Runs one already-tokenized command line from .cseshellrc through the same path builtins/external commands take at the interactive prompt.
static int run_rc_command(const char *command_line)
{
    char line[MAX_LINE];
    strncpy(line, command_line, MAX_LINE - 1);
    line[MAX_LINE - 1] = '\0';

    char *cmd[MAX_ARGS];
    int i = 0;
    char *token = strtok(line, " \t");
    while (token != NULL && i < MAX_ARGS - 1)
    {
        cmd[i++] = token;
        token = strtok(NULL, " \t");
    }
    cmd[i] = NULL;

    if (i == 0)
        return 0;

    return execute_command(cmd);
}

int load_cseshellrc(const char *base_dir)
{
    char rc_path[PATH_MAX];
    snprintf(rc_path, sizeof(rc_path), "%s/.cseshellrc", base_dir);

    FILE *rc = fopen(rc_path, "r");
    if (rc == NULL)
        return 0; // No rc file: not an error, just nothing to configure

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), rc) != NULL)
    {
        // classify_rc_line only trims leading whitespace; strip the
        // trailing newline fgets() leaves in, ourselves.
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
        {
            line[--len] = '\0';
        }

        const char *value;
        rc_line_type_t type = classify_rc_line(line, &value);

        if (type == RC_LINE_EMPTY)
        {
            continue;
        }
        else if (type == RC_LINE_PATH)
        {
            const char *existing = getenv("PATH");
            char combined[MAX_PATH_ENV];
            if (existing != NULL && existing[0] != '\0')
            {
                snprintf(combined, sizeof(combined), "%s:%s", existing, value);
            }
            else
            {
                snprintf(combined, sizeof(combined), "%s", value);
            }
            setenv("PATH", combined, 1);
        }
        else // RC_LINE_COMMAND
        {
            if (run_rc_command(value) == 1)
            {
                fclose(rc);
                return 1; // "exit" during startup: quit before the first prompt
            }
        }
    }

    fclose(rc);
    return 0;
}