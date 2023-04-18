#include "cot.h"
#include "udp.h"


int commUDP(char mensagem[], char buffer[], char regIP[], char regUDP[])
{
    struct addrinfo hints, *res;
    int fd, error;
    ssize_t n;
    struct sockaddr addr;
    socklen_t addrlen;

    struct timeval tv;
    tv.tv_sec = 5; //timeout de 5 segundos
    tv.tv_usec = 0;

    fd = socket(AF_INET,SOCK_DGRAM,0); //abrir socket UDP
    if(fd == -1)
    {
        printf("Error opening UDP Socket - Exiting...\n");
        exit(1);
    }

    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) //definir timeout nas opcoes do socket
    {
        printf("Error setting UDP socket options - Exiting...\n");
        exit(1);
    }

    memset(&hints,0,sizeof hints);
    hints.ai_family = AF_INET; //ipv4
    hints.ai_socktype = SOCK_DGRAM; //udp

    error = getaddrinfo(regIP, regUDP, &hints, &res); //ler endereço
    if(error != 0) exit(1);

    n = sendto(fd, mensagem, strlen(mensagem), 0, res->ai_addr, res->ai_addrlen); //enviar mensagem
    if(n == -1)
    {
        printf("Error communicating with nodeserver. Are you connected to the internet?\nExiting...");
        exit(1);
    } 

    addrlen=sizeof(addr);
    if(recvfrom(fd, buffer, 1024, 0, &addr, &addrlen) < 0)//receber mensagem -- verificar timeout
    {
        freeaddrinfo(res);
        close(fd);
        printf("UDP Timeout reached.\n");
        return -1;
    }

    struct sockaddr_in *addr_in = (struct sockaddr_in *)&addr; //verificar se mensagem vem de destinatário
    if(strcmp(inet_ntoa(addr_in->sin_addr), regIP) == 0)
    {
        freeaddrinfo(res);
        close(fd);
        return 0;
    }
    else
    {
        printf("Received unexpected message - exiting");
        freeaddrinfo(res);
        close(fd);
        exit(1);
    }
    return 1;
}