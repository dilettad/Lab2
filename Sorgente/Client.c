#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <ctype.h>


// librerie personali
#include "../Header/macro.h"
#include "../Header/Comunicazione.h"
#include "../Header/Matrice.h"
#include "../Header/Client.h"
// #include "../Header/Giocatore.h"

// define di progetto
#define HOST "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024 // Dimensione del buffer

/*TODO*/
/*
Aggiungere Segnali, però aspettare il server per farli
*/

char *ltrim(char *s)
{
    while (isspace(*s))
        s++;
    return s;
}

char *rtrim(char *s)
{
    char *back = s + strlen(s);
    while (isspace(*--back))
        ;
    *(back + 1) = '\0';
    return s;
}

char *trim(char *s)
{
    return rtrim(ltrim(s));
}

// define di progetto
//  #define HOST "127.0.0.1"
//  #define PORT 8080
#define BUFFER_SIZE 1024 // Dimensione del buffer

int client_sock;
int fd_server;
pthread_mutex_t message_mutex = PTHREAD_MUTEX_INITIALIZER;

// Definisco la funzione che gestisce la SIGINT
void GestoreSigint(int sig)
{
    int retvalue;
    // Distruggo il mutex utilizzato per la gestione dei messaggi
    retvalue = pthread_mutex_destroy(&message_mutex);
    // Stampo un messaggio sulla chiusura del client
    printf("\nChiusura Client tramite SIGINT\n");
    fflush(stdout); // Assicura che il messaggio venga stampato subito
                    // Controlla se il fd dell server è valido prima di chiuderlo
    if (fd_server >= 0)
    {
        // Chiudo il socket del server, gestendo errori
        SYSC(retvalue, close(fd_server), "Errore nella close Client");
    }
    // Chiuso il socket del client
    close(client_sock);
    // Chiudo il socket del server
    close(fd_server);
    // Esco dal programma
    exit(EXIT_SUCCESS);
}

void *receiver(void *args)
{
    int client_sock = *(int *)args; // Assumiamo che args contenga il socket
    message received_msg;

    while (1)
    {
        received_msg = receive_message(client_sock);
        cella **matrice = generateMatrix();
        // Gestione del messaggio in base al tipo
        pthread_mutex_lock(&message_mutex); // Inizio sezione critica
        switch (received_msg.type)
        {

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
            break;

        case MSG_MATRICE:
            if (matrice == NULL)
            {
                fprintf(stderr, "Errore nell'allocazione della matrice\n");
                pthread_mutex_unlock(&message_mutex);
                break;
            }
            InputStringa(matrice, received_msg.data);
            printf("\nMatrice: \n");
            stampaMatrice(matrice);
            free(matrice); // Dealloca la memoria della matrice
            break;

        case MSG_PUNTI_PAROLA:
            printf("\n%s\n", received_msg.data);//messaggio punti
            break;

        case MSG_TEMPO_PARTITA:
            printf("\n%s\n", received_msg.data);
            break;

        case MSG_PUNTI_FINALI:
            printf("\nClassifica generale:\n");
            printf("%s\n", (char *)received_msg.data);
            break;

        case MSG_SERVER_SHUTDOWN:
            printf("\n%s\n", received_msg.data);
            exit(EXIT_SUCCESS);
            // exit(0);
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

// Match tra accept e connect, viene creato un nuovo socket locale
int main(int argc, char *argv[])
{
    int client_sock, retvalue;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    if (argc < 3)
    {
        perror("errore numero parametri input");
        exit(0);
    }
    // creazione socket
    SYSC(client_sock, socket(AF_INET, SOCK_STREAM, 0), "Errore nella socket");
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));      // Specificare la porta
    server_addr.sin_addr.s_addr = inet_addr(argv[1]); // Indirizzo del server

    // Connessione
    while (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)))
    {
        if (errno == ENOENT)
        {
            printf("Errore nella connect, attesa riconnesione\n");
            sleep(1); // Aspetta 1secondo prima di riprovare
        }
    }
    // CICLO DI GIOCO
    pthread_t receiver_thread;
    char *aiuto = 
    " \n"
    "Aiuto: mostra i comandi disponibili\n\n"
    "registra_utente <nome_utente>: per registrarsi\n\n"
    "matrice: richiede al processo server la matrice corrente\n\n"
    "p <parola_indicata>: sottopone al server una parola per verificarne la correttezza e assegnare il punteggio\n\n"
    "login_utente <nome_utente>: per loggarsi\n\n"
    "fine: per uscire dal gioco\n";

    // char* fine =  "Hai deciso di uscire dal gioco!\n";
    // writef(retvalue,"prima del thread\n");
    // pthread_create()
    SYST(retvalue, pthread_create(&receiver_thread, NULL, receiver, &client_sock), "errore creazione thread receiver");
    // writef(retvalue,"prima del thread\n");
    while (1)
    {
        int nread;
        pthread_mutex_lock(&message_mutex);
        printf("Inserisci il messaggio da inviare al server (o 'fine' per uscire): \n");
        
        SYSC(nread, read(STDIN_FILENO, buffer, BUFFER_SIZE), "errore lettura utente");
        pthread_mutex_unlock(&message_mutex);
        char *input = (char *)malloc(nread + 1);
        // printf("input:%s\n",buffer);
        strncpy(input, buffer, nread);
        input[nread] = '\0';
        // printf("buffer:%s, input:%s\n",buffer,input);
        char *token;
        memset(buffer, 0, BUFFER_SIZE);

        // Controllo se contiene "aiuto"
        if (strcmp(input, "aiuto\n") == 0){
            printf("Ecco a te la lista dei comandi: %s\n", aiuto);

            continue;
        }
        // Controllo se contiene "registra_utente"
        else if (strncmp(input, "registra_utente", 15) == 0){
            token = strtok(input, " ");
            token = strtok(NULL, " ");
            if (token == NULL)
            {
                printf("Errore, manca il nome dell'utente\n");
                continue;
            }
            else
            {
                token[strcmp(token, "\n")] = 0;
                send_message(client_sock, MSG_REGISTRA_UTENTE, token);
            }
        }

           // Controllo se contiene "login_utente"
        else if (strncmp(input, "login_utente", 12) == 0){
            token = strtok(input, " ");
            token = strtok(NULL, " ");
            if (token == NULL)
            {
                printf("Errore, manca il nome dell'utente\n");
                continue;
            }
            else
            {
                token[strcmp(token, "\n")] = 0;
                send_message(client_sock, MSG_LOGIN_UTENTE, token);
            }
        }

        // Controllo se contiene "matrice"
        else if (strcmp(input, "matrice\n") == 0)
        {
            send_message(client_sock, MSG_MATRICE, input);
        }

        // Controllo se contiene "p"
        else if (strncmp(input, "p", 1) == 0)
        {
            // Ottenere il primo token (il comando "p")
            token = strtok(input, " ");

            // Ottenere il secondo token (la parola dopo "p")
            token = strtok(NULL, " ");
            if (token == NULL)
            {
                printf("Errore, manca la parola da controllare\n");
                continue;
            }
            else if (strlen(token) < 4)
            {
                printf("Parola troppo corta\n");
                continue;
            }

            send_message(client_sock, MSG_PAROLA, trim(token));
        }

        // Controllo se contiene "fine"
        else if (strcmp(input, "fine\n") == 0)
        {
            send_message(client_sock, MSG_FINE, input);
            break;
        }
        else
            printf("comando non disponibile\n");
        // Inviare messaggio al server
        // send_message(client_sock,MSG_OK,input);

        // Azzerare il buffer prima di ricevere la risposta
        // memset(buffer, 0, BUFFER_SIZE);
    }
    close(client_sock);
    return 0;
}