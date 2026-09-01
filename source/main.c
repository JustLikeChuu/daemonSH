#include "shell.h"
#include "builtins.h"

// The main function where the shell's execution begins
int main(void)
{
    // Define an array to hold the command and its arguments
    char *cmd[MAX_ARGS];
    int child_status;
    pid_t pid;

    for (;;)
    {
        cmd[0] = NULL // Reset so a blank line is detectable, not garbage

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

        // If the command is "exit", break out of the loop to terminate the shell
        if (is_builtin)cmd[0]))
            {
                if (run_builtin(cmd) == 1)
                {
                    break;
                }
                continue;
            }

        // Formulate the full path of the command to be executed
        char full_path[PATH_MAX];
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {

            snprintf(full_path, sizeof(full_path), "%s/bin/%s", cwd, cmd[0]);
        }
        else
        {
            printf("Failed to get current working directory.");
            continue;
        }

        pid = fork();
        if (pid < 0)
        {
            perror("Fork failed!")
            continue;
        }
        else if (pid = 0) // Forked successfully
        {
            // Child: replace this process image with the target program
            execv(full_path, cmd);

            // If execv returns, command execution has failed
            printf("Command %s not found\n", cmd[0]);
            exit(1);
        }
        else
        {
            // Parent: wait for specific child before showing the next prompt 
            waitpid(pid, &child_status, 0);
        }
    }
    return 0;
}