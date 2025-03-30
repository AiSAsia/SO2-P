#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <vector>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <signal.h>

using namespace std;

volatile bool running = true; // Flaga kontrolująca działanie programu
pthread_mutex_t coutMutex = PTHREAD_MUTEX_INITIALIZER; // Mutex dla wyjścia

void handleSignal(int) {
    running = false;
}

// Funkcja bezpiecznego wypisywania do konsoli
void safePrint(const string& message) {
    pthread_mutex_lock(&coutMutex);
    cout << message << endl;
    pthread_mutex_unlock(&coutMutex);
}

class Monitor {
private:
    vector<int> stan; // Stan filozofów (0 - jedzący, 1 - głodny, 2 - myślący)
    vector<pthread_cond_t> warunki; // Zmienne warunkowe dla filozofów
    int liczbaFilozofow; // Liczba filozofów
    pthread_mutex_t blokada; // Mutex do synchronizacji

public:
    Monitor(int n) : stan(n, 2), warunki(n), liczbaFilozofow(n) {
        pthread_mutex_init(&blokada, NULL);
        for (int i = 0; i < n; i++) {
            pthread_cond_init(&warunki[i], NULL);
        }
    }

    ~Monitor() {
        for (int i = 0; i < liczbaFilozofow; i++) {
            pthread_cond_destroy(&warunki[i]);
        }
        pthread_mutex_destroy(&blokada);
    }

    void test(int numerFilozofa) {
        if (stan[(numerFilozofa + 1) % liczbaFilozofow] != 0 &&
            stan[(numerFilozofa - 1 + liczbaFilozofow) % liczbaFilozofow] != 0 &&
            stan[numerFilozofa] == 1) {
            stan[numerFilozofa] = 0;
            printWithTime("Filozof " + to_string(numerFilozofa + 1) + " je.");
            pthread_cond_signal(&warunki[numerFilozofa]);
        }
    }

    void wez_widelce(int numerFilozofa) {
        pthread_mutex_lock(&blokada);
        stan[numerFilozofa] = 1;
        test(numerFilozofa);
        while (stan[numerFilozofa] != 0) {
            pthread_cond_wait(&warunki[numerFilozofa], &blokada);
        }
        pthread_mutex_unlock(&blokada);
    }

    void odloz_widelce(int numerFilozofa) {
        pthread_mutex_lock(&blokada);
        stan[numerFilozofa] = 2;
        test((numerFilozofa + 1) % liczbaFilozofow);
        test((numerFilozofa - 1 + liczbaFilozofow) % liczbaFilozofow);
        pthread_mutex_unlock(&blokada);
    }

    void printWithTime(const string& message) {
        auto now = chrono::system_clock::now();
        auto time = chrono::system_clock::to_time_t(now);
        stringstream ss;
        ss << "[" << put_time(localtime(&time), "%H:%M:%S") << "] " << message;
        safePrint(ss.str());
    }
};

struct FilozofData {
    int id;
    Monitor* monitor;
};

void* filozof(void* arg) {
    FilozofData* data = (FilozofData*)arg;
    int id = data->id;
    Monitor* monitor = data->monitor;

    while (running) {
        monitor->printWithTime("Filozof " + to_string(id + 1) + " myśli...");
        sleep(1);
        monitor->wez_widelce(id);
        sleep(1);
        monitor->odloz_widelce(id);
    }
    return nullptr;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "Użycie: " << argv[0] << " <liczba_filozofow>" << endl;
        return 1;
    }

    signal(SIGINT, handleSignal); // Obsługa sygnału Ctrl+C

    int liczbaFilozofow = atoi(argv[1]);
    Monitor monitor(liczbaFilozofow);
    vector<pthread_t> watki(liczbaFilozofow);
    vector<FilozofData> filozofowie(liczbaFilozofow);

    for (int i = 0; i < liczbaFilozofow; i++) {
        filozofowie[i] = {i, &monitor};
        pthread_create(&watki[i], NULL, filozof, &filozofowie[i]);
    }

    for (int i = 0; i < liczbaFilozofow; i++) {
        pthread_join(watki[i], NULL);
    }

    return 0;
}
