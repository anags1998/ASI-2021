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

#define SIZE_SHM 4096		// Memoria partekatuaren segmentu-luzera

#define MAX_DEV	4		// Gailu kopuru maximoa #[1..4]

#define DEVOFFSET 200		// Gailuen arteko Offseta

#define MAX_SEM 5	// Arrayaren semaforo kopurua

/* UDP mezuak*/
#define HELLO	"HLO"
#define OK	"OK"
#define WRITE	'W'

/* Mezu-ilararen komandoak */
#define DUMP	'D'
#define CLAVE 0x45916038L

struct shm_dev_reg{
	int egoera;	/* 1 lanean, gainerako edozer libre */
	int num_dev;	/* gailuaren zenbakia #[1..4] */
	char deskr[15];	/* gailuaren deskripzioa */
	int n_cont;	/* gailuaren kontagailu kopurua */
};


struct msgq_input {
	char cmd;	/* komandoa */
	int num_dev;	/* gailu zenbakia */
};

struct recibir{
	long canal;
	struct msgq_input men;
};
struct enviar{
	long canal;
	struct shm_dev_reg disp;
};

void espera(){
wait(NULL);
}

int main(int argc, char** argv) {

struct sockaddr_in servidor,cliente;
int ds,i,tam,pidM,num_dev,indice,valor,n,n_cont,idMem,idCola,idSem;
char cad[100],*dir,descr[100];
struct shm_dev_reg d;
struct sembuf restar[5] = {{0,-1,0},{1,-1,0},{2,-1,0},{3,-1,0},{4,-1,0}};
struct sembuf sumar[5] = {{0,1,0},{1,1,0},{2,1,0},{3,1,0},{4,1,0}};
struct recibir rec;
struct enviar env;

pidM = atoi(argv[1]);

idMem = shmget(CLAVE,4096,IPC_CREAT | 0666);
dir = shmat(idMem,0,0);

idSem = semget(CLAVE,5,0666);

idCola = msgget(CLAVE,IPC_CREAT | 0666);

if(fork() != 0){
	while(msgrcv(idCola,&rec,sizeof(struct recibir)-sizeof(long),pidM,0)>0) {
		if(rec.men.num_dev != 0){
			semop(idSem,&restar[rec.men.num_dev],1);
			memcpy(&env.disp,dir+(rec.men.num_dev-1)*DEVOFFSET,DEVOFFSET);
			semop(idSem,&sumar[rec.men.num_dev],1);
			env.canal = rec.men.num_dev;
			msgsnd(idCola,&env,DEVOFFSET,0);
		}else{
			semop(idSem,&restar[1],4);
			for(i=1;i<5;i++){
			memcpy(&env.disp,dir+(i-1)*DEVOFFSET,DEVOFFSET);
                        env.canal = i;
                        msgsnd(idCola,&env,DEVOFFSET,0);

			}
			semop(idSem,&sumar[1],4);
		}
	}

}else{
	signal(SIGCHLD,espera);
	servidor.sin_addr.s_addr = htonl(INADDR_ANY);
	servidor.sin_port= htons(3010);
	servidor.sin_family = AF_INET;
	bzero(&(servidor.sin_zero),8);

	ds = socket(AF_INET,SOCK_DGRAM,0);
	bind(ds,(struct sockaddr*)&servidor,sizeof(servidor));

	while(1){
		tam = sizeof(cliente);
		n = recvfrom(ds,cad,100,0,(struct sockaddr*)&cliente,&tam);
		cad[n] = '\0';
		if(*cad == 'H'){
			sscanf(cad,"HLO <%d> <%d> %s",&num_dev,&n_cont,descr);
			sprintf(cad,"OK <%d>",num_dev);
			sendto(ds,cad,strlen(cad),0,(struct sockaddr*)&cliente,sizeof(cliente));
			semop(idSem,&restar[num_dev],1);
			d = *((struct shm_dev_reg*)(dir+(num_dev-1)*DEVOFFSET));
			semop(idSem,&sumar[num_dev],1);
			d.num_dev = num_dev;
			d.n_cont = n_cont;
			d.egoera = 1;
			sprintf(d.deskr,"%s",descr);
			semop(idSem,&restar[num_dev],1);
			*((struct shm_dev_reg*)(dir+(num_dev-1)*DEVOFFSET)) = d;
			semop(idSem,&sumar[num_dev],1);
		}else if(*cad == 'W'){
			num_dev = *((int*)(cad+1));
			indice = *((int*)(cad+1+sizeof(int)));
			valor = *((int*)(cad+1+2*sizeof(int)));
			semop(idSem,&restar[num_dev],1);
			*((int*)(dir+(num_dev-1)*DEVOFFSET+sizeof(struct shm_dev_reg)+indice*sizeof(int))) = valor;
			semop(idSem,&sumar[num_dev],1);
			sprintf(cad,"OK <%d>",num_dev);
                	sendto(ds,cad,strlen(cad),0,(struct sockaddr*)&cliente,sizeof(cliente));
		}
	}

}
return (EXIT_SUCCESS);
}

