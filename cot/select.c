#include "cot.h"
#include "select.h"
#include "udp.h"
#include "aux.h"
#include "commTCP.h"

void tcpSelect(struct node *nodo, char regIP[16], char regUDP[6], char *net)
{
    int server_fd, max_fd, selfClient_fd = 0, client_fds[100];
    int num_clients = 0;
    int optval = 1;
    int i, j, fds;
    int fn = 0; //indicador primeiro nó
    int errcode = 0;
    int nNodes = 0;
    char line[32];
    char buffer[1024+1], input[128+1], message[128+1];

    char word_array[6][128];
    int word_count = 0;
    char *token;

    memset(buffer, 0, sizeof(buffer));
    memset(input, 0, sizeof(input));
    memset(message, 0, sizeof(message));

    char *command, *arg1, *arg2;

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
        snprintf(message, sizeof(message), "%s %s %s %s%s", "NEW", nodo->id, nodo->ip, nodo->port, "\n");
        send(selfClient_fd, message , strlen(message) , 0 );
        fn = 0;
    }
    else fn = 1;

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) exit(1);

    for (int i = 0; i < 100; i++) client_fds[i] = -1;

    if(listen(server_fd, 100) == -1) exit(1);

    FD_ZERO(&read_fds);

    //max_fd = server_fd > STDIN_FILENO ? server_fd : STDIN_FILENO;

    //max_fd = max3(server_fd, STDIN_FILENO, selfClient_fd);

    max_fd = max(server_fd, STDIN_FILENO);
    if(selfClient_fd > max_fd) max_fd = selfClient_fd;

    printf("Type help to display available commands.\n");
    printf("Enter a command:\n");

    while(1)
    {
        if(fn == 0)FD_SET(selfClient_fd, &read_fds);
        FD_SET(server_fd, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        for (i = 0 ; i < 100 ; i++)
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
                FD_CLR(selfClient_fd, &read_fds);
                if(commTCP(selfClient_fd, nodo, regIP, regUDP, net, selfClient_fd, client_fds) == 1) //externo saiu, enviar a novo externo NEW
                {
                    if(strcmp(nodo->bck, nodo->id) == 0)  //se for ancora e sair externo, tem de promover interno caso seja possivel
                    {//verificar se tem internos, promover se tiver, ext = id se nao
                        if(num_clients < 1) //nao tem internos
                        {
                            strcpy(nodo->ext, nodo->id);
                            strcpy(nodo->ipExt, nodo->ip);
                            strcpy(nodo->portExt, nodo->port);
                            close(selfClient_fd);
                            fn = 1;
                        }
                        else
                        {
                            for(i=0;i<100;i++)
                            {
                                if(strcmp(nodo->intr[i], "\0") != 0) //primeiro interno que encontra, cujo fd é i
                                {
                                    strcpy(nodo->ext, nodo->intr[i]);
                                    strcpy(nodo->ipExt, nodo->ipIntr[i]);
                                    strcpy(nodo->portExt, nodo->portIntr[i]);
                                    memset(nodo->ipIntr[i], 0, sizeof(nodo->ipIntr[i]));
                                    memset(nodo->portIntr[i], 0, sizeof(nodo->portIntr[i]));
                                    memset(nodo->intr[i], 0, sizeof(nodo->intr[i]));
                                    memset(message, 0, sizeof(message));
                                    snprintf(message, sizeof(message), "%s %s %s %s%s", "EXTERN", nodo->ext, nodo->ipExt, nodo->portExt, "\n");
                                    send(i, message, sizeof(message), 0);
                                    close(selfClient_fd);
                                    fn = 1;
                                    break;
                                }
                            }
                        }

                    }
                    else
                    {

                        strcpy(nodo->ext, nodo->bck);
                        strcpy(nodo->ipExt, nodo->ipBck);
                        strcpy(nodo->portExt, nodo->portBck);

                        close(selfClient_fd);

                        selfClient_fd = socket(AF_INET,SOCK_STREAM,0);

                        external_addr.sin_addr.s_addr = inet_addr(nodo->ipExt); //IP DO EXTERNO
                        external_addr.sin_port = htons(atoi(nodo->portExt)); //PORTA DO EXTERNO
                        if(connect(selfClient_fd, (struct sockaddr *)&external_addr, sizeof(external_addr)) != 0) exit(1);
                        memset(message, 0, sizeof(message));
                        snprintf(message, sizeof(message), "%s %s %s %s%s", "NEW", nodo->id, nodo->ip, nodo->port,"\n");
                        send(selfClient_fd, message , strlen(message) , 0 );
                        if(selfClient_fd > max_fd) max_fd = selfClient_fd;
                    }
                }
            }
        }

        for (i = 0; i < 100; i++) //atividade num interno
        {
            fds = client_fds[i];

            if (FD_ISSET(fds, &read_fds))
            {
                FD_CLR(fds, &read_fds);
                errcode = commTCP(fds, nodo, regIP, regUDP, net, selfClient_fd, client_fds);
                if(errcode == 0) // saida de interno
                {
                    close(fds);
                    num_clients--;
                    client_fds[i] = -1;
                }
                if(errcode == 1) //saida de outra ancora - promove interno a ancora
                {
                    close(fds);
                    num_clients--;
                    client_fds[i] = -1;
                    if(num_clients < 1) //nao tem clientes, esta sozinho na rede
                    {
                        strcpy(nodo->ext, nodo->id);
                        strcpy(nodo->ipExt, nodo->ip);
                        strcpy(nodo->portExt, nodo->port);
                    }
                    else
                    {
                        for(j=0;j<100;j++)
                        {
                            if(strcmp(nodo->intr[j], "\0") != 0) //primeiro interno que encontra, cujo fd é i
                            {
                                strcpy(nodo->ext, nodo->intr[j]);
                                strcpy(nodo->ipExt, nodo->ipIntr[j]);
                                strcpy(nodo->portExt, nodo->portIntr[j]);
                                memset(nodo->ipIntr[j], 0, sizeof(nodo->intr[j]));
                                memset(nodo->portIntr[j], 0, sizeof(nodo->intr[j]));
                                memset(nodo->intr[j], 0, sizeof(nodo->intr[j]));
                                memset(message, 0, sizeof(message));
                                snprintf(message, sizeof(message), "%s %s %s %s%s", "EXTERN", nodo->ext, nodo->ipExt, nodo->portExt,"\n");
                                send(j, message, sizeof(message), 0);
                                break;
                            }
                        }
                    }

                }

                //if(num_clients == 0) max_fd = server_fd > STDIN_FILENO ? server_fd : STDIN_FILENO;
                if(num_clients == 0) max_fd = max3(server_fd, STDIN_FILENO, selfClient_fd);
            }
        }


        if (FD_ISSET(STDIN_FILENO, &read_fds)) //input teclado
        {
            fgets(input, sizeof(input), stdin);

            if(strcmp(input, "\n") == 0)
            {
                command = input;
                memset(input, 0, sizeof(input));
            }
            else
            {
                word_count = 0;
                token = NULL;
                input[strcspn(input, "\n")] = '\0';

                token = strtok(input, " ");
                while (token != NULL && word_count < 20)
                {
                    strcpy(word_array[word_count++], token);
                    token = strtok(NULL, " ");
                }

                command = word_array[0];

                memset(input, 0, sizeof(input));
            }

            if(strcmp(command, "join") == 0) printf("Error: Already joined.\n");

            else if(strcmp(command, "djoin") == 0) printf("Error: Already joined.\n");

            else if(strcmp(command, "leave") == 0)
            {
                memset(message,0,sizeof(message));
                memset(buffer,0,sizeof(buffer));
                snprintf(message, sizeof(message), "%s %s %s", "UNREG", net, nodo->id);
                if(strncmp(message, "UNREG", 5) != 0) printf("Erro");
                //if(snprintf(message, sizeof(message), "%s %s %s", "UNREG", "105", nodo->id) !=0) exit(1); //erro neste snprintf
                errcode = commUDP(message, buffer, regIP, regUDP);
                if(errcode == 1)
                {
                    printf("UDP Error. Exiting");
                    free(nodo);
                    exit(1);
                }
                if(errcode == -1)
                {
                    printf("Could not communicate with node server.\n");
                    exit(1);
                }
                if(strcmp(buffer, "OKUNREG") == 0) printf("Unreg successful - leaving network...\n");
                FD_ZERO(&read_fds);
                close(server_fd);
                close(selfClient_fd);
                for(i = 0; i < 100; i++)
                {
                    fds = client_fds[i];
                    close(fds);
                }
                memset(message,0,sizeof(message));
                memset(buffer,0,sizeof(buffer));
                free(nodo);
                printf("Left the network, exiting...\n");
                exit(0);
            }

            else if(strcmp(command, "st") == 0)
            {
                printf("Node %s topology:\nExtern:\n%s %s %s\nBackup:\n%s %s %s\nIntern:\n", nodo->id,
                nodo->ext, nodo->ipExt, nodo->portExt, nodo->bck, nodo->ipBck, nodo->portBck);
                j=0;
                for(i=0;i<100;i++)
                {
                    if(strcmp(nodo->intr[i], "\0") != 0) 
                    {
                        printf("%s %s %s\n",
                        nodo->intr[i], nodo->ipIntr[i], nodo->portIntr[i]);
                        j++;
                    }   
                }
                if(j == 0) printf("None\n");
            }

            else if(strcmp(command, "create") == 0)
            {
                if(word_count == 1) printf("Content name not found.\n");
                else
                {
                    arg1 = word_array[1];
                    strcpy(nodo->content[nodo->ncontents], arg1);
                    printf("Content %s added. Number of contents: %d.\n", nodo->content[nodo->ncontents], nodo->ncontents+1);
                    nodo->ncontents++;
                }
            }
            else if(strcmp(command, "delete") == 0)
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
                            printf("Content %s deleted.\n", arg1);
                            nodo->ncontents--;
                            break;
                        }
                    }
                }
                if(errcode == nodo->ncontents) printf("Content not found.\n");

            }
            else if(strcmp(command, "sn") == 0)
            {
                if(nodo->ncontents == 0) printf("No contents in node.\n");
                else
                {
                    printf("List of contents:\n");
                    for(i=0;i<32;i++)
                    {
                        if(strcmp(nodo->content[i], "\0") != 0) printf("%d. %s\n", i+1, nodo->content[i]);
                    }
                }
            }
            else if(strcmp(command, "exit") == 0)
            {
                printf("Are you sure you want to exit without leaving the network? (y/n)\n");
                fgets(input, sizeof(input), stdin);
                if(strcmp(input, "y") == 0)
                {
                    free(nodo);
                    exit(0);
                }
                else printf("Resuming...\n");
            }

            else if(strcmp(command, "get") == 0)
            {
                if(word_count != 3 ) printf("Invalid Input.\n");

                int k=0;
                arg1 = word_array[1];
                arg2 = word_array[2];

                memset(message,0,sizeof(message));
                memset(buffer,0,sizeof(buffer));

                snprintf(message, sizeof(message), "%s %s", "NODES", "869"); // MUDAR PARA O ID DA REDE

                if(strncmp(message, "NODES", 5) != 0) printf("Erro");
                errcode = commUDP(message, buffer, regIP, regUDP);
                if(errcode == 1)
                {
                    printf("UDP Error. Exiting");
                    free(nodo);
                    exit(1);
                }
                if(errcode == -1)
                {
                    printf("Could not communicate with node server.\n");
                    exit(1);
                }

                for (i=0; buffer[i]; i++) nNodes += (buffer[i] == '\n');
                nNodes--;

                findNode(buffer, line, nNodes, arg1);

                if(strcmp(line, "\0") == 0)
                {
                    printf("Node %s not found in network.\n", arg1); // neste if eno proximo ele tem de aceitar o comando seguinte (ta a crashar)
                }
                if(strcmp(arg1, nodo->id) == 0)
                {
                    for (k = 0; k < 32; k++)
                    {
                        if(nodo->ncontents == 0)
                        {
                            printf("Content list empty.\n");
                            break;
                        }
                        if(k == nodo->ncontents)
                        {
                            printf("File not found.\n");
                            break;
                        }
                        if(strcmp(nodo->content[k], arg2) == 0)
                        {
                            printf("File found.\n");
                            break;
                        }
                    }

                }
                else
                {
                    memset(message,0,sizeof(message));
                    snprintf(message, sizeof(message), "%s %s %s %s%s", "QUERY", arg1, nodo->id, arg2,"\n");

                    if(selfClient_fd > 0)
                    {
                        send(selfClient_fd, message, strlen(message), 0);
                    }
                    for (int l = 0; l < 100; l++)
                    {
                        if(client_fds[l] != -1)
                        {
                            send(client_fds[l], message, strlen(message), 0);
                        }
                    }
                }
            }
            else if(strcmp(command, "help") == 0)
            {
                printf("Available commands:\n");
                printf("create - Create a new content\ndelete - Delete existing content\nsn - Show contents\n");
                printf("get - Search for content in other node\nst - Show node topology\nsr - Show node routing\n");
                printf("leave - Leave the network and close application\nexit - Close application without leaving\n");
            }

            else printf("Command not recognized.\n");

            FD_CLR(STDIN_FILENO, &read_fds);

        }
    }
}