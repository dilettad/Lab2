# Compilatore
CC = gcc

# Flag di compilazione 
CFLAGS = -Wall -pedantic -pthread -g -O0

# Directories
HEADER = ../Header/
SORGENTE = ../Sorgente/

# Eseguibili
SERVER = server
CLIENT = client

# File Oggetto
OBJS = $(SERVER).o
OBJC = $(CLIENT).o
OBJECT_FILES = $(OBJC) $(OBJS)

# Sorgenti
SRCS = $(SORGENTE)$(SERVER).c
SRCC = $(SORGENTE)$(CLIENT).c

# Input
PORT = 20000
PORT2 = 30000
PORT3 = 40000
HOST = 127.0.0.1

# Test
SERVER_RUN = ./$(SERVER) $(HOST) $(PORT) --matrici ../Text/Matrici.txt --diz ../Text/Dizionario.txt 
SERVER_RUN2 = ./$(SERVER) $(HOST) $(PORT2) --durata 2 --seed 500
CLIENT_RUN = ./$(CLIENT) $(HOST) $(PORT)
CLIENT_RUN2 = ./$(CLIENT) $(HOST) $(PORT2)

# Target
all: $(SERVER) $(CLIENT) obj_clean other_clean

# Dipendenze file Eseguibili
$(SERVER): $(OBJS)
	@echo Compilazione del Server
	@$(CC) $(CFLAGS) $(OBJS) -o $(SERVER)

$(CLIENT): $(OBJC)
	@echo Compilazione del Client
	@$(CC) $(CFLAGS) $(OBJC) -o $(CLIENT)

# Dipendenze File Oggetto
# Server.o
$(OBJS): $(SRCS)
	@$(CC) $(CFLAGS) -c $(SRCS)

# Client.o
$(OBJC): $(SRCC)
	@$(CC) $(CFLAGS) -c $(SRCC)

obj_clean:
	@rm -f $(OBJECT_FILES)
	@echo File oggetto rimossi

other_clean:
	@rm -f $(HEADER_FILES)

clear: 
	@rm -f $(OBJECT_FILES) $(SERVER) $(CLIENT)

run-server:
	@echo Esecuzione server sulla porta $(PORT)
	@$(SERVER_RUN)

run-server2:
	@echo Esecuzione server sulla porta $(PORT2)
	@$(SERVER_RUN2)

run-scorer:
	@echo Test scorer sulla porta $(PORT3)
	@$(SCORER_RUN)

test-scorer:
	@echo Client scorer in esecuzione
	@$(SCORER_CLIENT)

test-client:
	@echo Client online sulla porta $(PORT)
	@$(CLIENT_RUN)

test-client2:
	@echo Client online sulla porta $(PORT2)
	@$(CLIENT_RUN2)

.PHONY: all obj_clean other_clean clear run-server run-server2 run-scorer test-scorer test-client test-client2