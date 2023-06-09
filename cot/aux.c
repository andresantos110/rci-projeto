#include "cot.h"

//recebe buffer, numero de linhas, nó a procurar - line toma o valor da linha onde esta o nó
void findNode(char buffer[], char line[], int nNodes, char node [])
{
    strcpy(line, "\0");
    char *auxString, auxBuffer[1024], el1[4], el2[16], el3[6];
    int i;

    strcpy(auxBuffer,buffer);
    auxString = strtok(auxBuffer, "\n");

    for(i = 0; i < nNodes; i++)
    {
        auxString = strtok(NULL, "\n");
        sscanf(auxString, "%s %s %s", el1, el2, el3);
        if(strcmp(el1, node) == 0) strcpy(line, auxString);

    }

}

int max3(int a, int b, int c)
{
    int max;
    if(a > b) max = a;
    else max = b;
    if(c > max) max = c;
    return max;
}

int initNode(struct node *nodo)
{
    int i;

    memset(nodo->id, 0, sizeof(nodo->id));
    memset(nodo->ip, 0, sizeof(nodo->ip));
    memset(nodo->port, 0, sizeof(nodo->port));

    memset(nodo->bck, 0, sizeof(nodo->bck));
    memset(nodo->ipBck, 0, sizeof(nodo->ipBck));
    memset(nodo->portBck, 0, sizeof(nodo->portBck));

    memset(nodo->ext, 0, sizeof(nodo->ext));
    memset(nodo->ipExt, 0, sizeof(nodo->ipExt));
    memset(nodo->portExt, 0, sizeof(nodo->portExt));

    for(i = 0; i < 100; i++)
    {
        memset(nodo->intr[i], 0, sizeof(nodo->intr[i]));
        memset(nodo->ipIntr[i], 0, sizeof(nodo->ipIntr[i]));
        memset(nodo->portIntr[i], 0, sizeof(nodo->portIntr[i]));
    }

    for(i = 0; i < 100; i++)
    {
        memset(nodo->table1[i], 0, sizeof(nodo->table1[i]));
        memset(nodo->table2[i], 0, sizeof(nodo->table2[i]));
    }
    return 0;
}

int updateTable(char origem[], char dest[], struct node *nodo)
{
    int i;
    if(strcmp(origem, nodo->id) == 0) return 0;

    if(nodo->ntabela == 100)
    {
        return -1;
    }

    for(i=0;i<100;i++)
    if(strcmp(nodo->table1[i], origem) == 0)
    {
        return -1;
    }
    for(i=0;i<100;i++)
    {
        if(strcmp(nodo->table1[i], "\0") == 0)
        {
            strcpy(nodo->table1[i], origem);
            strcpy(nodo->table2[i], dest);
            nodo->ntabela++;
            return 0;
        }
    }

    return -1;
}
