#include "cot.h"
#include "udp.h"

int main(int argc, char **argv)
{
    char regIP[16], regUDP[6], IP[16], TCP[6];
    char buffer[128+1];
    char message[128+1] = "NODES ";
    char input[128];
    char *command, *arg1, *arg2, *arg3, *arg4, *arg5;
    int errcode;
    struct node *nodo = (struct node*) malloc (sizeof(struct node));



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

    printf("Enter a command: \n");
    fgets(input, sizeof(input), stdin);

    command = strtok(input, " ");
    arg1 = strtok(NULL, " ");
    arg2 = strtok(NULL, " ");
    arg3 = strtok(NULL, " ");
    arg4 = strtok(NULL, " ");
    arg5 = strtok(NULL, "");

    if(strcmp(command, "join") == 0) //arg1 = net; arg2 = id
    {
        if(strlen(arg1) != 3) exit(1);

        strcat(message, arg1);

        errcode = commUDP(message, buffer, regIP, regUDP);
        if(errcode != 0) return -1;

        printf("Enviada:\n%s\nRecebida:\n%s\n", message, buffer);

        //if(compare_udp_messages(buffer, arg2) == 1) nodo->id = atoi(arg2+1); 
        //else nodo->id = atoi(arg2);

        //printf("%d \n", nodo->id);

        //errcode = commUDP(snprintf(message, sizeof(message), "%s %s %s %s", arg1, arg2, IP, TCP), buffer, regIP, regUDP);
        //if(errcode != 0) return -1;

        //enviar REG por tcp
        //abrir SERVIDOR tcp para primeiro nó - verificar tamanho da resposta do servidor
        //estabelecer ligacao para nos seguintes
        //definir ext, bck e intr.
        //falta select??? para ver se vai ler do teclado ou receber ligacao
    }

    if(strcmp(command, "leave") == 0)
    {
        //fechar servidor/ligacao TCP
        //verificar ligacao a outros nós - necessario atribuir backup??


    }

    free(nodo);


    //errcode = commUDP(message, buffer, regIP, regUDP);

        //if(errcode != 0) return -1;
        //else printf("Enviada: %s \nRecebida: %s\n", message, buffer);

    return 0;
}