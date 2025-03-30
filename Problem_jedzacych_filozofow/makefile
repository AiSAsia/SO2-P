# Kompilator i flagi
CXX = g++
CXXFLAGS = -pthread -std=c++11 -Wall -Wextra

# Nazwa programu
TARGET = filozofowie

# Pliki źródłowe
SRCS = filoczas.cpp

# Domyślna liczba filozofów
NUM_FILOSOFOW ?= 5

# Kompilacja programu
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Uruchomienie programu z dynamiczną liczbą filozofów
run: $(TARGET)
	./$(TARGET) $(NUM_FILOSOFOW)

# Czyszczenie plików binarnych
clean:
	rm -f $(TARGET)

# Phony targets dla reguł "all", "run" i "clean"
.PHONY: all run clean
