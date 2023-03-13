#include <arpa/inet.h>

int commUDP(char mensagem[], char buffer[], char regIP[], char regUDP[]);

struct node{
    char id;
    char ext;
    char bck;
    int intr[128];
};