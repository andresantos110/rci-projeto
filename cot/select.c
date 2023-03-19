#include "cot.h"
#include "select.h"

void tcpSelect(struct node *nodo, char IP[16], char TCP[6], char infoExt[32], char regIP[16], char regUDP[16])
{
    int server_fd, max_fd, selfClient_fd, client_fds[100];
    int num_clients = 0;
    int optval = 1;
    int i, fds;
    int valread;
    char buffer[128+1], input[128+1], message[128+1];
    char ipExt[16], tcpExt[6];
    char auxIP[16], auxTCP[6];

    char *command, *arg1, *arg2, *arg3, *arg4, *arg5;

    struct sockaddr_in server_addr, client_addr, external_addr;
    socklen_t client_addrlen = sizeof(client_addr);
    fd_set read_fds;

    server_fd = socket(AF_INET,SOCK_STREAM,0); //abrir socket TCP
    if(server_fd == -1) exit(1);

    selfClient_fd = socket(AF_INET,SOCK_STREAM,0); //abrir socket TCP de cliente(self)
    if(server_fd == -1) exit(1);

    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) exit(1);

    server_addr.sin_family = AF_INET; //IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY; //aceitar de qualquer ip
    server_addr.sin_port = htons(atoi(TCP)); //port

    if(strcmp(infoExt, "\0") != 0) // verificar se é primeiro nó, entra no if se nao for
    {
        sscanf(infoExt, "%s %s %s", nodo->ext, ipExt, tcpExt);
        external_addr.sin_family = AF_INET; //IPv4
        external_addr.sin_addr.s_addr = inet_addr(ipExt); //IP DO EXTERNO
        external_addr.sin_port = htons(atoi(tcpExt)); //PORTA DO EXTERNO
        if(connect(selfClient_fd, (struct sockaddr *)&external_addr, sizeof(external_addr)) != 0) exit(1);
        snprintf(message, sizeof(message), "%s %s %s %s", "NEW", nodo->id, IP, TCP);
        send(selfClient_fd, message , strlen(message) , 0 ); 
    }

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) exit(1);

    for (int i = 0; i < 100; i++) client_fds[i] = -1;

    if(listen(server_fd, 100) == -1) exit(1);

    FD_ZERO(&read_fds);

    //max_fd = server_fd > STDIN_FILENO ? server_fd : STDIN_FILENO;

    max_fd = max(server_fd, STDIN_FILENO);

    while(1)
    {

        FD_SET(server_fd, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        if(strcmp(nodo->ext, "\0") == 0) FD_SET(selfClient_fd, &read_fds);

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
            FD_CLR(server_fd, &read_fds);

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
                FD_CLR(fds, &read_fds);

                if ((valread = read(fds, buffer, 129)) == 0)  
                {  
                    //saiu
                    getpeername(fds, (struct sockaddr*)&server_addr , \
                        (socklen_t*)sizeof(server_addr));  
                    printf("Host disconnected , ip %s , port %d \n" , 
                          inet_ntoa(client_addr.sin_addr) , ntohs(client_addr.sin_port));  
                         
                    //fechar socket 
                    close(fds);  
                    num_clients--;
                    client_fds[i] = -1;  
                }               
                //Echo da mensagem - a mudar para comunicacao
                else 
                {  
                    if(strstr(buffer, "NEW") != NULL)
                    {
                        if(strcmp(nodo->ext, "\0"))
                        {

                            sscanf(buffer, "%s %s %s %s", command, nodo->ext, auxIP, auxTCP);
                            strcpy(ipExt, auxIP);
                            strcpy(tcpExt, auxTCP);
                            external_addr.sin_family = AF_INET; //IPv4
                            external_addr.sin_addr.s_addr = inet_addr(ipExt); //IP DO EXTERNO
                            external_addr.sin_port = htons(atoi(tcpExt)); //PORTA DO EXTERNO
                            if(connect(selfClient_fd, (struct sockaddr *)&external_addr, sizeof(external_addr)) != 0) exit(1);

                        }
                        else
                        {
                            //caso nao seja o primeiro nó
                        }
                    }  
                } 
                //if(num_clients == 0) max_fd = server_fd > STDIN_FILENO ? server_fd : STDIN_FILENO;
                if(num_clients == 0) max_fd = max(server_fd, STDIN_FILENO);
            } 
        } 


        if (FD_ISSET(STDIN_FILENO, &read_fds))
        {
            fgets(input, sizeof(input), stdin);

            input[strcspn(input, "\n")] = 0; 

            char *word_array[6]; 
            int word_count = 0;

            char *token = strtok(input, " ");
            while (token != NULL && word_count < 20) {
                word_array[word_count++] = token;
                token = strtok(NULL, " ");
            }

            command = word_array[0];

            if(strcmp(command, "join") == 0) printf("Error: Already joined.\n");
            if(strcmp(command, "djoin") == 0) printf("Error: Already joined.\n");
            if(strcmp(command, "leave") == 0)
            {
                arg1 = word_array[1];
                if(snprintf(message, sizeof(message), "%s %s %s", "UNREG", arg1, nodo->id) !=0) exit(1);
                if(commUDP(message, buffer, regIP, regUDP) != 0) exit(1);
                printf("Enviada: %s\nRecebida: %s\n", message, buffer);
                if(strcmp(buffer, "OKUNREG") == 0) printf("Unreg successful.");     
            }
        }
    }




}

void commTCP(int fd)
{

}