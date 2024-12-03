# Compilatore
CC = gcc

# Flag di compilazione 
CFLAGS = -Wall -pedantic -pthread -g -O0

# Directories
HEADER = ../Header/
SORGENTE = ../Sorgente/

# Eseguibili
SERVER = Server
CLIENT = Client
TEST = Test
LISTA = Lista
TRIE = Trie
MATRICE = Matrice
COMMS = Comunicazione

# Header
HDRS_COMUNICAZIONE = $(HEADER)Comunicazione.h
HDRS_TRIE = $(HEADER)$(TRIE).h
HDRS_MATRICE = $(HEADER)$(MATRICE).h
HDRS_LISTA = $(HEADER)$(LISTA).h
SERVER_HEADERS = $(HDRS_MATRICE) $(HDRS_TRIE) $(HDRS_LISTA) $(HDRS_COMUNICAZIONE)
CLIENT_HEADERS = $(HDRS_MATRICE) $(HDRS_TRIE) $(HDRS_LISTA) $(HDRS_COMUNICAZIONE)
TEST_HEADERS = $(HDRS_MATRICE) $(HDRS_TRIE) $(HDRS_LISTA) $(HDRS_COMUNICAZIONE)

# File Oggetto
OBJS = $(SERVER).o
OBJC = $(CLIENT).o
OBJHM = $(MATRICE).o
OBJHT = $(TRIE).o
OBJHL = $(LISTA).o
OBJHCS = $(COMMS).o
OBJT = $(TEST).o
SERVER_OBJS = $(OBJHM) $(OBJHT) $(OBJHL) $(OBJHCS)
CLIENT_OBJS = $(OBJHM) $(OBJHT) $(OBJHL) $(OBJHCS)
TEST_OBJS = $(OBHM) $(OBJHL)

# Sorgenti
SRCS = $(SORGENTE)$(SERVER).c
SRCC = $(SORGENTE)$(CLIENT).c
SRCHM = $(SORGENTE)$(MATRICE).c
SRCHT = $(SORGENTE)$(TRIE).c
SRCHCS = $(SORGENTE)$(COMMS).c
SRCHL = $(SORGENTE)$(LISTA).c
SRCT = $(SORGENTE)$(TEST).c

# Input
PORT = 20000
PORT2 = 30000
HOST = 127.0.0.1

# Comandi di esecuzione
SERVER_RUN = ./$(SERVER) $(HOST) $(PORT) --matrici ../Text/Matrici.txt --diz ../Text/Dizionario.txt 
CLIENT_RUN = ./$(CLIENT) $(HOST) $(PORT)
SERVER_RUN2 = ./$(SERVER) $(HOST) 3000 --matrici ../Text/Matrici.txt --diz ../Text/Dizionario.txt 
CLIENT_RUN2 = ./$(CLIENT) $(HOST) 3000
TEST_RUN = ./$(TEST)

# Target principali
all: $(SERVER) $(CLIENT) $(TEST) clean_objs

$(SERVER): $(SERVER_OBJS)
	@echo Compilazione del Server
	@$(CC) $(CFLAGS) $(SRCS) $(SRCHCS) $(SRCHM) $(SRCHL) $(SRCHT) -o $(SERVER)

$(CLIENT): $(CLIENT_OBJS)
	@echo Compilazione del Client
	@$(CC) $(CFLAGS)  $(SRCC) $(SRCHCS) $(SRCHM) $(SRCHL) $(SRCHT) $(CLIENT_HEADERS) -o $(CLIENT)

$(TEST): $(TEST_OBJS)
	@echo Compilazione dei Test
	@$(CC) $(CFLAGS) $(SRCT) $(SRCHCS) $(SRCHM) $(SRCHL) $(SRCHT) $(TEST_HEADERS) $(TEST_HEADERS) -o $(TEST)



$(OBJHM):$(SRCHM)
	@$(CC) $(CFLAGS) -c $(SRCHM) $(HDRS_MATRICE)

$(OBJHCS):$(SRCHCS)
	@$(CC) $(CFLAGS) -c $(SRCHCS) $(HDRS_COMUNICAZIONE)

$(OBJHL):$(SRCHL)
	@$(CC) $(CFLAGS) -c $(SRCHL) $(HDRS_LISTA)

#Server.o
$(OBJS):$(SRCS)
	@echo compilo 
	@$(CC) $(CFLAGS) -c $(SRCS) $(SRCHCS) $(SRCHM) $(SRCHL) $(SRCHT) $(SERVER_HEADERS)

#Client.o
$(OBJC):$(SRCC)
	@$(CC) $(CFLAGS) -c $(SRCC) $(CLIENT_HEADERS)
#Test.o
$(OBJT):$(SRCT)
	@$(CC) $(CFLAGS) -c $(SRCT) 

$(OBJHT):$(SRCHT)
	@$(CC) $(CFLAGS) -c $(SRCHT) $(HDRS_TRIE)

# Pulizia
clean_objs:
	@rm -f *.o
	@rm -f ../Header/*.gch
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

run-server2:
	@echo Esecuzione server sulla porta $(3000)
	@$(SERVER_RUN2)

run-client2:
	@echo Esecuzione client sulla porta $(3000)
	@$(CLIENT_RUN2)

run-test:
	@echo Esecuzione dei test
	@$(TEST_RUN)

.PHONY: all clean_objs clean_all run-server run-client run-test