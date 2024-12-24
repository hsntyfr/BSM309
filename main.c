#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h> // bool tipi için gerekli
#include <sys/types.h> // Veri türleri için : pid_t
#include <sys/wait.h> // WNOHANG için
#include <unistd.h> // fork, execvp gibi POSIX sistem çağrıları için
#include <errno.h> // Hata kontrolü için (errno != ECHILD)
#include <fcntl.h>



#include "main.h"
#define MAX_GIRDI_BOYUTU 256
#define MAX_ARGUMAN_BOYUTU 50

// Fonksiyon bildirimleri
void quit();
void girdiBolucu(char* girdi, char* argv[], int* argc);
void komutlariYazdir(char* argv[], int* argc);
void girdiAl(char girdi[]);
bool kontrol(char* argv[], int argc, char* karakterDizisi);
void tekliKomutYurut(char* argv[], int argc, bool arkaplan);
void komutYurutucu(char* argv[], int argc);
void girisYonlendime(char* argv[], int argc); // Giriş yönlendirmesi kontrolü
void cikisYonlendime(char* argv[], int argc);
void sirali(char* argv[], int argc);
void pipeKomutYurut(char* argv[], int argc);






int main() {
    char girdi[MAX_GIRDI_BOYUTU];
    char* argv[MAX_ARGUMAN_BOYUTU];
    int argc = 0;

    while (1) {
        printf("> ");
        fflush(stdout); // Kullanıcıdan girdi beklemeden önce talimat ekranda görünür.
        girdiAl(girdi);
        // Girdinin sonundaki newline karakterini kaldır
        girdi[strcspn(girdi, "\n")] = '\0';
        argc = 0;
        girdiBolucu(girdi, argv, &argc);

        // Tehlikeli karakter kontrolü
        komutlariYazdir(argv, &argc);

        // "quit" komutu kontrolü
        if (argc > 0 && strcmp(argv[0], "quit") == 0) {
            quit();
        }

        // Komutları yürüt
        komutYurutucu(argv, argc);
        aktifArkaPlanVarMi(); // Arka plandaki işlemleri sürekli kontrol et
    }
}

void quit() {
    // buraya pid kontrolleri gelecek
    quitAktifArkaPlanVarMi(); // Sadece arka planda başlatılmış ve hala çalışan işlemleri kontrol eder.
    printf("Program sonlanıyor...\n");
    exit(0);
}

void girdiBolucu(char* girdi, char* argv[], int* argc) {
    const char* ayraclar = " ";
    char* token = strtok(girdi, ayraclar);

    while (token != NULL) {
        // Tırnak işaretlerini kaldır
        if (token[0] == '\"' && token[strlen(token) - 1] == '\"') {
            token[strlen(token) - 1] = '\0';  // Sağdaki tırnağı kaldır
            token++;                         // Soldaki tırnağı atla
        }

        argv[(*argc)++] = token;
        if (*argc >= MAX_ARGUMAN_BOYUTU) {
            fprintf(stderr, "Argüman sayısı fazla!\n");
            return;
        }
        token = strtok(NULL, ayraclar);
    }
    argv[*argc] = NULL; // Argüman sonlandırma
}


void komutlariYazdir(char* argv[], int* argc) {
    // Parçalanmış komutları yazdırır
    printf("Toplam %d parça bulundu:\n", (*argc));
    for (int i = 0; i < *argc; i++) {
        printf("argv[%d]: %s\n", i, argv[i]);
    }
}

void girdiAl(char girdi[]) {
    // Kullanıcıdan girdi alır
    if (fgets(girdi, MAX_GIRDI_BOYUTU, stdin) == NULL) { // parantez içerisinde girdiyi alıyoruz ve kontrolünü de sağlamış oluyoruz
        perror("Girdi okuma başarısız!");
    }
}

bool kontrol(char* argv[], int argc, char* karakterDizisi) {
    // Argümanlarda tehlikeli karakterlerin varlığını kontrol eder
    for (int i = 0; i < argc; i++) {
        for (int j = 0; j < strlen(karakterDizisi); j++) {
            if (strchr(argv[i], karakterDizisi[j]) != NULL) {
                return true; // Eğer herhangi bir tehlikeli karakter bulunursa true döner
            }
        }
    }
    return false; // Hiçbir tehlikeli karakter bulunmadıysa false döner
}

void tekliKomutYurut(char* argv[], int argc, bool arkaplan){
    printf("tekliKomutYurut içerisindeyiz\n");
    if(arkaplan){
            argv[argc-1]=NULL; // "&" karakterini NULL yapıyoruz çünk onu okuyunca hata veriyor.
    }
    pid_t pid=fork();// Yeni bir proses oluştur "-1" dönmesi başarısız demek
    if (pid == 0) { // Çocuk süreç . Arka plan olmaz.
    // printf("Alt süreç başlatılıyor. PID: %d\n", getpid());
        if (execvp(argv[0], argv) == -1) {
            // execvp başarısız olduysa, hata mesajı yazdır ve çık
            perror("execvp başarısız");
            exit(EXIT_FAILURE);
        }
    }
    else if(pid>0){
        if(arkaplan){
            // Arka plan işlem için mesaj yazdır
            printf("[Arka Plan] PID: %d\n", pid);
        }
        else{
            printf("Ana proses başladı. Alt PID: %d\n", pid);
            waitpid(pid, NULL, 0);  // Alt prosesin bitmesini bekle
            // bir prosesin (özellikle bir alt prosesin) tamamlanmasını beklemek için kullanılan bir sistem çağrısıdır.
            printf("Alt süreç tamamlandı.\n");
        }
    }
    else {
        perror("Fork -1 dönmüş , başarısız\n");
    }
    printf("tekliKomutYurut methodu çıkış.\n");
}
void quitAktifArkaPlanVarMi(){
    int status;
    pid_t pid;
    printf("Arka plandaki işlemler kontrol ediliyor...\n");
    while ((pid = waitpid(-1, &status, 0)) > 0) { // tüm arka plan işlemleri bitene kadar dön
        if (WIFEXITED(status)) { // Prosesin normal bir şekilde (başarıyla veya hatayla) tamamlandığını kontrol eder.
            printf("[PID: %d] Tamamlandı, Çıkış Kodu: %d\n", pid, WEXITSTATUS(status)); // WEXITSTATUS(status) : Prosesin çıkış kodunu döner.
        } else if (WIFSIGNALED(status)) { // Prosesin bir sinyal ile sonlandırılıp sonlandırılmadığını kontrol eder.
            printf("[PID: %d] Sinyalle Sonlandı, Sinyal: %d\n", pid, WTERMSIG(status)); // WTERMSIG(status) : Prosesin sonlanmasına sebep olan sinyalin numarasını döner.
        } else {
        printf("[PID: %d] Tamamlanmamış veya beklenmeyen bir durum\n", pid);
        }
    }
    if (pid == -1 && errno != ECHILD) { // Hata durumu varsa bildir
        perror("Arka plan işlemleri beklenirken bir hata oluştu");
    }

    printf("Tüm arka plan işlemleri tamamlandı.\n");
}
void aktifArkaPlanVarMi(){
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        /*
        * -1 ile mevcut olan tüm alt prosesleri beklemeyi sağlıyoruz ,
        * &status:
          Alt prosesin çıkış durumunu saklamak için bir bellek adresi.
          Bu durum, işlemin neden sonlandığını veya neyle sonuçlandığını anlamamızı sağlar.
        * WNOHANG:
          Bu bayrak, waitpid'in bloklanmasını (beklemesini) önler.
          Eğer beklenen proses henüz tamamlanmadıysa, waitpid hemen geri döner ve programın başka işler yapmasına izin verir.
        */
        if (WIFEXITED(status)) { // Prosesin normal bir şekilde (başarıyla veya hatayla) tamamlandığını kontrol eder.
            printf("[PID: %d] Tamamlandı, Çıkış Kodu: %d\n", pid, WEXITSTATUS(status)); // WEXITSTATUS(status) : Prosesin çıkış kodunu döner.
        } else if (WIFSIGNALED(status)) { // Prosesin bir sinyal ile sonlandırılıp sonlandırılmadığını kontrol eder.
            printf("[PID: %d] Sinyalle Sonlandı, Sinyal: %d\n", pid, WTERMSIG(status)); // WTERMSIG(status) : Prosesin sonlanmasına sebep olan sinyalin numarasını döner.
        } else {
        printf("[PID: %d] Tamamlanmamış veya beklenmeyen bir durum\n", pid);
        }
        
    }
}

void komutYurutucu(char* argv[], int argc) {
    if (kontrol(argv, argc, ";")) {
        sirali(argv, argc); // Eğer komutlar arasında ";" varsa, sırayla işlem yap
    } else if (kontrol(argv, argc, "|")) {
        pipeKomutYurut(argv, argc); // Eğer "|" varsa, pipe işlemini yap
    }
    else if (kontrol(argv, argc, ">")) {
        cikisYonlendime(argv, argc);
    }
    else if (kontrol(argv, argc, "<")) {
        girisYonlendime(argv, argc);
    }
    else if (kontrol(argv, argc, "&")) {
        tekliKomutYurut(argv, argc, true);  // Arka planda çalışacak komut
    } else {
        tekliKomutYurut(argv, argc, false); // Normal komut
    }
}


void cikisYonlendime(char* argv[], int argc) {
    // Yeterli argüman olup olmadığını kontrol et
    if (argc < 3) {
        fprintf(stderr, "Hatalı kullanım!\n");
        return;  // Argüman sayısı yetersiz
    }

    // > ifadesine kadar olan komutları ayıklamak için bir dizi
    char *output[argc];  // Çıktı komutlarını saklamak için dizi
    int outputCount = 0;

    // args dizisini gez, '>' ifadesini bulana kadar
    int i;
    for (i = 0; i < argc; i++) {
        if (strcmp(argv[i], ">") == 0) {
            break;  // '>' ifadesini bulduk, duruyoruz
        }
        output[outputCount] = argv[i];  // Elemanı output dizisine atıyoruz
        outputCount++;  // Sayıyı artırıyoruz
    }

    // '>' ifadesinin ardından gelen dosya ismini alıyoruz
    if (i + 1 < argc) {
        char *outputFile = argv[i + 1];  // Dosya ismi
        printf("Çıktı dosyaya yönlendirilecek: %s\n", outputFile);

        // Yeni bir süreç oluşturuyoruz
        pid_t pid = fork();

        if (pid == -1) {
            perror("Fork hatası");
            return;
        }

        if (pid == 0) {  // Çocuk süreç
            // Çıktıyı dosyaya yönlendirmek için dosyayı açıyoruz
            FILE *file = fopen(outputFile, "w");
            if (file == NULL) {
                perror("Dosya açılamadı");
                exit(1);
            }
            // Standart çıktıyı dosyaya yönlendiriyoruz
            dup2(fileno(file), STDOUT_FILENO);
            fclose(file);

            // Komutları çalıştırmak için execvp kullanıyoruz
            output[outputCount] = NULL;  // execvp ile kullanılacak argümanlar NULL ile bitmelidir
            if (execvp(output[0], output) == -1) {
                perror("Execvp hatası");
                exit(1);
            }
        } else {  // Ana süreç
            // Çocuğun bitmesini bekliyoruz
            wait(NULL);
        }
    } else {
        fprintf(stderr, "Hatalı kullanım! Yönlendirme dosyası belirtilmemiş.\n");
    }
}
void girisYonlendime(char* argv[], int argc) {
    // Yeterli argüman olup olmadığını kontrol et
    if (argc < 3) {
        fprintf(stderr, "Hatalı kullanım!\n");
        return;  // Argüman sayısı yetersiz
    }

    // '<' ifadesine kadar olan komutları ayıklamak için bir dizi
    char *inputFile = NULL;
    int i;

    // args dizisini gez, '<' ifadesini bulana kadar
    for (i = 0; i < argc; i++) {
        if (strcmp(argv[i], "<") == 0) {
            break;  // '<' ifadesini bulduk, duruyoruz
        }
    }

    // '<' ifadesinin ardından gelen dosya ismini alıyoruz
    if (i + 1 < argc) {
        inputFile = argv[i + 1];  // Dosya ismi
        printf("Girdi dosyasından veri alınacak: %s\n", inputFile);

        // Yeni bir süreç oluşturuyoruz
        pid_t pid = fork();

        if (pid == -1) {
            perror("Fork hatası");
            return;
        }

        if (pid == 0) {  // Çocuk süreç
            // Dosyayı okuma modunda açıyoruz
            int fd = open(inputFile, O_RDONLY);
            if (fd == -1) {
                perror("Dosya açılamadı");
                exit(1);
            }

            // Standart girdi (stdin) dosyasını, dosya descriptor'u ile değiştiriyoruz
            if (dup2(fd, STDIN_FILENO) == -1) {
                perror("Girdi yönlendirme hatası");
                close(fd);
                exit(1);
            }
            close(fd);  // fd'yi kapatıyoruz çünkü artık standart girdi ile yönlendirildi

            // Komutları çalıştırmak için execvp kullanıyoruz
            if (execvp(argv[0], argv) == -1) {
                perror("Execvp hatası");
                exit(1);
            }
        } else {  // Ana süreç
            // Çocuğun bitmesini bekliyoruz
            wait(NULL);
        }
    } else {
        fprintf(stderr, "Hatalı kullanım! Girdi dosyası belirtilmemiş.\n");
    }
}


/*void yonlendirme(char* argv[], int argc, char yon){
    if (argc < 3)
    {
        printf("kodun calisabilmesi icin yeterli arguman yok\n");
    }
    
    if (yon == '<')
    {
        printf("giris\n");

    }
    else {
        printf("cikis\n");
    }
}*/

void sirali(char* argv[], int argc) {
    int start = 0;
    for (int i = 0; i <= argc; i++) {
        if (i == argc || strcmp(argv[i], ";") == 0) {
            if (start < i) {
                char* alt_argv[MAX_ARGUMAN_BOYUTU];
                int alt_argc = 0;

                for (int j = start; j < i; j++) {
                    alt_argv[alt_argc++] = argv[j];
                }
                alt_argv[alt_argc] = NULL;

                // Ayrılan komutu tekrar değerlendir
                komutYurutucu(alt_argv, alt_argc);
            }
            start = i + 1;
        }
    }
}





void pipeKomutYurut(char* argv[], int argc) {
    int num_pipes = 0;
    int pipefds[2 * MAX_ARGUMAN_BOYUTU];  // Pipe'lar için gerekli dosya tanıtıcıları
    int command_start = 0;
    
    // Boru (|) sayısını belirle
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "|") == 0) {
            num_pipes++;
        }
    }

    // Gerekli sayıda pipe oluştur
    for (int i = 0; i < num_pipes; i++) {
        if (pipe(pipefds + i * 2) < 0) {
            perror("Pipe oluşturulamadı");
            exit(EXIT_FAILURE);
        }
    }

    // Komutları çalıştır
    for (int i = 0; i <= num_pipes; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // Çocuk süreç (alt proses)

            // İlk komut dışında stdin'i önceki pipe'dan al
            if (i > 0) {
                if (dup2(pipefds[(i - 1) * 2], STDIN_FILENO) < 0) {
                    perror("dup2 stdin başarısız");
                    exit(EXIT_FAILURE);
                }
            }

            // Son komut dışında stdout'u sonraki pipe'a yaz
            if (i < num_pipes) {
                if (dup2(pipefds[i * 2 + 1], STDOUT_FILENO) < 0) {
                    perror("dup2 stdout başarısız");
                    exit(EXIT_FAILURE);
                }
            }

            // Tüm pipe'ları kapat
            for (int j = 0; j < 2 * num_pipes; j++) {
                close(pipefds[j]);
            }

            // Komutun argümanlarını ayıkla
            char* command[MAX_ARGUMAN_BOYUTU];
            int arg_idx = 0;

            // Komutları argümanlara ayır
            for (int k = command_start; k < argc && strcmp(argv[k], "|") != 0; k++) {
                command[arg_idx++] = argv[k];
            }
            command[arg_idx] = NULL;

            // Komutu çalıştır
            if (execvp(command[0], command) < 0) {
                perror("execvp başarısız");
                exit(EXIT_FAILURE);
            }
        } else if (pid < 0) {
            perror("Fork başarısız");
            exit(EXIT_FAILURE);
        }

        // Sonraki komutun başlangıcını bul
        while (command_start < argc && strcmp(argv[command_start], "|") != 0) {
            command_start++;
        }
        command_start++; // Pipe karakterini atla
    }

    // Tüm pipe'ları kapat
    for (int i = 0; i < 2 * num_pipes; i++) {
        close(pipefds[i]);
    }

    // Tüm çocuk süreçlerin bitmesini bekle
    for (int i = 0; i <= num_pipes; i++) {
        wait(NULL);
}
}

