#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define CLAVE 0x45916038L

typedef struct mensaje{
long canal;
char cad[100];
}MEN;


int pidM,pidH1,pidH2,idCola,idMem,idSem,num,desp,desp1,op1,op2;
char cad1[100],cad2[100],*dir;
struct sembuf sumar[1] = {0,1,0},restar[1]={0,-1,0};

void ejer1(){
MEN m1;
m1.canal = 1L;
sprintf(m1.cad,"H<%d>",getpid());
msgsnd(idCola,&m1,sizeof(m1)-sizeof(long),0);
}

void ejer2(){
MEN m1,m2;
msgrcv(idCola,&m1,sizeof(m1)-sizeof(long),getpid(),0);
if(* m1.cad =='E'){
num = *((int*)(m1.cad + 1));
m2.canal = 1L;
sprintf(m2.cad,"e<%d><%d>",getpid(),num);
msgsnd(idCola,&m2,sizeof(m2)-sizeof(long),0);
}if(*m1.cad == 'R'){
desp = *((int*)(m1.cad + 1));
num = *((int*)(dir +desp));;
*(long*)(dir+desp) = getpid();
m2.canal = 1L;
sprintf(m2.cad,"r<%d><%d>",getpid(),num);
msgsnd(idCola,&m2,sizeof(m2)-sizeof(long),0);
}if(*m1.cad == '+'){
desp = *((int*)(m1.cad + 1));
desp1 = *((int*)(m1.cad + 1 +sizeof(int)));
op1 = *((int*)(dir +desp));
op2 = *((int*)(dir +desp1));
num = op1+op2;
m2.canal = 1L;
sprintf(m2.cad,"+<%d><%d>",getpid(),num);
msgsnd(idCola,&m2,sizeof(m2)-sizeof(long),0);
}if(*m1.cad == '*'){
desp = *((int*)(m1.cad + 1));
desp1 = *((int*)(m1.cad + 1 +sizeof(int)));
semop(idSem,&restar[0],1);
op1 = *((int*)(dir +desp));
op2 = *((int*)(dir +desp1));
semop(idSem,&sumar[0],1);
num = op1*op2;
m2.canal = 1L;
sprintf(m2.cad,"*<%d><%d>",getpid(),num);
msgsnd(idCola,&m2,sizeof(m2)-sizeof(long),0);
}
}

int main(int argc, char** argv){
	pidM = atoi(argv[1]);
	idCola = msgget(CLAVE,IPC_CREAT | 0666);
	sleep(1);
	kill(pidM, SIGUSR1);
	sleep(1);
	signal(SIGUSR1,ejer1);
	idMem = shmget(CLAVE,1024,IPC_CREAT | 0666);
	dir = shmat(idMem,0,0);
	idSem = semget(CLAVE,1,IPC_CREAT | 0666);
	semctl(idSem,0,SETVAL,0);

	pidH1 = fork();
	switch(pidH1){
	case -1: break;
	case 0: //pause();
		while(1){
		ejer2();
		}
		break;
	default:pidH2 = fork();
		switch(pidH2){
		case -1: break;
		case 0:// pause();
			while(1){
                	ejer2();
                	}
			break;
		default://pause();
			while(1){
                	ejer2();
                	}
			getchar();
			break;
		}
	}
return (EXIT_SUCCESS);
}
