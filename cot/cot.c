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

    initNode(nodo);

    nodo->ncontents = 0;

    srand(time(0));

    if(argc != 5 && argc != 3) exit(1); //inicializacao dos valores dados como argumento
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

    /*if (word_count == 1){
        command = word_array[0];
    }

    if (word_count == 3){
        command = word_array[0];
        arg1 = word_array[1];
        arg2 = word_array[2];
    }

    if (word_count == 6){
        command = word_array[0];
        arg1 = word_array[1];
        arg2 = word_array[2];
        arg3 = word_array[3];
        arg4 = word_array[4];
        arg5 = word_array[5];
    }*/


    while(1)
    {

        printf("Enter a command: \n");
        fgets(input, sizeof(input), stdin);

        input[strcspn(input, "\n")] = '\0';

        char word_array[6][128];
        int word_count = 0;

        char *token = strtok(input, " ");
        while (token != NULL && word_count < 20)
        {
            strcpy(word_array[word_count++], token);
            token = strtok(NULL, " ");
        }

        memset(input, 0, sizeof(input));

        command = word_array[0];

        if(strcmp(command, "join") == 0) //arg1 = net; arg2 = id
        {
            if(word_count != 3)
            {
                printf("Arguments missing. Exiting.\n");
                exit(1);
            }
            arg1 = word_array[1];
            arg2 = word_array[2];
            if(strlen(arg1) != 3) exit(1);

            strcat(message, arg1);

            errcode = commUDP(message, buffer, regIP, regUDP);
            if(errcode != 0) return -1;

            //printf("Sent:\n%s\nReceived:\n%s\n", message, buffer);

            for (i=0; buffer[i]; i++) nNodes += (buffer[i] == '\n');
            nNodes--;
            printf("Number of nodes in the network: %d\n", nNodes);
            if(nNodes>0) printf("These nodes are:\n%s\n", buffer);

            if(nNodes == 0)
            {
                strcpy(nodo->id, arg2);
                strcpy(nodo->ext, arg2);
                strcpy(nodo->bck, arg2);
                strcpy(nodo->ipExt, "\0");
                strcpy(nodo->portExt, "\0");
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

            //printf("Informação do nó:\nid: %s\next: %s\nbck: %s\n", nodo->id, nodo->ext, nodo->bck);

            memset(buffer,0,sizeof(buffer));
            errcode = commUDP(message, buffer, regIP, regUDP); //enviar REG
            if(errcode != 0) return -1;

            printf("Sent:\n%s\nReceived:\n%s\n", message, buffer);

            if(strcmp(buffer, "OKREG") == 0) tcpSelect(nodo, regIP, regUDP, arg1);
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

            strcpy(nodo->id, arg2);
            strcpy(nodo->ext, arg3);
            strcpy(nodo->ipExt, arg4);
            strcpy(nodo->portExt, arg5);

            strcpy(nodo->bck, arg2);
            strcpy(nodo->ipBck, nodo->ip);
            strcpy(nodo->portBck, nodo->port);

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
            //free memoria conteudos?
        }
        else if(strcmp(command, "create") == 0)
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
                        free(nodo->content[i]);
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
                for(i=0;i<nodo->ncontents;i++)
                {
                    printf("%d. %s\n", i, nodo->content[i]);
                }
            }
        }
        else printf("Command not recognized.\n");
    }

    /*free(nodo->id);
    free(nodo->ext);
    free(nodo->bck);
    free(nodo);*/


    //errcode = commUDP(message, buffer, regIP, regUDP);

        //if(errcode != 0) return -1;
        //else printf("Enviada: %s \nRecebida: %s\n", message, buffer);

    return 0;
}


