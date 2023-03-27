#include <errno.h> 
#include <netinet/in.h> 
#include <sys/time.h>
#include <arpa/inet.h>

#ifndef max
    #define max(a,b) ((a) > (b) ? (a) : (b))
#endif

void tcpSelect(struct node *nodo, char regIP[16], char regUDP[6], char *net);

int commTCP(int fd, struct node *nodo, char *regIP, char *regUDP, char *net);