#ifndef BUILTINS_H
#define BUILTINS_H

// Returns 1 if name is one of the seven required builtins, 0 otherwise.
int is_builtin(const char *name);

// Runs the builtin named by cmd[0] in the current process.
// Returns 1 if the shell should exit (i.e. "exit" was run), 0 to keep looping.
int run_builtin(char **cmd);

#endif