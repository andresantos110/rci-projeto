#include <errno.h> 
#include <netinet/in.h> 
#include <sys/time.h>
#include <arpa/inet.h>

void tcpSelect(struct node *nodo, char IP[16], char TCP[6]);