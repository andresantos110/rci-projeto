#include <errno.h> 
#include <netinet/in.h> 
#include <sys/time.h>
#include <arpa/inet.h>

#ifndef max
    #define max(a,b) ((a) > (b) ? (a) : (b))
#endif

void tcpSelect(struct node *nodo, char IP[16], char TCP[6], char infoExt[32], char regIP[16], char regUDP[16]);