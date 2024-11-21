#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>


//librerie personali
#include "../Header/macro.h"
#include "../Header/Comunicazione.h"
#include "../Header/Trie.h"
#include "../Header/Lista.h"


//define di progetto
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


// GIOCATORE 
//Funzione per aggiungere client alla lista
void add_client(Fifo* lista, int client_fd, char* username){
    Client* new_client = (Client*) malloc(sizeof(Client));
    new_client -> fd = client_fd;
    strcpy(new_client->username, username);
    new_client->next = NULL;
    
}      
  
//Funzione per registrazione del cliente
void registrazione_client(int client_fd, char* username){
    Client* current = clients_head;
    // Controllo se l'username è già presente
    while(current != NULL) {
        // Se l'username è già presente, manda un messaggio di errore
        if (strcmp(current->username, username) == 0) {
            send_message(client_fd,MSG_ERR,"Username già esistente, scegline un altro");
            break; //Esco dalla funzione
        }
        current = current ->next;
    }
    // Controllo se l'username contiene solo caratteri validi
    if (!controlla_caratteri(client_fd)) {
        send_message(client_fd, MSG_ERR, "Username non valido, non deve contenere caratteri ASCII");
        return; // Esci dalla funzione se l'username non è valido
    }
    // Se l'username è valido, invio un messaggio di conferma 
    send_message(client_fd,MSG_OK,"username valido");

    //Inserisco il client nella lista e ne incremento il numero
    add_client(&clients_head, client_fd,username );
    num_client++;
}  

//Controllo caratteri dell'username: non deve contenere caratteri ASCII
int controlla_caratteri(const char* username){
    while(*username){
        if((unsigned char)*username < 128){ // Controlla se il carattere è ASCII
            return 0; // Se contiene caratteri ASCII, restituisci 0
        }
        username++;
    }
    return 1; // Se tutti i caratteri sono validi (non ASCII), restituisci 1
}

//Funzione per stampare la lista dei client
void stampa_lista_clienti(){
    Client* current = clients_head;
    while (current != NULL) {
        printf("client_fd: %d, Usernale: %s\n", current -> fd, current -> username);
        current = current->next;
        }
}

// Funzione per liberare la memoria della lista
void libera_lista_clienti() {
    Client* current = clients_head; // Inizializzo il puntatore alla prima cella della lista
    Client* next; // Inizializzo un puntatore per iterare la lista
// Itero la lista e libero la memoria di ogni cella
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    clients_head = NULL;
}


