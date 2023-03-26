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