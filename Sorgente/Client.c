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
    message received_msg;

    while (1) {
        received_msg = receive_message(client_sock);
        cella** matrice = generateMatrix();
        // Gestione del messaggio in base al tipo
        pthread_mutex_lock(&message_mutex); // Inizio sezione critica
        switch (received_msg.type) {
            
            case MSG_OK:
                printf("\n%s\n", received_msg.data);
                fflush(0);
                break;

            case MSG_ERR:
                pthread_mutex_unlock(&message_mutex);
                printf("\n%s\n", received_msg.data);
                fflush(0);
                break;


            case MSG_FINE:
                pthread_mutex_unlock(&message_mutex);
                printf("\n%s\n", received_msg.data);
                exit(EXIT_SUCCESS);


            case MSG_MATRICE:
                
                if (matrice == NULL) {
                    fprintf(stderr, "Errore nell'allocazione della matrice\n");
                    pthread_mutex_unlock(&message_mutex);
                    break;
                }
                InputStringa(matrice, received_msg.data);
                printf("\nMatrice: \n");
                stampaMatrice(&matrice);
                free(matrice); // Dealloca la memoria della matrice
                break;

            case MSG_PUNTI_PAROLA:
                printf("\nTotalizzato Punti parola: %d\n", *((int*)received_msg.data));
                break;

            case MSG_TEMPO_PARTITA:
                printf("\nTempo partita: %d\n", *((int*)received_msg.data));
                break;

            case MSG_PUNTI_FINALI:
                printf("\nClassifica generale:\n");
                printf("%s\n", (char*)received_msg.data);
                break;

            default:
                fprintf(stderr, "Tipo di messaggio sconosciuto: %d\n", received_msg.type);
                break;
        }
        pthread_mutex_unlock(&message_mutex); // Fine sezione critica
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
        int nread;
        printf("Inserisci il messaggio da inviare al server (o 'exit' per uscire): \n");
        SYSC(nread,read(STDIN_FILENO,buffer,BUFFER_SIZE),"errore lettura utente");
        char* input = (char*)malloc(nread+1);
        printf("input:%s\n",buffer);
        strncpy(input,buffer,nread);
        input[nread] = '\0';
        printf("buffer:%s, input:%s\n",buffer,input);

        // Inviare messaggio al server
        send_message(client_sock,MSG_OK,input);

        // Azzerare il buffer prima di ricevere la risposta
        //memset(buffer, 0, BUFFER_SIZE);

        // Ricevere risposta dal server
        //ssize_t bytes_received = recv(client_sock, buffer, BUFFER_SIZE, 0);
        message response_message = receive_message(client_sock);
        printf("esco dalla receive\n");
        // Stampare la risposta del server
        printf("Risposta del server: %s\n", response_message.data);
    }

//TASK DI OGGI: OCCUPARSI DELLA CAMPIONATURA DEI MESSAGGI SCRITTI DA RIGA DI COMANDO SUL CLIENT   
// registra_utente nome_utente -> registra un nuovo utente e manda mess registra utente con ok o err
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


// void client_sender (void * args) {
//     int client_fd = *(int*)args; // Estrae il file descriptor del client
//     int value;
//     char buffer[BUFFER_SIZE];  // Buffer per lettura dell'input dell'utente
//     ssize_t n_read;
//     // aiuto -> mostrare i comandi disponibili, quindi registra_utente, nome_utente, matrice parola_indicata
//     char* aiuto =   "I comandi disponibili sono: \naiuto -> mostra i comandi disponibili\nregistra_utente nome_utente --> per registrarsi\nmatrice --> richiede al processo server la matrice corrente relativa alla fase in cui si è \np parola_indicata --> sottopone al server una parola, per capirne la correttezza e assegnare il punteggio\nfine --> uscire dal giorco \n"; 
//     char* fine =  "Hai deciso di uscire dal gioco!\n";

//     // Ciclo infinito per gestire i messaggi dell'utente
//     while (1) {
//         char * token;
//         int retvalue;
//         SYSC(n_read, read(STDIN_FILENO, buffer, BUFFER_SIZE), "Errore lettura");

//         // Rimuove il newline finale, se presente
//         buffer[strcspn(buffer, "\n")] = 0;

//         // Controllo "aiuto"
//         if (strcmp(buffer, "aiuto") == 0) {
//             writef(retvalue, aiuto);
//             continue;
//         }
//         // Controllo registra utente
//         else if (strcmp(buffer, "registra_utente") == 0) {
//             token = strtok(NULL, "\n");
//             if (token == NULL) {
//                 writef (retvalue, "Nome utente non valido\n");
//                 continue;
//             }
//             send_message(client_fd, token, MSG_REGISTRA_UTENTE);
//         }   
//         // Controllo matrice
//         else if (strcmp(buffer, "matrice") == 0) {
//             send_message(client_fd, NULL, MSG_MATRICE);
//         } 
//         else if (strncmp(buffer, "p ", 2) == 0) { // Controllo per parola
//             token = strtok(buffer + 2, "\n"); // Ottiene la parola dopo "p "
//             if (token == NULL) {
//                 writef (retvalue, "Parola non valida\n");
//                 continue;
//             } 
//             else if (strlen(token) < 4) {
//                 writef (retvalue, "Parola troppo corta non valida\n");
//                 continue;
//             } 
//             send_message(client_fd, token, MSG_PAROLA);
//         }
//         else if (strcmp(buffer, "fine") == 0) {
//             send_message(client_fd, NULL, MSG_FINE);
//             break;
//         }
//         writef(retvalue,"comando non valido");
//     }
// }