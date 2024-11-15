#include <stdio.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "../Header/macro.h"
typedef struct cella{
    boolean usato; 
    char value;
}cella;