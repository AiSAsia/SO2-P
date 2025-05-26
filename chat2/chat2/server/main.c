#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "socketutil.h"

#define MAX_CLIENTS 10
#define MAX_HISTORY 100
#define BUFFER_SIZE 1024

struct AcceptedSocket {
    int acceptedSocketFD;
    struct sockaddr_in address;
    int error;
    bool acceptedSuccessfully;
};

// Struktura do przekazywania argumentów do wątku
struct ThreadArgs {
    int socketFD;
    struct ServerState* state;
};

// Struktura zarządzająca stanem serwera
struct ServerState {
    struct AcceptedSocket acceptedSockets[MAX_CLIENTS];
    int acceptedSocketsCount;
    bool serverRunning;
    pthread_mutex_t mutex;
    char messageHistory[MAX_HISTORY][BUFFER_SIZE]; // Historia wiadomości
    int historyCount; // Liczba wiadomości w historii
};

// Inicjalizacja stanu serwera
void initServerState(struct ServerState* state) {
    state->acceptedSocketsCount = 0;
    state->serverRunning = true;
    state->historyCount = 0; // Inicjalizuj licznik historii
    pthread_mutex_init(&state->mutex, NULL);
}

// Niszczenie stanu serwera
void destroyServerState(struct ServerState* state) {
    pthread_mutex_destroy(&state->mutex);
}

struct AcceptedSocket *acceptIncomingConnection(int serverSocketFD, struct ServerState* state);
void startAcceptingIncomingConnections(int serverSocketFD, struct ServerState* state);
void receiveAndPrintIncomingDataOnSeparateThread(struct AcceptedSocket *pSocket, struct ServerState* state);
void sendReceivedMessageToTheOtherClients(char *buffer, int socketFD, struct ServerState* state);
void receiveAndPrintIncomingData(struct ThreadArgs* args);
void cleanUpClosedSockets(struct ServerState* state);
void* cleanUpThread(void* arg);
void sendHistoryToClient(int socketFD, struct ServerState* state);

void startAcceptingIncomingConnections(int serverSocketFD, struct ServerState* state) {
    while (true) {
        // Sprawdź czy serwer powinien nadal działać
        pthread_mutex_lock(&state->mutex);
        bool running = state->serverRunning;
        pthread_mutex_unlock(&state->mutex);

        if (!running) break;

        struct AcceptedSocket* clientSocket = acceptIncomingConnection(serverSocketFD, state);
        if (clientSocket->acceptedSuccessfully) {
            pthread_mutex_lock(&state->mutex);

            if (state->acceptedSocketsCount < MAX_CLIENTS) {
                state->acceptedSockets[state->acceptedSocketsCount++] = *clientSocket;
                pthread_mutex_unlock(&state->mutex);
                receiveAndPrintIncomingDataOnSeparateThread(clientSocket, state);
            } else {
                pthread_mutex_unlock(&state->mutex);
                close(clientSocket->acceptedSocketFD);
                printf("Osiagnieto maksymalna liczbe polaczen\n");
            }
        }
        free(clientSocket);
    }
}

void receiveAndPrintIncomingDataOnSeparateThread(struct AcceptedSocket *pSocket, struct ServerState* state) {
    pthread_t id;
    struct ThreadArgs *args = malloc(sizeof(struct ThreadArgs));

    args->socketFD = pSocket->acceptedSocketFD;
    args->state = state;

    pthread_create(&id, NULL, (void* (*)(void*))receiveAndPrintIncomingData, args);
    pthread_detach(id);
}

void receiveAndPrintIncomingData(struct ThreadArgs* args) {
    int socketFD = args->socketFD;
    struct ServerState* state = args->state;
    free(args); // Zwolnienie pamięci po argumencie

    char buffer[BUFFER_SIZE];

    while (true) {
        ssize_t amountReceived = recv(socketFD, buffer, BUFFER_SIZE, 0);
        if (amountReceived > 0) {
            buffer[amountReceived] = 0;
            printf("%s\n", buffer);
            sendReceivedMessageToTheOtherClients(buffer, socketFD, state);
        }
        if (amountReceived <= 0) {
            // Zamknięcie połączenia
            pthread_mutex_lock(&state->mutex);
            for (int i = 0; i < state->acceptedSocketsCount; i++) {
                if (state->acceptedSockets[i].acceptedSocketFD == socketFD) {
                    state->acceptedSockets[i].acceptedSuccessfully = false;
                    break;
                }
            }
            pthread_mutex_unlock(&state->mutex);
            close(socketFD);
            break;
        }
    }
}

void sendReceivedMessageToTheOtherClients(char *buffer, int socketFD, struct ServerState* state) {
    pthread_mutex_lock(&state->mutex);

    // Zapisz wiadomość w historii
        if (state->historyCount < MAX_HISTORY) {
        strncpy(state->messageHistory[state->historyCount++], buffer, BUFFER_SIZE);
    }

    for (int i = 0; i < state->acceptedSocketsCount; i++) {
        if (state->acceptedSockets[i].acceptedSocketFD != socketFD &&
            state->acceptedSockets[i].acceptedSuccessfully) {

            if (send(state->acceptedSockets[i].acceptedSocketFD, buffer, strlen(buffer), 0) < 0) {
                // Błąd wysyłania - oznacz połączenie jako nieaktywne
                state->acceptedSockets[i].acceptedSuccessfully = false;
                close(state->acceptedSockets[i].acceptedSocketFD);
            }
        }
    }

    pthread_mutex_unlock(&state->mutex);
}

void sendHistoryToClient(int socketFD, struct ServerState* state) {
    pthread_mutex_lock(&state->mutex);
    for (int i = 0; i < state->historyCount; i++) {
        send(socketFD, state->messageHistory[i], strlen(state->messageHistory[i]), 0);
    }
    pthread_mutex_unlock(&state->mutex);
}

struct AcceptedSocket *acceptIncomingConnection(int serverSocketFD, struct ServerState* state) {
    struct sockaddr_in clientAddress;
    int clientAddressSize = sizeof(struct sockaddr_in);
    int clientSocketFD = accept(serverSocketFD, (struct sockaddr *)&clientAddress, &clientAddressSize);

    struct AcceptedSocket* acceptedSocket = malloc(sizeof(struct AcceptedSocket));
    acceptedSocket->address = clientAddress;
    acceptedSocket->acceptedSocketFD = clientSocketFD;
    acceptedSocket->acceptedSuccessfully = clientSocketFD > 0;

    if (!acceptedSocket->acceptedSuccessfully) {
        acceptedSocket->error = clientSocketFD;
    } else {
        // Wyślij historię do nowego klienta
        sendHistoryToClient(clientSocketFD, state);
    }

    return acceptedSocket;
}

void cleanUpClosedSockets(struct ServerState* state) {
    pthread_mutex_lock(&state->mutex);

    int newCount = 0;
    for (int i = 0; i < state->acceptedSocketsCount; i++) {
        if (state->acceptedSockets[i].acceptedSuccessfully) {
            state->acceptedSockets[newCount++] = state->acceptedSockets[i];
        }
    }
    state->acceptedSocketsCount = newCount;

    pthread_mutex_unlock(&state->mutex);
}

void* cleanUpThread(void* arg) {
    struct ServerState* state = (struct ServerState*)arg;

    while (true) {
        sleep(5); // Czyszczenie co 5 sekund

        // Sprawdź czy serwer nadal działa
        pthread_mutex_lock(&state->mutex);
        bool running = state->serverRunning;
        pthread_mutex_unlock(&state->mutex);

        if (!running) break;

        cleanUpClosedSockets(state);
    }
    return NULL;
}

int main() {
    struct ServerState state;
    initServerState(&state);

    int serverSocketFD = createTCPIpv4Socket();
    struct sockaddr_in *serverAddress = createIPv4Address("", 2000);

    int result = bind(serverSocketFD, serverAddress, sizeof(*serverAddress));
    if (result == 0)
        printf("Nawiazano polaczenie\n");

    int listenResult = listen(serverSocketFD, 10);
    if (listenResult < 0) {
        perror("Listen failed");
        free(serverAddress);
        destroyServerState(&state);
        return 1;
    }

    // Uruchom wątek czyszczący
    pthread_t cleanerId;
    pthread_create(&cleanerId, NULL, cleanUpThread, &state);

    // Uruchom główną pętlę serwera
    startAcceptingIncomingConnections(serverSocketFD, &state);

    // Zatrzymanie serwera
    pthread_mutex_lock(&state.mutex);
    state.serverRunning = false;
    pthread_mutex_unlock(&state.mutex);

    // Zamknięcie wszystkich aktywnych połączeń
    pthread_mutex_lock(&state.mutex);
    for (int i = 0; i < state.acceptedSocketsCount; i++) {
        if (state.acceptedSockets[i].acceptedSuccessfully) {
            close(state.acceptedSockets[i].acceptedSocketFD);
        }
    }
    pthread_mutex_unlock(&state.mutex);

    // Oczekiwanie na zakończenie wątku czyszczącego
    pthread_join(cleanerId, NULL);

    shutdown(serverSocketFD, SHUT_RDWR);
    free(serverAddress);
    destroyServerState(&state);

    return 0;
}

