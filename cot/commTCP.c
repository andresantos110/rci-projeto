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
    int i = 0, k = 0, u = 0, l = 0, p = 0, flg = 0, flg2 = 0;

    //if(read(fd, buffer, 1024) == 0) return 0;
    //else return 1;

    memset(buffer, 0, sizeof(buffer));
    memset(auxBuffer, 0, sizeof(auxBuffer));
    memset(message, 0, sizeof(message));
    memset(command, 0, sizeof(command));
    memset(arg1, 0, sizeof(arg1));
    memset(arg2, 0, sizeof(arg2));
    memset(arg3, 0, sizeof(arg3));

    //verificar se foi saída
    if(read(fd, buffer, 1024) == 0) //return 0 caso o nó onde houve atividade tenha saido da rede.
    {

        //INSERIR ENVIO DE WITHDRAW AQUI

        for(i = 0;i < 100;i++) //verificar se saiu interno
        {
            if(strcmp(nodo->intr[i], "\0") != 0 && i == fd) //saiu um interno
            {
                strcpy(nodo->intr[i], "\0");
                strcpy(nodo->ipIntr[i], "\0");
                strcpy(nodo->portIntr[i], "\0"); //limpar informacao do nó que saiu
                return 0;
            }
        }

		if(strcmp(nodo->bck, nodo->id) == 0) return 1;
        else //se nao for ancora, apenas saiu um externo
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
        //printf("RECEBIDO: %s\n", buffer);
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
            if(strstr(buffer, "EXTERN") != NULL) //se for null
            {
                sscanf(buffer, "%s %s %s %s", command, arg1, arg2, arg3);
                strcpy(nodo->bck, arg1);
                strcpy(nodo->ipBck, arg2);
                strcpy(nodo->portBck, arg3);
                memset(buffer,0,sizeof(buffer));
            }

            if(strstr(buffer, "QUERY") != NULL)
            {
                printf("recebi um ola\n");
                flg = 0;
                flg2 = 0;

                sscanf(buffer, "%s %s %s %s", command, arg1, arg2, arg3);

                updateTable(arg2, nodo->intr[fd], nodo->table1, nodo->table2, nodo->ntabela); // MUDAR pode ser interno ou externo

                if(strcmp(arg1, nodo->id) == 0)
                {
                    if(nodo->ncontents == 0)
                    {
                        snprintf(message, sizeof(message), "%s %s %s %s%s", "NOCONTENT",  arg2, nodo->id, arg3, "\n");
                        send(fd, message, strlen(message), 0);
                        flg2 = 1;
                    }
                    if(flg2 == 0)
                    {
                        for (k = 0; k < 32; k++)
                        {
                            if(nodo->content[k] != NULL){
                                if(strcmp(arg3, nodo->content[k]) == 0)
                                {
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

                //printf("Tabela 2 item é %s\n", nodo->intr[fd]); // MUDAR pode ser interno ou externo

                //printf("%s   %s\n",nodo->table1[0], nodo->table2[0]);
                //printf("%s   %s\n",nodo->table1[1], nodo->table2[1]);
                memset(buffer,0,sizeof(buffer));
            }

            if(strstr(buffer, "CONTENT") != NULL)
            {

                updateTable(arg2, nodo->intr[fd], nodo->table1, nodo->table2, nodo->ntabela); // MUDAR pode ser interno ou externo

                sscanf(buffer, "%s %s %s %s", command, arg1, arg2, arg3);

                if(strcmp(arg1, nodo->id) == 0)
                {
                    printf("Encontrei o ficheiro\n");
                }
                else
                {
                    snprintf(message, sizeof(message), "%s %s %s %s%s", "CONTENT", arg1, arg2, arg3, "\n");
                    // função WHO THE BITCH
                    for (l = 0; l < 100; l++)
                    {
                        if(nodo->table1[l] == arg1)
                        {
                            if(strcmp(nodo->ext, arg1) == 0){
                                send(selfClient_fd, message, strlen(message), 0);
                                flg = 1;
                            }
                            else
                            {
                                for (u = 0; u < 100; u++)
                                {
                                    if(u == atoi(arg1))
                                    {
                                        send(client_fds[u], message, strlen(message), 0); //send message to fd of table2[i]
                                        flg = 1;
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    if(flg == 0){
                        if(selfClient_fd > 0)
                        {
                            send(selfClient_fd, message, strlen(message), 0);
                        }
                        for (p = 0; p < 100; p++)
                        {
                            if(client_fds[p] != -1)
                            {
                                send(client_fds[p], message, strlen(message), 0);
                                break;
                            }
                        }
                    }
                }
                memset(buffer,0,sizeof(buffer));
            }

            if(strstr(buffer, "NOCONTENT") != NULL)
            {

                updateTable(arg2, nodo->intr[fd], nodo->table1, nodo->table2, nodo->ntabela); // MUDAR pode ser interno ou externo

                sscanf(buffer, "%s %s %s %s", command, arg1, arg2, arg3);

                if(strcmp(arg1, nodo->id) == 0)
                {
                    printf("Não encontrei o ficheiro\n");
                }
                else
                {
                    for (l = 0; l < 100; l++)
                    {
                        if(nodo->table1[l] == arg1)
                        {
                            if(strcmp(nodo->ext, arg1) == 0){
                                send(selfClient_fd, message, strlen(message), 0);
                                flg = 1;
                            }
                            else
                            {
                                for (u = 0; u < 100; u++)
                                {
                                    if(u == atoi(arg1))
                                    {
                                        send(client_fds[u], message, strlen(message), 0); //send message to fd of table2[i]
                                        flg = 1;
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    if(flg == 0){
                        if(selfClient_fd > 0)
                        {
                            send(selfClient_fd, message, strlen(message), 0);
                        }
                        for (p = 0; p < 100; p++)
                        {
                            if(client_fds[p] != -1)
                            {
                                send(client_fds[p], message, strlen(message), 0);
                                break;
                            }
                        }
                    }
                }
                memset(buffer,0,sizeof(buffer));
            }
            if(strstr(buffer, "WITHDRAW") != NULL)
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
                    if(strcmp(nodo->table1[i], "\0") == 0) break;
                }

                //enviar withdraw para outros nos
                /*snprintf(message, sizeof(message), "%s %s", "WITHDRAW", arg1);
                for (int i = 0; i < 100; i++)
                {

                }*/
                memset(buffer,0,sizeof(buffer));
            }
            return 2;
        }
        else
        {
            printf("Mensagem incompleta.\n");
            return -1;
        }
    }
    return -1;

}
