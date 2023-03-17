#include "cot.h"
#include "udp.h"
#include "select.h"

int main()
{
    struct node *nodo = (struct node*) malloc (sizeof(struct node));
    char *IP = "127.127.127.127";
    char *TCP = "58001";
    tcpSelect(nodo, IP, TCP);
    return 0;


}

void tcpSelect(struct node *nodo, char IP[16], char TCP[6])
{
    int server_fd, max_fd, client_fds[100];
    int num_clients = 0;
    int optval = 1;
    int i, fds;
    int valread;
    char buffer[128+1];
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addrlen = sizeof(client_addr);
    fd_set read_fds;

    server_fd = socket(AF_INET,SOCK_STREAM,0); //abrir socket TCP
    if(server_fd == -1) exit(1);

    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) exit(1);

    server_addr.sin_family = AF_INET; //IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY; //aceitar de qualquer ip
    server_addr.sin_port = htons(atoi(TCP)); //port

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) exit(1);

    for (int i = 0; i < 100; i++) client_fds[i] = -1;

    if(listen(server_fd, 100) == -1) exit(1);

    FD_ZERO(&read_fds);
    FD_SET(server_fd, &read_fds);
    FD_SET(STDIN_FILENO, &read_fds);

    max_fd = server_fd > STDIN_FILENO ? server_fd : STDIN_FILENO;

    while(1)
    {
        for ( i = 0 ; i < 100 ; i++)  
        {  
            fds = client_fds[i];  
                 
            if(fds > 0)  
                FD_SET(fds, &read_fds);  
                 
            if(fds > max_fd)  
                max_fd = fds;  
        }


        if(select(max_fd +1, &read_fds, NULL, NULL, NULL) == -1) exit(1);
        

        if(FD_ISSET(server_fd, &read_fds))
        {
            int client_fd;
            client_fd = accept(server_fd, (struct sockaddr*)&client_addr, & client_addrlen);
            if(client_fd == -1) exit(1);

            printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , client_fd, inet_ntoa(client_addr.sin_addr) , ntohs
                  (client_addr.sin_port));  

            for (int i = 0; i < 100; i++)
            {
                if (client_fds[i] == -1)
                {
                    client_fds[i] = client_fd;
                    num_clients++;
                    break;
                }
            }
        }

        //else some other socket
        for (i = 0; i < 100; i++)  
        {  
            fds = client_fds[i];  
                 
            if (FD_ISSET(fds, &read_fds))  
            {  
                //Check if it was for closing , and also read the 
                //incoming message 
                if ((valread = read(fds, buffer, 129)) == 0)  
                {  
                    //Somebody disconnected , get his details and print 
                    getpeername(fds, (struct sockaddr*)&server_addr , \
                        (socklen_t*)sizeof(server_addr));  
                    printf("Host disconnected , ip %s , port %d \n" , 
                          inet_ntoa(server_addr.sin_addr) , ntohs(server_addr.sin_port));  
                         
                    //Close the socket and mark as 0 in list for reuse 
                    close(fds);  
                    client_fds[i] = 0;  
                }  
                     
                //Echo back the message that came in 
                else 
                {  
                    //set the string terminating NULL byte on the end 
                    //of the data read 
                    buffer[valread] = '\0';  
                    send(fds, buffer , strlen(buffer) , 0 );  
                } 
            } 
        } 

        if (FD_ISSET(STDIN_FILENO, &read_fds))
        {
            fgets(buffer, 129, stdin);
            printf("Received input from stdin: %s", buffer);
        }
    }




}
