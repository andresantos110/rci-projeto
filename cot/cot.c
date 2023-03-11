#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    struct addrinfo hints, *res;
    int fd, error;
    ssize_t n;
    char buffer[128+1];
    struct sockaddr addr;
    socklen_t addrlen;
    char *regIP, *regUDP, *IP, *TCP;

    regIP = calloc(15, sizeof(char));
    regUDP = calloc(5, sizeof(char));
    IP  = calloc(15, sizeof(char));
    TCP = calloc(5, sizeof(char));


    if(argc != 5 || argc != 3) exit(1); //inicializacao dos valores dados como argumento
    if(argc == 3) 
    {
       regIP = "193.136.138.142";
       regUDP = "59000"; 
    }
    if(argc == 5)
    {
        regIP = argv[3];
        regUDP = argv[4];
    }
    IP = argv[1];
    TCP = argv[2];

    free(regIP);
    free(regUDP);
    free(IP);
    free(TCP);

    printf("Enter a command:\n")
    fgets(command, sizeof(command), stdin);

    if(strncmp(command, "join", 4) == 0)
    {

    }

    return 0;
}