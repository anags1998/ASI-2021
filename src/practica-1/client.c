#include <stdio.h>
#include <stdlib.h>
#include <stdio_ext.h>
#include <string.h>
#include <ctype.h>

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <unistd.h>

#include "client.h"		// Include propio en directorio actual

#define NOMFICH "Fichero.txt"

/********************* PROGRAMA DE LA PRACTICA X **************************/


void atenderSenialH1() {
    printf("H1: Señal recibida.\n");
}

void quitarSalto(char *cad) {
    if(cad[strlen(cad)-1]=='\n'){
        cad[strlen(cad)-1]='\0';
    }
}

void convertirMayusculas(char*  cad) {
    for( int i = 0; cad[i] != '\0'; i++){
    cad[i] = toupper(cad[i]);
    }
}

void matarH1(){
    printf("H1: Voy a morir\n");
}

void matarH2(){
    printf("H2: Voy a morir\n");
    printf("FIN DEL PROGRAMA\n");
}

void esperaPadre(){
    printf("P: SIGUSR1 recibido.\n");
}

int main(int argc, char** argv) {
    int pidP, pidH1, pidH2;
    char cad[256];
    FILE*  f;

    printf("EMPIEZA EL PROGRAMA:\n");

    pidP = getpid(); //Devolver PID del proceso
    pidH1 = fork(); //Crear proceso idéntico al padre, y devolver PID del proceso

    switch (pidH1) {
        case -1: printf("Error\n");
            break;
 
        case 0: printf("H1: Soy el hijo 1 con pid %d.\n", getpid());
            printf("H1: Me duermo\n");
            signal(SIGUSR1, atenderSenialH1);
            pause();
            
            printf("H1: He despertado!\n");
            printf("H1: Voy a abrir %s\n", NOMFICH);
            f=fopen(NOMFICH, "r");
            if(f!=(FILE*)NULL){
                fread(&cad, sizeof(cad), 1, f);
                convertirMayusculas(cad);
                printf("H1: He traducido a: %s\n", cad);
            }
            fclose(f);
            printf("H1: Fichero cerrado, aviso a PADRE con SIGUSR1.\n");
            kill(pidP, SIGUSR1);
            printf("H1: Espero a morir.\n");
            signal(SIGUSR1,matarH1);
            pause();
            break;

        default: pidH2 = fork();
            switch (pidH2) {
                case -1: printf("Error 2\n");
                    break;
                   
                case 0: printf("H2: Soy el hijo 2 con pid %d.\n", getpid());
                    printf("H2: Espero una cadena\n");
                    fgets(cad, 256, stdin);
                    quitarSalto(cad);
                    printf("H2: La cadena es: %s\n", cad);
                    printf("H2: Voy a abrir fichero\n"); 
                    f = fopen(NOMFICH, "w");
                    if (f != (FILE*) NULL) {
                        fwrite(cad, sizeof (cad), 1, f);
                        printf("H2: Hecho!\n");
                    }

                    fclose(f);
                    printf("H2: Fichero cerrado, aviso a H1.\n");
                    kill(pidH1, SIGUSR1);

                    printf("H2: Espero a morir.\n");
                    signal(SIGUSR1, matarH2);
                    pause();
                    break;
                   
                default: printf("P: Soy el padre con pid %d.\n", getpid());
                    printf("P: Me duermo\n");
                    printf("P: Espero SIGUSR1 de H1 para terminar.\n");
                    signal(SIGUSR1, esperaPadre);
                    pause();
                    
                    printf("P: Procedo a notificar muerte a H1 y H2.\n");
                    kill(pidH1, SIGUSR1);
                    kill(pidH2, SIGUSR1);
		     
		   break;
            }
	break;

    }

    return (EXIT_SUCCESS);
}

