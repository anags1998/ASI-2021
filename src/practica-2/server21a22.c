
#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <string.h>
#include <ctype.h>

#define CLAVE 0x45916038B
#define TAM 25

typedef struct mensaje {
    long tipo;
    char cad[TAM];
} MENSAJE;

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

    int i, idCola;
    MENSAJE m;

    idCola = msgget(CLAVE, IPC_CREAT | 0666); //Creamos la cola de mensajes
    
    msgrcv(idCola, &m, sizeof (m) - sizeof (long), 1, 0); //Recibimos por el canal 1
    convertirMayusculas(m.cad);
    m.tipo = 2L;//Seleccionamos el canal
    
    msgsnd(idCola, &m, sizeof (m) - sizeof (long), 0); //Enviamos el mensaje traducido
    
    msgctl(idCola, IPC_RMID, NULL); //Eliminamos la cola de mensajes
    return (EXIT_SUCCESS);
}

