
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

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

    char cad[TAM];
    int pidH1, pidH2, f[2], i;

    pipe(f); //Creamos el Pipe
    pidH1 = fork(); //Creamos la jerarquía de procesos
    switch (pidH1) {
        case -1: printf("Error\n");
            break;
        case 0: close(f[0]); //Cerramos el descriptor de lectura
            printf("H1: Introduce una frase: ");
            fflush(stdin);
            __fpurge(stdin);
            fgets(cad, TAM, stdin);
            write(f[1], cad, strlen(cad) + 1); //Escribimos en el pipe
            break;

        default: pidH2 = fork();
            switch (pidH2) {
                case -1: printf("Error 2\n");
                    break;
                case 0: close(f[1]); //Cerramos el descriptor de escritura
                    read(f[0], cad, TAM); //Leemos del Pipe
                    convertirMayusculas(cad);
                    printf("H2: La cadena en mayusculas es: %s", cad);
                    break;
                default: wait(NULL);
                    wait(NULL);
                    printf("FIN\n");
            }

    }


    return (EXIT_SUCCESS);
}


