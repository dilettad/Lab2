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

// Funzioni get
void get_username(listaGiocatori* lista, pthread_t tid){
    for (int i = 0; i < lista-> num_giocatori; i++) {
        if (pthread_equal(lista->giocatori[i].tid, tid)){
            return lista->giocatori[i].username;
        }
    }
    return NULL; // ID non trovato
}

//ATTRAVERSO IL TID RISALE AL PUNTEGGIO DEL GIOCATORE
void get_punteggio(listaGiocatori* lista, pthread_t tid) {
    for (int i = 0; i < lista->num_giocatori; i++) {
        if (pthread_equal(lista->giocatori[i].tid, tid)) {
            return lista->giocatori[i].punteggio;
        }
    }
    return -1; // Thread ID non trovato (o punteggio non valido)
}

//Funzione di invio classifica ai giocatori
void sendClassifica(listaGiocatori* lista, pthread_t tid, pthread_mutex_t lista_mutex, char* classifica, time_t tempo_iniziale, int durata_pausa_){
    pthread_mutex_lock(&lista_mutex);
    giocatore* current = lista->head;
    while(current != NULL){
        if(pthread_equal(corrente->tid, tid)){
            send_message(corrente->client_fd, strlen(classifica), MSG_PUNTI_FINALI, classifica);
            char *temp = calcola_tempo_rimanente(start_time, durata_pausa_in_secondi);
            send_message(corrente->client_fd, strlen(temp), MSG_TEMPO_ATTESA, temp);
        }
        current = current->next;
    }
    pthread_mutex_unlock(&lista_mutex);
}

// Funzione per confrontare i punteggi dei giocatori -> qsort
int compare_score (const void *a, const void *b){
    risGiocatore *playerA = (risGiocatore*) a;
    risGiocatore *playerB = (risGiocatore*) b;
    return (playerB->punteggio - playerA->punteggio); // Ordinamento decrescente
}
