void girdiBolucu(char* girdi, char* argv[], int* argc);
//void girdiAl();
void quit();
void aktifArkaPlanVarMi();
void quitAktifArkaPlanVarMi();
void komutYurutucu(char* argv[], int argc);  // Komutları çalıştırmak için
void sirali(char* argv[], int argc);         // Komutları sıralamak ve işlemi kontrol etmek için
bool kontrol(char* argv[], int argc, char* karakterDizisi); // Tehlikeli karakterleri kontrol etmek için
void girisYonlendime(char* argv[], int argc); // Giriş yönlendirmesi kontrolü
void cikisYonlendime(char* argv[], int argc); // Çıkış yönlendirmesi kontrolü
void yonlendirme(char* argv[], int argc, char yon); // Yönlendirme işlemleri (giriş/çıkış)
void komutlariYazdir(char* argv[], int* argc);
void girdiAl(char girdi[]);
bool kontrol(char* argv[], int argc, char* karakterDizisi);
void tekliKomutYurut(char* argv[], int argc, bool arkaplan);
void pipeKomutYurut(char* argv[], int argc);
