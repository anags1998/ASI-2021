#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>

#define CLAVE 0x45916038L
#define TAM_SEGMENTO 1024
#define TAM_TXT 50

typedef struct mensaje {
    long tipo;
    char cad[TAM_TXT];
} MENSAJE;

int main(int argc, char** argv) {

    int i, idCola;
    MENSAJE m;

    char cadena[TAM_TXT];
    //char cadenas[4][TAM_TEXT]={"Hola","Funciona","Si","No"};
    char siono;

    idCola = msgget(CLAVE, 0666);
    m.tipo = getpid();

    do {
        printf("\nIntroduce una cadena de caracteres: ");
        fflush(stdin);
        __fpurge(stdin);
        fgets(cadena, TAM_TXT, stdin);
        if (cadena[strlen(cadena) - 1] == '\n') {
            cadena[strlen(cadena) - 1] = '\0';
        }

        
        sprintf(m.cad, "%s", cadena);
        msgsnd(idCola, &m, sizeof (m) - sizeof (long), 0);
        
       
        msgrcv(idCola, &m, sizeof (m) - sizeof (long), getpid(), 0);
        printf("En mayusculas: %s\n", m.cad);
      

        printf("Otra?(s/n): ");
        scanf("%c", &siono);
    } while (siono == 's');


    return (EXIT_SUCCESS);
}