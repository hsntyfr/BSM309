void girdiBolucu(char* girdi, char* argv[], int* argc);
void komutlariYazdir(char**, int*);
void girdiAl();
void quit();
void aktifArkaPlanVarMi();
void quitAktifArkaPlanVarMi();
void pipeKomutYurut(char* argv[], int argc); // Pipe işlemi yapan fonksiyon
void komutYurutucu(char* argv[], int argc);  // Komutları çalıştırmak için
void sirali(char* argv[], int argc);         // Komutları sıralamak ve işlemi kontrol etmek için
bool kontrol(char* argv[], int argc, char* karakterDizisi); // Tehlikeli karakterleri kontrol etmek için
bool girisYonlendime(char* argv[], int argc); // Giriş yönlendirmesi kontrolü
bool cikisYonlendime(char* argv[], int argc); // Çıkış yönlendirmesi kontrolü
void yonlendirme(char* argv[], int argc, char yon); // Yönlendirme işlemleri (giriş/çıkış)
