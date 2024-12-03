#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h> 
#include <arpa/inet.h>
#include <pthread.h>
#include "../Header/macro.h"
#include "../Header/Trie.h"
#include "../Header/Lista.h"
#include "../Header/Comunicazione.h"


//Funzione per inviare un messaggio
void send_message(int client_socket, char type, char* data) {
    int retvalue;
    int len = strlen(data);
    //printf("invio messaggio\n");
    //invio lunghezza messaggio
    SYSC(retvalue,write(client_socket,&len,sizeof(int)),"errore lettura lunghezza messaggio\n");
    //printf("invio messaggio\n");
    //invio tipo messaggio
    SYSC(retvalue,write(client_socket,&type,sizeof(char)),"errore lettur atipo messaggio\n")
    //printf("invio messaggio\n");
    //invio del campo data
    SYSC(retvalue,write(client_socket,data,len),"nell'invio del payload")
    //printf("invio messaggio\n");
}

//Funzione per ricevere un messaggio
message receive_message(int client_socket){
    message msg;int retvalue;
    //ricevo lunghezza messaggio
    //printf("attendo messaggio\n");
    SYSC(retvalue,read(client_socket,&msg.length,sizeof(int)),"errore lettura lunghezza messaggio\n");
    //ricevo tipo messaggio
    SYSC(retvalue,read(client_socket,&msg.type,sizeof(char)),"errore lettur atipo messaggio\n")
    //alloco campo data del messaggio
    msg.data = (char*)malloc(msg.length+1); //Alloca in memoria lo spazio necessario per il data
    //ricevo messagio 
    SYSC(retvalue,read(client_socket,msg.data,msg.length),"nella ricezione del payload");
    msg.data[msg.length+1] = '\0';
    printf("messaggio arrivato:\ntipo:%c\nlunghezza:%d\npayload:%s\n",msg.type,msg.length,msg.data);

    return msg;
}