#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#define ZIP_NAME "Clues.zip"
#define CLUES_DIR "Clues"
#define FILTERED_DIR "Filtered"
#define GDRIVE_ID "1xFn1OBJUuSdnApDseEczKhtNzyGekauK"

void download_and_extract();
int is_valid_filename(const char *name);
void traverse_and_filter(const char *base_path);
void filter_files();

void download_and_extract() {
    struct stat st;

    if (stat(CLUES_DIR, &st) == 0 && S_ISDIR(st.st_mode)) {
        printf("Ingpo File e wes onok mas e!!! SKIP!!!.\n");
        return;
    }

    printf("ngedonwload file zip\n");
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "gdown --id %s -O %s", GDRIVE_ID, ZIP_NAME);
    int ret = system(cmd);
    if (ret != 0) {
        printf("Barakallah Tuhan Memberkati\n");
        return;
    }

    printf("Lagi ngeekstrak file zip\n");
    snprintf(cmd, sizeof(cmd), "unzip -q %s -d %s", ZIP_NAME, CLUES_DIR);
    ret = system(cmd);
    if (ret != 0) {
        printf("aku gagal...maafkan diriku\n");
        return;
    }

    remove(ZIP_NAME);
    printf("File Clue udh nongol\n");
}

int is_valid_filename(const char *name) {
    size_t len = strlen(name);
    return (len == 5 && isalnum(name[0]) && strcmp(name + 1, ".txt") == 0);
}

void traverse_and_filter(const char *base_path) {
    DIR *d = opendir(base_path);
    if (!d) return;

    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
            continue;

        char path[512];
        snprintf(path, sizeof(path), "%s/%s", base_path, dir->d_name);

        if (dir->d_type == DT_DIR) {
            traverse_and_filter(path);
        } else if (dir->d_type == DT_REG) {
            if (is_valid_filename(dir->d_name)) {
                mkdir(FILTERED_DIR, 0755);
                char dest[512];
                snprintf(dest, sizeof(dest), "%s/%s", FILTERED_DIR, dir->d_name);
                rename(path, dest);
            } else {
                remove(path);
            }
        }
    }
    closedir(d);
}

void filter_files() {
    traverse_and_filter(CLUES_DIR);
    printf("Ngefilter selesai.\n");
}

int compare_numeric(const void *a, const void *b) {
    int num1 = atoi(*(const char **)a);
    int num2 = atoi(*(const char **)b);
    return num1 - num2;
}

int compare_alpha(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

void combine_files() {
    DIR *d = opendir(FILTERED_DIR);
    if (!d) {
        printf("Ga nemu file filter anjay\n");
        return;
    }

    struct dirent *dir;
    char *num_files[1000];
    char *alpha_files[1000];
    int num_count = 0, alpha_count = 0;

    while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
            continue;

        if (strlen(dir->d_name) == 5 && dir->d_name[1] == '.' && dir->d_name[2] == 't') {
            if (isdigit(dir->d_name[0])) {
                num_files[num_count++] = strdup(dir->d_name);
            } else if (isalpha(dir->d_name[0])) {
                alpha_files[alpha_count++] = strdup(dir->d_name);
            }
        }
    }
    closedir(d);

    qsort(num_files, num_count, sizeof(char *), compare_numeric);
    qsort(alpha_files, alpha_count, sizeof(char *), compare_alpha);

    FILE *out = fopen("Combined.txt", "w");
    if (!out) {
        perror("fopen");
        return;
    }

    int i = 0, j = 0;
    while (i < num_count || j < alpha_count) {
        if (i < num_count) {
            char path[256];
            snprintf(path, sizeof(path), "%s/%s", FILTERED_DIR, num_files[i]);
            FILE *f = fopen(path, "r");
            if (f) {
                int c = fgetc(f);
                if (c != EOF) fputc(c, out);
                fclose(f);
                remove(path);
            }
            free(num_files[i++]);
        }
        if (j < alpha_count) {
            char path[256];
            snprintf(path, sizeof(path), "%s/%s", FILTERED_DIR, alpha_files[j]);
            FILE *f = fopen(path, "r");
            if (f) {
                int c = fgetc(f);
                if (c != EOF) fputc(c, out);
                fclose(f);
                remove(path);
            }
            free(alpha_files[j++]);
        }
    }

    fclose(out);
    printf("file combine udh jadi\n");
}

void rot13(char *str) {
    for (int i = 0; str[i]; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z')
            str[i] = (str[i] - 'A' + 13) % 26 + 'A';
        else if (str[i] >= 'a' && str[i] <= 'z')
            str[i] = (str[i] - 'a' + 13) % 26 + 'a';
    }
}

void decode_file() {
    FILE *in = fopen("Combined.txt", "r");
    if (!in) {
        printf("sek bentar gagal buka combine\n");
        return;
    }

    FILE *out = fopen("Decoded.txt", "w");
    if (!out) {
        printf("njir ga bisa decode\n");
        fclose(in);
        return;
    }

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), in)) {
        rot13(buffer);
        fputs(buffer, out);
    }

    fclose(in);
    fclose(out);
    printf("file decode udh dibuat dari BrainRot13\n");
}

int main(int argc, char *argv[]) {
    if (argc == 1) {

        download_and_extract();
    } else if (argc == 3 && strcmp(argv[1], "-m") == 0) {
        if (strcmp(argv[2], "Filter") == 0) {
           
            filter_files();
        } else if (strcmp(argv[2], "Combine") == 0) {
  
            combine_files();
        } else if (strcmp(argv[2], "Decode") == 0) {

            decode_file();
        } else {
            printf("Usage:\n");
            printf("  ./action               # donwload & extract zip\n");
            printf("  ./action -m Filter     # filter itunya\n");
            printf("  ./action -m Combine    # ngegabung isi filter ke combine\n");
            printf("  ./action -m Decode     # ngedecode pake brainROT13\n");
        }
    } else {
        printf("Usage:\n");
        printf("  ./action               # untuk download & ekstrak zip\n");
        printf("  ./action -m Filter     # filter itunya\n");
        printf("  ./action -m Combine    # ngegabung isi filter ke combine\n");
        printf("  ./action -m Decode    # ngedecode pake brainROT13\n");
    }

    return 0;
}

