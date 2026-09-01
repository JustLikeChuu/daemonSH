#include "system_program.h"
#include <sys/sysinfo.h>
#include <sys/statvfs.h>

static void print_banner(void)
{
    printf(COLOR_CYAN);
    printf("     _                                  ____  _   _ \n");
    printf("  __| | __ _  ___ _ __ ___   ___  _ __ / ___|| | | |\n");
    printf(" / _` |/ _` |/ _ \\ '_ ` _ \\ / _ \\| '_ \\\\___ \\| |_| |\n");
    printf("| (_| | (_| |  __/ | | | | | (_) | | | |___) |  _  |\n");
    printf(" \\__,_|\\__,_|\\___|_| |_| |_|\\___/|_| |_|____/|_| |_|\n");
    printf(COLOR_RESET "\n");
}

int main(void)
{
    print_banner();

    struct utsname sys_info;
    if (uname(&sys_info) != 0)
    {
        perror("uname");
        return EXIT_FAILURE;
    }

    struct sysinfo mem_info;
    if (sysinfo(&mem_info) != 0)
    {
        perror("sysinfo");
        return EXIT_FAILURE;
    }

    // sysinfo() reports memory in units of mem_unit bytes, not always 1 byte
    unsigned long total_mem_mb = (mem_info.totalram * mem_info.mem_unit) / (1024 * 1024);
    unsigned long free_mem_mb = (mem_info.freeram * mem_info.mem_unit) / (1024 * 1024);
    unsigned long used_mem_mb = total_mem_mb - free_mem_mb;

    long uptime_sec = mem_info.uptime;
    long uptime_hr = uptime_sec / 3600;
    long uptime_min = (uptime_sec % 3600) / 60;

    struct passwd *pw = getpwuid(getuid());
    const char *username = (pw != NULL) ? pw->pw_name : "unknown";

    // No syscall exposes the CPU model directly on Linux; read /proc/cpuinfo instead
    char cpu_model[256] = "unknown";
    FILE *cpuinfo = fopen("/proc/cpuinfo", "r");
    if (cpuinfo != NULL)
    {
        char line[256];
        while (fgets(line, sizeof(line), cpuinfo) != NULL)
        {
            if (strncmp(line, "model name", 10) == 0)
            {
                char *colon = strchr(line, ':');
                if (colon != NULL)
                {
                    char *value = colon + 2; // skip ": "
                    size_t len = strlen(value);
                    if (len > 0 && value[len - 1] == '\n')
                        value[len - 1] = '\0';
                    strncpy(cpu_model, value, sizeof(cpu_model) - 1);
                }
                break;
            }
        }
        fclose(cpuinfo);
    }

    struct statvfs disk_info;
    double disk_total_gb = 0, disk_used_gb = 0;
    int disk_percent = 0;
    if (statvfs("/", &disk_info) == 0)
    {
        double block_size = disk_info.f_frsize;
        disk_total_gb = (disk_info.f_blocks * block_size) / (1024.0 * 1024 * 1024);
        double disk_free_gb = (disk_info.f_bfree * block_size) / (1024.0 * 1024 * 1024);
        disk_used_gb = disk_total_gb - disk_free_gb;
        if (disk_total_gb > 0)
            disk_percent = (int)((disk_used_gb / disk_total_gb) * 100);
    }

    printf(COLOR_GREEN "%s@%s\n" COLOR_RESET, username, sys_info.nodename);
    printf("----------------\n");
    printf(COLOR_CYAN "OS:       " COLOR_RESET "%s %s\n", sys_info.sysname, sys_info.machine);
    printf(COLOR_CYAN "Host:     " COLOR_RESET "%s\n", sys_info.nodename);
    printf(COLOR_CYAN "Kernel:   " COLOR_RESET "%s\n", sys_info.release);
    printf(COLOR_CYAN "Uptime:   " COLOR_RESET "%ldh %ldm\n", uptime_hr, uptime_min);
    printf(COLOR_CYAN "User:     " COLOR_RESET "%s\n", username);
    printf(COLOR_CYAN "Shell:    " COLOR_RESET "cseshell\n");
    printf(COLOR_CYAN "CPU:      " COLOR_RESET "%s\n", cpu_model);
    printf(COLOR_CYAN "Memory:   " COLOR_RESET "%lu MiB / %lu MiB\n", used_mem_mb, total_mem_mb);
    printf(COLOR_CYAN "Disk (/): " COLOR_RESET "%.1f GiB / %.1f GiB (%d%%)\n", disk_used_gb, disk_total_gb, disk_percent);

    return EXIT_SUCCESS;
}