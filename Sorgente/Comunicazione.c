#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
//librerie di progetto
#include "../Header/macro.h"
#include "../Header/comunicazione.h"


//Funzione per inviare un messaggio
void send_message(int client_socket, char type, char* data) {
    int retvalue, length = strlen(data); 
    //invio del campo tipo
    SYSC(retvalue,write(client_socket,&type,sizeof(char)),"nell'invio del tipo");
    //invio del campo lunghezza
    SYSC(retvalue,write(client_socket,&length,sizeof(int)),"nell'invio della lunghezza");
    //invio del campo data
    SYSC(retvalue,write(client_socket,data,sizeof(data)),"nell'invio del payload");
}

//Funzione per ricevere un messaggio
message receive_message(int client_socket){
    message msg;int retvalue;
    SYSC(retvalue,read(client_socket,&(msg.type),sizeof(char)),"nella ricezione del payload");
    SYSC(retvalue,read(client_socket,&(msg.length),sizeof(int)),"nella ricezione del payload");

    msg.data = (char*)malloc(msg.length + 1); //Alloca in memoria lo spazio necessario per il data
  
    SYSC(retvalue,read(client_socket,msg.data,sizeof(char*)),"nella ricezione del payload");
    return msg;
}





