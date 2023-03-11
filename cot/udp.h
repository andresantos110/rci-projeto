#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>

int commUDP(char mensagem[], char buffer[], char regIP[], char regUDP[]);