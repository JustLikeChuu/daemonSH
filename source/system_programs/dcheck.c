#include "system_program.h"

#define TMP_OUTPUT_PATH "/tmp/dcheck_ps_output.txt"

int main(void)
{
    setenv("PATH", "/usr/bin:/bin:/usr/local/bin", 1);

    char command[512];
    snprintf(command, sizeof(command), "ps -eo pid,tty,stat,comm | grep dspawn | grep -Ev 'tty|pts' | grep -v 'Z' | grep -v grep > %s", TMP_OUTPUT_PATH);

    if (system(command) == -1)
    {
        perror("system");
        return EXIT_FAILURE;
    }

    FILE *fptr = fopen(TMP_OUTPUT_PATH, "r");
    if (fptr == NULL)
    {
        perror("fopen");
        return EXIT_FAILURE;
    }

    int count = 0;
    char line[1024];
    while (fgets(line, sizeof(line), fptr) != NULL)
    {
        count++;
    }
    fclose(fptr);
    remove(TMP_OUTPUT_PATH);

    printf(COLOR_CYAN "Live dspawn daemons: " COLOR_RESET "%d\n", count);

    return EXIT_SUCCESS;
}