#include <stdio.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "../Header/macro.h"
#include "../Header/Comunicazione.h"
#include "../Header/Trie.h"
#include "../Header/Lista.h"
#include "../Header/Matrice.h"

#define MAX_CLIENTS 32 
#define MAX_LENGTH_USERNAME 10 //Numero massimo di lunghezza dell'username
#define NUM_THREADS 5 //Numero di thread da creare
#define BUFFER_SIZE 1024 //dimensione del buffer

//MAIN
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

// // GIOCATORE 
// //Funzione per aggiungere client alla lista
// void add_client(Fifo* lista, int client_fd, char* username){
//     Client* new_client = (Client*) malloc(sizeof(Client));
//     new_client -> fd = client_fd;
//     strcpy(new_client->username, username);
//     new_client->next = NULL;   
// }
  
// //Funzione per registrazione del cliente
// void registrazione_client(int client_fd, char* username){
//     Client* current = clients_head;
//     // Controllo se l'username è già presente
//     while(current != NULL) {
//         // Se l'username è già presente, manda un messaggio di errore
//         if (strcmp(current->username, username) == 0) {
//             send_message(client_fd,MSG_ERR,"Username già esistente, scegline un altro");
//             break; //Esco dalla funzione
//         }
//         current = current ->next;
//     }
//     // Controllo se l'username contiene solo caratteri validi
//     if (!controlla_caratteri(client_fd)) {
//         send_message(client_fd, MSG_ERR, "Username non valido, non deve contenere caratteri ASCII");
//         return; // Esci dalla funzione se l'username non è valido
//     }
//     // Se l'username è valido, invio un messaggio di conferma 
//     send_message(client_fd,MSG_OK,"username valido");

//     //Inserisco il client nella lista e ne incremento il numero
//     add_client(&clients_head, client_fd,username );
//     num_client++;
// }

// //Controllo caratteri dell'username: non deve contenere caratteri ASCII
// int controlla_caratteri(const char* username){
//     while(*username){
//         if((unsigned char)*username < 128){ // Controlla se il carattere è ASCII
//             return 0; // Se contiene caratteri ASCII, restituisci 0
//         }
//         username++;
//     }
//     return 1; // Se tutti i caratteri sono validi (non ASCII), restituisci 1
// }

// //Funzione per stampare la lista dei client
// void stampa_lista_clienti(){
//     Client* current = clients_head;
//     while (current != NULL) {
//         printf("client_fd: %d, Usernale: %s\n", current -> fd, current -> username);
//         current = current->next;
//         }
// }

// // Funzione per liberare la memoria della lista
// void libera_lista_clienti() {
//     Client* current = clients_head; // Inizializzo il puntatore alla prima cella della lista
//     Client* next; // Inizializzo un puntatore per iterare la lista
// // Itero la lista e libero la memoria di ogni cella
//     while (current != NULL) {
//         next = current->next;
//         free(current);
//         current = next;
//     }
//     clients_head = NULL;
// }




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
    //char message [128];
    
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
        
        /*read(client_sock, message, 128);
        printf("Client: %s\n", message);*/
        // Chiudi i socket
        
        //close(client_sock); // Chiudi il socket del client
        //close(server_sock); // Chiudi il socket del server
        //return 0;
        


        //THREAD
        pthread_t thread_id;

        // Creazione del thread a cui passo il fd associato al socket del client
        if(pthread_create(&thread_id, NULL, thread_func, &client_sock) != 0){
            perror("Failed to create thread");
            return 1;
        }

        // Aspetta che ogni thread termini e recupera il valore di ritorno
        /*for (int i = 0; i < NUM_THREADS; i++) {
            pthread_join(threads[i], (void**)&ret);
            // Stampa il valore di ritorno dal thread
            printf("Valore di ritorno dal thread %d: %d\n", i + 1, *ret);
            free(ret); // Libera la memoria allocata
        }
        return 0;*/
    }


    cella ** matrice = generateMatrix();
    InputStringa(&matrice,"");



}




