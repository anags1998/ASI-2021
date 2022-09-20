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
#include <sys/wait.h>
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

struct enviar{
	long canal;
	struct shm_dev_reg d;
};

int ds,tam,num_dev,contador,valor,idSem,idCola,idMem;
char cad[100],descr[100],*dir;
struct sockaddr_in servidor,cliente;
struct shm_dev_reg d;
struct sembuf sumar[5] = {{0,1,0},{1,1,0},{2,1,0},{3,1,0},{4,1,0}};
struct sembuf restar[5] = {{0,-1,0},{1,-1,0},{2,-1,0},{3,-1,0},{4,-1,0}};

void espera(){
wait(NULL);
}

void ejer5(){
int num_dev;
struct enviar env;

num_dev = *((int*)(dir+DEV_DUMP));
if(num_dev != -1){
	env.canal = num_dev + MSQ_TYPE_BASE;
	semop(idSem,&restar[num_dev],1);
	env.d = *((struct shm_dev_reg*)(dir+num_dev*sizeof(struct shm_dev_reg)));
	semop(idSem,&sumar[num_dev],1);
	msgsnd(idCola,&env,sizeof(env)-sizeof(long),0);
}else{
	semop(idSem,restar,5);
	for (num_dev = 0;num_dev<5;num_dev++){
	env.canal = num_dev + MSQ_TYPE_BASE;
	env.d = *((struct shm_dev_reg*)(dir+num_dev*sizeof(struct shm_dev_reg)));
	msgsnd(idCola,&env,sizeof(env)-sizeof(long),0);
	}
	semop(idSem,sumar,5);
}
}

int main(int argc, char** argv) {

idMem = shmget(CLAVE,4096,IPC_CREAT | 0666);
dir = shmat(idMem,0,0);

idSem = semget(CLAVE,5,0666);

if(fork() != 0){
idCola = msgget(CLAVE,IPC_CREAT | 0666);
signal(SIGUSR1,ejer5);
while(1){
pause();
}

}else{

signal(SIGCHLD,espera);

servidor.sin_addr.s_addr = htonl(INADDR_ANY);
servidor.sin_port = htons(3010);
servidor.sin_family = AF_INET;

bzero(&(servidor.sin_zero),8);
ds = socket(AF_INET,SOCK_DGRAM,0);
bind(ds,(struct sockaddr*)&servidor,sizeof(servidor));

while(1){
	tam = sizeof(cliente);
	recvfrom(ds,cad,100,0,(struct sockaddr*)&cliente,&tam);
	if(*cad == 'H'){
		num_dev = *((int*)(cad+1));
		cad[0] ='O';
		*((int*)(cad+1)) = num_dev;
		sendto(ds,cad,5,0,(struct sockaddr*)&cliente,sizeof(cliente));
		semop(idSem,&restar[num_dev],1);
		d = *((struct shm_dev_reg*)(dir+num_dev*sizeof(struct shm_dev_reg)));
		semop(idSem,&sumar[num_dev],1);
		d.num_dev = num_dev;
		d.estado = 1;
		sprintf(d.descr,"%s",cad+1+sizeof(int));
		semop(idSem,&restar[num_dev],1);
		*((struct shm_dev_reg*)(dir+num_dev*sizeof(struct shm_dev_reg))) = d;
		semop(idSem,&sumar[num_dev],1);
	}else if(*cad == 'W'){
		num_dev = *((int*)(cad+1));
		contador = *((int*)(cad+1+sizeof(int)));
		valor = *((int*)(cad+1+2*sizeof(int)));
		semop(idSem,&restar[num_dev],1);
		d = *((struct shm_dev_reg*)(dir+num_dev*sizeof(struct shm_dev_reg)));
		semop(idSem,&sumar[num_dev],1);
		d.num_dev = num_dev;
		d.contador[contador] = valor;
		semop(idSem,&restar[num_dev],1);
		*((struct shm_dev_reg*)(dir+num_dev*sizeof(struct shm_dev_reg))) = d;
		semop(idSem,&sumar[num_dev],1);
		cad[0] ='O';
                *((int*)(cad+1)) = num_dev;
                sendto(ds,cad,5,0,(struct sockaddr*)&cliente,sizeof(cliente));
	}else if(*cad == 'R'){
		num_dev = *((int*)(cad+1));
		contador = *((int*)(cad+1+sizeof(int)));
		semop(idSem,&restar[num_dev],1);
		d = d = *((struct shm_dev_reg*)(dir+num_dev*sizeof(struct shm_dev_reg)));
		semop(idSem,&sumar[num_dev],1);
                d.num_dev = num_dev;
		valor = d.contador[contador];
		cad[0] ='O';
                *((int*)(cad+1)) = num_dev;
		*((int*)(cad+1+sizeof(int))) = valor;
                sendto(ds,cad,9,0,(struct sockaddr*)&cliente,sizeof(cliente));
	}
}
}
return (EXIT_SUCCESS);
}


