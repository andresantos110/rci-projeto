#include "cot.h"
#include "udp.h"
#include "select.h"

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

    input[strcspn(input, "\n")] = 0; 

    char *word_array[6]; 
    int word_count = 0;

    char *token = strtok(input, " ");
    while (token != NULL && word_count < 20) {
        word_array[word_count++] = token;
        token = strtok(NULL, " ");
    }

    // Print out the words in the array
    printf("Words in the array:\n");
    for (int i = 0; i < word_count; i++) {
        printf("%s\n", word_array[i]);
    }

    command = word_array[0];

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

    if(strcmp(command, "join") == 0) //arg1 = net; arg2 = id
    {
        arg1 = word_array[1];
        arg2 = word_array[2];
        if(strlen(arg1) != 3) exit(1);

        strcat(message, arg1);

        errcode = commUDP(message, buffer, regIP, regUDP);
        if(errcode != 0) return -1;

        printf("Enviada:\n%s\nRecebida:\n%s\n", message, buffer);

        if(strstr(buffer, arg2) != NULL) nodo->id = atoi(arg2+1); //verificar se nó já existe
        else nodo->id = atoi(arg2);

        errcode = snprintf(message, sizeof(message), "%s %s %s %s %s", "REG", arg1, arg2, IP, TCP); //juntar strings para enviar
        if(errcode >= sizeof(message)) return -1;

        errcode = commUDP(message, buffer, regIP, regUDP); //enviar REG
        if(errcode != 0) return -1;

        printf("Enviada:\n%s\nRecebida:\n%s\n", message, buffer);

        tcpSelect(nodo, IP, TCP);

        //abrir SERVIDOR tcp para primeiro nó - verificar tamanho da resposta do servidor
        //estabelecer ligacao para nos seguintes
        //definir ext, bck e intr.
        //falta select??? para ver se vai ler do teclado ou receber ligacao
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

        nodo->id = atoi(arg2);
        errcode = snprintf(message, sizeof(message), "%s %s %s %s %s", "REG", arg1, arg2, IP, TCP); //juntar strings para enviar
    }

    if(strcmp(command, "leave") == 0) printf("Not yet on the network.");

    free(nodo);


    //errcode = commUDP(message, buffer, regIP, regUDP);

        //if(errcode != 0) return -1;
        //else printf("Enviada: %s \nRecebida: %s\n", message, buffer);

    return 0;
}
