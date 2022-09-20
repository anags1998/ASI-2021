#include <stdio_ext.h>
#include <stdlib.h>
#include <fcntl.h> //Para fifos
#include <string.h>

#define NOM_FIFOW "/home/ana/Escritorio/ASI/ASI_V2/src/practica-2/FW"
#define NOM_FIFOR "/home/ana/Escritorio/ASI/ASI_V2/src/practica-2/FR"
#define TAM 25

int main(int argc, char** argv) {

    char cad[TAM], resp;
    int fifoW, fifoR;

    //Abrimos los named pipes
    fifoW = open(NOM_FIFOW, O_RDWR);
    fifoR = open(NOM_FIFOR, O_RDWR);

    do {
        printf("Introduce una cadena de caracteres: \n");
        fflush(stdin);
        __fpurge(stdin);
        fgets(cad, TAM, stdin);
        write(fifoW, cad, strlen(cad));
        read(fifoR, cad, TAM);
        printf("El servidor ha escrito en la fifo: \n%s\n", cad);
        printf("Otra (s/n): \n");
        fflush(stdin);
        __fpurge(stdin);
        scanf("%c", &resp);
    } while (resp == 's');
    return (EXIT_SUCCESS);
}
