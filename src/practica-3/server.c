
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/types.h>

#define CLAVE 0x45916038L
#define TAM_SEGMENTO 1024
#define TAM_SEMAFORO 7
#define TAM_CONTADORES 6
#define TAM_TXT 50
#define MAX_CLI_TRAD 2 

typedef struct mensaje {
    long tipo;
    char cad[TAM_TXT];
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

    int idSemaforo, idMemoria, mensajesRecibidos, charsRecibidos;
    char *dir;
    struct sembuf arraySumar[TAM_SEMAFORO] = {
        {0, 1, 0},
        {1, 1, 0},
        {2, 1, 0},
        {3, 1, 0},
        {4, 1, 0},
        {5, 1, 0},
        {6, 1, 0}
    };
    struct sembuf arrayRestar[TAM_SEMAFORO] = {
        {0, -1, 0},
        {1, -1, 0},
        {2, -1, 0},
        {3, -1, 0},
        {4, -1, 0},
        {5, -1, 0},
        {6, -1, 0}
    };

    
    idCola = msgget(CLAVE, IPC_CREAT | 0666); //Creamos la cola de mensajes
    idMemoria = shmget(CLAVE, TAM_SEGMENTO, IPC_CREAT | 0666); //Creamos el segmento de memoria compartida
    idSemaforo = semget(CLAVE, TAM_SEMAFORO, IPC_CREAT | 0666); //Creamos los semaforos
    

    
    dir = shmat(idMemoria, 0, 0); 
   
    for (i = 0; i < TAM_SEMAFORO - 1; i++) {
        semctl(idSemaforo, i, SETVAL, 1);
    }
    semctl(idSemaforo, TAM_SEMAFORO - 1, SETVAL, 2); 
 
    for (i = 0; i < TAM_CONTADORES; i++) {
        semop(idSemaforo, &arrayRestar[i], 1); 
        (*((int*) (dir + i * sizeof (int)))) = 0; 
        semop(idSemaforo, &arraySumar[i], 1); 
    }

  
    while (1) {
        msgrcv(idCola, &m, sizeof (m) - sizeof (long), 0, 0);
        
        semop(idSemaforo, &arrayRestar[1], 1);
        (*((int*) (dir + sizeof (int))))++;
        semop(idSemaforo, &arraySumar[1], 1);
       
        semop(idSemaforo, &arrayRestar[3], 1);
        (*((int*) (dir + 3 * sizeof (int))))++;
        semop(idSemaforo, &arraySumar[3], 1);
       
        semop(idSemaforo, &arrayRestar[4], 1);
        (*((int*) (dir + 4 * sizeof (int)))) = (*((int*) (dir + 4 * sizeof (int)))) + strlen(m.cad);
        semop(idSemaforo, &arraySumar[4], 1);
        
        semop(idSemaforo, &arrayRestar[1], 1);
        mensajesRecibidos = (*((int*) (dir + sizeof (int))));
        semop(idSemaforo, &arraySumar[1], 1);
      
        semop(idSemaforo, &arrayRestar[4], 1);
        charsRecibidos = (*((int*) (dir + 4 * sizeof (int))));
        semop(idSemaforo, &arraySumar[4], 1);
        
        semop(idSemaforo, &arrayRestar[5], 1);
        (*((int*) (dir + 5 * sizeof (int)))) = charsRecibidos / mensajesRecibidos;
        semop(idSemaforo, &arraySumar[5], 1);

        switch (fork()) {
            case -1: printf("Error\n");
                break;
            case 0: 
                semop(idSemaforo, &arrayRestar[6], 1);
                
                semop(idSemaforo, &arrayRestar[0], 1);
                (*((int*) dir))++;
                semop(idSemaforo, &arraySumar[0], 1);
               
                convertirMayusculas(m.cad);

                semop(idSemaforo, &arrayRestar[2], 1);
                (*((int*) (dir + 2 * sizeof (int))))++;
                semop(idSemaforo, &arraySumar[2], 1);
                
                semop(idSemaforo, &arrayRestar[3], 1);
                (*((int*) (dir + 3 * sizeof (int))))--;
                semop(idSemaforo, &arraySumar[3], 1);
            
                msgsnd(idCola, &m, sizeof (m) - sizeof (long), 0);
          
                semop(idSemaforo, &arrayRestar[0], 1);
                (*((int*) dir))--;
                semop(idSemaforo, &arraySumar[0], 1);

               
                semop(idSemaforo, &arraySumar[6], 1);
                break;
        }
    }
    return (EXIT_SUCCESS);
}


