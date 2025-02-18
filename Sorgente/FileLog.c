//Definizione Librerie
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "../Header/macro.h"
#include "../Header/Lista.h"
#include "../Header/FileLog.h"

#define LOG_FILE "FileLog.txt"
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Funzione per registrare un evento nel file di log -> 1° versione originale
void file_log(char* utente, char* testo) {
    pthread_mutex_lock(&log_mutex);    
    FILE* log_file = fopen(LOG_FILE, "a"); // Apri il file in modalità append
    
    if (log_file == NULL) {
        perror("Errore nell'apertura del file di log");
        pthread_mutex_unlock(&log_mutex);
        return;
    }
    utente[strcspn(utente, "\n")] = '\0';
    printf("LOGGING: [%s]%s\n", utente, testo);
    fprintf(log_file, "[%s]%s\n",utente, testo);
    //fflush(stdout);
    fflush(NULL);
    //fflush (log_file);
    fclose(log_file); // Chiudi il file
    pthread_mutex_unlock(&log_mutex);
}*/
/* // VERSIONE CON COPIA UTENTE
void file_log(char* utente, char* testo) {
    pthread_mutex_lock(&log_mutex);
    FILE* log_file = fopen(FILELOG, "a"); // Apri il file in modalità append
    if (log_file == NULL) {
        perror("Errore nell'apertura del file di log");
        pthread_mutex_unlock(&log_mutex);
        return;
    }

    // Crea una copia della stringa per evitare di modificarla direttamente
    char utente_copy[64];  // Assumendo che i nomi utente siano corti
    (utente_copy, utente, sizeof(utente_copy) - 1);
    utente_copy[sizeof(utente_copy) - 1] = '\0'; // Assicura terminazione corretta


    utente_copy[strcspn(utente_copy, "\r\n")] = '\0';  // Rimuove newline

    printf("LOGGING: [%s] %s\n", utente_copy, testo);
    fprintf(log_file, "[%s] %s\n", utente_copy, testo);
    fflush(log_file); // Flush solo sul file

    fclose(log_file);
    pthread_mutex_unlock(&log_mutex);
}
*/
// VERSIONE CON COPIA UTENTE E TESTO
void file_log(char* utente, char* testo) {
    pthread_mutex_lock(&log_mutex);
    
    printf("DEBUG: file_log() chiamato con:\n");
    printf("DEBUG: utente -> '%s'\n", utente);
    printf("DEBUG: testo  -> '%s'\n", testo);

    FILE* log_file = fopen(LOG_FILE, "a");
    if (log_file == NULL) {
        perror("Errore nell'apertura del file di log");
        pthread_mutex_unlock(&log_mutex);
        return;
    }

    // Rimuove eventuali newline dall'utente
    char utente_copy[64];
    strncpy(utente_copy, utente, sizeof(utente_copy) - 1);
    utente_copy[sizeof(utente_copy) - 1] = '\0';
    utente_copy[strcspn(utente_copy, "\r\n")] = '\0';

    // Rimuove newline anche dal testo
    char testo_copy[256];
    strncpy(testo_copy, testo, sizeof(testo_copy) - 1);
    testo_copy[sizeof(testo_copy) - 1] = '\0';
    testo_copy[strcspn(testo_copy, "\r\n")] = '\0';

    printf("DEBUG: Dopo la pulizia:\n");
    printf("DEBUG: utente -> '%s'\n", utente_copy);
    printf("DEBUG: testo  -> '%s'\n", testo_copy);

    fprintf(log_file, "[%s] %s\n", utente_copy, testo_copy);
    fflush(log_file);
    fclose(log_file);
    
    pthread_mutex_unlock(&log_mutex);
}
