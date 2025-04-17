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

