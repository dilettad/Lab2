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
GIOCATORE = Giocatore
BACHECA = Bacheca
FILELOG = FileLog

# Header
HDRS_COMUNICAZIONE = $(HEADER)Comunicazione.h
HDRS_TRIE = $(HEADER)$(TRIE).h
HDRS_MATRICE = $(HEADER)$(MATRICE).h
HDRS_LISTA = $(HEADER)$(LISTA).h
HDRS_FILELOG = $(HEADER)$(FILELOG).h
SERVER_HEADERS = $(HDRS_MATRICE) $(HDRS_TRIE) $(HDRS_LISTA) $(HDRS_COMUNICAZIONE)
CLIENT_HEADERS = $(HDRS_MATRICE)  $(HDRS_COMUNICAZIONE)
TEST_HEADERS = $(HDRS_MATRICE) $(HDRS_TRIE) $(HDRS_LISTA) $(HDRS_COMUNICAZIONE) 
HDRS_GIOCATORE = $(HEADER)$(GIOCATORE).h
HDRS_BACHECA = $(HEADER)$(BACHECA).h

# File Oggetto
OBJS = $(SERVER).o
OBJC = $(CLIENT).o
OBJHM = $(MATRICE).o
OBJHT = $(TRIE).o
OBJHL = $(LISTA).o
OBJHCS = $(COMMS).o
OBJT = $(TEST).o
OBJG = $(GIOCATORE).o
OBJB = $(BACHECA).o
OBJFL = $(FILELOG).o
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
SRCG = $(SORGENTE)$(GIOCATORE).c
SRCB = $(SORGENTE)$(BACHECA).c
SRCFL = $(SORGENTE)$(FILELOG).c

# Input
PORT = 20000
PORT2 = 30003
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
	@$(CC) $(CFLAGS) $(SRCS) $(SRCHCS) $(SRCHM) $(SRCHL) $(SRCHT) $(SRCG) $(SRCB) $(SRCFL) -o $(SERVER)
	@$(CC) $(CFLAGS) $(SRCS) $(SRCHCS) $(SRCHM) $(SRCHL) $(SRCHT) $(SRCG) $(SRCB) $(SRCFL) -o $(SERVER)

$(CLIENT): $(CLIENT_OBJS)
	@echo Compilazione del Client
	@$(CC) $(CFLAGS)  $(SRCC) $(SRCHCS) $(SRCHM)  $(CLIENT_HEADERS) -o $(CLIENT)
	@$(CC) $(CFLAGS)  $(SRCC) $(SRCHCS) $(SRCHM) $(SRCFL) $(CLIENT_HEADERS) -o $(CLIENT)

$(TEST): $(TEST_OBJS)
	@echo Compilazione dei Test
	@$(CC) $(CFLAGS) $(SRCT) $(SRCHCS) $(SRCHM) $(SRCHL) $(SRCHT) $(SRCG) $(SRCB) $(TEST_HEADERS) $(TEST_HEADERS) -o $(TEST)


$(OBJHM):$(SRCHM)
	@$(CC) $(CFLAGS) -c $(SRCHM) $(HDRS_MATRICE)

$(OBJHCS):$(SRCHCS)
	@$(CC) $(CFLAGS) -c $(SRCHCS) $(HDRS_COMUNICAZIONE)

$(OBJHL):$(SRCHL)
	@$(CC) $(CFLAGS) -c $(SRCHL) $(HDRS_LISTA)

$(OBJG):$(SRCG)
	@$(CC) $(CFLAGS) -c $(SRCG) $(HDRS_GIOCATORE)

$(OBJB):$(SRCB)
	@$(CC) $(CFLAGS) -c $(SRCB) $(HDRS_BACHECA)

$(OBJFL):$(SRCFL)
	@$(CC) $(CFLAGS) -c $(SRCFL) $(HDRS_FILELOG)

#Server.o
$(OBJS):$(SRCS)
	@echo compilo 
	@$(CC) $(CFLAGS) -c $(SRCS) $(SRCHCS) $(SRCHM) $(SRCHL) $(SRCHT) $(SRCG) $(SRCB) $(SRCFL)  $(SERVER_HEADERS)
	@$(CC) $(CFLAGS) -c $(SRCS) $(SRCHCS) $(SRCHM) $(SRCHL) $(SRCHT) $(SRCG) $(SRCB) $(SRCFL)  $(SERVER_HEADERS)

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
	@echo Esecuzione server sulla porta $(3003)
	@$(SERVER_RUN2)

run-client2:
	@echo Esecuzione client sulla porta $(3003)
	@$(CLIENT_RUN2)

run-test:
	@echo Esecuzione dei test
	@$(TEST_RUN)
