CC = gcc          # Derleyici
CFLAGS = -Wall    # Derleme bayrakları
TARGET = main     # Çıktı dosyasının adı

all: $(TARGET)
	@./$(TARGET)   # Programı çalıştırırken komutu gizle

$(TARGET): main.c main.h
	@$(CC) $(CFLAGS) -o $(TARGET) main.c  # Derleme komutunu gizle

clean:
	@rm -f $(TARGET)  # Temizleme komutunu gizle
