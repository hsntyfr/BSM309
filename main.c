#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h> // bool tipi için gerekli
#include <sys/types.h> // Veri türleri için : pid_t
#include <sys/wait.h> // WNOHANG için
#include <unistd.h> // fork, execvp gibi POSIX sistem çağrıları için
#include <errno.h> // Hata kontrolü için (errno != ECHILD)


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
bool girisYonlendime(char* argv[], int argc);
bool cikisYonlendime(char* argv[], int argc);
void yonlendirme(char* argv[], int argc, char yon);
void sirali(char* argv[], int argc);




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
    // Konsola girilen komutları boşluk karakterine göre parçalayıp bir dizi halinde döndürür
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
    argv[*argc] = NULL; 
    /*
    execvp çağrısının doğru çalışması için, argümanların sonunda NULL olması gerektiğinden emin olun. Bu, girdiBolucu fonksiyonunda zaten belirtilmiş.
    */
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
    //bu metot eğer girdi çıktı vb varsa onları değerlendirmek için başka yere gidecek
    if (kontrol(argv, argc, ";|<>")) {
        printf("Ek işlem gerekti.\n");
        if (girisYonlendime(argv, argc)){
            yonlendirme(argv, argc, '<');
        }
        else if (cikisYonlendime(argv, argc))
        {
            yonlendirme(argv, argc, '>');
        }
        else {
            sirali(argv, argc);
        }
    } 
    else {
        if (kontrol(argv, argc, "&")) {
            tekliKomutYurut(argv, argc, true);  // Arka planda çalışacak komut
        } else {
            tekliKomutYurut(argv, argc, false); // Normal komut
        }
    }
}

bool girisYonlendime(char* argv[], int argc){
    for (int i = 0; i < argc; i++) {
        if (strchr(argv[i], '<') != NULL) {
            //printf("giriss\n");

            return true; // Eğer herhangi bir tehlikeli karakter bulunursa true döner
        }
    }
    return false;
}
bool cikisYonlendime(char* argv[], int argc){
    for (int i = 0; i < argc; i++) {
        if (strchr(argv[i], '>') != NULL) {
            //printf("cikis\n");
            return true; // Eğer herhangi bir tehlikeli karakter bulunursa true döner
        }
    }
        return false;

}

void yonlendirme(char* argv[], int argc, char yon){
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
}

void sirali(char* argv[], int argc){
    bool baglantili = false;
    for (int i = 0; i < argc; i++) {
        if (strchr(argv[i], '|') != NULL) {
            //printf("giris\n");

            baglantili = true;
            break;
        }
    }
    if (baglantili == true)
    {
        
        printf("| var\n");
    }
    else
    {
        //bosluk gorene kadar veya ; görene kadar tek argv ye al
        for (int i = 0; i < argc; i++) {
            char* noktalivirgul;
            while ((noktalivirgul = strchr(argv[i], ';')) != NULL) {
                memmove(noktalivirgul, noktalivirgul + 1, strlen(noktalivirgul));
            }
        }
        komutYurutucu(argv, argc);

        printf("; var\n");
        komutlariYazdir(argv, &argc);

    }
    
    

}
