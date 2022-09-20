#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <string.h>
#include <ctype.h>

#define CLAVE 0x45916038B
#define TAM 25

typedef struct mensaje{
    long tipo;
    int pid;
    char cad[TAM];
}MENSAJE;

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

    int i,idCola;
    MENSAJE m;
    
    idCola = msgget(CLAVE, IPC_CREAT|0666); //Creamos la cola de mensajes
    while(1){
        msgrcv(idCola, &m, sizeof(m)-sizeof(long),1,0);
        convertirMayusculas(m.cad);
        m.tipo = (long)m.pid;//obtenemos el canal del pid
        msgsnd(idCola,&m, sizeof(m)-sizeof(long),0);
    }
    return (EXIT_SUCCESS);
}
