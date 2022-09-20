
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define CLAVE 0x45916038L
#define TAM_SEGMENTO 1024 
#define TAM_CADENA 20

int main(int argc, char** argv) {
    
    int idMemoria, secreto1, offsetSecreto2;
    char *dir, cadena[TAM_CADENA];
   
	union senum{
		int val;
		struct semid_ds *buf;
		unsigned short *array;
		struct seminfo *__buf;

	};
    int idSemaforo;
   union senum arg; 
    int idSemaforo2;
   
    struct sembuf sumar = {0, 1, 0}, restar = {0, -1, 0};
    int num;
    
    struct sembuf arraySumar[2] = {
        {0, 1, 0},
        {1, 1, 0}
    }, arrayRestar[2] = {
        {0, -1, 0},
        {1, -1, 0}
    };

   
    


    
    idMemoria = shmget(CLAVE, TAM_SEGMENTO, 0666);
    
    dir = shmat(idMemoria, 0, 0); //Asociar segmento de memoria (idMemoria) al 
  
    secreto1 = *((int*) dir); //Obtenemos el entero contenido al comienzo
    
    offsetSecreto2 = *((int*) (dir + sizeof (int)));
    
    sprintf(cadena, "%5s", dir + offsetSecreto2);
    
    sleep(3);
    shmdt(dir);

    printf("Fin del ejercicio 1\n");
    getchar();

    
    idMemoria = shmget(CLAVE, TAM_SEGMENTO, IPC_CREAT | 0666);

    dir = shmat(idMemoria, 0, 0);
    
    sprintf(dir, "%5s", cadena); //dir <-- cadena = <secreto2> = <999>
   
    sscanf(cadena, "<%d>", &offsetSecreto2); //Offset = 999
    sprintf(dir + offsetSecreto2 + 16, "<%3d>", secreto1); //inicio + 999 + 16 <-- <secreto1>
    
    sleep(2);
    shmdt(dir);
    printf("Fin del ejercicio 2.\n");
    getchar();


    
    idSemaforo = semget(CLAVE, 1, IPC_CREAT | 0666); 
    arg.val = 38;
    semctl(idSemaforo, 0, SETVAL,arg); 
    printf("Fin del ejercicio 3.\n");
    getchar();


   
    idSemaforo2 = semget(CLAVE, 2, IPC_CREAT | 0666); 
    semctl(idSemaforo2, 0, SETVAL, 1); 
    semctl(idSemaforo2, 1, SETVAL, 2); 
    printf("Fin del ejercicio 4.\n");
    getchar();


    
    idSemaforo = semget(CLAVE, 1, 0666);
    idMemoria = shmget(CLAVE, 1024, 0666);
    dir = shmat(idMemoria, 0, 0);
    
    semop(idSemaforo, &restar, 1); 
    num = *((int*) dir); //Obtenemos <8>
    *((int*) dir) = -num; //Sustituimos por <-8> en la dirección
    
    sleep(3);
    
    semop(idSemaforo, &sumar, 1);
    printf("Fin del ejercicio 5.\n");
    getchar();


 
    idSemaforo = semget(CLAVE, 2, 0666);
    idMemoria = shmget(CLAVE, 1024, 0666);
    dir = shmat(idMemoria, 0, 0);
    
    semop(idSemaforo, arrayRestar, 2); 
    num = *((int*) dir); 
    *((int*) dir) = -num; 
    sleep(3);
    
    semop(idSemaforo, arraySumar, 2); 
    printf("Fin del ejercicio 6.\n");
    getchar();

    
    idMemoria = shmget(CLAVE, 1024, 0666);
    dir = shmat(idMemoria, 0, 0);
    idSemaforo = semget(CLAVE, 1, IPC_CREAT | 0666);
    
    semop(idSemaforo, &restar, 1); 
    semctl(idSemaforo, 0, IPC_RMID, NULL); 
    shmctl(idMemoria, IPC_RMID, NULL); 

    printf("Fin del ejercicio 7.\nFIN DE LA PRUEBA.\n");
    return (EXIT_SUCCESS);
}

