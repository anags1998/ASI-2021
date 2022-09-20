#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> // Para fifos
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/msg.h>

#define NOM_FIFOW "/home/ana/Escritorio/ASI/ASI_V2/src/practica-2/FR"
#define NOM_FIFOR "//home/ana/Escritorio/ASI/ASI_V2/src/practica-2/FW"
//Ojo cuidado, el servidor lee de la fifo donde el cliente escribe
//Y escribe en la fifo donde el cliente lee
#define TAM 25

void convertirMayusculas(char *cad) {
    int i = 0;
    char c;
    while (cad[i]) {
        c = cad[i];
        cad[i] = toupper(c);
        i++;
    }
}

int main(int argc, char** argv) {

    int fifoR, fifoW, i, numBytes;
    char cad[TAM];

    mkfifo(NOM_FIFOW, 0666);
    mkfifo(NOM_FIFOR, 0666);

    fifoR = open(NOM_FIFOR, O_RDWR);
    fifoW = open(NOM_FIFOW, O_RDWR);

    while (read(fifoR, cad, TAM) != 0) {
        convertirMayusculas(cad);
        write(fifoW, cad, strlen(cad));
    }
    
    unlink(fifoR);
    unlink(fifoW);
    
    return (EXIT_SUCCESS);
}
