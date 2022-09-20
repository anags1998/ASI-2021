
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/types.h>


int pidMonitor, pidH1, pidH2, contHilos, finBucle, contSeniales;

void atencionSigusr2() {
    printf("Rutina de atención a la señal SIGUSR2, enviar SIGUSR2 de vuelta\n");
    kill(pidMonitor, SIGUSR2);
}

void *funcionThread() {
    int i;
    for (i = 0; i < 4; i++) {
        printf("\nSoy el thread %li", pthread_self());
        sleep(5);
    }
    pthread_exit(0);
}

void ejercicio4() {
    pthread_t t;
    pthread_create(&t, NULL, &funcionThread, NULL);
    pthread_detach(t); //Para que el thread sea independietne
    contHilos++;
}

void salirDelBucle() {
    finBucle = 1;
}

void ejercicio5(){
    kill(pidMonitor, SIGUSR2);
    contSeniales++;
}

int main(int argc, char** argv) {
    //EJERCICIO 1
    pidMonitor = atoi(argv[1]); //String to integer
    kill(pidMonitor, SIGUSR1); //Enviar SIGUSR1 al monitor
    sleep(2); //Asegurarse de que sigue vivo >1seg
    printf("Fin del ejercicio 1\n");
    getchar(); //Para controlar el flujo del programa

    //EJERCICIO 2
    kill(pidMonitor, SIGUSR1); //Enviar SIGUSR1
    signal(SIGUSR2, atencionSigusr2); //Definimos la atención a la señal SIGUSR2
    pause(); //Cuando se desbloquee el proceso al recibir la señal SIGUSR2, se ejecutará la rutina de atención
    printf("Fin del ejercicio 2\n");
    getchar();

    //EJERCICIO 3
    kill(pidMonitor, SIGUSR1);
    sleep(2);
    pidH1 = fork();
    switch (pidH1) {
        case -1: printf("Error\n");
            break;
        case 0: printf("Soy el hijo 1\n");
            //sleep(1); //Para que los hijos no mueran antes de ser detectados
            alarm(20); //Mandamos SIGALRM pasados 20 segs
            signal(SIGALRM, salirDelBucle);
            signal(SIGUSR2, ejercicio4);
            finBucle = 0;
            while (finBucle == 0) {
                pause();
            }
            printf("\nSe han creado %d hilos.\n", contHilos);
            break;
        default: pidH2 = fork();
            switch (pidH2) {
                case -1: printf("Error 2\n");
                    break;
                case 0: printf("Soy el hijo 2\n");
                    //sleep(1); //Para que los hijos no mueran antes de ser detectados
                    //EJERCICIO 5
                    /*Para iniciar el ejercicio se espera recibir la señal SIGUSR1.
                    El proceso monitor enviará la señal SIGUSR1 al hijo 2 del proceso cliente
                    para avisar del inicio de un periodo de unos 60 segundos donde se mandará
                    un número aleatorio de señales SIGUSR2 al proceso cliente hijo 2.
                    Se espera que el proceso cliente responda a cada señal SIGUSR2 con otra señal
                    SIGUSR2 dirigida al proceso monitor.
                     El final del periodo estará marcado por una señal SIGUSR1 desde el monitor al
                    proceso cliente. El programa cliente debe contar las señales SIGUSR2 recibidas
                    entre las dos señales SIGUSR1 de inicio y final.
                    Se revelará el secreto <5> cuando se compruebe que se ha respondido adecuadamente
                    en número y tiempo a las señales SIGUSR2 transmitidas.
                    */
                
                /*Hacemos un primer bucle para que el hijo 2 se quede esperando a la primera señal SIGUSR1*/
                signal(SIGUSR1, salirDelBucle);
                
                finBucle=0;
                while(finBucle==0){
                    pause();
                }
                
                /*Hacemos el bucle de espera de señales SIGUSR2*/
                contSeniales=0;
                signal(SIGUSR2, ejercicio5);
                finBucle=0;
                while(finBucle==0){
                    pause();
                }
                
                printf("Numero de señales recibidas: %d\n", contSeniales);
                    break;
                default: printf("Soy el padre\n");
                    kill(pidMonitor, SIGUSR1);
                    printf("Fin del ejercicio 3\n");
                    getchar();
                    waitpid(pidH1, NULL, 0);
                    printf("Fin del ejercicio 4\n");
                    getchar();
                    kill(pidMonitor, SIGUSR1);
                    waitpid(pidH2, NULL, 0);
                    printf("Fin del ejercicio 5\n");
                    getchar();
                    //EJERCICIO 6
                    /*Para iniciar el ejercicio se espera recibir la señal SIGUSR1.
                    La aplicación cliente tendrá que enviar la señal SIGUSR1 y después de temporizar
                    un segundo debe de cerrar correctamente todos los procesos sin dejar procesos zombies.
                    Se revelará el secreto <6> cuando se compruebe que se ha cerrado adecuadamente la
                    aplicación cliente.
                    Para iniciar el ejercicio se espera recibir la señal SIGUSR1.

                    La aplicación cliente tendrá que enviar la señal SIGUSR1 y después de temporizar
                    un segundo debe de cerrar correctamente todos los procesos sin dejar procesos zombies.
                    Se revelará el secreto <6> cuando se compruebe que se ha cerrado adecuadamente la
                    aplicación cliente.
                    */
                    kill(pidMonitor, SIGUSR1);
                    sleep(1);
            }
    }
    return (EXIT_SUCCESS);
}

