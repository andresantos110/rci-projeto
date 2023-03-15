#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int server_fd, max_fd, client_fds[MAX_CLIENTS], num_clients = 0;
    fd_set read_fds;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addrlen = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // Create TCP socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set socket options to reuse address and port
    int optval = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, sizeof(optval)) == -1) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Bind socket to port
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(12345);
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, MAX_CLIENTS) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Initialize client_fds array
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_fds[i] = -1;
    }

    // Add server_fd and stdin to read_fds set
    FD_ZERO(&read_fds);
    FD_SET(server_fd, &read_fds);
    FD_SET(STDIN_FILENO, &read_fds);
    max_fd = server_fd > STDIN_FILENO ? server_fd : STDIN_FILENO;

    // Main loop
    while (1) {
        // Wait for activity on one of the file descriptors
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        // Check if server_fd has incoming connection
        if (FD_ISSET(server_fd, &read_fds)) {
            int client_fd;
            if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addrlen)) == -1) {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            printf("New connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            // Add new client_fd to client_fds array
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_fds[i] == -1) {
                    client_fds[i] = client_fd;
                    num_clients++;
                    break;
                }
            }
            // Add new client_fd to read_fds set
            FD_SET(client_fd, &read_fds);
            if (client_fd > max_fd) {
                max_fd = client_fd;
            }
        }

        // Check if stdin has input
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            fgets(buffer, BUFFER_SIZE, stdin);
            printf("Received input from stdin: %s", buffer);
        }

       
