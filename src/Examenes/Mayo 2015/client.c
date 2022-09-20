//INCLUDES--------------------------------------------------------
#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <unistd.h>
#include <strings.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/shm.h>

//VARIABLES GLOBALES-----------------------------------------------
#define CLAVE 0x45916038L
#define MEM 1024
#define PORT 3010
#define TAM 256
#define MAXCX 8



void espera(){
wait(NULL);
return;
}

//FUNCION PRINCIPAL-------------------------------------------------

int main(int argc, char **argv){

struct sockaddr_in servidor,cliente;
int dc,ds,tam,pidH,idMem,idSem,num,desp,desp1,op1,op2;
char cad[256],cad1[256],*dir,*cadena,*cadena1;
struct sembuf sumar[3]={{0,1,0},{1,1,0},{2,1,0}},restar[3]={{0,-1,0},{1,-1,0},{2,-1,0}};

idMem = shmget(CLAVE,1024,0666);
dir = shmat(idMem,0,0);
idSem = semget(CLAVE,3,0666);
signal(SIGCHLD,espera);
servidor.sin_addr.s_addr = INADDR_ANY;
servidor.sin_port = htons(PORT);
servidor.sin_family = AF_INET;
bzero(&(servidor.sin_zero),8);

ds = socket(AF_INET,SOCK_STREAM,0);
bind(ds,(struct sockaddr*)&servidor,sizeof(servidor));
listen(ds,SOMAXCONN);

while(1){
	tam = sizeof(cliente);
	dc = accept(ds,(struct sockaddr*)&cliente,&tam);
	semop(idSem,&sumar[0],1);
	if(fork() == 0){
		semop(idSem,&restar[1],1);
		close(ds);
		while(read(dc,cad,100)>0){
			if(*cad == 'h'){	
				sprintf(cad1,"OK\n");
				write(dc,cad1,strlen(cad1)+1);
			}else{
			cadena = strtok(cad,">");
			cadena1 = strtok(NULL,"");
			sscanf(cadena,"<%d",&desp);
			if(*cadena1 == '='){
			sscanf(cadena1,"=%d",&num);
			semop(idSem,&restar[2],1);
			*(int*)(dir+desp)=num;
			semop(idSem,&sumar[2],1);
			sprintf(cad1,"\n");
			write(dc,cad1,strlen(cad1));
			}else if(*cadena1 == '?'){
			semop(idSem,&restar[2],1);
			num = *(int*)(dir+desp);
			semop(idSem,&sumar[2],1);
			sprintf(cad1,"%d\n",num);
			write(dc,cad1,strlen(cad1));
			}else if(*cadena1 == '+'){
			sscanf(cadena1,"+<%d>",&desp1);
			semop(idSem,&restar[2],1);
			op1 = *((int*)(dir+desp));
			semop(idSem,&sumar[2],1);
			semop(idSem,&restar[2],1);
			op2 = *((int*)(dir+desp1));
			semop(idSem,&sumar[2],1);
			num = op1+op2;
			sprintf(cad1,"%d\n",num);
			write(dc,cad1,strlen(cad1));
			}
 
			}
		}
		close(dc);
		semop(idSem,&sumar[1],1);
		semop(idSem,&restar[0],1);
		exit(0);
	}else{

	close(dc);
	}
}
//close(ds);
return(EXIT_SUCCESS);
}
