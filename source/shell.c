#include "shell.h"
#include "builtins.h"

// Function to read a command from the user input
// Returns 1 if EOF was hit before any input, 0 otherwise
int read_command(char **cmd)
{
  // Define a character array to store the command line input
  char line[MAX_LINE];
  // Initialize count to keep track of the number of characters read
  int count = 0, i = 0;
  // Array to hold pointers to the parsed command arguments
  char *array[MAX_ARGS], *command_token;

  // Infinite loop to read characters until a newline or maximum line length is reached
  for (;;)
  {
    // Read a single character from standard input
    int current_char = fgetc(stdin);

    if (current_char == EOF)
    {
      if (count == 0)
      {
        return 1; // Nothing typed yet, tell the caller to exit
      }
      break; // Partial line before EOF, treat like a newline
    }

    // Store the character in the line array and increment count
    line[count++] = (char)current_char;

    // If a newline character is encountered, break out of the loop
    if (current_char == '\n')
    {
      break;
    }

    // If the command exceeds the maximum length, print an error and exit
    if (count >= MAX_LINE)
    {
      printf("Command is too long, unable to process\n");
      exit(1);
    }
  }

  // Null-terminate the command line string
  line[count] = '\0';

  // If only the newline character was entered, return without processing
  if (count <= 1)
  {
    cmd[0] = NULL;
    return 0;
  }

  // Use strtok to parse the first token (word) of the command
  command_token = strtok(line, " \n");

  // Continue parsing the line into words and store them in the array
  while (command_token != NULL)
  {
    array[i++] = strdup(command_token);  // Duplicate the token and store it
    command_token = strtok(NULL, " \n"); // Get the next token
  }

  // Copy the parsed command and its parameters to the cmd array
  for (int j = 0; j < i; j++)
  {
    cmd[j] = array[j];
  }
  // Null-terminate the cmd array to mark the end of arguments
  cmd[i] = NULL;
  return 0;
}

// Function to display the shell prompt
void type_prompt()
{
  // Use a static variable to check if this is the first call to the function
  static int first_time = 1;
  if (first_time)
  {
    if (isatty(fileno(stdin)))
    {
      printf("\033[H\033[J"); // ANSI: cursor home + clear screen
    }

    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));

    printf("========================================\n");
    printf("  Welcome to daemonSH\n");
    printf("========================================\n");
    printf("Working directory: %s\n", cwd);
    printf("PATH: %s\n", getenv("PATH"));
    printf("Type 'help' to see available commands.\n");
    printf("----------------------------------------\n");

    first_time = 0;
  }
  fflush(stdout); // Flush the output buffer
  printf("$$ ");  // Print the shell prompt
}

int execute_command(char **cmd)
{
  if (is_builtin(cmd[0]))
  {
    return run_builtin(cmd); // 1 signals "exit" was run
  }

  pid_t pid = fork();
  if (pid < 0)
  {
    perror("Fork failed!");
    return 0;
  }
  else if (pid == 0)
  {
    // Child: PATH-based lookup, covers both ./bin/ and anything
    // .cseshellrc's PATH= line added.
    execvp(cmd[0], cmd);
    printf("Command %s not found\n", cmd[0]);
    exit(1);
  }
  else
  {
    int status;
    waitpid(pid, &status, 0);
    return 0;
  }
}