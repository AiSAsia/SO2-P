#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "socketutil.h"

volatile bool running = true;

void *listenAndPrint(void *arg);
void readConsoleEntriesAndSendToServer(int socketFD);

int main() {
    int socketFD = createTCPIpv4Socket();
    struct sockaddr_in *address = createIPv4Address("127.0.0.1", 2000);

    int result = connect(socketFD, address, sizeof(*address));
    if (result == 0) {
        printf("Nawiazano polaczenie z serwerem\n");
    } else {
        perror("Nie udalo sie nawiazac polaczenia");
        free(address);
        return 1;
    }

    pthread_t listenerThread;
    if (pthread_create(&listenerThread, NULL, listenAndPrint, (void *)(intptr_t)socketFD) != 0) {
        perror("Nie udalo sie utworzyc watku nasluchujacego");
        close(socketFD);
        free(address);
        return 1;
    }

    readConsoleEntriesAndSendToServer(socketFD);
    running = false;
    pthread_join(listenerThread, NULL); // Czekaj na zakończenie wątku nasłuchującego
    printf("Koniec Programu\n");
    close(socketFD);
    free(address);

    return 0;
}

void *listenAndPrint(void *arg) {
    int socketFD = (int)(intptr_t)arg;
    char buffer[1024];

    while (running) {
        ssize_t amountReceived = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
        if (amountReceived > 0) {
            buffer[amountReceived] = 0;
            printf("Otrzymano: %s\n", buffer);
        } else if (amountReceived == 0) {
            printf("Połączenie zostało zamknięte przez serwer.\n");
            break;
        } else {
            perror("Błąd podczas odbierania danych");
            break;
        }
    }

    close(socketFD);
    return NULL;
}

void readConsoleEntriesAndSendToServer(int socketFD) {
    char *name = NULL;
    size_t nameSize = 0;
    printf("Podaj swoj nick:\n");
    ssize_t nameCount = getline(&name, &nameSize, stdin);
    if (nameCount > 0) {
        name[nameCount - 1] = 0; // Usunięcie znaku nowej linii
    }

    char *line = NULL;
    size_t lineSize = 0;
    printf("Możesz już pisać wiadomości (napisz 'exit' by wyjść)...\n");

    char buffer[1024];

    while (true) {
        ssize_t charCount = getline(&line, &lineSize, stdin);
        if (charCount > 0) {
            if (line[charCount - 1] == '\n') {
                line[charCount - 1] = 0; // Usunięcie znaku nowej linii
            }

            if (strcmp(line, "exit") == 0) {
                running = false;
                break;
            }

            sprintf(buffer, "%s: %s", name, line);
            send(socketFD, buffer, strlen(buffer), 0);
        }
    }

    free(name);
    free(line);
}
