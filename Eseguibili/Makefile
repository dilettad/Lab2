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
TEST = test

# Header
HDRS_COMUNICAZIONE = $(HEADER)comunicazione.h
HDRS_TRIE = $(HEADER)Trie.h
HDRS_MATRICE = $(HEADER)Matrice.h

# File Oggetto
OBJS_SERVER = $(SERVER).o comunicazione.o Trie.o
OBJS_CLIENT = $(CLIENT).o comunicazione.o
OBJS_TEST = $(TEST).o Matrice.o

# Sorgenti
SRCS_SERVER = $(SORGENTE)$(SERVER).c $(SORGENTE)comunicazione.c $(SORGENTE)Trie.c
SRCS_CLIENT = $(SORGENTE)$(CLIENT).c $(SORGENTE)comunicazione.c
SRCS_TEST = $(SORGENTE)$(TEST).c $(SORGENTE)Matrice.c

# Input
PORT = 20000
PORT2 = 30000
HOST = 127.0.0.1

# Comandi di esecuzione
SERVER_RUN = ./$(SERVER) $(HOST) $(PORT) --matrici ../Text/Matrici.txt --diz ../Text/Dizionario.txt 
CLIENT_RUN = ./$(CLIENT) $(HOST) $(PORT)
TEST_RUN = ./$(TEST)

# Target principali
all: $(SERVER) $(CLIENT) $(TEST) clean_objs

$(SERVER): $(OBJS_SERVER)
	@echo Compilazione del Server
	$(CC) $(CFLAGS) $^ -o $@

$(CLIENT): $(OBJS_CLIENT)
	@echo Compilazione del Client
	$(CC) $(CFLAGS) $^ -o $@

$(TEST): $(OBJS_TEST)
	@echo Compilazione dei Test
	$(CC) $(CFLAGS) $^ -o $@

# Regole generiche per file oggetto
%.o: $(SORGENTE)%.c $(HDRS_COMUNICAZIONE) $(HDRS_TRIE) $(HDRS_MATRICE)
	$(CC) $(CFLAGS) -c $< -o $@

# Pulizia
clean_objs:
	@rm -f *.o
	@echo File oggetto rimossi

clean_all: clean_objs
	@rm -f $(SERVER) $(CLIENT) $(TEST)
	@echo Tutti gli eseguibili rimossi

# Comandi di esecuzione
run-server:
	@echo Esecuzione server sulla porta $(PORT)
	@$(SERVER_RUN)

run-client:
	@echo Esecuzione client sulla porta $(PORT)
	@$(CLIENT_RUN)

run-test:
	@echo Esecuzione dei test
	@$(TEST_RUN)

.PHONY: all clean_objs clean_all run-server run-client run-test
