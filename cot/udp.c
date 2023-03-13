#include "cot.h"
#include "udp.h"


int commUDP(char mensagem[], char buffer[], char regIP[], char regUDP[])
{
    struct addrinfo hints, *res;
    int fd, error;
    ssize_t n;
    struct sockaddr addr;
    socklen_t addrlen;

    fd = socket(AF_INET,SOCK_DGRAM,0); //abrir socket UDP
    if(fd == -1) exit(1);

    memset(&hints,0,sizeof hints);
    hints.ai_family = AF_INET; //ipv4
    hints.ai_socktype = SOCK_DGRAM; //udp

    error = getaddrinfo(regIP, regUDP, &hints, &res); //ler endereço
    if(error != 0) exit(1);

    n = sendto(fd, mensagem, strlen(mensagem), 0, res->ai_addr, res->ai_addrlen); //enviar mensagem
    if(n == -1) exit (1);

    addrlen=sizeof(addr);
    n = recvfrom(fd, buffer, 128, 0, &addr, &addrlen); //receber mensagem
    if(n == -1) exit (1);

    struct sockaddr_in *addr_in = (struct sockaddr_in *)&addr; //verificar se mensagem vem de destinatário
    if(strcmp(inet_ntoa(addr_in->sin_addr), regIP) == 0) return 0;

    freeaddrinfo(res);
    close(fd);
    return 1;
}