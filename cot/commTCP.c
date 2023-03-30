#include "cot.h"
#include "select.h"
#include "udp.h"
#include "aux.h"

int commTCP(int fd, struct node *nodo, char *regIP, char *regUDP, char *net, int selfClient_fd, int client_fds[100]) //funcao a ser chamada quando ha atividade no fd de uma ligacao tcp.
{
    //RETURN -1 ERRO
    //RETURN 0 Saida de interno
    //RETURN 1 Saida de externo
    //RETURN 2 Comunicacao normal
    char buffer[1024+1];
    char auxBuffer[1024+1];
    char message[1024+1];
    char command[16], arg1[32], arg2[32], arg3[32];
    int i = 0, k = 0, u = 0, l = 0, flg2 = 0;

    //if(read(fd, buffer, 1024) == 0) return 0;
    //else return 1;

    memset(buffer, 0, sizeof(buffer));
    memset(auxBuffer, 0, sizeof(auxBuffer));
    memset(message, 0, sizeof(message));
    memset(command, 0, sizeof(command));
    memset(arg1, 0, sizeof(arg1));
    memset(arg2, 0, sizeof(arg2));
    memset(arg3, 0, sizeof(arg3));

    strcpy(buffer, "\0");

    strcpy(arg1, "\0");
    strcpy(arg2, "\0");
    strcpy(arg3, "\0");

    //verificar se foi saída
    if(read(fd, buffer, 1024) == 0) //return 0 caso o nó onde houve atividade tenha saido da rede.
    {

        //INSERIR ENVIO DE WITHDRAW AQUI
        for(i = 0;i < 100;i++) //verificar se saiu interno
        {
            if(strcmp(nodo->intr[i], "\0") != 0 && i == fd) 
            {
                printf("Lost connection to internal node.\nType st to show new topology.\n");
                return 0;
            }
        }

        printf("Lost connection to external node.\nType st to show new topology.\n");
        return 1;
    }
    else //caso exista comunicacao, return 2.
    {
        printf("RECEBIDO: %s\n", buffer);
        /*verificar o que recebeu
            new
            extern
            withdraw
            query
            content
            nocontent*/
        if(strstr(buffer, "\n") != NULL)
        {
            if(strstr(buffer, "NEW") != NULL) //se for new
            {
                //arg1 = id; arg2 = IP; arg3 = TCP
                if(strcmp(nodo->ext, nodo->id) == 0)
                {
                    sscanf(buffer, "%s %s %s %s", command, arg1, arg2, arg3);
                    strcpy(nodo->ext, arg1);
                    strcpy(nodo->ipExt, arg2);
                    strcpy(nodo->portExt, arg3);
                    nodo->fdExt = fd;
                    snprintf(message, sizeof(message), "%s %s %s %s%s", "EXTERN", nodo->ext, nodo->ipExt, nodo->portExt, "\n");
                    send(fd, message, strlen(message), 0);
                    printf("Incoming connection from node %s...\n", nodo->ext);
                }
                else //se nao for o primeiro nó
                {
                    sscanf(buffer, "%s %s %s %s", command, arg1, arg2, arg3);
                    strcpy(nodo->intr[fd], arg1);
                    strcpy(nodo->ipIntr[fd], arg2);
                    strcpy(nodo->portIntr[fd], arg3);
                    snprintf(message, sizeof(message), "%s %s %s %s%s", "EXTERN", nodo->ext, nodo->ipExt, nodo->portExt, "\n");
                    send(fd, message, strlen(message), 0);
                    printf("Incoming connection from node %s...\n", nodo->intr[fd]);
                }
                memset(buffer,0,sizeof(buffer));
            }
            else if(strstr(buffer, "EXTERN") != NULL) //se for null
            {
                sscanf(buffer, "%s %s %s %s", command, arg1, arg2, arg3);
                if(strcmp(arg1, nodo->id) == 0) nodo->fdExt = fd;
                strcpy(nodo->bck, arg1);
                strcpy(nodo->ipBck, arg2);
                strcpy(nodo->portBck, arg3);
                memset(buffer,0,sizeof(buffer));
            }

            else if(strstr(buffer, "QUERY") != NULL) //QUERY destino origem content
            {
                flg2 = 0;

                sscanf(buffer, "%s %s %s %s", command, arg1, arg2, arg3);

                if(nodo->fdExt == fd)
                {
                    updateTable(arg2, nodo->ext, nodo);
                }
                else
                {
                    for(u = 0; u < 100; u++)
                    {
                        if(client_fds[u] == fd)
                        {
                            updateTable(arg2, nodo->intr[fd], nodo);
                        }
                    }
                }

                if(strcmp(arg1, nodo->id) == 0)
                {
                    if(nodo->ncontents == 0)
                    {
                        printf("NOCONTENT sent\n");
                        snprintf(message, sizeof(message), "%s %s %s %s%s", "NOCONTENT",  arg2, nodo->id, arg3, "\n");
                        send(fd, message, strlen(message), 0);
                        flg2 = 1;
                    }
                    if(flg2 == 0)
                    {
                        for (k = 0; k < 32; k++)
                        {
                            if(strcmp(nodo->content[k], "\0"))
                            {
                                if(strcmp(arg3, nodo->content[k]) == 0)
                                {
                                    printf("CONTENT sent\n");
                                    snprintf(message, sizeof(message), "%s %s %s %s%s", "CONTENT",  arg2, nodo->id, arg3, "\n");
                                    send(fd, message, strlen(message), 0);
                                    flg2 = 2;
                                    break;
                                }
                            }

                        }
                    }
                    if(flg2 == 0)
                    {
                        printf("NOCONTENT sent\n");
                        snprintf(message, sizeof(message), "%s %s %s %s%s", "NOCONTENT",  arg2, nodo->id, arg3, "\n");
                        send(fd, message, strlen(message), 0);
                    }
                }
                else
                {
                    snprintf(message, sizeof(message), "%s %s %s %s%s", "QUERY", arg1, arg2, arg3, "\n");

                    for(u = 0; u < 100; u++)
                    {
                        if(client_fds[u] != fd && u > 0) send(client_fds[u], message, strlen(message), 0);
                    }

                    if(selfClient_fd > 0) send(selfClient_fd, message, strlen(message), 0);
                }

                memset(buffer,0,sizeof(buffer));
            }

            else if(strstr(buffer, "CONTENT") != NULL) //NO/CONTENT DEST ORIGEM NOME
            {
                sscanf(buffer, "%s %s %s %s", command, arg1, arg2, arg3);

                printf("%d %d\n", nodo->fdExt, fd);
                if(nodo->fdExt == fd)
                {
                    updateTable(arg2, nodo->ext, nodo);
                }
                else
                {
                    for(u = 0; u < 100; u++)
                    {
                        if(client_fds[u] == fd)
                        {
                            updateTable(arg2, nodo->intr[fd], nodo);
                        }
                    }
                }
                printf("COMMAND: %s\nSIZEOF: %ld\n", command, strlen(command));
                if(strlen(command) == 9)
                {
                    if(strcmp(arg1, nodo->id) == 0)
                    {
                        printf("File not found.\n");
                        return 2;
                    }
                    else
                    {
                        for (l = 0; l < 100; l++)
                        {
                            if(strcmp(nodo->table1[l], arg1) == 0)
                            {
                                printf("Arg1 found: %s\n", nodo->table1[l]);
                                if(strcmp(nodo->table2[l], nodo->ext) == 0)
                                {
                                    printf("M1\n");
                                    send(nodo->fdExt, buffer, strlen(buffer), 0);
                                    break;
                                }
                                else
                                {
                                    for(u = 0; u < 100; u++)
                                    {
                                        printf("M3\n");
                                        if(strcmp(nodo->table2[l], nodo->intr[u]) == 0)
                                        {
                                            printf("M4\n");
                                            send(u, buffer, sizeof(buffer), 0);
                                            break;
                                        }
                                    }

                                }
                            }
                        }
                    }

                }
                else
                {
                    if(strcmp(arg1, nodo->id) == 0)
                    {
                        printf("File found.\n");
                        return 2;
                    }
                    else
                    {

                        for (l = 0; l < 100; l++)
                        {
                            if(strcmp(nodo->table1[l], arg1) == 0)
                            {
                                if(strcmp(nodo->table2[l], nodo->ext) == 0)
                                {
                                    send(nodo->fdExt, buffer, strlen(buffer), 0);
                                    break;
                                }
                                else
                                {
                                    for(k = 0; k < 100; k++)
                                    {
                                        if(strcmp(nodo->table2[l], nodo->intr[k]) == 0)
                                        {
                                            send(k, buffer, sizeof(buffer), 0);
                                            break;
                                        }
                                    }

                                }
                            }
                        }
                    }

                }
                memset(buffer,0,sizeof(buffer));
            }
            else if(strstr(buffer, "WITHDRAW") != NULL)
            {

                sscanf(buffer, "%s %s", command, arg1);

                for (int i = 0; i < 100; i++)
                {
                    if(strcmp(nodo->table1[i], arg1) == 0)
                    {
                        memset(nodo->table1[i],0,strlen(nodo->table1[i]));
                        memset(nodo->table2[i],0,strlen(nodo->table2[i]));
                        break;
                    }
                }
                for (k = 0; k < 100; k++)
                {
                    if(strcmp(nodo->table2[k], arg1) == 0)
                    {
                        memset(nodo->table1[k],0,strlen(nodo->table1[k]));
                        memset(nodo->table2[k],0,strlen(nodo->table2[k]));
                        break;
                    }
                }
                if(strcmp(arg1, nodo->ext) == 0)
                {
                    send(selfClient_fd, buffer, strlen(buffer), 0);
                }
                else
                {
                    for(u = 0; u < 100; u++)
                    {
                        if(strcmp(arg1, nodo->intr[u]) == 0)
                        {
                            send(u, buffer, sizeof(buffer), 0);
                        }
                    }

                }

            }
            else
            {
                printf("Message does not follow protocol. Closing connection to source node.\n");
                return -1;
            }
            return 2;
        }
        else
        {
            printf("Message does not follow protocol. Closing connection to source node.\n");
            return -1;
        }
    }
    return -1;

}
