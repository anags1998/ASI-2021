

#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define CLAVE 0x45916038L
#define TAM_SEGMENTO 1024
#define TAM_SEMAFORO 7

int main(int argc, char** argv) {
    int idMemoria, idSemaforo;
    int clientesActivos = 0, mensajesRecibidos = 0, mensajesTraducidos = 0,
            mensajesPorTraducir = 0, datosRecibidos = 0, longitudMedia = 0;
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

    
    idSemaforo = semget(CLAVE, TAM_SEMAFORO, 0666);
    idMemoria = shmget(CLAVE, TAM_SEGMENTO, 0666);

    
    dir = shmat(idMemoria, 0, 0);

    
    semop(idSemaforo, &arrayRestar[0], 1);
    clientesActivos = *((int*) dir);
    printf("\nNúmero de clientes activos. %d\n", clientesActivos);
    semop(idSemaforo, &arraySumar[0], 1);
   
    semop(idSemaforo, &arrayRestar[1], 1);
    mensajesRecibidos = *((int*) (dir + sizeof (int)));
    printf("\nNúmero de mensajes recibidos. %d\n", mensajesRecibidos);
    semop(idSemaforo, &arraySumar[1], 1);
    
    semop(idSemaforo, &arrayRestar[2], 1);
    mensajesTraducidos = *((int*) (dir + 2 * sizeof (int)));
    printf("\nNúmero de mensajes traducidos. %d\n", mensajesTraducidos);
    semop(idSemaforo, &arraySumar[2], 1);
 
    semop(idSemaforo, &arrayRestar[3], 1);
    mensajesPorTraducir = *((int*) (dir + 3 * sizeof (int)));
    printf("\nNúmero de mensajes por traducir. %d\n", mensajesPorTraducir);
    semop(idSemaforo, &arraySumar[3], 1);
    
    semop(idSemaforo, &arrayRestar[4], 1);
    datosRecibidos = *((int*) (dir + 4 * sizeof (int)));
    printf("\nNúmero de caracteres recibidos. %d\n", datosRecibidos);
    semop(idSemaforo, &arraySumar[4], 1);
    
    semop(idSemaforo, &arrayRestar[5], 1);
    longitudMedia = *((int*) (dir + 5 * sizeof (int)));
    printf("\nLongitud media de mensajes recibidos. %d\n", longitudMedia);
    semop(idSemaforo, &arraySumar[5], 1);
   /* 
    FILE *pf;
    pf = fopen(nomFich, "ab+");
    if (pf != (FILE*) NULL) {
        fwrite(cad, sizeof (cad), 1, pf);
        fwrite(&clientesActivos, sizeof (int), 1, pf);
        fwrite(&mensajesRecibidos, sizeof (int), 1, pf);
        fwrite(&mensajesTraducidos, sizeof (int), 1, pf);
        fwrite(&mensajesPorTraducir, sizeof (int), 1, pf);
        fwrite(&datosRecibidos, sizeof (int), 1, pf);
        fwrite(&longitudMedia, sizeof (int), 1, pf);
        fclose(pf);
    }

    
    FILE *pf2;
    pf2 = fopen(nomFich, "rb");
    if (pf2 != (FILE*) NULL) {
        fread(cad, sizeof (cad), 1, pf2);
        fread(&clientesActivos, sizeof (int), 1, pf2);
        fread(&mensajesRecibidos, sizeof (int), 1, pf2);
        fread(&mensajesTraducidos, sizeof (int), 1, pf2);
        fread(&mensajesPorTraducir, sizeof (int), 1, pf2);
        fread(&datosRecibidos, sizeof (int), 1, pf2);
        fread(&longitudMedia, sizeof (int), 1, pf2);
        fclose(pf2);
    }
     */
    return (EXIT_SUCCESS);
}

