//Definizioni di macro per syscall
#define buff_size 256

// Per la gestione degli errori delle syscall che restituiscono un intero
#define SYSC(v,c,m)\
    if ((v = c ) == -1){perror(m);exit(errno);};

// Per la gestione degli errori delle syscall che restituiscono un puntatore
#define SYSCN(v,c,m)\
    if ((v = c) == NULL){perror(m);exit(errno);};
//Per la gestione degli errori delle syscall che restituiscono un valore diverso da zero
#define SYST(v,c,m)\
    if ((v = c) != 0){perror(m);exit(errno);};

/*Shortcut per usare la write come printf*/
#define writef(retvalue,message)\
    SYSC(retvalue,write(STDOUT_FILENO,message,strlen(message)),"nella writef");


void fun();