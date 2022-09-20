#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <sys/signal.h>

#define SERVER_PORT 3010
#define SERVERIP "127.0.0.1"

#define CLAVE 0x45916038L	// Clave de los recursos. Sustituir por DNI.

#define SIZE_SHM 4096		// Tamaño del segmento de memoria compartida

#define MAX_DEV	5		// Máximo # dispositivos # [0..4]



/* El dispositivo 1 comienza en la posición 0 de la memoria
	El dispositivo 2 en la posición 0+OFFSET
	El dispositivo 3 en OFFSET*2
	El dispositivo 4 en OFFSET*3
*/

/* Formato de un registro de dispositivo */

struct shm_dev_reg{
	int estado;	/* 1 activo, cualquier otra cosa libre */
	int num_dev;	/* numero de dispositivo #[1..4] */
	char descr[16];	/* descripción del dispositivo*/
	int contador[4];	/* contadores para el dispositivo*/
};

/* Detrás del registro del dispositivo (+sizeof(struct shm_dev_reg)) 
   se almacenan uno tras otro sus valores en formato (int)
*/


/* El array de semáforos a crear tiene tamaño 5
	El semáforo 0 no se utilizará
	El semáforo 1 controla el acceso al registro del dispositivo 1 
	El semáforo # controla el acceso al registro del dispositivo # [1..4]
*/
#define MAX_SEM 5	// Número de semáforos en el array

/* Lista de mensajes UDP */
#define HELLO	"HLO"
#define OK	"OK"
#define WRITE	'W'

/* Comandos DUMP cola de mensajes mediante señales */

#define MSQ_TYPE_BASE 100
#define DUMP_ALL -1			//Código para el dump de la totalidad de los dispositivos
#define DEV_DUMP 1500		// Posición del dispositivo del que se pide el dump

typedef struct mensaje{
    long canal;
    struct shm_dev_reg dato;
}MENSAJE;

struct sockaddr_in servidor, cliente;
int ds,tam,idMem,nd,cont,valor,idSem,idCola;
char cad[100],resp[100],*dir;
struct shm_dev_reg d;
struct sembuf sumar[5]={{0,1,0},{1,1,0},{2,1,0},{3,1,0},{4,1,0}};
struct sembuf restar[5]={{0,-1,0},{1,-1,0},{2,-1,0},{3,-1,0},{4,-1,0}};
    

void esperar(){
    wait(NULL);
}

void tratarSenial(){
    
    MENSAJE m;
    nd = *((int*)(dir+DEV_DUMP));
    if(nd!=-1){
        
        m.canal = nd + MSQ_TYPE_BASE;
        semop(idSem,&restar[nd],1);
        m.dato = *((struct shm_dev_reg*)(dir+nd*sizeof(struct shm_dev_reg)));
        semop(idSem,&sumar[nd],1);
        msgsnd(idCola,&m,sizeof(m)-sizeof(long),0);
      
    }else{
        semop(idSem,restar,MAX_SEM);
        for(nd=0;nd<=4;nd++){
            m.canal = nd + MSQ_TYPE_BASE;
            m.dato = *((struct shm_dev_reg*)(dir+nd*sizeof(struct shm_dev_reg)));
            msgsnd(idCola,&m,sizeof(m)-sizeof(long),0);
        }
        semop(idSem,sumar,MAX_SEM);
    }
}

int main(int argc, char** argv) {

    struct sockaddr_in servidor, cliente;
    int ds,tam,idMem,nd,cont,valor,idSem,idCola;
    char cad[100],resp[100],*dir;
    struct shm_dev_reg d;
    struct sembuf sumar[5]={{0,1,0},{1,1,0},{2,1,0},{3,1,0},{4,1,0}};
    struct sembuf restar[5]={{0,-1,0},{1,-1,0},{2,-1,0},{3,-1,0},{4,-1,0}};
    
    idMem = shmget(CLAVE,SIZE_SHM,IPC_CREAT|0666);
    dir = shmat(idMem,0,0);
    idSem = semget(CLAVE,5,0666);
    
    if(fork()==0){
        signal(SIGUSR1, tratarSenial);
        idCola = msgget(CLAVE,IPC_CREAT|0666);
        while(1){
            pause();
        }
        exit(0);
    }else{
        signal(SIGCHLD,esperar);
        servidor.sin_addr.s_addr = htonl(INADDR_ANY);
        servidor.sin_port = htons(3010);
        servidor.sin_family = AF_INET;
        bzero(&(servidor.sin_zero),8);
        ds = socket(AF_INET,SOCK_DGRAM,0);
        bind(ds,(struct sockaddr*)&servidor,sizeof(servidor));
        while(1){
            tam = sizeof(cliente);
            recvfrom(ds,cad,100,0,(struct sockaddr*)&cliente,&tam);
            if(fork()==0){
                if(*cad=='H'){
                    d.num_dev = *((int*)(cad+1));
                    sprintf(d.descr,"%s",cad+1+sizeof(int));
                    resp[0] = 'O';
                    *((int*)(resp+1)) = d.num_dev;
                    d.estado = 1;
                    sendto(ds,resp,5,0,(struct sockaddr*)&cliente,sizeof(cliente));
                    *((struct shm_dev_reg*)(dir+d.num_dev*sizeof(struct shm_dev_reg)))=d;
                    
                }else if(*cad=='W'){
                    nd = *((int*)(cad+1));
                    cont = *((int*)(cad+1+sizeof(int)));
                    valor = *((int*)(cad+1+2*sizeof(int)));
                    semop(idSem,&restar[nd],1);
                    d = *((struct shm_dev_reg*)(dir+nd*sizeof(struct shm_dev_reg)));
                    d.contador[cont] = valor;
                    *((struct shm_dev_reg*)(dir+nd*sizeof(struct shm_dev_reg)))=d;
                    semop(idSem,&sumar[nd],1);
                    resp[0] = 'O';
                    *((int*)(resp+1)) = nd;
                    sendto(ds,resp,5,0,(struct sockaddr*)&cliente,sizeof(cliente));
                }else if(*cad=='R'){
                    nd = *((int*)(cad+1));
                    cont = *((int*)(cad+1+sizeof(int)));
                    semop(idSem,&restar[nd],1);
                    d = *((struct shm_dev_reg*)(dir+nd*sizeof(struct shm_dev_reg)));
                    semop(idSem,&sumar[nd],1);
                    valor = d.contador[cont];
                    resp[0] = 'O';
                    *((int*)(resp+1)) = nd;
                    *((int*)(resp+1+sizeof(int))) = valor;
                    sendto(ds,resp,9,0,(struct sockaddr*)&cliente,sizeof(cliente));
                }
                exit(0);
            }
        }
        
        wait(NULL);
    }
    return (EXIT_SUCCESS);
}


