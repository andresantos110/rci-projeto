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

        if(strcmp(nodo->intr[fd], "\0") != 0)
        {
            
            for (i = 0; i < 100; i++)
            {
                if(strcmp(nodo->table1[i], nodo->intr[fd]) == 0)
                {
                    memset(nodo->table1[i],0,sizeof(nodo->table1[i]));
                    memset(nodo->table2[i],0,sizeof(nodo->table2[i]));
                    strcpy(nodo->table1[i], "\0");
                    strcpy(nodo->table2[i], "\0");
                }
                if(strcmp(nodo->table2[i], nodo->intr[fd]) == 0)
                {
                    memset(nodo->table1[i],0,sizeof(nodo->table1[i]));
                    memset(nodo->table2[i],0,sizeof(nodo->table2[i]));
                    strcpy(nodo->table1[i], "\0");
                    strcpy(nodo->table2[i], "\0");
                }
            }
            snprintf(message, sizeof(message), "%s %s%s", "WITHDRAW", nodo->intr[fd], "\n");
            for(u = 0; u < 100; u++)
            {
                if(client_fds[u] != fd && client_fds[u] > 0) send(client_fds[u], message, strlen(message), 0);
            }

            if(selfClient_fd > 0 && selfClient_fd != fd) send(selfClient_fd, message, strlen(message), 0);
            strcpy(nodo->intr[fd], "\0");
            strcpy(nodo->ipIntr[fd], "\0");
            strcpy(nodo->portIntr[fd], "\0"); //limpar informacao do nó que saiu
            printf("Lost connection to internal node. Updating routing...\nType st to show new topology.\n");
            return 0;

        }

        for (i = 0; i < 100; i++)
        {
            if(strcmp(nodo->table1[i], nodo->ext) == 0)
            {
                memset(nodo->table1[i],0,sizeof(nodo->table1[i]));
                memset(nodo->table2[i],0,sizeof(nodo->table2[i]));
                strcpy(nodo->table1[i], "\0");
                strcpy(nodo->table2[i], "\0");
            }
            if(strcmp(nodo->table2[i], nodo->ext) == 0)
            {
                memset(nodo->table1[i],0,sizeof(nodo->table1[i]));
                memset(nodo->table2[i],0,sizeof(nodo->table2[i]));
                strcpy(nodo->table1[i], "\0");
                strcpy(nodo->table2[i], "\0");
            }
        }

        snprintf(message, sizeof(message), "%s %s%s", "WITHDRAW", nodo->ext, "\n");
        for(u = 0; u < 100; u++)
        {
            if(client_fds[u] != fd && client_fds[u] > 0) send(client_fds[u], message, strlen(message), 0);
        }

        if(selfClient_fd > 0 && selfClient_fd != fd) send(selfClient_fd, message, strlen(message), 0);
        printf("Lost connection to external node. Updating routing...\nType st to show new topology.\n");
        if(strcmp(nodo->bck, nodo->id) == 0) return 1;
        else
        {
            strcpy(nodo->ext, nodo->bck);
            strcpy(nodo->ipExt, nodo->ipBck);
            strcpy(nodo->portExt, nodo->portBck);
            return 1;
        }

        return -1;
    }
    else //caso exista comunicacao, return 2.
    {
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
                    printf("Received query for content %s from node %s.\n", arg3, arg2);
                    if(nodo->ncontents == 0)
                    {
                        snprintf(message, sizeof(message), "%s %s %s %s%s", "NOCONTENT",  arg2, arg1, arg3, "\n");
                        printf("Content not found. Replying...\n");
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
                                    printf("Content found. Replying...\n");
                                    snprintf(message, sizeof(message), "%s %s %s %s%s", "CONTENT",  arg2, arg1, arg3, "\n");
                                    send(fd, message, strlen(message), 0);
                                    flg2 = 2;
                                    break;
                                }
                            }

                        }
                    }
                    if(flg2 == 0)
                    {
                        printf("Content not found. Replying...\n");
                        snprintf(message, sizeof(message), "%s %s %s %s%s", "NOCONTENT",  arg2, arg1, arg3, "\n");
                        send(fd, message, strlen(message), 0);
                    }
                }
                else
                {
                    snprintf(message, sizeof(message), "%s %s %s %s%s", "QUERY", arg1, arg2, arg3, "\n");

                    for(u = 0; u < 100; u++)
                    {

                        if(client_fds[u] != fd && client_fds[u] > 0)
                        {
                            send(client_fds[u], message, strlen(message), 0);
                        }
                    }

                    if(selfClient_fd > 0 && selfClient_fd != fd)
                    {

                        send(selfClient_fd, message, strlen(message), 0);
                    }
                }

                memset(buffer,0,sizeof(buffer));
            }

            else if(strstr(buffer, "CONTENT") != NULL) //NO/CONTENT DEST ORIGEM NOME
            {
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
                if(strlen(command) == 9)
                {
                    if(strcmp(arg1, nodo->id) == 0)
                    {
                        printf("Could not find content %s in node %s.\n", arg3, arg2);
                        return 2;
                    }
                    else
                    {
                        for (l = 0; l < 100; l++)
                        {
                            if(strcmp(nodo->table1[l], arg1) == 0)
                            {
                                snprintf(message, sizeof(message), "%s %s %s %s%s", "NOCONTENT",  arg1, arg2, arg3, "\n");
                                if(strcmp(nodo->table2[l], nodo->ext) == 0)
                                {
                                    send(nodo->fdExt, message, strlen(message), 0);
                                    break;
                                }
                                else
                                {
                                    for(u = 0; u < 100; u++)
                                    {
                                        if(strcmp(nodo->table2[l], nodo->intr[u]) == 0)
                                        {
                                            send(u, message, strlen(message), 0);
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
                        printf("Found content %s in node %s.\n", arg3, arg2);
                        return 2;
                    }
                    else
                    {
                        for (l = 0; l < 100; l++)
                        {
                            if(strcmp(nodo->table1[l], arg1) == 0)
                            {
                                snprintf(message, sizeof(message), "%s %s %s %s%s", "CONTENT",  arg1, arg2, arg3, "\n");
                                if(strcmp(nodo->table2[l], nodo->ext) == 0)
                                {
                                    send(nodo->fdExt, message, strlen(message), 0);
                                    break;
                                }
                                else
                                {
                                    for(k = 0; k < 100; k++)
                                    {
                                        if(strcmp(nodo->table2[l], nodo->intr[k]) == 0)
                                        {
                                            send(k, message, strlen(message), 0);
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
            else if(strstr(buffer, "WITHDRAW") != NULL) //WITHDRAW arg1= nó a retirar
            {
                sscanf(buffer, "%s %s", command, arg1);
                printf("Received warning that node %s left the network. Updating routing...\n", arg1);

                for (i = 0; i < 100; i++)
                {
                    if(strcmp(nodo->table1[i], arg1) == 0)
                    {
                        memset(nodo->table1[i],0,sizeof(nodo->table1[i]));
                        memset(nodo->table2[i],0,sizeof(nodo->table2[i]));
                        strcpy(nodo->table1[i], "\0");
                        strcpy(nodo->table2[i], "\0");
                    }
                    if(strcmp(nodo->table2[i], arg1) == 0)
                    {
                        memset(nodo->table1[i],0,sizeof(nodo->table1[i]));
                        memset(nodo->table2[i],0,sizeof(nodo->table2[i]));
                        strcpy(nodo->table1[i], "\0");
                        strcpy(nodo->table2[i], "\0");
                    }
                }
                snprintf(message, sizeof(message), "%s %s%s", "WITHDRAW", arg1, "\n");
                for(u = 0; u < 100; u++)
                {
                    if(client_fds[u] != fd && client_fds[u] > 0)
                    {
                        send(client_fds[u], message, strlen(message), 0);
                    } 
                }
                if(selfClient_fd > 0 && selfClient_fd != fd)
                {
                    send(selfClient_fd, message, strlen(message), 0);
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
