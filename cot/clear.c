#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
    struct addrinfo hints, *res;
    int fd, error;
    ssize_t n;
    struct sockaddr addr;
    socklen_t addrlen;

    int i;
    char stringdoint[3];
    char aux[3];
    char mensagem[128];

    fd = socket(AF_INET,SOCK_DGRAM,0); //abrir socket UDP
    if(fd == -1) exit(1);

    if(argc != 2)
    {
        printf("falta argumentos");
        exit(1);
    }

    memset(&hints,0,sizeof hints);
    hints.ai_family = AF_INET; //ipv4
    hints.ai_socktype = SOCK_DGRAM; //udp
    
    error = getaddrinfo("193.136.138.142", "59000", &hints, &res); //ler endereço
    if(error != 0) exit(1);

    for(i=0;i<100;i++)
    {
        sprintf(stringdoint, "%d", i);
        if(strlen(stringdoint) == 1)
        {
            strcpy(aux, "0");
            strcat(aux, stringdoint);
            strcpy(stringdoint, aux);
        }
    snprintf(mensagem, sizeof(mensagem), "%s %s %s", "UNREG", argv[1], stringdoint);
    n = sendto(fd, mensagem, strlen(mensagem), 0, res->ai_addr, res->ai_addrlen); //enviar mensagem
    if(n == -1) exit (1);

    }

    printf("NET %s CLEARED\n", argv[1]);

}
