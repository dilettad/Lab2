#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include "../Header/macro.h"
#define HOST "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024 //Dimensione del buffer

/*TODO*/
/*
1)Rendere Multi-Thread il client
2)Implementare Libreria di comunicazione per messaggi Client-Server come richiesta dal progetto
*/




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
        } else if (bytes_received == 0) {
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
}




