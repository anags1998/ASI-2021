
#include <stdio_ext.h>
#include <stdlib.h>
#include <sys/msg.h>

#define CLAVE 0x45916038B
#define TAM 25

typedef struct mensaje {
    long tipo;
    int pid;
    char cad[TAM];
} MENSAJE;

int main(int argc, char** argv) {

    int i, idCola;
    MENSAJE m;

    char cadenas[4][TAM] = {"Hola", "a", "VeR", "qUe", "tal"};
    idCola = msgget(CLAVE, 0666);
    m.tipo = 1L;
    m.pid = getpid();
    printf("\nIntroduce una cadena de caracteres: ");
    __fpurge(stdin);
    //fgets(m.cad, TAM, stdin);
    fgets(cadenas[0],TAM,stdin);
    for (i = 0; i < 4; i++) {
        sprintf(m.cad, "%s", cadenas[i]);
        msgsnd(idCola, &m, sizeof (m) - sizeof (long), 0);
    }
    for (i = 0; i < 4; i++) {
        msgrcv(idCola, &m, sizeof (m) - sizeof (long), getpid(), 0);
        printf("En mayusculas: %s\n", m.cad);
    }

    return (EXIT_SUCCESS);
}
