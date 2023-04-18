#include "cot.h"
#include "udp.h"
#include "select.h"
#include "aux.h"

int main(int argc, char **argv)
{
    char regIP[16], regUDP[6];
    char buffer[1024+1];
    char message[128+1] = "NODES ";
    char input[128];
    char line[32];
    char *command, *arg1, *arg2, *arg3, *arg4, *arg5;
    int errcode, i;
    int nNodes = 0;
    struct node *nodo = (struct node*) malloc (sizeof(struct node));

    char word_array[6][128];
    int word_count = 0;
    char *token;

    initNode(nodo); //memset a 0 de todos os vetores do nó - evitar lixo

    nodo->ncontents = 0;

    for(i = 0; i < 32; i++)
    {
        strcpy(nodo->content[i], "\0");
    }
    for(i = 0; i < 100; i++)
    {
        strcpy(nodo->table1[i], "\0");
    }
    for(i = 0; i < 100; i++)
    {
        strcpy(nodo->table2[i], "\0");
    }

    memset(buffer, 0, sizeof(buffer)); //inicializar buffer

    srand(time(0)); //inicializar variavel random para atribuir numero de nó caso seja repetido

    if(argc != 5 && argc != 3)
    {
        free(nodo);
        printf("Arguments missing. Exiting\n");
        exit(1);
    } 
    //inicializacao dos valores dados como argumento
    if(argc == 3)
    {
       strcpy (regIP, "193.136.138.142");
       strcpy (regUDP, "59000");
    }
    if(argc == 5)
    {
        strcpy(regIP, argv[3]);
        strcpy(regUDP, argv[4]);
    }

    strcpy(nodo->ip, argv[1]);
    strcpy(nodo->port, argv[2]);

    printf("cot - Group 105\n");
    printf("Type help to show available commands.\n");
    printf("Enter a command:\n");

    while(1)
    {
        //processamento de input

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


        if(strcmp(command, "join") == 0) //arg1 = net; arg2 = id
        {
            if(word_count != 3)
            {
                printf("Arguments missing. Exiting.\n");
                free(nodo);
                exit(1);
            }
            arg1 = word_array[1];
            arg2 = word_array[2];

            if(strlen(arg1) != 3)
            {
                printf("Invalid argument (%s). Exiting.\n", arg1);
                free(nodo);
                exit(1);
            }

            if(atoi(arg2) > 99 || atoi(arg2) < 0)
            {
                printf("Invalid node number. Exiting.\n");
                free(nodo);
                exit(1);
            }

            strcat(message, arg1);

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
            printf("Number of nodes in the network: %d\n", nNodes);
            if(nNodes>0) printf("These nodes are:\n%s", buffer+14);

            if(nNodes == 0)
            {
                strcpy(nodo->id, arg2);
                strcpy(nodo->ext, arg2);
                strcpy(nodo->bck, arg2);
                strcpy(nodo->ipExt, nodo->ip);
                strcpy(nodo->portExt, nodo->port);
            }
            else
            {
                findNode(buffer, line, nNodes, arg2);

                if(strcmp(line, "\0") != 0)
                {
                    while(strcmp(line, "\0") != 0)
                    {
                        sprintf(nodo->id, "%d", rand()%100+1);
                        if(strcmp(nodo->id, "100") == 0) strcpy(nodo->id, "49");
                        memset(line, 0, sizeof(line));
                        if(strlen(nodo->id) == 1) //colocar um 0 antes do id caso este seja apenas um caracter
                        {
                            strcpy(arg2, "0");
                            strcat(arg2, nodo->id);
                            strcpy(nodo->id, arg2);
                        }
                        findNode(buffer, line, nNodes, nodo->id);
                    }
                    printf("Node already exists. New id: %s\n", nodo->id);
                }
                else strcpy(nodo->id, arg2);

                strcpy(nodo->ext, nodo->id);

                while(strcmp(nodo->ext, nodo->id) == 0)
                {
                    printf("Select the node to connect to:\n");
                    fgets(input, sizeof(input), stdin);
                    input[strcspn(input, "\n")] = 0;
                    strcpy(nodo->ext, input);
                    if(strlen(nodo->ext) != 2) printf("Please enter two characters.\n");
                    findNode(buffer, line, nNodes, nodo->ext);
                    if(strcmp(line, "\0") == 0)
                    {
                        printf("Node does not exist. Try again.\n");
                        strcpy(nodo->ext, nodo->id);
                    }
                    memset(input, 0, sizeof(input));
                }
                sscanf(line, "%s %s %s", nodo->ext, nodo->ipExt, nodo->portExt);
            }

            strcpy(nodo->bck, nodo->id);
            strcpy(nodo->ipBck, nodo->ip);
            strcpy(nodo->portBck, nodo->port);

            memset(message,0,sizeof(message));
            errcode = snprintf(message, sizeof(message), "%s %s %s %s %s", "REG", arg1, nodo->id, nodo->ip, nodo->port); //juntar strings para enviar
            if(errcode >= sizeof(message)) return -1;

            memset(buffer,0,sizeof(buffer));
            errcode = commUDP(message, buffer, regIP, regUDP); //enviar REG
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

            if(strcmp(nodo->ext, nodo->id) == 0) printf("First node to join.\n");
            else printf("Connecting to node %s with IP %s and port %s\n", nodo->ext, nodo->ipExt, nodo->portExt);
            if(strcmp(buffer, "OKREG") == 0)
            {
                printf("Joining network %s...\n", arg1);
                tcpSelect(nodo, regIP, regUDP, arg1);
            }
            else
            {
                printf("UDP Error.");
                exit(1);
            }
            memset(input, 0, sizeof(input));
            memset(buffer, 0, sizeof(buffer));
        }

        if(strcmp(command, "djoin") == 0) //arg1 = net; arg2 = id; arg3 = bootid; arg4 = bootIP; arg5=bootTCP
        {
            if(word_count != 6)
            {
                printf("Arguments missing. Exiting.\n");
                exit(1);
            }

            arg1 = word_array[1];
            arg2 = word_array[2];
            arg3 = word_array[3];
            arg4 = word_array[4];
            arg5 = word_array[5];

            if(strlen(arg1) != 3)
            {
                printf("Invalid argument (%s). Exiting.\n", arg1);
                exit(1);
            }

            if(atoi(arg2) > 99 || atoi(arg2) < 1)
            {
                printf("Invalid node number. Exiting.\n");
                exit(1);
            }

            if(atoi(arg3) > 99 || atoi(arg3) < 1)
            {
                printf("Invalid argument (%s). Exiting.\n", arg3);
                exit(1);
            }

            strcpy(nodo->id, arg2);
            strcpy(nodo->ext, arg3);
            strcpy(nodo->ipExt, arg4);
            strcpy(nodo->portExt, arg5);

            strcpy(nodo->bck, arg2);
            strcpy(nodo->ipBck, nodo->ip);
            strcpy(nodo->portBck, nodo->port);

            if(strcmp(nodo->ext, nodo->id) == 0) printf("First node to join.\n");
            else printf("Connecting to node %s with IP %s and port %s\n", nodo->ext, nodo->ipExt, nodo->portExt);
            tcpSelect(nodo, regIP, regUDP, arg1);
        }

        if(strcmp(command, "leave") == 0) printf("Not yet on the network.\n");
        else if(strcmp(command, "st") == 0) printf("Not yet on the network.\n");
        else if(strcmp(command, "sr") == 0) printf("Not yet on the network.\n");
        else if(strcmp(command, "get") == 0) printf("Not yet on the network.\n");
        else if(strcmp(command, "exit") == 0)
        {
            free(nodo);
            return 0;
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
        else if(strcmp(command, "help") == 0)
        {
            printf("join - Join the network\ndjoin - Join a known network\n");
            printf("create - Create a new content\ndelete - Delete existing content\nsn - Show contents\n");
            printf("exit - Close application\nMore commands available after joining a network.\n");
        }
        else
        {
            printf("Command not recognized.\n");
        }
    }

    free(nodo);
    return 0;
}


