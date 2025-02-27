//Definizione Librerie
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#include "../Header/macro.h"
#include "../Header/Lista.h"
#include "../Header/FileLog.h"
#define FILELOG "FileLog.txt"

//Inizializza il mutex 
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
void file_log(char* utente, char* testo) {
    pthread_mutex_lock(&log_mutex);
    FILE* log_file = fopen(FILELOG, "a");

    if (log_file == NULL) {
        perror("Errore nell'apertura del file di log");
        pthread_mutex_unlock(&log_mutex);
        return;
    }

    // Verifica stringhe valide
    char* safe_utente = utente ? utente : "UNKNOWN";
    char* safe_testo = testo ? testo : "Nessun messaggio";

    if (fprintf(log_file, "[%s] %s\n", safe_utente, safe_testo) < 0) {
        perror("Errore nella scrittura su file di log");
    }
    
    fflush(log_file);
    fclose(log_file);
    pthread_mutex_unlock(&log_mutex);
}
