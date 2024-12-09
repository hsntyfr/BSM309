#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h> // bool tipi için gerekli

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
        fflush(stdout);
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
    }
}

void quit() {
    // buraya pid kontrolleri gelecek
    exit(0);
}

void girdiBolucu(char* girdi, char* argv[], int* argc) {
    // Konsola girilen komutları boşluk karakterine göre parçalayıp bir dizi halinde döndürür
    const char* ayraclar = " ";
    char* token = strtok(girdi, ayraclar);

    while (token != NULL) {
        argv[(*argc)++] = token;
        if (*argc >= MAX_ARGUMAN_BOYUTU) {
            fprintf(stderr, "Argüman sayısı fazla!\n");
            return;
        }
        token = strtok(NULL, ayraclar);
    }
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
    if (fgets(girdi, MAX_GIRDI_BOYUTU, stdin) == NULL) {
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
    if (arkaplan) {
        //burada doğrudan komut yürütülecek argüman listesi hazır
        printf("Komut arka planda çalışacak: ");
    } else {
        //arkaplanda çalışması gereken komut burada çalışıyor
        printf("Komut normal çalışacak: ");
    }

    for (int i = 0; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");
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