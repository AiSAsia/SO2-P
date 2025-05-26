# Kompilator
CC = gcc
# Flagi kompilacji
CFLAGS = -Wall -Wextra -pthread -std=gnu11
# Flagi dla suppressowania konkretnych ostrzeżeń
SUPPRESS_WARNINGS = -Wno-pointer-sign -Wno-incompatible-pointer-types -Wno-cast-function-type
# Katalogi źródłowe
CLIENT_DIR = client
SERVER_DIR = server
# Pliki wykonywalne
CLIENT_EXEC = chat_client
SERVER_EXEC = chat_server

all: client server

client:
	$(CC) $(CFLAGS) $(SUPPRESS_WARNINGS) $(CLIENT_DIR)/main.c -o $(CLIENT_EXEC)

server:
	$(CC) $(CFLAGS) $(SUPPRESS_WARNINGS) $(SERVER_DIR)/main.c -o $(SERVER_EXEC)

clean:
	rm -f $(CLIENT_EXEC) $(SERVER_EXEC)

.PHONY: all client server clean
