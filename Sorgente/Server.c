#include <stdio.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>

#include "../Header/macro.h"
#include "../Header/Comunicazione.h"
#include "../Header/Trie.h"
#include "../Header/Lista.h"
#include "../Header/Matrice.h"

#define MAX_CLIENTS 32 
#define MAX_LENGTH_USERNAME 10 //Numero massimo di lunghezza dell'username
#define NUM_THREADS 5 //Numero di thread da creare
#define BUFFER_SIZE 1024 //dimensione del buffer
#define MATRIX_SIZE 4
#define DIZIONARIO "../Dizionario.txt"

void invio_matrice(int client_fd, char matrix[MATRIX_SIZE][MATRIX_SIZE]);
void calcola_tempo_rimanente(time_t tempo_iniziale, int durata);

int pausa_gioco = 0; //Gioco
// La partita dura 5 minuti quindi 300s
int durata_partita = 300;
//La pausa della partita dura 1.5 minuti
int durata_pausa = 90;
int punteggio = 0; // Devo aggiungere il punteggio tramite le mutex?
int classifica = 0; // Classifica non disponibile
char*  classificaError = "Classifica non disponibile"
//SOCKET
// Funzione del thread
void* thread_func(void* arg) {
    // Dichiara un puntatore per il valore di ritorno
    int client_sock = *(int*)arg;


// Gestione dei comandi ricevuti dal client
// MSG_MATRICE: invia la matrice e il tempo rimanente o il tempo di pausa 
// MSG_PAROLA: controllo punti della parola in base ai caratteri, se presente nella matrice, nel dizionario e accredita punti, se già trovata 0
// MSG_REGISTRA_UTENTE: registra l'utente e controllo se già registrato
// MSG_PUNTI_FINALI: calcolo i punti totali
    char parola = receive_message(client_sock);
    int retvalue;
    while(1){
        message client_message = receive_message(client_sock);
        writef(retvalue,client_message.data);
        switch (client_message.type){
            case MSG_MATRICE:
                if(pausa_gioco == 0){
                    // Gioco quindi invio la matrice attuale e il tempo di gioco rimanente
                    invio_matrice(client_sock, matrice);
                    char *temp = calcola_tempo_rimanente(tempo_iniziale, durata_partita);
                    send_message(client_sock, MSG_TEMPO_PARTITA, temp);
                } else {
                    // Invio il tempo di pausa rimanente
                    char *temp = calcola_tempo_rimanente(tempo_iniziale, durata_pausa);
                    send_message(client_sock, MSG_TEMPO_ATTESA, temp);
                }
                break;

            case MSG_PAROLA:
                if (pausa_gioco==0){
                // Controllo se la parola è già stata trovata 
                   // Inserire una lista parole per confrontare
                    if(esiste_paroleTrovate(listaParoleTrovate, data)){ //SISTEMARE 
                        send_message(client_sock, 1, MSG_PUNTI_PAROLA, "0");
                        break;
                    }
                //Controllo se parola è in matrice
                    else if(!trovaParola(matrice, data)){
                        send_message(client_sock, MSG_ERR, "Parola nella matrice non trovata");
                        break;
                    }
                   
                //Controllo se parola è nel dizionario
                    else if(!search_Trie(root, data)){
                        send_message(client_sock, MSG_ERR, "Parola nel dizionario non trovata");
                        break;
                    }
                // Se i controlli hanno esito positivo, allora aggiungo parola alla lista delle parole trovate
                else{
                    // Aggiungo la parola alla lista delle parole trovate
                    paroleTrovate = aggiungiParolaTrovata(listaParoleTrovate, data); //DA SISTEMARE LA LISTA
                    int puntiparola = strlen(data);
                    // Se la parola contiene "Q" con "u" a seguito, sottraggo di uno i punti
                    if (strstr(data,"Qu")){
                        puntiparola--;
                    } 
                    // Invio i punti della parola
                    send_message(client_sock, puntiparola, MSG_PUNTI_PAROLA, data);
                    punteggio += puntiparola;
                }
            } else {
                // Invio il messaggio di errore
                send_message(client_sock, strlen(errno), MSG_ERR, errno);
            }
            break;             
            
            case MSG_REGISTRA_UTENTE:
                //controllare nome utente
                send_message(client_sock, MSG_ERR, "Utente già registrato");
                break;

            case MSG_PUNTI_FINALI:
                if(pausa_gioco == 1 && classifica == 1){
                    send_message(client_sock, strlen(classifica), MSG_PUNTI_FINALI, classifica);
                    char *temp = calcola_tempo_rimanente(tempo_iniziale, durata);
                    send_message(client_sock, strlen(temp), MSG_TEMPO_ATTESA, temp);
                }
                else{
                    send_message(client_sock, strlen(classificaError), MSG_ERR, classificaError);
                }
                break;
            }
        send_message(client_sock,MSG_OK,"ciao diletta");
    }
    // Terminazione del thread con valore di ritorno
    pthread_exit(NULL);
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

        //Aspetta che ogni thread termini e recupera il valore di ritorno
        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_join(thread_id,NULL);
            //Stampa il valore di ritorno dal thread
            //free(ret); // Libera la memoria allocata
            printf("thread ucciso\n");
        }
        return 0;
    }


    cella** matrice = generateMatrix();
        // Libera la memoria
    for (int i = 0; i < MATRIX_SIZE; i++) {
        free(matrice[i]);
    }
    free(matrice);

    return 0;
    InputStringa(matrice,"");



}

//int pausa_gioco = 1; // 1 = si, 0 = no

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
void invio_matrice(int client_fd, char matrix [MATRIX_SIZE][MATRIX_SIZE]){
    int length = MATRIX_SIZE * MATRIX_SIZE;
    char data[length];
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            data [i * MATRIX_SIZE + j] = matrix[i][j];
        }
    }    
    printf("Invio matrice al client %d\n", client_fd);
    send_message(client_fd, MSG_MATRICE, data);
}





/*MAIN
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