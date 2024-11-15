#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "../Header/macro.h"

#define MSG_OK "K"
#define MSG_ERR "E"
#define MSG_REGISTRA_UTENTE "R"
#define MSG_MATRICE "M"
#define MSG_TEMPO_PARTITA "T"
#define MSG_TEMPO_ATTESA "A"
#define MSG_PAROLA "W"
#define MSG_PUNTI_FINALI "F"
#define MSG_PUNTI_PAROLA "P"
#define MSG_SERVER_SHUTDONW "B"
#define MSG_POST_BACHECA "H"
#define MSG_SHOW_BACHECA "S"

//Definizione di una struttura richiesta e risposta
typedef struct {
    char type; 
    unsigned int length; 
    char* data; 
} message;
 

//Funzione per inviare un messaggio
void send_message(int client_socket, char type, char* data) {
    int retvalue;
    //invio del campo data
    SYSC(retvalue,write(client_socket,data,sizeof(data)),"nell'invio del payload")
}

//Funzione per ricevere un messaggio
message receive_message(int client_socket){
    message msg;int retvalue;
    msg.data = (char*)malloc(msg.length + 1); //Alloca in memoria lo spazio necessario per il data
    SYSC(retvalue,read(client_socket,msg.data,msg.length),"nella ricezione del payload");
    return msg;
}

//MSG_REGISTRA_UTENTE: Registra un nuovo utente, se non preso -> messaggio ok; se preso -> messaggio err e ciclo finchè non arrivo a un nome nuovo
void registrazione_utente(int client_socket, char* username){

}






