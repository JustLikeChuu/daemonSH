#include "builtins.h"
#include "shell.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern char **environ;

static const char *BUILTIN_NAMES[] = {
    "cd", "help", "exit", "usage", "env", "setenv", "unsetenv"};

#define NUM_BUILTINS (sizeof(BUILTIN_NAMES) / sizeof(BUILTIN_NAMES[0]))

int is_builtin(const char *name)
{
    if (name == NULL)
        return 0;

    for (size_t i = 0; i < NUM_BUILTINS; i++)
    {
        if (strcmp(name, BUILTIN_NAMES[i]) == 0)
            return 1;
    }
    return 0;
}

// Changes the shell's own working directory. 
// Must run in-process: a forked child's chdir() would never be visible to the parent shell.
static void do_cd(char **cmd)
{
    // cmd[1] to be the destination directory
    // If NULL, falls back to $HOME
    const char *target = (cmd[1] != NULL) ? cmd[1] : getenv("HOME");
    // getenv("HOME") reads an env variable, returns NULL if not set

    if (target == NULL)
    {
        printf("cd: no directory specified and HOME is not set\n");
        return;
    }

    if (chdir(target) != 0) // 0 on success, -1 on failure
    {
        perror("cd");
    }
}

static void do_help(void)
{
    printf("daemonSH builtins:\n");
    printf("  cd [dir]        Change the current directory\n");
    printf("  help            List available builtins\n");
    printf("  exit            Exit the shell\n");
    printf("  usage           Show shell usage information\n");
    printf("  env             List all environment variables\n");
    printf("  setenv KEY=VAL  Set an environment variable\n");
    printf("  unsetenv KEY    Remove an environment variable\n");
}

static void do_usage(void)
{
    printf("Usage: type a command name to run it, or one of the builtins\n");
    printf("listed by 'help'. External commands are looked up under ./bin/.\n");
}

static void do_env(void)
{
    // environ: global array of C-strings, KEY=VALUE each; terminated by a NULL pointer instead of storing a length
    for (char **entry = environ; *entry != NULL; entry++)
    {
        printf("%s\n", *entry);
    }
}

// setenv KEY=VALUE, split on '='
static void do_setenv(char **cmd)
{
    if (cmd[1] == NULL)
    {
        printf("setenv: usage: setenv KEY=VALUE\n");
        return;
    }

    // strdup makes a heap copy of the string to prevent corruption
    char *arg = strdup(cmd[1]);

    // strchr finds the first = and returns a pointer to its exact position inside arg
    char *eq = strchr(arg, '=');
    if (eq == NULL)
    {
        printf("setenv: usage: setenv KEY=VALUE\n");
        free(arg);
        return;
    }

    // Overwrite with null terminator; then assign remainder
    *eq = '\0';
    const char *key = arg;
    const char *value = eq + 1;

    if (setenv(key, value, 1) != 0) // setenv: 1 means overwrite if the variable already exists
    {
        perror("setenv");
    }

    free(arg);
}

static void do_unsetenv(char **cmd)
{
    if (cmd[1] == NULL)
    {
        printf("unsetenv: usage: unsetenv KEY\n");
        return;
    }

    if (unsetenv(cmd[1]) != 0)
    {
        perror("unsetenv");
    }
}

int run_builtin(char **cmd)
{
    if (strcmp(cmd[0], "cd") == 0)
    {
        do_cd(cmd);
    }
    else if (strcmp(cmd[0], "help") == 0)
    {
        do_help();
    }
    else if (strcmp(cmd[0], "exit") == 0)
    {
        return 1;
    }
    else if (strcmp(cmd[0], "usage") == 0)
    {
        do_usage();
    }
    else if (strcmp(cmd[0], "env") == 0)
    {
        do_env();
    }
    else if (strcmp(cmd[0], "setenv") == 0)
    {
        do_setenv(cmd);
    }
    else if (strcmp(cmd[0], "unsetenv") == 0)
    {
        do_unsetenv(cmd);
    }

    return 0;
}