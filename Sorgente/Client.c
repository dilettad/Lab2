#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>

/*TODO*/
/*
1)Rendere Multi-Thread il client, grado di gestire più utenti contemporaneamente

2)Implementare Libreria di comunicazione per messaggi Client-Server come richiesta dal progetto
*/

//librerie personali
#include "../Header/macro.h"
#include "../Header/Comunicazione.h"
#include "../Header/Trie.h"
#include "../Header/Lista.h"
#include "../Header/Matrice.h"
#include "../Header/Client.h"
// #include "../Header/Giocatore.h"

//define di progetto
#define HOST "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024 //Dimensione del buffer

/*TODO*/
/*
1)Rendere Multi-Thread il client
2)Implementare Libreria di comunicazione per messaggi Client-Server come richiesta dal progetto
*/

//define di progetto
// #define HOST "127.0.0.1"
// #define PORT 8080 
#define BUFFER_SIZE 1024 //Dimensione del buffer

int client_sock;
int fd_server;
pthread_mutex_t message_mutex = PTHREAD_MUTEX_INITIALIZER;

//Definisco la funzione che gestisce la SIGINT
void GestoreSigint(int sig){
    int retvalue;
    //Distruggo il mutex utilizzato per la gestione dei messaggi
    retvalue = pthread_mutex_destroy(&message_mutex);
    //Stampo un messaggio sulla chiusura del client 
    printf("\nChiusura Client tramite SIGINT\n"); 
    fflush(stdout); // Assicura che il messaggio venga stampato subito
    //Controlla se il fd dell server è valido prima di chiuderlo
        if (fd_server >= 0){
            // Chiudo il socket del server, gestendo errori 
            SYSC(retvalue, close(fd_server), "Errore nella close Client");
        }  
    //Chiuso il socket del client           
    close(client_sock);
    // Chiudo il socket del server
    close(fd_server);
    //Esco dal programma 
    exit(EXIT_SUCCESS);

}

void receiver(void* args) {
    int client_sock = *(int*)args; // Assumiamo che args contenga il socket
    msg * received_msg;

    while (1) {
        received_msg = receive_message(client_sock);
        if (received_msg == NULL) {
            perror("Errore nella ricezione del messaggio");
            break; // Esci dal loop se c'è un errore
        }

        // Gestione del messaggio in base al tipo
        pthread_mutex_lock(&message_mutex); // Inizio sezione critica
        switch (received_msg->type) {
            
            case MSG_OK:
                printf("\n%s\n", receive_message -> msg);
                fflush(0);
                break;

            case MSG_ERROR:
                pthread_mutex_unlock(&messagge_mutex);
                printf("\n%s\n", receive_message -> msg);
                fflush(0);
                break;


            case MSG_FINE:
                pthread_mutex_unlock(&messagge_mutex);
                printf("\n%s\n", receive_message -> msg);
                exit(EXIT_SUCCESS);


            case MSG_MATRICE:
                void* matrice = generateMatrix();
                if (matrice == NULL) {
                    fprintf(stderr, "Errore nell'allocazione della matrice\n");
                    pthread_mutex_unlock(&message_mutex);
                    break;
                }
                InputStringa(matrice, received_msg->msg);
                printf("\nMatrice: \n");
                stampaMatrice(matrice);
                free(matrice); // Dealloca la memoria della matrice
                break;

            case MSG_PUNTI_PAROLA:
                printf("\nTotalizzato Punti parola: %d\n", *((int*)received_msg->msg));
                break;

            case MSG_TEMPO_PARTITA:
                printf("\nTempo partita: %d\n", *((int*)received_msg->msg));
                break;

            case MSG_PUNTI_FINALI:
                printf("\nClassifica generale:\n");
                printf("%s\n", (char*)received_msg->msg);
                break;

            default:
                fprintf(stderr, "Tipo di messaggio sconosciuto: %d\n", received_msg->type);
                break;
        }
        pthread_mutex_unlock(&message_mutex); // Fine sezione critica

        free(received_msg); // Dealloca la memoria del messaggio ricevuto
    }

    // Chiusura del socket e pulizia
    close(client_sock);
}


//Match tra accept e connect, viene creato un nuovo socket locale
int main(int argc, char* argv[]){
    int client_sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    if(argc<3){
        perror("errore numero parametri input");
        exit(0);
    }
    //creazione socket
    SYSC(client_sock,socket(AF_INET,SOCK_STREAM,0),"Errore nella socket");
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2])); // Specificare la porta
    server_addr.sin_addr.s_addr = inet_addr(argv[1]); // Indirizzo del server


    // Connessione 
    while (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr))){
        if(errno == ENOENT){
            printf("Errore nella connect, attesa riconnesione\n");
            sleep(1); //Aspetta 1secondo prima di riprovare
        }
    }
    //CICLO DI GIOCO
    while (1) {
        printf("Inserisci il messaggio da inviare al server (o 'exit' per uscire): ");
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            perror("Errore nella lettura dell'input");
            continue; // Riprova a leggere l'input
        }

        // Rimuovere il carattere di nuova linea se presente
        buffer[strcspn(buffer, "\n")] = 0;

        // Condizione per uscire dal ciclo //cambiare in base al testo
        if (strcmp(buffer, "exit") == 0) {
            printf("Uscita dal programma...\n");
            break;
        }

        // Inviare messaggio al server
        if (send(client_sock, buffer, strlen(buffer), 0) < 0) {
            perror("Errore nell'invio del messaggio");
            continue; // Riprova a inviare un nuovo messaggio
        }

        // Azzerare il buffer prima di ricevere la risposta
        memset(buffer, 0, BUFFER_SIZE);

        // Ricevere risposta dal server
        ssize_t bytes_received = recv(client_sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received < 0) {
            perror("Errore nella ricezione della risposta");
            continue; // Riprova a inviare un nuovo messaggio
        } else if (bytes_received == 0){
            printf("Il server ha chiuso la connessione.\n");
            break; // Uscire se il server ha chiuso la connessione
        }

        // Stampare la risposta del server
        printf("\nRisposta del server: %s", buffer);
    }

    /* Invio del messaggio
    strcpy(message, "ciao");
    send(client_sock, message, strlen(message), 0);
    */

    // Chiusura del socket
    close(client_sock);
    return 0;

//RECEIVER -> comunicazione
// MSG_SERVER_SHUTDOWN -> terminazione della connessione
    // In bacheca i messaggi devono essere di 126 caratteri -> controllo qua?
}

