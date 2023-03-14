#include "cot.h"
#include "select.h"

#define TRUE   1 
#define FALSE  0 

void tcpSelect(struct node *nodo, char IP[16], char TCP[6])
{
    int opt = TRUE;  
    int master_socket , addrlen , new_socket , client_socket[30] , 
          max_clients = 30 , activity, i , valread , sd;  
    int max_sd;  
    struct sockaddr_in address; 

    int tcpPort = atoi(TCP);

    fd_set readfds;

    for (i = 0; i < max_clients; i++)  
    {  
        client_socket[i] = 0;  
    }  

    if((master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) exit(1); //abrir socket TCP

    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ) exit(1);

    address.sin_family = AF_INET;  //IPv4
    address.sin_addr.s_addr = INADDR_ANY; //aceitar de qualquer ip  
    address.sin_port = htons(tcpPort); //definir port dado
    
    if(bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0) exit(1); /

    if(listen(master_socket, 3) < 0) exit(1);

    addrlen = sizeof(address);

    //ciclo while para o select

}
