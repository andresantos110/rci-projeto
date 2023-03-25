#include "cot.h"
#include "select.h"
#include "udp.h"

void tcpSelect(struct node *nodo, char regIP[16], char regUDP[6])
{
    int server_fd, max_fd, selfClient_fd, client_fds[100];
    int num_clients = 0;
    int optval = 1;
    int i, fds;
    int fn = 0; //indicador primeiro nó
    int errcode;
    char buffer[1024+1], input[128+1], message[128+1];

    memset(buffer, 0, sizeof(buffer));
    memset(input, 0, sizeof(input));
    memset(message, 0, sizeof(message));

    char *command, *arg1, *arg2, *arg3, *arg4, *arg5;

    struct sockaddr_in server_addr, client_addr, external_addr;
    socklen_t client_addrlen = sizeof(client_addr);
    fd_set read_fds;

    server_fd = socket(AF_INET,SOCK_STREAM,0); //abrir socket TCP
    if(server_fd == -1) exit(1);

    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) exit(1);

    server_addr.sin_family = AF_INET; //IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY; //aceitar de qualquer ip
    server_addr.sin_port = htons(atoi(nodo->port)); //port

    if(strcmp(nodo->ext, nodo->id) != 0) // verificar se é primeiro nó, entra no if se nao for
    {
        selfClient_fd = socket(AF_INET,SOCK_STREAM,0); //abrir socket TCP de cliente(self)
        if(selfClient_fd == -1) exit(1);

        external_addr.sin_family = AF_INET; //IPv4
        external_addr.sin_addr.s_addr = inet_addr(nodo->ipExt); //IP DO EXTERNO
        external_addr.sin_port = htons(atoi(nodo->portExt)); //PORTA DO EXTERNO
        if(connect(selfClient_fd, (struct sockaddr *)&external_addr, sizeof(external_addr)) != 0) exit(1);
        snprintf(message, sizeof(message), "%s %s %s %s", "NEW", nodo->id, nodo->ip, nodo->port);
        send(selfClient_fd, message , strlen(message) , 0 );
        fn = 0;
    }
    else fn = 1;

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) exit(1);

    for (int i = 0; i < 100; i++) client_fds[i] = -1;

    if(listen(server_fd, 100) == -1) exit(1);

    FD_ZERO(&read_fds);

    //max_fd = server_fd > STDIN_FILENO ? server_fd : STDIN_FILENO;

    max_fd = max(server_fd, STDIN_FILENO);

    while(1)
    {
        FD_SET(selfClient_fd, &read_fds);
        FD_SET(server_fd, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        for ( i = 0 ; i < 100 ; i++)
        {
            fds = client_fds[i];

            if(fds > 0)
                FD_SET(fds, &read_fds);

            if(fds > max_fd)
                max_fd = fds;
        }


        if(select(max_fd +1, &read_fds, NULL, NULL, NULL) == -1) exit(1);

        if(FD_ISSET(server_fd, &read_fds)) //ligacao ao servidor
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
        if(fn == 0)
        {
            if(FD_ISSET(selfClient_fd, &read_fds)) //atividade no externo - nao esta a entrar?
            {
                commTCP(selfClient_fd, nodo);
                FD_CLR(selfClient_fd, &read_fds);
            }
        }

        for (i = 0; i < 100; i++) //atividade num interno
        {
            fds = client_fds[i];

            if (FD_ISSET(fds, &read_fds))
            {
                if(commTCP(fds, nodo) == 0) // o read = 0 da merda?
                {
                    close(fds);
                    num_clients--;
                    client_fds[i] = -1;
                }
                else printf("Informação do nó:\nid: %s\next: %s\nbck: %s\n", nodo->id, nodo->ext, nodo->bck);
                //remover else, para teste apenas
                FD_CLR(fds, &read_fds);

                /*if ((errcode = read(fds, buffer, 1024)) == 0)
                {
                    printf("ha merda ");
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
                //Comunicação entre nós
                else
                {

                    buffer[errcode] = '\0';
                    send(fds, buffer , strlen(buffer) , 0 );
                }*/
                //if(num_clients == 0) max_fd = server_fd > STDIN_FILENO ? server_fd : STDIN_FILENO;
                if(num_clients == 0) max_fd = max(server_fd, STDIN_FILENO);
            }
        }


        if (FD_ISSET(STDIN_FILENO, &read_fds)) //input teclado
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
                memset(message,0,sizeof(message));
                memset(buffer,0,sizeof(buffer));
                snprintf(message, sizeof(message), "%s %s %s", "UNREG", "820", nodo->id);
                if(strncmp(message, "UNREG", 5) != 0) printf("Erro");
                //if(snprintf(message, sizeof(message), "%s %s %s", "UNREG", "105", nodo->id) !=0) exit(1); //erro neste snprintf
                if(commUDP(message, buffer, regIP, regUDP) != 0) exit(1);
                printf("Enviada: %s\nRecebida: %s\n", message, buffer);
                if(strcmp(buffer, "OKUNREG") == 0) printf("Unreg successful - leaving network...\n");
                FD_ZERO(&read_fds);
                close(server_fd);
                close(selfClient_fd);
                for(i = 0; i < 100; i++)
                {
                    fds = client_fds[i];
                    close(fds);
                }
                return;
            }

            if(strcmp(command, "st") == 0)
            {
                printf("Problema: %s\n", nodo->portExt);
                printf("Node %s topology:\nExtern: %s %s %s\nBackup: %s %s %s\nIntern:\n", nodo->id,
                nodo->ext, nodo->ipExt, nodo->portExt, nodo->bck, nodo->ipBck, nodo->portBck);
                for(i=0;i<100;i++)
                {
                    if(strcmp(nodo->intr[i], "\0") != 0) printf("%s %s %s\n",
                    nodo->intr[i], nodo->ipIntr[i], nodo->portIntr[i]);
                }
            }

            if(strcmp(command, "create") == 0)
            {
                if(word_count == 1) printf("Content name not found.\n");
                else
                {
                    arg1 = word_array[1];
                    nodo->content[nodo->ncontents] = calloc(strlen(arg1)+1, sizeof(char));
                    strcpy(nodo->content[nodo->ncontents], arg1);
                    printf("Content %s added. Number of contents: %d.\n", nodo->content[nodo->ncontents], nodo->ncontents+1);
                    nodo->ncontents++;
                }
            }
            if(strcmp(command, "delete") == 0)
            {
                arg1 = word_array[1];
                errcode = nodo->ncontents;
                if(nodo->ncontents == 0) printf("There are no contents.\n");
                else
                {
                    for(i=0;i<nodo->ncontents;i++)
                    {
                        if(strcmp(nodo->content[i], arg1) == 0)
                        {
                            strcpy(nodo->content[i], "\0");
                            free(nodo->content[i]);
                            printf("Content %s deleted.\n", arg1);
                            nodo->ncontents--;
                            break;
                        }
                    }
                }
                if(errcode == nodo->ncontents) printf("Content not found.\n");

            }
            if(strcmp(command, "sn") == 0)
            {
                if(nodo->ncontents == 0) printf("No contents in node.\n");
                else
                {
                    printf("List of contents:\n");
                    for(i=0;i<nodo->ncontents;i++)
                    {
                        printf("%d. %s\n", i+1, nodo->content[i]);
                    }
                }
            }




        }
    }




}


int commTCP(int fd, struct node *nodo) //funcao a ser chamada quando ha atividade no fd de uma ligacao tcp.
{ //meter um return -1 caso haja erros
    char buffer[1024+1];
    char message[1024+1];
    char command[16], arg1[32], arg2[32], arg3[32];
    int i = 0;

    //if(read(fd, buffer, 1024) == 0) return 0;
    //else return 1;

    memset(buffer, 0, sizeof(buffer));
    memset(message, 0, sizeof(message));
    memset(command, 0, sizeof(command));
    memset(arg1, 0, sizeof(arg1));
    memset(arg2, 0, sizeof(arg2));
    memset(arg3, 0, sizeof(arg3));

    //verificar se foi saída
    if(read(fd, buffer, 1024) == 0) //return 0 caso o nó onde houve atividade tenha saido da rede.
    {

        //INSERIR ENVIO DE WITHDRAW AQUI
        for(i = 0;i < 100;i++)
        {
            if(strcmp(nodo->intr[i], "\0") != 0 && i == fd) //saiu um interno
            {
                strcpy(nodo->intr[i], "\0");
                strcpy(nodo->ipIntr[i], "\0");
                strcpy(nodo->portIntr[i], "\0"); //limpar informacao do nó que saiu
                return 0;
            }
        }

	//caso saia o outro nó ancora, tem de promover um interno a externo
	    if(strcmp(nodo->bck, nodo->id) == 0)
        {
            for(i = 0;i < 100;i++)
            {
                if(strcmp(nodo->intr[i], "\0") != 0) //saiu um interno
                {
                    strcpy(nodo->ext, nodo->intr[i]);
                    strcpy(nodo->ipExt, nodo->ipIntr[i]);
                    strcpy(nodo->portExt, nodo->portIntr[i]); //promover primeiro interno que encontrar a externo
                    break;
                }
            }
            if(i != 100)
            {
                snprintf(message, sizeof(message), "%s %s %s %s", "EXTERN", nodo->ext, nodo->ipExt, nodo->portExt);
                send(fd, message, strlen(message), 0);
            }
            else
            {
                strcpy(nodo->ext, "-1");
                strcpy(nodo->ipExt, "\0");
                strcpy(nodo->portExt, "\0"); //nao tem internos, ficou sozinho na rede
            }
        }
        else
        {
            strcpy(nodo->ext, nodo->bck);
            strcpy(nodo->ipExt, nodo->ipBck);
            strcpy(nodo->portExt, nodo->portBck);
            snprintf(message, sizeof(message), "%s %s %s %s", "NEW", nodo->id, nodo->ip, nodo->port);
            send(fd, message, strlen(buffer), 0);
        }
        return 0;
    }
    else //caso exista comunicacao, return 1.
    {
        printf("RECEBIDO: %s\n", buffer);
        /*verificar o que recebeu
            new
            extern
            withdraw
            query
            content
            nocontent*/

        if(strstr(buffer, "NEW") != NULL) //se for new
        {
            //arg1 = id; arg2 = IP; arg3 = TCP
            if(strcmp(nodo->ext, nodo->id) == 0)
            {
                sscanf(buffer, "%s %s %s %s", command, arg1, arg2, arg3);
                strcpy(nodo->ext, arg1);
                strcpy(nodo->ipExt, arg2);
                strcpy(nodo->portExt, arg3);
                snprintf(message, sizeof(message), "%s %s %s %s", "EXTERN", nodo->ext, nodo->ipExt, nodo->portExt);
                send(fd, message, strlen(message), 0);
            }
            else //se nao for o primeiro nó
            {
                sscanf(buffer, "%s %s %s %s", command, arg1, arg2, arg3);
                strcpy(nodo->intr[fd], arg1);
                strcpy(nodo->ipIntr[fd], arg2);
                strcpy(nodo->portIntr[fd], arg3);
                snprintf(message, sizeof(message), "%s %s %s %s", "EXTERN", nodo->ext, nodo->ipExt, nodo->portExt);
                send(fd, message, strlen(message), 0);
            }
        }
        if(strstr(buffer, "EXTERN") != NULL) //se for null
        {
            sscanf(buffer, "%s %s %s %s", command, arg1, arg2, arg3);
            strcpy(nodo->bck, arg1);
            strcpy(nodo->ipBck, arg2);
            strcpy(nodo->portBck, arg3);
        }
        return 1;
    }
    return -1;

}
