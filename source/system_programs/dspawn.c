#include "system_program.h"

static char output_file_path[PATH_MAX];

static int daemon_work(void)
{
    int num = 0;
    FILE *fptr;
    char buffer[1024];
    char *cwd;

    fptr = fopen(output_file_path, "a");
    if (fptr == NULL)
    {
        return EXIT_FAILURE;
    }

    fprintf(fptr, "Daemon process running with PID: %d, PPID: %d, opening logfile with FD %d\n",
            getpid(), getppid(), fileno(fptr));

    cwd = getcwd(buffer, sizeof(buffer));
    if (cwd == NULL)
    {
        fprintf(fptr, "getcwd() failed\n");
        fclose(fptr);
        return EXIT_FAILURE;
    }
    fprintf(fptr, "Current working directory: %s\n", cwd);
    fclose(fptr);

    while (1)
    {
        fptr = fopen(output_file_path, "a");
        if (fptr == NULL)
        {
            return EXIT_FAILURE;
        }

        fprintf(fptr, "PID %d Daemon writing line %d to the file.\n", getpid(), num);
        num++;

        fclose(fptr);

        sleep(10);

        if (num == 10)
            break;
    }

    return EXIT_SUCCESS;
}

int main(void)
{
    // Capture the log path BEFORE any chdir happens
    if (getcwd(output_file_path, sizeof(output_file_path)) == NULL)
    {
        perror("getcwd() error, exiting now.");
        return 1;
    }
    strcat(output_file_path, "/dspawn.log");

    // First fork
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork failed");
        return 1;
    }
    else if (pid > 0)
    {
        // Original dspawn process exits immediately
        // The shell's waitpid() on this pid returns now, prompt comes right back
        exit(1);
    }

    // Session leader, losing the controlling TTY
    if (setsid() < 0)
    {
        exit(1);
    }

    // Ignore SIGCHLD (daemon is reaped immediately, no zombie) and SIGHUP (daemon survives this session leader's termination).
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    // Second fork; Intermediate (the session leader) terminates, so the daemon itself can never be a session leader
    pid = fork();
    if (pid < 0)
    {
        exit(1);
    }
    else if (pid > 0)
    {
        exit(0); // intermediate process: job done
    }

    // Newly created files are world RW+executable; can't interactively fix permissions on anything this daemon creates.
    umask(0);

    // Move off any directory that might need to be unmounted
    if (chdir("/") < 0)
    {
        exit(1);
    }

    // Close every inherited fd, then reattach 0/1/2 to /dev/null
    for (int fd = (int)sysconf(_SC_OPEN_MAX); fd >= 0; fd--)
    {
        close(fd);
    }
    int fd0 = open("/dev/null", O_RDWR);
    dup(fd0); // becomes fd 1
    dup(fd0); // becomes fd 2

    // Terminate after 10 iterations rather than running forever
    return daemon_work();
}