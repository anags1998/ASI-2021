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
}MENSAJE;

int pidM,pidH1,pidH2,idMem,idSem,idCola,num, desp,desp1,oper1,oper2,oper3;
char *dir;
struct sembuf restar={0, -1, 0}, sumar = {0, 1, 0};

void ejercicio1(){
MENSAJE m;

m.canal = 1L;
sprintf(m.cad, "H<%d>",getpid());
msgsnd(idCola,&m,sizeof(m)-sizeof(long),0);
}

void ejercicio2(){
MENSAJE m,m1;
msgrcv(idCola,&m,sizeof(m)-sizeof(long),getpid(),0);
if(*m.cad == 'E'){
num = *((int*)(m.cad+1));
m1.canal = 1L;
sprintf(m1.cad, "e<%d><%d>",getpid(),num);
msgsnd(idCola,&m1,sizeof(m1)-sizeof(long),0);

}else if( *m.cad =='R'){
desp = *((int*)(m.cad+1));
num = *((int*)(dir+desp));
m1.canal =  1L;
*((long*)(dir+desp))= getpid();
sprintf(m1.cad, "r<%d><%d>",getpid(),num);
msgsnd(idCola,&m1,sizeof(m1)-sizeof(long),0);

}else if( *m.cad =='+'){
desp = *((int*)(m.cad+1));
desp1 = *((int*)(m.cad+1+sizeof(int)));
oper1 = *((int*)(dir+desp));
oper2 = *((int*)(dir+desp1));
oper3 = oper1+oper2;
m1.canal = 1L;
sprintf(m1.cad, "+<%d><%d>",getpid(),oper3);
msgsnd(idCola,&m1,sizeof(m1)-sizeof(long),0);

}else if( *m.cad =='*'){
desp = *((int*)(m.cad+1));
desp1 = *((int*)(m.cad+1+sizeof(int)));
semop(idSem, &restar,1);
oper1 = *((int*)(dir+desp));
oper2 = *((int*)(dir+desp1));
oper3 = oper1* oper2;
semop(idSem, &sumar,1);
m1.canal= 1L;
sprintf(m1.cad, "*<%d><%d>",getpid(),oper3);
msgsnd(idCola,&m1,sizeof(m1)-sizeof(long),0);
}
}

int main(int argc, char** argv){
pidM = atoi(argv[1]);
idCola = msgget(CLAVE, IPC_CREAT | 0666);
sleep(1);
kill(pidM,SIGUSR1);
sleep(1);
signal(SIGUSR1,ejercicio1);
idMem = shmget(CLAVE,1024, IPC_CREAT | 0666);
dir = shmat(idMem,0,0);
idSem = semget(CLAVE , 1, IPC_CREAT | 0666);
semctl(idSem,0,SETVAL,0);

pidH1 = fork();

switch(pidH1){
case -1:break;
case 0:
      while(1){
ejercicio2();
	}
       break;
default:pidH2 = fork();
	switch (pidH2){
	case -1:break;
	case 0:
		while(1){	
       ejercicio2();	
		}
	       break;
	default:
		while(1){
	ejercicio2();
		}
		getchar();
		break;
}
}return (EXIT_SUCCESS);
}
