#ifndef RC_STARTUP_H
#define RC_STARTUP_H

// Loads and runs .daemonshellrc from base_dir, if present. 
// Applies PATH= directives to the environment and runs command lines via execute_command.
// Returns 1 if a startup command was "exit" (shell should quit immediately), 0 otherwise.
int load_daemonshellrc(const char *base_dir);

#endif