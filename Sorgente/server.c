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


#define MAX_CLIENTS 32 
#define MAX_LENGTH_USERNAME 10 //Numero massimo di lunghezza dell'username
#define NUM_THREADS 5 //Numero di thread da creare
#define BUFFER_SIZE 1024 //dimensione del buffer


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
    while(1){
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
}

