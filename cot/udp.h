#include <arpa/inet.h>
#include <ctype.h>

int commUDP(char mensagem[], char buffer[], char regIP[], char regUDP[]);
int compare_udp_messages(char message[], char known_chars[]);

struct node{
    char id;
    char ext;
    char bck;
    int intr[128];
};