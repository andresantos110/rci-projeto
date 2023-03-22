#include "cot.h"
#include "udp.h"
#include "select.h"
#include "aux.h"

int main(int argc, char **argv)
{
    char regIP[16], regUDP[6], IP[16], TCP[6];
    char buffer[1024+1];
    char message[128+1] = "NODES ";
    char input[128];
    char line[32];
    char *command, *arg1, *arg2, *arg3, *arg4, *arg5;
    int errcode, i;
    int nNodes = 0;
    struct node *nodo = (struct node*) malloc (sizeof(struct node));

    nodo->id = calloc(2, sizeof(char));
    nodo->ext = calloc(2, sizeof(char));
    nodo->bck = calloc(2, sizeof(char));
    nodo->ncontents = 0;



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

    strcpy(IP, argv[1]);
    strcpy(TCP, argv[2]);

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

    //ADICIONAR WHILE PARA NAO SAIR DO PROGRAMA DEPOIS DE 1 COMANDO
    while(1)
    {

        printf("Enter a command: \n");
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
            printf("Number of nodes in the network: %d\n", nNodes);

            findNode(buffer, line, nNodes, arg2);

            if(strcmp(line, "\0") != 0)
            {
                sprintf(nodo->id, "%d", atoi(arg2)+1);
                if(strlen(nodo->id) == 1) //colocar um 0 antes do id caso este seja apenas um caracter
                {
                    strcpy(arg2, "0");
                    strcat(arg2, nodo->id);
                    strcpy(nodo->id, arg2);
                }
                printf("Node already exists. New id: %s\n", nodo->id);
            }
            else strcpy(nodo->id, arg2);

            strcpy(nodo->bck, nodo->id);


            if(nNodes != 0)
            {
                while(strlen(input) != 3)
                {
                    printf("Select the node to connect to:\n");
                    fgets(input, sizeof(input), stdin);
                    sscanf(input, "%s", nodo->ext);
                    if(strlen(input) != 3) printf("Please enter two characters.\n");
                    findNode(buffer, line, nNodes, nodo->ext);
                    if(strcmp(line, "\0") == 0)
                    {
                        printf("Node does not exist. Try again.\n");
                        sscanf("ERROR", "%s", input);
                    }
                }
            }
            else
            {
                strcpy(line, "\0");
            }

            //se line="\0" entao é o primeiro nó

            errcode = snprintf(message, sizeof(message), "%s %s %s %s %s", "REG", arg1, nodo->id, IP, TCP); //juntar strings para enviar
            if(errcode >= sizeof(message)) return -1;

            printf("Informação do nó:\nid: %s\next: %s\nbck: %s\n", nodo->id, nodo->ext, nodo->bck);

            errcode = commUDP(message, buffer, regIP, regUDP); //enviar REG
            if(errcode != 0) return -1;

            if(strcmp(buffer, "OKREG") == 0) tcpSelect(nodo, IP, TCP, line, regIP, regUDP);
            else
            {
                printf("UDP Error.");
                exit(1);
            }
        }

        if(strcmp(command, "djoin") == 0) //arg1 = net; arg2 = id; arg3 = bootid; arg4 = bootIP; arg5=bootTCP
        {
            arg1 = word_array[1];
            arg2 = word_array[2];
            arg3 = word_array[3];
            arg4 = word_array[4];
            arg5 = word_array[5];

            //necessário preencher estrutura do nodo, abrir servidor TCP e perguntar ao nó dado por bootip e
            //boottcp o comando EXTERN, informar-se com NEW

            nodo->id = arg2;
            errcode = snprintf(message, sizeof(message), "%s %s %s %s %s", "REG", arg1, nodo->id, IP, TCP); //juntar strings para enviar
        }

        if(strcmp(command, "leave") == 0) printf("Not yet on the network.");
        if(strcmp(command, "st") == 0) printf("Not yet on the network.");
        if(strcmp(command, "sr") == 0) printf("Not yet on the network.");
        if(strcmp(command, "get") == 0) printf("Not yet on the network.");
        if(strcmp(command, "exit") == 0)
        {
            free(nodo->id);
            free(nodo->ext);
            free(nodo->bck);
            free(nodo);
            return 0;
            //free memoria conteudos?
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
                    printf("%d. %s\n", i, nodo->content[i]);
                }
            }
        }
    }

    //elsifs para comando nao reconhecido?

    /*free(nodo->id);
    free(nodo->ext);
    free(nodo->bck);
    free(nodo);*/


    //errcode = commUDP(message, buffer, regIP, regUDP);

        //if(errcode != 0) return -1;
        //else printf("Enviada: %s \nRecebida: %s\n", message, buffer);

    return 0;
}


