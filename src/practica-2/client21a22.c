
#include <stdio.h>
#include <stdlib.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <sys/msg.h>

#define CLAVE 0x45916038B
#define TAM 25

typedef struct mensaje {
    long tipo; //Canal=Tipo=idMensaje
    char cad[TAM];
} MENSAJE;

int main(int argc, char** argv) {

    int idCola;
    MENSAJE m;

    idCola = msgget(CLAVE, 0666); //Obtenemos la cola que crea el servidor
    m.tipo = 1L;//Seleccionamos el canal 1
    
    printf("Introduce una frase: ");
    __fpurge(stdin);
    fgets(m.cad, TAM, stdin);
    
    msgsnd(idCola, &m, sizeof (m) - sizeof (long), 0); //Enviamos el mensaje
    msgrcv(idCola, &m, sizeof (m) - sizeof (long), 2, 0); //Recibimos por el canal 2 el mensaje traducido

    printf("\nLa frase en mayusculas es: %s", m.cad);
    return (EXIT_SUCCESS);
}

