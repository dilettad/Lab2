#include <stdio.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "../Header/macro.h"
#include "../Header/comunicazione.h"
#include "../Header/Trie.h"


/*
int main(int argc, char* argv[]){
    //Controllo se il numero di parametri passati è corretto
    if(argc<3){
        //Errore
        printf("Errore: Numero errato di parametri passati");
        return 0; 
    }
    //Prendo il parametro nome_server
    char* nome_server = argv[1];
     //Controllo se il nome del server preso è quello corretto
    if(argv[1]!= "serverd"){
        printf("Errore:Nome del server errato");
        return 0;
    }
    //Prendo il parametro porta_server
    int porta_server = atoi(argv<[2]);
    
  //Parametri opzionali: [--matrici data_filename] [--durata durata_in_minuti] [--seed rnd_seed] [--diz dizionario] 
  //!! riguarda in base alla struttura che scegli
  
    //Gestione
    int opt;
    while((opt = getopt(argc, argv, "m:d:s:x:")) != -1){
        switch (opt) {
        //--matrici `e seguito dal nome del file dal quale caricare le matrici 
        case 'm':
            parametri.data_filename= optarg;
            break;
        //--durata permette di indicare la durata del gioco in minuti. Se non espresso di default va considerato 3 minuti
        case 'd':
                parametri.durata = atoi(optarg);
                break;
        //--seed permette di indicare il seed da usare per la generazione dei numeri pseudocasuali
            case 's':
                parametri.rnd_seed = atoi(optarg);
                break;
        //--diz permette di indicare il dizionario da usare per la verifica di leicità delle parole ricevute dal client.
            case 'x':
                parametri.dizionario = optarg;
                break;
            default:
                fprintf(stderr, "Uso: %s nome_server porta_server [-m data_filename] [-d durata_in_minuti] [-s rnd_seed] [-x dizionario]\n", argv[0]);
                return 1;
        }
    }

}   
*/




//SOCKET
#define NUM_THREADS 5 //Numero di thread da creare
#define MAX_CLIENTS 32 //Numero massimo di clienti
#define BUFFER_SIZE 1024 //dimensione del buffer

// Funzione del thread
void* thread_func(void* arg) {
    // Dichiara un puntatore per il valore di ritorno
    int* retdata = (int*)malloc(sizeof(int));
    if (retdata == NULL) {
        perror("malloc failed");
        pthread_exit(NULL); // Termina il thread se la malloc fallisce
    }

    // Recupero del thread corrente
    pthread_t thread = pthread_self();

    // Stampa del valore passato al thread
    printf("Thread %ld, value: %d\n", (long)thread, *(int*)arg);

    // Assegna valore di ritorno
    *retdata = *(int*)arg * 2; // Restituisce il valore passato moltiplicato per 2

    // Terminazione del thread con valore di ritorno
    pthread_exit(retdata);
}


int main(int argc, char* argv[]) {
    int server_sock;
    struct sockaddr_in server_addr;
    char message [128];
    
    if(argc<3){
        //Errore
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

    // Accetta la connessione
    int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);
    if (client_sock < 0) {
        perror("Accept fallita");
        close(server_sock);
        return 1;
    }

    printf("Connessione accettata\n");
    read(client_sock, message, 128);
    printf("Client: %s\n", message);
    // Chiudi i socket
    
    //close(client_sock); // Chiudi il socket del client
    close(server_sock); // Chiudi il socket del server
    return 0;
    


    //THREAD
    pthread_t threads[NUM_THREADS];
    int values[NUM_THREADS];
    int* ret;

    // Creazione dei thread
    for (int i = 0; i < NUM_THREADS; i++) {
        values[i] = i + 1; // Valore da passare al thread
        if (pthread_create(&threads[i], NULL, thread_func, (void*)&values[i]) != 0) {
            perror("Failed to create thread");
            return 1;
        }
    }

    // Aspetta che ogni thread termini e recupera il valore di ritorno
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], (void**)&ret);
        // Stampa il valore di ritorno dal thread
        printf("Valore di ritorno dal thread %d: %d\n", i + 1, *ret);
        free(ret); // Libera la memoria allocata
    }
    return 0;

}





/*

void *client_handler(void *socket_desc);

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creazione del socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Attaccare il socket al porto
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Iniziare ad ascoltare
    if (listen(server_fd, 3) < 0) {
        perror("Listen");
        exit(EXIT_FAILURE);
    }

    printf("Server in ascolto sulla porta %d\n", PORT);

    while (1) {
        // Accettare la connessione del client
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept");
            exit(EXIT_FAILURE);
        }

        printf("Nuovo collegamento accettato\n");

        // Creare un thread per gestire il client
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, client_handler, (void *)&new_socket) < 0) {
            perror("Could not create thread");
            return 1;
        }
    }

    return 0;
}

void *client_handler(void *socket_desc) {
    int sock = *(int*)socket_desc;
    char buffer[BUFFER_SIZE];
    int read_size;

    // Ricezione dei messaggi dal client
    while ((read_size = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[read_size] = '\0'; // Null-terminate the string
        printf("Messaggio ricevuto: %s\n", buffer);
        
        // Inviare una risposta al client
        send(sock, "Messaggio ricevuto", strlen("Messaggio ricevuto"), 0);
    }

    if (read_size == 0) {
        printf("Client disconnesso\n");
    } else if (read_size == -1) {
        perror("recv failed");
    }

    close(sock);
    return 0;
}*/




/*#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    // Creazione del socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Errore nella creazione del socket \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convertire gli indirizzi IPv4 e IPv6 da testo a binario
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\n Indirizzo non valido/ Indirizzo non supportato \n");
        return -1;
    }

    // Connessione al server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\n Connessione al server fallita \n");
        return -1;
    }

    // Inviare un messaggio al server
    char *message = "Ciao Server!";
    send(sock, message, strlen(message), 0);
    printf("Messaggio inviato: %s\n", message);

    // Ricevere la risposta dal server
    int valread = read(sock, buffer, BUFFER_SIZE);
    printf("Risposta dal server: %s*/