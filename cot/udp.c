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




int compare_udp_messages(char *message, char *known_chars) 
{
    char *line;
    char *next_line;
    char *token;
    char *saveptr;
    int line_count = 0;
    int found_match = 0;

    char known_char_copy[strlen(known_chars) + 1];
    strcpy(known_char_copy, known_chars);
    token = strtok_r(known_char_copy, " ", &saveptr);
    for (int i = 0; i < 2; i++) {
        token = strtok_r(NULL, " ", &saveptr);
    }
    int third_char = atoi(token);

    // Get the first line of the message
    line = strtok_r(message, "\n", &next_line);

    // Loop through the remaining lines
    while (line != NULL) {
        // Skip the first line
        if (line_count > 0) {
            // Count the number of integers in the line
            int num_integers = 0;
            char *p = line;
            while (*p) {
                if (isdigit(*p)) {
                    num_integers++;
                    while (isdigit(*p)) {
                        p++;
                    }
                } else {
                    p++;
                }
            }

            // Allocate an array of integers to hold the numbers in the line
            int *nums = malloc(sizeof(int) * num_integers);

            // Extract the numbers from the line and store them in the nums array
            p = line;
            int i = 0;
            while (*p) {
                if (isdigit(*p)) {
                    nums[i] = strtol(p, &p, 10);
                    i++;
                } else {
                    p++;
                }
            }

            // Check if the third number matches the third element of known_chars
            if (nums[0] == third_char) {
                found_match = 1;
            }

            // Free the nums array
            free(nums);
        }

        // Move to the next line
        line = strtok_r(NULL, "\n", &next_line);
        line_count++; //igual ao numero de nos
    }

    if (!found_match) return 0;
    else return 1;
}