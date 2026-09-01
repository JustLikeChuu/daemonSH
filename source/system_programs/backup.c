#include "system_program.h"

int main(void)
{
    setenv("PATH", "/usr/bin:/bin:/usr/local/bin", 1);

    const char *backup_dir = getenv("BACKUP_DIR");
    if (backup_dir == NULL || backup_dir[0] == '\0')
    {
        printf("backup: BACKUP_DIR is not set. Use 'setenv BACKUP_DIR <name>' first.\n");
        return EXIT_FAILURE;
    }

    struct stat target_stat;
    if (stat(backup_dir, &target_stat) != 0)
    {
        printf("backup: '%s' does not exist.\n", backup_dir);
        return EXIT_FAILURE;
    }

    int is_directory = S_ISDIR(target_stat.st_mode);

    // Resolve [PROJECT_DIR]/archive/ relative to wherever its being run from right now, matching the convention dspawn uses for its own log.
    char project_dir[PATH_MAX];
    if (getcwd(project_dir, sizeof(project_dir)) == NULL)
    {
        perror("getcwd");
        return EXIT_FAILURE;
    }

    char archive_dir[PATH_MAX];
    snprintf(archive_dir, sizeof(archive_dir), "%s/archive", project_dir);
    mkdir(archive_dir, 0755); // fine if it already exists

    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", tm_info);

    char archive_path[PATH_MAX];
    snprintf(archive_path, sizeof(archive_path), "%s/%s_%s.zip", archive_dir, backup_dir, timestamp);

    char command[PATH_MAX * 2];
    if (is_directory)
    {
        snprintf(command, sizeof(command), "zip -r -q \"%s\" \"%s\"", archive_path, backup_dir);
    }
    else
    {
        snprintf(command, sizeof(command), "zip -q \"%s\" \"%s\"", archive_path, backup_dir);
    }

    int result = system(command);
    if (result != 0)
    {
        printf("backup: zip command failed (is 'zip' installed?).\n");
        return EXIT_FAILURE;
    }

    printf(COLOR_GREEN "Backup complete:\n" COLOR_RESET);
    printf("  Source:  %s (%s)\n", backup_dir, is_directory ? "directory" : "file");
    printf("  Archive: %s\n", archive_path);

    return EXIT_SUCCESS;
}