#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <stdarg.h>
#include <errno.h>
#include <zip.h>
#include <sys/wait.h>

#define STARTER_ZIP "starterkit.zip"
#define ZIP_URL "https://drive.usercontent.google.com/u/0/uc?id=1_5GxIGfQr3mNKuavJbte_AoRkEQLXSKS&export=download"
#define STARTER_DIR "starter_kit"
#define QUARANTINE_DIR "quarantine"
#define LOG_FILE "activity.log"
#define PID_FILE "daemon.pid"
#define BUFFER_SIZE 1024

void note_log(const char *format, ...) {
    FILE *log = fopen(LOG_FILE, "a");
    if (!log) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timetotal[64];
    strftime(timetotal, sizeof(timetotal), "[%d-%m-%Y][%H:%M:%S]", t);
    fprintf(log, "%s - ", timetotal);

    va_list args;
    va_start(args, format);
    vfprintf(log, format, args);
    va_end(args);

    fprintf(log, "\n");
    fclose(log);
}

void clean_filename(char *str) {
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\'' || str[len - 1] == '\r'))
        str[--len] = '\0';
    if (str[0] == '\'') memmove(str, str + 1, len);
}

void download_zip() {
    if (access(STARTER_ZIP, F_OK) == 0) {
        printf("ZIP file already exists.\n");
        return;
    }

    printf("Downloading ZIP...\n");
    pid_t pid = fork();
    if (pid == 0) {
        execlp("wget", "wget", "-O", STARTER_ZIP, ZIP_URL, (char *)NULL);
        perror("wget failed");
        exit(1);
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (!(WIFEXITED(status) && WEXITSTATUS(status) == 0)) {
            fprintf(stderr, "Download failed.\n");
            exit(1);
        }
    }
}

void unzip_zip() {
    int err = 0;
    struct zip *z = zip_open(STARTER_ZIP, 0, &err);
    if (!z) {
        fprintf(stderr, "Failed to open zip file.\n");
        return;
    }

    mkdir(STARTER_DIR, 0755);

    for (zip_int64_t i = 0; i < zip_get_num_entries(z, 0); i++) {
        struct zip_stat st;
        zip_stat_index(z, i, 0, &st);

        char filepath[BUFFER_SIZE];
        snprintf(filepath, sizeof(filepath), "%s/%s", STARTER_DIR, st.name);

        FILE *f = fopen(filepath, "wb");
        if (!f) continue;

        struct zip_file *zf = zip_fopen_index(z, i, 0);
        char buf[BUFFER_SIZE];
        zip_int64_t len;
        while ((len = zip_fread(zf, buf, BUFFER_SIZE)) > 0) {
            fwrite(buf, 1, len, f);
        }

        fclose(f);
        zip_fclose(zf);
    }

    zip_close(z);
    remove(STARTER_ZIP);
}

int base64_value(char c) {
    if ('A' <= c && c <= 'Z') return c - 'A';
    if ('a' <= c && c <= 'z') return c - 'a' + 26;
    if ('0' <= c && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

void decode_base64(const char *in, char *out) {
    int buffer = 0, bits = 0, len = 0;
    for (int i = 0; in[i] && in[i] != '='; i++) {
        int val = base64_value(in[i]);
        if (val == -1) continue;
        buffer = (buffer << 6) | val;
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            out[len++] = (buffer >> bits) & 0xFF;
        }
    }
    out[len] = '\0';
}

int is_base64(const char *str) {
    int len = strlen(str);
    if (len % 4 != 0) return 0;
    for (int i = 0; i < len; i++) {
        if (!(isalnum(str[i]) || str[i] == '+' || str[i] == '/' || str[i] == '=')) {
            return 0;
        }
    }
    return 1;
}

int is_printable(const char *str) {
    for (int i = 0; str[i]; i++) {
        if (!isprint((unsigned char)str[i])) return 0;
    }
    return 1;
}

void move_file(const char *from, const char *to, const char *action) {
    if (rename(from, to) == 0) {
        char *filename = strrchr(to, '/');
        note_log("%s - Successfully %s", filename ? filename + 1 : to, action);
    } else {
        note_log("Failed to move %s → %s: %s", from, to, strerror(errno));
    }
}

void decrypt_files() {
    FILE *pf = fopen("daemon.pid", "w");
    if (pf) {
        fprintf(pf, "%d\n", getpid());
        fclose(pf);
    }

    mkdir(QUARANTINE_DIR, 0755);
    note_log("Successfully started decryption process with PID %d", getpid());

    while (1) {
        DIR *d = opendir(STARTER_DIR);
        struct dirent *f;
        if (!d) {
            sleep(2);
            continue;
        }

        while ((f = readdir(d))) {
            if (strcmp(f->d_name, ".") == 0 || strcmp(f->d_name, "..") == 0) continue;

            char clean[BUFFER_SIZE];
            strncpy(clean, f->d_name, BUFFER_SIZE);
            clean_filename(clean);

            if (!is_base64(clean)) {
                continue;
            }

            char decoded[BUFFER_SIZE];
            decode_base64(clean, decoded);
            decoded[strcspn(decoded, "\n\r")] = '\0';

            if (!is_printable(decoded)) {
                note_log("%s - Skipped: decoded name contains non-printable characters.", clean);
                continue;
            }

            char src[BUFFER_SIZE], dst[BUFFER_SIZE];
            snprintf(src, sizeof(src), "%s/%s", STARTER_DIR, f->d_name);
            snprintf(dst, sizeof(dst), "%s/%s", QUARANTINE_DIR, decoded);
            move_file(src, dst, "decrypted and quarantined");
        }

        closedir(d);
        sleep(2);
    }
}

void move_all(const char *from, const char *to, const char *action) {
    DIR *d = opendir(from);
    struct dirent *f;
    if (!d) return;

    while ((f = readdir(d))) {
        if (strcmp(f->d_name, ".") == 0 || strcmp(f->d_name, "..") == 0) continue;

        char src[BUFFER_SIZE], dst[BUFFER_SIZE];
        snprintf(src, sizeof(src), "%s/%s", from, f->d_name);
        snprintf(dst, sizeof(dst), "%s/%s", to, f->d_name);
        move_file(src, dst, action);
    }

    closedir(d);
}

void delete_all(const char *dir) {
    DIR *d = opendir(dir);
    struct dirent *f;
    if (!d) return;

    while ((f = readdir(d))) {
        if (strcmp(f->d_name, ".") == 0 || strcmp(f->d_name, "..") == 0) continue;
        char path[BUFFER_SIZE];
        snprintf(path, sizeof(path), "%s/%s", dir, f->d_name);
        if (remove(path) == 0) {
            note_log("%s - Successfully deleted", f->d_name);
        }
    }

    closedir(d);
}

void shutdown() {
    FILE *fp = fopen("daemon.pid", "r");
    if (!fp) {
        fprintf(stderr, "PID file not found.\n");
        return;
    }

    pid_t pid;
    fscanf(fp, "%d", &pid);
    fclose(fp);

    if (kill(pid, SIGTERM) == 0) {
        printf("Daemon with PID %d ended.\n", pid);
        note_log("Successfully shut off decryption process with PID.", pid);
        remove("daemon.pid");
    } else {
        perror("Failed to ended daemon");
    }
}

int main(int argc, char *argv[]) {
    mkdir(STARTER_DIR, 0755);
    mkdir(QUARANTINE_DIR, 0755);

    if (argc < 2) {
        download_zip();
        unzip_zip();
        printf("Starter kit is ready. Run ./starterkit --decrypt\n");
        return 0;
    }

    if (strcmp(argv[1], "--decrypt") == 0) {
        pid_t pid = fork();
        if (pid == 0) {
            decrypt_files();
        } else {
            printf("Decryption daemon started with PID %d\n", pid);
        }
    } else if (strcmp(argv[1], "--quarantine") == 0) {
        move_all(STARTER_DIR, QUARANTINE_DIR, "moved to quarantine directory");
    } else if (strcmp(argv[1], "--return") == 0) {
        move_all(QUARANTINE_DIR, STARTER_DIR, "returned to starter_kit directory");
    } else if (strcmp(argv[1], "--eradicate") == 0) {
        delete_all(QUARANTINE_DIR);
    } else if (strcmp(argv[1], "--shutdown") == 0) {
        shutdown();
    } else {
        fprintf(stderr, "Unknown command: %s\n", argv[1]);
        fprintf(stderr, "Usage: ./starterkit [--decrypt | --quarantine | --return | --eradicate | --shutdown]\n");
    }

    return 0;
}
