//questa serve per non avere gli error squiggles sulla sigaction
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>

#include "../Header/macro.h"
#include "../Header/Comunicazione.h"
#include "../Header/Matrice.h"
#include "../Header/Client.h"


#define HOST "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024    // Dimensione del buffer

int durata_partita = 8;     // La partita dura 5 minuti quindi 300s
int durata_pausa = 5;       // La pausa della partita dura 1 minuto
int client_sock;
int fd_server;
pthread_mutex_t message_mutex = PTHREAD_MUTEX_INITIALIZER;

// Definisco la funzione che gestisce la SIGINT
void GestoreSigint(int sig){
    //int retvalue;
    pthread_mutex_destroy(&message_mutex);                   //Distrugge il mutex utilizzato per la gestione dei messaggi
    printf("\nChiusura Client tramite SIGINT\n");                               
    send_message(client_sock, MSG_FINE, "");               //Messaggio di chiusura del client
    close(client_sock);
    exit(EXIT_SUCCESS);
}

void *receiver(void *args){
    int client_sock = *(int *)args;                                     //Args contenga il socket
    message received_msg;                                               

    while (1){
        received_msg = receive_message(client_sock);                    //Riceve il messaggio dal server
        cella **matrice = generateMatrix();                             //Genera matrice di gioco
       
        //GESTIONE DEL MESSAGGIO IN BASE AL TIPO
        pthread_mutex_lock(&message_mutex); 
        switch (received_msg.type){

        case MSG_OK:
            printf("\n%s\n", received_msg.data);
            printf("Inserisci il messaggio da inviare al server (o 'fine' per uscire): \n");
            fflush(0);
            break;

        case MSG_ERR:
            pthread_mutex_unlock(&message_mutex);
            printf("\n%s\n", received_msg.data);
            printf("Inserisci il messaggio da inviare al server (o 'fine' per uscire): \n");
            fflush(0);
            break;

        case MSG_FINE:
            printf("\n%s\n", received_msg.data);
            close(client_sock);                                         //Chiusura del socket e termine programma
            exit(EXIT_SUCCESS);
            break;

        case MSG_MATRICE:
            printf("\nMatrice: \n");
            if (matrice == NULL){                                       //Se allocazione della matrice fallita stampa un messaggio di errore
                fprintf(stderr, "Errore nell'allocazione della matrice\n");
                pthread_mutex_unlock(&message_mutex);
                break;
            }
            InputStringa(matrice, received_msg.data);                   //Matrice con i dati ricevuti
            stampaMatrice(matrice);                                  
            free(matrice);                                              //Dealloca la memoria della matrice
            break;

        case MSG_PUNTI_PAROLA:
            printf("\n%s\n", received_msg.data);                        //Messaggio con i punti parola
            printf("Inserisci il messaggio da inviare al server (o 'fine' per uscire): \n");
            break;

        case MSG_TEMPO_PARTITA:
            printf("\n%s\n", received_msg.data);                        //Messaggio con tempo partita 
            printf("Inserisci il messaggio da inviare al server (o 'fine' per uscire): \n");
    
            break;
        case MSG_TEMPO_ATTESA:
            printf("\n Tempo di attesa per la prossima partita: %d\n", durata_pausa); 
            printf("\n%s\n", received_msg.data);                        //Messaggio con tempo di attesa
            printf("Inserisci il messaggio da inviare al server (o 'fine' per uscire): \n");
            break;

        case MSG_PUNTI_FINALI:
            printf("\nClassifica generale e punteggio personale ricevuti:\n");
            printf("%s\n", received_msg.data);                          //Messaggio con punti finali
            printf("Inserisci il messaggio da inviare al server (o 'fine' per uscire): \n");
            break;
        
        case MSG_SERVER_SHUTDOWN:
            printf("\n%s\n", received_msg.data);                        //Messaggio di chiusura del server
            exit(EXIT_SUCCESS);
            break;

        case MSG_SHOW_BACHECA:
            printf("\nMessaggi sulla bacheca:\n");
            printf("%s\n", (char *)received_msg.data);                  //Stampa i messaggi sulla bacheca (utente:messaggio)
            printf("Inserisci il messaggio da inviare al server (o 'fine' per uscire): \n");
            break;

        case MSG_POST_BACHECA:
            printf("\nMessaggio postato sulla bacheca:\n");
            printf("%s\n", (char *)received_msg.data);                  //Stampa del conferma del messaggio postato sulla bacheca
            printf("Inserisci il messaggio da inviare al server (o 'fine' per uscire): \n");
            break;    

        default:
            fprintf(stderr, "Tipo di messaggio sconosciuto: %d\n", received_msg.type); //Messaggio stampato di default
            printf("Inserisci il messaggio da inviare al server (o 'fine' per uscire): \n");
            break;
        }
        pthread_mutex_unlock(&message_mutex);
    }
    close(client_sock);                                                 //Chiusura del socket
}


int main(int argc, char *argv[]){
    int  retvalue;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    struct sigaction azione_segnale;
    sigset_t maschera_segnale;
    
    //Controllo se sono stati passati i 3 parametri
    if (argc < 3){
        perror("[Errore] Numero dei parametri in input errati");
        return 0;
    }

    /*IMPOSTO LA MASCHERA DEI SEGNALI*/
    sigemptyset(&maschera_segnale);
    SYSC(retvalue,sigaddset(&maschera_segnale,SIGINT),"aggiunta SIGINT alla maschera");
    SYSC(retvalue,sigaddset(&maschera_segnale,SIGQUIT),"aggiunta SIGQUIT alla maschera");

    /*IMPOSTO LA SIGACTION*/
    azione_segnale.sa_handler = GestoreSigint;
    azione_segnale.sa_mask = maschera_segnale;
  
    /*IMPOSTO IL GESTORE*/
    sigaction(SIGINT,&azione_segnale,NULL);
    sigaction(SIGQUIT,&azione_segnale,NULL);

    //Creazione socket
    SYSC(client_sock, socket(AF_INET, SOCK_STREAM, 0), "Errore nella socket"); 
    server_addr.sin_family = AF_INET;                                   //IPv4
    server_addr.sin_port = htons(atoi(argv[2]));                        //Porta
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);                   //Indirizzo del server

    // Connessione al server
    while (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr))){
        if (errno == ENOENT){
            printf("Errore nella connect, attesa riconnesione\n");
            sleep(1);                                                   //Aspetta 1secondo prima di riprovare
        }
    }

    //CICLO DI GIOCO
    pthread_t receiver_thread;                                          //Dichiarazione del thread
    char *aiuto =                                                       //Stringa aiuto che contiene la lista dei comandi disponibili
    " \n"
    "aiuto: mostra i comandi disponibili\n\n"
    "registra_utente <nome_utente>: per registrarsi\n\n"
    "matrice: richiede al processo server la matrice corrente\n\n"
    "p <parola_indicata>: sottopone al server una parola per verificarne la correttezza e assegnare il punteggio\n\n"
    "login_utente <nome_utente>: per loggarsi\n\n"
    "cancella_utente <nome_utente>: per cancellare l'utente\n\n"
    "show-msg: per mostrare i messaggi della bacheca\n\n"
    "msg <testo_messaggio>: per postare un messaggio sulla bacheca\n\n"
    "fine: per uscire dal gioco\n";

    //Creazione del thread receiver
    SYST(retvalue, pthread_create(&receiver_thread, NULL, receiver, &client_sock), "errore creazione thread receiver");
    printf("Inserisci il messaggio da inviare al server (o 'fine' per uscire): \n");
    
    while (1){
        int nread;
        SYSC(nread, read(STDIN_FILENO, buffer, BUFFER_SIZE), "errore lettura utente"); //Legge inpute dell'utente 
        char *input = (char *)malloc(nread + 1);                                       //Alloca memoria per la stringa di input
        strncpy(input, buffer, nread);                                                 //Copia l'input nel buffer
        input[nread] = '\0';
        char *token;                                                                 
        memset(buffer, 0, BUFFER_SIZE);                                                //Azzera il buffer

    //INIZIO CONTROLLO STRINGHE

        if (strcmp(input, "aiuto\n") == 0){                                           //Controllo input = "aiuto" e stampa la lista dei comandi
            printf("Ecco a te la lista dei comandi: %s\n", aiuto);
            continue;
        } 
        else if (strncmp(input, "registra_utente", 15) == 0){                         //Controllo l'input = "registra_utente"
            token = strtok(input, " ");                                               //Prima tokenizzazione con il comando
            token = strtok(NULL, "\n");                                               //Seconda tokenizzazione con l'username
            if (token == NULL){                                                       //Controllo se username è NULL, in tal caso errore
                printf("Errore, manca il nome dell'utente\n");
                continue;
            } else {
                send_message(client_sock, MSG_REGISTRA_UTENTE, token);                //Invio del messaggio al server
            }
        }
        else if (strncmp(input, "login_utente", 12) == 0){                            //Controllo l'input = "login_utente"
            token = strtok(input, " ");                                               //Prima tokenizzazione con il comando
            token = strtok(NULL, "\n");                                               //Seconda tokenizzazione con l'username
            if (token == NULL){                                                       //Controllo se username è NULL, in tal caso errore
                printf("Errore, manca il nome dell'utente\n");
                continue;
            }
            else{
                send_message(client_sock, MSG_LOGIN_UTENTE, token);                   //Invio del messaggio al server
            }
        }
        else if (strncmp(input, "cancella_utente", 15) == 0){                         //Controllo l'input = "cancella_utente"
            token = strtok(input, " ");                                               //Prima tokenizzazione con il comando
            token = strtok(NULL, "\n");                                               //Seconda tokenizzazione con l'username
            if (token == NULL){                                                       //Controllo se username è NULL, in tal caso errore
                printf("Errore, manca il nome dell'utente\n");
                continue;
            }
            else{
                send_message(client_sock, MSG_CANCELLA_UTENTE, token);                //Invio del messaggio al server
            }
        }
        else if (strcmp(input, "matrice\n") == 0){                                   //Controllo l'input = "matrice"
            send_message(client_sock, MSG_MATRICE, input);                           //Invio del messaggio al server
        }
        else if (strncmp(input, "p", 1) == 0){                                       //Controllo l'input = "p"
            token = strtok(input, " ");                                              //Prima tokenizzazione con il comando
            token = strtok(NULL, "\n");                                              //Seconda tokenizzazione con la parola
            if (token == NULL){                                                      //Controllo se parola è vuota, in tal caso errore
                printf("Errore, manca la parola da controllare\n");
                continue;
            }
            else if (strlen(token) < 4){                                             //Controllo se la parola è minore di 4, in tal caso errore
                printf("Parola troppo corta\n");
                continue;
            }
            send_message(client_sock, MSG_PAROLA, token);                            //Invio del messaggio al server
        }
        else if (strcmp(input, "show-msg\n") == 0){                                  //Controllo l'input = "show-msg"
            send_message(client_sock, MSG_SHOW_BACHECA, input);                      //Invio del messaggio al server
        }
        else if (strncmp(input, "msg", 3) == 0){                                     //Controllo l'input = "msg" 
            token = strtok(input, " ");                                              //Prima tokenizzazione con il comando
            token = strtok(NULL, "\n");                                              //Seconda tokenizzazione con il testo del messaggio
            if (token == NULL){                                                      //Controllo se il testo è assente, in tal caso errore
                printf("Errore, manca il messaggio \n");
                continue;
            }
            else{
                send_message(client_sock, MSG_POST_BACHECA, token);                  //Invio del messaggio al server
            }
        }
        else if (strcmp(input, "fine\n") == 0)                                       //Controllo l'input = "fine"
        {
            send_message(client_sock, MSG_FINE, input);                              //Invio del messaggio al server  
            break;
        }
        else
            printf("comando non disponibile\n");                                     //In caso in cui scriva qualcosa non presente, invio messaggio
    }
    close(client_sock);                                                             //Chiusura socket del client
    return 0;
}