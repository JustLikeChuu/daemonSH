#include "shell.h"
#include "builtins.h"
#include "rc_startup.h"

// The main function where the shell's execution begins
int main(void)
{
    // Define an array to hold the command and its arguments
    char *cmd[MAX_ARGS];

    // Capture shell's own base directory once; external commands to resolve against this, not live cwd
    char base_dir[PATH_MAX];
    if (getcwd(base_dir, sizeof(base_dir)) == NULL)
    {
        perror("getcwd");
        return 1;
    }

    // Bundled system programs live at <base_dir>/bin.
    // This is the shell's PATH until/unless .cseshellrc extends it with PATH=.
    char initial_path[PATH_MAX];
    snprintf(initial_path, sizeof(initial_path), "%s/bin", base_dir);
    setenv("PATH", initial_path, 1);

    if (load_cseshellrc(base_dir) == 1)
        return 0; // .cseshellrc itself called "exit"

    for (;;)
    {
        cmd[0] = NULL; // Reset so a blank line is detectable, not garbage

        type_prompt();               // Display the prompt
        int eof = read_command(cmd); // Read a command from the user

        if (eof) // Exit the loop; same as typing "exit"
        {
            break;
        }

        if (cmd[0] == NULL) // Blank line; re-prompt since nothing to run
        {
            continue;
        }

        if (execute_command(cmd) == 1)
        {
            break;
        }
    }

    return 0;
}