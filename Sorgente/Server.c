#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

#include "../Header/macro.h"
#include "../Header/Comunicazione.h"
#include "../Header/Trie.h"
#include "../Header/Lista.h"
#include "../Header/Matrice.h"
#include "../Header/FunzioniServer.h"

#define MAX_CLIENTS 32 
#define MAX_LENGTH_USERNAME 10 // Numero massimo di lunghezza dell'username
#define NUM_THREADS 5 // Numero di thread da creare
#define BUFFER_SIZE 1024 // Dimensione del buffer
#define MATRIX_SIZE 4
#define DIZIONARIO "../Dizionario.txt"

void invio_matrice(int client_fd, char matrix[MATRIX_SIZE][MATRIX_SIZE]);
char* calcola_tempo_rimanente(time_t tempo_iniziale, int durata_partita);
int pausa_gioco = 0; // Gioco
int durata_partita = 300; // La partita dura 5 minuti quindi 300s
int durata_pausa = 90; // La pausa della partita dura 1.5 minuti
int punteggio = 0; // Devo aggiungere il punteggio tramite le mutex?
int classifica = 0; // Classifica non disponibile
int scorer = 0; // Scorer

// MUTEX
pthread_mutex_t pausa_gioco_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t matrix_mutex;
pthread_mutex_t lista_mutex;
pthread_mutex_t scorer_mutex;
pthread_cond_t scorer_cond; // Corretto da pthread_mutex_t a pthread_cond_t
pthread_t scorer_tid; // Corretto da pthread_mutex_t a pthread_t
listaGiocatori lista; // Lista giocatori

// Handler dei segnali
void alarm_handler(int sig) {
    // Quando scade il tempo se il gioco era in pausa, cambia lo stato in modo da ricominciare
    if (pausa_gioco == 1) {
        pthread_mutex_lock(&pausa_gioco_mutex);
        pausa_gioco = 0;
        printf("Il gioco è in corso.\n");
        pthread_mutex_unlock(&pausa_gioco_mutex);
    } else {
        // Quando scade il tempo, il gioco si ferma e cambia lo stato
        pthread_mutex_lock(&pausa_gioco_mutex);
        pausa_gioco = 1;
        printf("Il gioco è in pausa.\n");
        pthread_mutex_unlock(&pausa_gioco_mutex);
        
        // Invio del segnale a tutti i thread giocatori 
        if (lista.count > 0) {
            scorer = 1;
            invia_SIG(&lista, SIGUSR1, &lista_mutex);
            int retvalue = pthread_create(&scorer_tid, NULL, scorer, NULL);
            if (retvalue != 0) {
                perror("Errore nella pthread_create dello scorer");
            }    
        }
    }
}

// Funzione per terminazione del server
void sigint_handler(int sig) {
    // Scorro la lista dei thread attivi, in modo da chiuderli
    if (lista.count != 0) {
        pthread_mutex_lock(&lista_mutex);
        for (int i = 0; i < lista.count; i++) {
            // Invia un messaggio di chiusura a ciascun giocatore
            if (send_message(lista.array[i].client_sock, "Il gioco è finito.\n") < 0) {
                perror("Errore nell'invio del messaggio di chiusura");
            }
            // Cancella il thread del giocatore
            if (pthread_cancel(lista.array[i].thread_id) != 0) {
                perror("Errore nella cancellazione del thread");
            }
        }
        pthread_mutex_unlock(&lista_mutex); // Spostato fuori dal ciclo
    }
    
    // Distruggi lista dei giocatori 
    distruggi_lista(&lista);
    
    // Chiudi socket
    if (close(server_fd) == -1) {
        perror("Errore nella chiusura del socket del server");
    }
    printf("Chiusura del server\n");
    exit(EXIT_SUCCESS);
}

// Funzione del thread
void* thread_func(void* arg) {
    int client_sock = *(int*)arg;
    free(arg); // Libera la memoria allocata per il socket

    // Gestione dei comandi ricevuti dal client
    while (1) {
        message client_message = receive_message(client_sock);
        switch (client_message.type) {
            case MSG_MATRICE:
                if (pausa_gioco == 0) {
                    // Gioco quindi invio la matrice attuale e il tempo di gioco rimanente
                    invio_matrice(client_sock, matrice);
                    char* temp = calcola_tempo_rimanente(tempo_iniziale, durata_partita);
                    send_message(client_sock, MSG_TEMPO_PARTITA, temp);
                } else {
                    // Invio il tempo di pausa rimanente
                    char* temp = calcola_tempo_rimanente(tempo_iniziale, durata_pausa);
                    send_message(client_sock, MSG_TEMPO_ATTESA, temp);
                }
                break;

            case MSG_PAROLA:
                if (pausa_gioco == 0) {
                    // Controllo se la parola è già stata trovata 
                    if (esiste_paroleTrovate(listaParoleTrovate, client_message.data)) {
                        send_message(client_sock, MSG_PUNTI_PAROLA, "0");
                        break;
                    }
                    // Controllo se parola è in matrice
                    else if (!trovaParola(matrice, client_message.data)) {
                        send_message(client_sock, MSG_ERR, "Parola nella matrice non trovata");
                        break;
                    }
                    // Controllo se parola è nel dizionario
                    else if (!search_Trie(root, client_message.data)) {
                        send_message(client_sock, MSG_ERR, "Parola nel dizionario non trovata");
                        break;
                    }
                    // Se i controlli hanno esito positivo, allora aggiungo parola alla lista delle parole trovate
                    else {
                        paroleTrovate = aggiungiParolaTrovata(listaParoleTrovate, client_message.data);
                        int puntiparola = strlen(client_message.data);
                        // Se la parola contiene "Q" con "u" a seguito, sottraggo di uno i punti
                        if (strstr(client_message.data, "Qu")) {
                            puntiparola--;
                        }
                        // Invio i punti della parola
                        send_message(client_sock, MSG_PUNTI_PAROLA, puntiparola);
                        punteggio += puntiparola;
                    }
                } else {
                    // Invio il messaggio di errore
                    send_message(client_sock, MSG_ERR, "Gioco in pausa!");
                }
                break;

            case MSG_REGISTRA_UTENTE:
                // Controllo nome utente
                send_message(client_sock, MSG_ERR, "Utente già registrato");
                break;

            case MSG_PUNTI_FINALI:
                if (pausa_gioco == 1 && classifica == 1) {
                    send_message(client_sock, MSG_PUNTI_FINALI, classifica);
                    char* temp = calcola_tempo_rimanente(tempo_iniziale, durata_partita);
                    send_message(client_sock, MSG_TEMPO_ATTESA, temp);
                } else {
                    send_message(client_sock, MSG_ERR, "Classifica non disponibile");
                }
                break;
        }
        send_message(client_sock, MSG_OK, "ciao diletta");
    }
    // Terminazione del thread con valore di ritorno
    pthread_exit(NULL);
}

// Funzione per gestire lo scorer
void* scorer(void* arg) {
    printf("Scorer in esecuzione\n");

    // Registrazione dei segnali
    signal(SIGUSR2, sigusr2_handler); // Cambiato SIGINT in SIGUSR2 per evitare conflitti

    // Prendo il numero di giocatori registrati
    pthread_mutex_lock(&lista_mutex);
    int num_giocatori = lista.count; // Assicurati che lista sia definito correttamente
    pthread_mutex_unlock(&lista_mutex);

    risGiocatore scorerVector[MAX_CLIENTS];

    // Ciclo per raccogliere i risultati
    for (int i = 0; i < num_giocatori; i++) {
        pthread_mutex_lock(&scorer_mutex);
        while (scoreList == NULL) {
            pthread_cond_wait(&scorer_cond, &scorer_mutex);
        }

        risGiocatore* temp = PopRisList(&scoreList);
        if (temp == NULL) {
            pthread_mutex_unlock(&scorer_mutex);
            fprintf(stderr, "Errore: impossibile popolare la lista dei risultati.\n");
            return NULL; // Esci in caso di errore
        }

        scorerVector[i].username = temp->username; // Assicurati che username sia allocato correttamente
        scorerVector[i].punteggio = temp->punteggio;
        free(temp); // Libera la memoria allocata per temp
                pthread_mutex_unlock(&scorer_mutex);
    }

    // Ordinamento dei risultati
    qsort(scorerVector, num_giocatori, sizeof(risGiocatore), compare_score_func);

    // Invio segnale a tutti i thread giocatori notificandoli che possono prelevare la classifica
    invia_SIG(&listaGiocatori, SIGUSR2, &lista_mutex); // Cambiato SIGINT in SIGUSR2

    printf("Classifica pronta. %d giocatori registrati.\n", num_giocatori);
    return NULL;
}

// Funzione che gestisce il thread di gioco
void* game(void* arg) {
    printf("Giocatore in esecuzione\n");
    int round = 0;
    time_t tempo_iniziale;

    while (1) {
        pthread_mutex_lock(&lista_mutex);
        // Attesa fino a quando non ci sono giocatori registrati
        while (lista.count == 0) {
            printf("Nessun giocatore registrato. Attesa...\n");
            pthread_cond_wait(&lista.cond, &lista.mutex);
        }
        pthread_mutex_unlock(&lista_mutex);

        // Inizia la pausa
        sleep(1);
        alarm(durata_pausa);
        printf("La partita è in pausa, inizierà: %d secondi\n", durata_pausa);
        
        // PREPARAZIONE DEL ROUND
        if (round == 0) {
            pthread_mutex_lock(&matrix_mutex);
            if (Carica_MatricedaFile("file-txt", matrice) != 0) {
                perror("Errore nel caricamento della matrice");
            }
            pthread_mutex_unlock(&matrix_mutex);
            round = 1;
        }

        // Se alla fine della pausa non ci sono giocatori registrati, si ripete il ciclo
        pthread_mutex_lock(&lista_mutex);
        if (lista.count == 0) {
            pausa_gioco = 1;
            pthread_mutex_unlock(&lista_mutex);
            continue;
        }
        pthread_mutex_unlock(&lista_mutex);    
       
        // Inizia la partita
        time(&tempo_iniziale);
        alarm(durata_partita);
        printf("La partita è iniziata, terminerà: %d secondi\n", durata_partita);

        pthread_mutex_lock(&lista_mutex);
        giocatore* current = lista.head;
        while (current != NULL) {
            char* temp = calcola_tempo_rimanente(tempo_iniziale, durata_partita);
            if (send_message(current->client_fd, MSG_TEMPO_PARTITA, temp) < 0) {
                perror("Errore nell'invio del messaggio");
            }
            current = current->next;
        }
        pthread_mutex_unlock(&lista_mutex);

        while (!pausa_gioco) {
            // Attesa
        }

        // Reset dalla pausa per il prossimo round 
        pausa_gioco = 0;
        round = 0;
    }    
}

int main(int argc, char* argv[]) {
    // Inizializzazione della lista dei giocatori
    listaGiocatori lista; // Assicurati che la struttura sia definita correttamente
    lista.count = 0; // Inizializza il conteggio dei giocatori
    lista.head = NULL; // Inizializza il puntatore head
    lista.tail = NULL; // Inizializza il puntatore tail

    int server_sock;
    struct sockaddr_in server_addr;

    if (argc < 3) {
        // Errore
        printf("Errore: Numero errato di parametri passati");
        return 0; 
    }

    // Creazione del socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Inizializzazione dell'indirizzo del server
    memset(&server_addr, 0, sizeof(server_addr)); // Azzerare la struttura
    server_addr.sin_family = AF_INET; // IPv4
    server_addr.sin_addr.s_addr = inet_addr(argv[1]); // Indirizzo server
    server_addr.sin_port = htons(atoi(argv[2])); // Porta del server

    // BIND() assegna un indirizzo al socket
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind fallita");
        close(server_sock);
        return 1;
    }

    // Se funziona il bind, il socket ascolta
    if (listen(server_sock, 3) < 0) {
        perror("Listen fallita");
        close(server_sock);
        return 1;
    }

    printf("In attesa di connessioni...\n");

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    while (1) {
        // Accetta la connessione
        int* client_sock = malloc(sizeof(int)); // Allocazione dinamica per il socket del client
        *client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);

        if (*client_sock < 0) {
            perror("Connessione accettata fallita");
            free(client_sock); // Libera la memoria in caso di errore
            continue; // Continua il ciclo per accettare altre connessioni
        }

        printf("Connessione accettata\n");

        // Creazione del thread a cui passo il fd associato al socket del client
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, thread_func, client_sock) != 0) {
            perror("Failed to create thread");
            free(client_sock); // Libera la memoria in caso di errore
            continue; // Continua il ciclo per accettare altre connessioni
        }

        // Non è necessario joinare il thread qui, poiché vogliamo che il server continui ad accettare connessioni
        // pthread_join(thread_id, NULL); // Rimuovi questa riga per non bloccare il server

        // Se vuoi gestire la terminazione del server, puoi aggiungere un gestore di segnali qui
    }

    // Chiudi il socket del server
    close(server_sock);
    return 0;
}

// Calcola tempo rimanente
void calcola_tempo_rimanente(time_t tempo_iniziale, int durata) {
    time_t tempo_attuale = time(NULL);
    double tempo_trascorso = difftime(tempo_attuale, tempo_iniziale);
    int tempo_rimanente_secondi = durata - (int)tempo_trascorso;

    if (tempo_rimanente_secondi < 0) {
        printf("Il gioco è già terminato\n");
    } else {
        printf("Il tempo rimanente è: %d secondi\n", tempo_rimanente_secondi);
    }
}

// Invio della matrice e del tempo rimanente in base alla fase del gioco in cui è il giocatore
void invio_matrice(int client_fd, char matrix[MATRIX_SIZE][MATRIX_SIZE]) {
    int length = MATRIX_SIZE * MATRIX_SIZE;
    char data[length];
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            data[i * MATRIX_SIZE + j] = matrix[i][j];
        }
    }
    printf("Invio matrice al client %d\n", client_fd);
    send_message(client_fd, MSG_MATRICE, data);
}
// Funzione per gestire la terminazione del server
void cleanup() {
    // Distruggi lista dei giocatori
    distruggi_lista(&listaGiocatori);

    // Chiudi il socket del server
    if (close(server_sock) == -1) {
        perror("Errore nella chiusura del socket del server");
    }
    printf("Chiusura del server completata.\n");
}

// Funzione principale
int main(int argc, char* argv[]) {
    // Inizializzazione della lista dei giocatori
    listaGiocatori listaGiocatori; // Assicurati che la struttura sia definita correttamente
    listaGiocatori.count = 0; // Inizializza il conteggio dei giocatori
    listaGiocatori.head = NULL; // Inizializza il puntatore head
    listaGiocatori.tail = NULL; // Inizializza il puntatore tail

    int server_sock;
    struct sockaddr_in server_addr;

    if (argc < 3) {
        // Errore
        printf("Errore: Numero errato di parametri passati\n");
        return 0; 
    }

    // Creazione del socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Inizializzazione dell'indirizzo del server
    memset(&server_addr, 0, sizeof(server_addr)); // Azzerare la struttura
    server_addr.sin_family = AF_INET; // IPv4
    server_addr.sin_addr.s_addr = inet_addr(argv[1]); // Indirizzo server
    server_addr.sin_port = htons(atoi(argv[2])); // Porta del server

    // BIND() assegna un indirizzo al socket
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind fallita");
        close(server_sock);
        return 1;
    }

    // Se funziona il bind, il socket ascolta
    if (listen(server_sock, MAX_CLIENTS) < 0) {
        perror("Listen fallita");
        close(server_sock);
        return 1;
    }

    printf("In attesa di connessioni...\n");

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
    // Gestione dei segnali per la terminazione del server
    signal(SIGINT, sigint_handler);
    signal(SIGUSR1, alarm_handler); // Assicurati di registrare il gestore per SIGUSR1

    while (1) {
        // Accetta la connessione
        int* client_sock = malloc(sizeof(int)); // Allocazione dinamica per il socket del client
        *client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);

        if (*client_sock < 0) {
            perror("Connessione accettata fallita");
            free(client_sock); // Libera la memoria in caso di errore
            continue; // Continua il ciclo per accettare altre connessioni
        }

        printf("Connessione accettata\n");

        // Creazione del thread a cui passo il fd associato al socket del client
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, thread_func, client_sock) != 0) {
            perror("Failed to create thread");
            free(client_sock); // Libera la memoria in caso di errore
            continue; // Continua il ciclo per accettare altre connessioni
        }

        // Non è necessario joinare il thread qui, poiché vogliamo che il server continui ad accettare connessioni
    }

    // Chiudi il socket del server
    cleanup(); // Chiamata alla funzione di pulizia
    return 0;
}