#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

struct node{
    char id[3];
    char ip[16];
    char port[6];
    
    char ext[3];
    char ipExt[16];
    char portExt[6];

    char bck[3];
    char ipBck[16];
    char portBck[6];

    char intr[100][3];
    char ipIntr[100][16];
    char portIntr[100][6];
    
    char *content[32]; //max de 32 conteudos
    int ncontents;
};