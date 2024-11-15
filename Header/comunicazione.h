//Definizione del messaggio
typedef struct{
    char type;
    unsigned int length;
    char* data;
} message;

//invio messaggio
void send_message(int client_socket, char type, char* data);
//ricezione messaggio sul socket
message receive_message(int client_socket);