#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/msg.h>

#define FIFO1 "/tmp/fifo_monitor_1"
#define FIFO2 "/tmp/fifo_monitor_2"
#define CLAVE 0x45916038L

typedef struct mensaje{
    long tipo;
    char cad[100];
}MENSAJE;


int main(int argc, char** argv) {
    
    int pidMonitor, fifo1, fifo2, numBytes, idCola;
    char cad[100];
    MENSAJE m1, m2, m3;
    
    pidMonitor=atoi(argv[1]);
    
    fifo1=open(FIFO1, O_RDWR);
    numBytes=read(fifo1, cad, 100);
    printf("Numero de bytes leidos: %d y el secreto 1: %s\n", numBytes, cad);
    printf("Fin del ejercicio 1\n");
    getchar();
    
    mkfifo(FIFO2, 0666);
    fifo2=open(FIFO2,O_RDWR);
    write(fifo2,cad, strlen(cad));
    read(fifo1, cad, 100);
    write(fifo2, cad, strlen(cad));
    printf("Fin del ejercicio 2\n");
    getchar();
    
    idCola=msgget(CLAVE,0666);
    msgrcv(idCola, &m1, sizeof(m1)-sizeof(long),0,0);
    printf("%s/n", m1.cad);
    printf("Fin del ejercicio 3\n");
    getchar();
    
    idCola=msgget(CLAVE, IPC_CREAT|0666);
    m2.tipo=1L;
    sprintf(m2.cad, "<%3ld>%s", m1.tipo, m1.cad);
    msgsnd(idCola, &m2, sizeof(m2)-sizeof(long), 0);
    printf("Fin del ejercicio 4 \n");
    getchar();
    
    msgrcv(idCola, &m3, sizeof(m3)-sizeof(long), getpid(), 0);
    printf("Fin del ejercicio 5 \n");
    getchar();
    
    m3.tipo=(long)pidMonitor;
    msgsnd(idCola, &m3, sizeof(m3)-sizeof(long),0);
    printf("Fin del ejercicio 6 \n");
    getchar();
    
    msgsnd(idCola,&m3,sizeof(m3)-sizeof(long),0);
    unlink(FIFO1);
    unlink(FIFO2);
    msgctl(idCola, IPC_RMID, NULL);
    
    return (EXIT_SUCCESS);
}


