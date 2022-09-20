#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/shm.h>
#include <signal.h>
#include <sys/sem.h>
#include <sys/msg.h>


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

struct mensaje{
    char cad[DEVOFFSET+1];
    long canal;
};

struct mensaje2{
    long canal;
    struct msgq_input m;
    
};


void esperar(){
    wait(NULL);
}
int main(int argc, char** argv) {
    struct sockaddr_in servidor, cliente;
    int ds, tam, num_dev, n_cont,idMem,indice,valor,n,idSem,idCola,pidMonitor,i;
    char cad[100],decr[100],*dir;
    struct shm_dev_reg x;
    struct sembuf asumar[5]={{0,1,0},{1,1,0},{2,1,0},{3,1,0},{4,1,0}};
struct sembuf arestar[5]={{0,-1,0},{1,-1,0},{2,-1,0},{3,-1,0},{4,-1,0}};
struct mensaje respuesta;
struct mensaje2 mensaje;
   
  pidMonitor = atoi(argv[1]);
idCola = msgget(CLAVE,IPC_CREAT|0666);
  
    idMem = shmget(CLAVE,SIZE_SHM,IPC_CREAT|0666);
    dir = shmat(idMem,0,0);
idSem = semget(CLAVE,MAX_SEM,0666);
    
    if(fork()==0){
        signal(SIGCHLD,esperar);
        servidor.sin_addr.s_addr = htonl(INADDR_ANY);
        servidor.sin_port = htons(3010);
        servidor.sin_family = AF_INET;
        bzero(&(servidor.sin_zero),8);

        ds = socket(AF_INET, SOCK_DGRAM,0);
        bind(ds,(struct sockaddr*)&servidor,sizeof(servidor));
        while(1){
            tam = sizeof(cliente);
            n=recvfrom(ds,cad,100,0,(struct sockaddr*)&cliente,&tam);
            cad[n]='\0';
            printf("\nCadena recibida: %s", cad);
          //  if(fork()==0){
                if(*cad=='H'){
                    sscanf(cad,"HLO <%d> <%d> %s",&num_dev,&n_cont,decr);
                    sprintf(cad,"OK <%d>",num_dev);
                    sendto(ds,cad,strlen(cad),0,(struct sockaddr*)&cliente,sizeof(cliente));
                   semop(idSem,&arestar[num_dev],1);
                    x= *((struct shm_dev_reg*)(dir+(num_dev-1)*DEVOFFSET));
                    semop(idSem,&asumar[num_dev],1);
                    x.num_dev = num_dev;
                    x.n_cont = n_cont;
                    x.egoera = 1;
                    sprintf(x.deskr,"%s",decr);
                    semop(idSem,&arestar[num_dev],1);
                    *((struct shm_dev_reg*)(dir+(num_dev-1)*DEVOFFSET)) = x;
                   semop(idSem,&asumar[num_dev],1);
                }else if(*cad=='W'){
                        num_dev = *((int*)(cad+1));
                        indice = *((int*)(cad+1+sizeof(int)));
                        valor = *((int*)(cad+1+2*sizeof(int)));
                        semop(idSem,&arestar[num_dev],1);
                        *((int*)(dir+(num_dev-1)*DEVOFFSET+sizeof(struct shm_dev_reg)+indice*sizeof(int))) = valor;
                        semop(idSem,&asumar[num_dev],1);
                        sprintf(cad,"OK <%d>",num_dev);
                        printf("\nCADENA: %s", cad);
                        sendto(ds,cad,strlen(cad),0,(struct sockaddr*)&cliente,sizeof(cliente));
                }
              //  exit(0);
            //}
        
        exit(0);
    }
}else{
       
      while(msgrcv(idCola,&mensaje,sizeof(struct mensaje2)-sizeof(long),pidMonitor,0)>0){
            
            printf("\nMensaje recibido %c %d", mensaje.m.cmd, mensaje.m.num_dev);
            if(mensaje.m.cmd=='D'){
                if(mensaje.m.num_dev != '0'){
                    printf("\nEntra al IF");
                    respuesta.canal = mensaje.m.num_dev;
                   semop(idSem,&arestar[mensaje.m.num_dev-1],1);
                    memcpy(respuesta.cad,dir+(mensaje.m.num_dev-1)*DEVOFFSET,DEVOFFSET);
                    
                  semop(idSem,&asumar[mensaje.m.num_dev-1],1);
                    msgsnd(idCola,&respuesta, sizeof(respuesta)-sizeof(long),0);
                }else{
                   printf("\nEntra el ELSE");
                    semop(idSem,arestar,4);
                    for(i=1;i<=4;i++){
                        respuesta.canal = i;
                        memcpy(respuesta.cad,dir+(mensaje.m.num_dev-1)*DEVOFFSET,DEVOFFSET);
                        msgsnd(idCola,&respuesta,sizeof(respuesta)-sizeof(long),0);
                        
                    }
                   semop(idSem,asumar,4);
                }
            }
        }
       
        
    }
    return (EXIT_SUCCESS);
}


