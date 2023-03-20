#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

struct node{
    char *id;
    char *ext;
    char *bck;
    int intr[128+1];
    char *content[32]; //max de 32 conteudos
    int ncontents;
};