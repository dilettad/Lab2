//STRUTTURA DEL SINGOLO GIOCATORE REGISTRATO CORRETTAMENTE
typedef struct giocatore {
    char* username;
    pthread_t tid;
    int client_fd;
    int punteggio;
    struct giocatore* next;
}giocatore;

//LISTA DEI GIOCATORI REGISTRATI CORRETTAMENTE
typedef struct{
    giocatore* head;
    int count;
}listaGiocatori;

// Calcola tempo rimanente
char* calcola_tempo_rimanente(time_t tempo_iniziale, int durata) {
    time_t tempo_attuale = time(NULL);
    double tempo_trascorso = difftime(tempo_attuale, tempo_iniziale);
    int tempo_rimanente_secondi = durata - (int)tempo_trascorso;
    //char* outstring = "Il tempo rimanente è: %d secondi\n";
    if (tempo_rimanente_secondi < 0) {
        return "Il gioco è già terminato\n";
    } else {
        return "Diletta devi aggiustare il tempo rimanente";
    }
}
// // Funzioni get
// void get_username(listaGiocatori* lista, pthread_t tid, int num_giocatori){
//     listaGiocatori* lista_cpy = lista;
//     for (int i = 0; i < num_giocatori; i++) {
//         if (pthread_equal(lista->head->tid, tid)){
//             return lista->head.username;
//         }else{
//             lista_cpy->head = lista_cpy->head->next;
//         }
//     }
//     return NULL; // ID non trovato
// }

//ATTRAVERSO IL TID RISALE AL PUNTEGGIO DEL GIOCATORE
int get_punteggio(listaGiocatori* lista, pthread_t tid) {
    listaGiocatori* lista_cpy = lista;
    while(lista_cpy->head != NULL){
        if (pthread_equal(lista_cpy->head->tid, tid)) {
            return lista_cpy->head->punteggio;
        }
        lista_cpy->head = lista_cpy->head->next;
    }
    return -1; // Thread ID non trovato (o punteggio non valido)
}

//Funzione di invio classifica ai giocatori
void sendClassifica(listaGiocatori* lista, pthread_t tid, pthread_mutex_t lista_mutex, char* classifica, time_t tempo_iniziale, int durata_pausa_){
    pthread_mutex_lock(&lista_mutex);
    giocatore* current = lista->head;
    while(current != NULL){
        if(pthread_equal(current->tid, tid)){
            send_message(current->client_fd,MSG_PUNTI_FINALI, classifica);
            char *temp = calcola_tempo_rimanente(tempo_iniziale, durata_pausa_);
            send_message(current->client_fd, MSG_TEMPO_ATTESA, temp);
        }
        current = current->next;
    }
    pthread_mutex_unlock(&lista_mutex);
}

// Funzione per confrontare i punteggi dei giocatori -> qsort
int compare_score (const void *a, const void *b){
    giocatore *playerA = (giocatore*) a;
    giocatore *playerB = (giocatore*) b;
    return (playerB->punteggio - playerA->punteggio); // Ordinamento decrescente
}
