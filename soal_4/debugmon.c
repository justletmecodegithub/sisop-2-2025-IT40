#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>

#define LOG_FILE "debugmon.log"
#define PID_FILE "debugmon_daemon.pid"
#define BLOCKED_USERS_FILE "blocked_users.txt"
#define BUFFER_SIZE 1024

void write_log(const char* process_name, const char* status) {
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    char timestamp[25]; // Increased buffer size
    
    // Fixed format: [dd:mm:yyyy]-[hh:mm:ss]
    strftime(timestamp, sizeof(timestamp), "[%d:%m:%Y]-[%H:%M:%S]", tm);
    
    FILE *log = fopen(LOG_FILE, "a");
    if (log) {
        fprintf(log, "%s_%s_%s\n", timestamp, process_name, status);
        fclose(log);
    }
}
void list_user_processes(const char *user) {
    char cmd[BUFFER_SIZE];
    snprintf(cmd, BUFFER_SIZE, "ps -u %s -o pid,comm,%%cpu,%%mem", user);
    
    FILE *ps = popen(cmd, "r");
    if (ps) {
        char buf[BUFFER_SIZE];
        while (fgets(buf, BUFFER_SIZE, ps)) {
            printf("%s", buf);
            
            // Log running processes
            int pid;
            char name[BUFFER_SIZE];
            if (sscanf(buf, "%d %s", &pid, name) == 2) {
                write_log(name, "RUNNING");
            }
        }
        pclose(ps);
    }
}

void start_daemon(const char *user) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }
    
    if (pid > 0) { // Parent
        FILE *pf = fopen(PID_FILE, "w");
        if (pf) {
            fprintf(pf, "%d", pid);
            fclose(pf);
        }
        exit(0);
    }
    
    // Daemon
    umask(0);
    setsid();
    chdir("/");
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    while (1) {
        char cmd[BUFFER_SIZE];
        snprintf(cmd, BUFFER_SIZE, "ps -u %s -o pid,comm", user);
        
        FILE *ps = popen(cmd, "r");
        if (ps) {
            char buf[BUFFER_SIZE];
            fgets(buf, BUFFER_SIZE, ps); // Skip header
            
            while (fgets(buf, BUFFER_SIZE, ps)) {
                int pid;
                char name[BUFFER_SIZE];
                if (sscanf(buf, "%d %s", &pid, name) == 2) {
                    write_log(name, "RUNNING");
                }
            }
            pclose(ps);
        }
        sleep(5);
    }
}

void stop_monitoring(const char *user) {
    FILE *pf = fopen(PID_FILE, "r");
    if (!pf) {
        printf("No active monitoring\n");
        return;
    }
    
    pid_t pid;
    fscanf(pf, "%d", &pid);
    fclose(pf);
    
    if (kill(pid, SIGTERM) == -1) {
        perror("kill");
    } else {
        remove(PID_FILE);
    }
}
void fail_processes(const char *user) {
    char cmd[BUFFER_SIZE];
    snprintf(cmd, BUFFER_SIZE, "ps -u %s -o pid,comm --no-headers", user);
    
    FILE *ps = popen(cmd, "r");
    if (ps) {
        char buf[BUFFER_SIZE];
        while (fgets(buf, BUFFER_SIZE, ps)) {
            int pid;
            char name[BUFFER_SIZE];
            if (sscanf(buf, "%d %s", &pid, name) == 2) {
                if (kill(pid, SIGKILL) == 0) {
                    write_log(name, "FAILED");
                }
            }
        }
        pclose(ps);
    }
    
    // Block user from new processes
    FILE *blocked = fopen(BLOCKED_USERS_FILE, "a");
    if (blocked) {
        fprintf(blocked, "%s\n", user);
        fclose(blocked);
    }
}

/* Revert all to RUNNING status */
void revert_user(const char *user) {
    // Unblock user first
    FILE *blocked = fopen(BLOCKED_USERS_FILE, "r");
    if (blocked) {
        char tmpfile[] = "/tmp/debugmon.XXXXXX";
        int fd = mkstemp(tmpfile);
        FILE *tmp = fdopen(fd, "w");
        
        char buf[BUFFER_SIZE];
        while (fgets(buf, BUFFER_SIZE, blocked)) {
            if (!strstr(buf, user)) {
                fputs(buf, tmp);
            }
        }
        fclose(blocked);
        fclose(tmp);
        rename(tmpfile, BLOCKED_USERS_FILE);
    }
    
    // Log current processes as RUNNING
    char cmd[BUFFER_SIZE];
    snprintf(cmd, BUFFER_SIZE, "ps -u %s -o comm --no-headers", user);
    
    FILE *ps = popen(cmd, "r");
    if (ps) {
        char name[BUFFER_SIZE];
        while (fgets(name, BUFFER_SIZE, ps)) {
            // Remove newline if present
            name[strcspn(name, "\n")] = 0;
            write_log(name, "RUNNING");
        }
        pclose(ps);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <command> <user>\n", argv[0]);
        printf("Commands: list, daemon, stop, fail, revert\n");
        return 1;
    }
    
    const char *cmd = argv[1];
    const char *user = argv[2];
    
    if (strcmp(cmd, "list") == 0) {
        list_user_processes(user);
    } else if (strcmp(cmd, "daemon") == 0) {
        start_daemon(user);
    } else if (strcmp(cmd, "stop") == 0) {
        stop_monitoring(user);
    } else if (strcmp(cmd, "fail") == 0) {
        fail_processes(user);
    } else if (strcmp(cmd, "revert") == 0) {
        revert_user(user);
    } else {
        printf("Invalid command\n");
        return 1;
    }
    
    return 0;
} 
