//Definizione Librerie
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "../Header/macro.h"
#include "../Header/Lista.h"
#include "../Header/FileLog.h"
#define FILELOG "FileLog.txt"
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

// Funzione per registrare un evento nel file di log
void file_log(char* utente, char* testo) {
    pthread_mutex_lock(&log_mutex);
    FILE* log_file = fopen(FILELOG, "a"); // Apri il file in modalità append
    if (log_file == NULL) {
        perror("Errore nell'apertura del file di log");
        pthread_mutex_unlock(&log_mutex);
        return;
    }
    utente[strcspn(utente, "\n")] = ' ';
    // Scrivi l'evento nel file di log
    fprintf(log_file, "[ %s]%s\n",utente, testo);
    fflush(log_file);

    fclose(log_file); // Chiudi il file
    pthread_mutex_unlock(&log_mutex);
}