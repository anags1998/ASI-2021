#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define F1 "/tmp/45916038B-W"
#define F2 "/tmp/45916038B-R"
#define CLAVE 0x45916038L

int pidM,pidH1,pidH2,f1,f2,num,desp,desp1,oper1,oper2,oper3,idMem,idSem,i;
char *dir;
struct sembuf arraySumar[4] = {
        {0, 1, 0},
        {1, 1, 0},
	{2, 1, 0},
	{3, 1, 0}
    }, arrayRestar[4] = {
        {0, -1, 0},
        {1, -1, 0},
	{2, -1, 0},
        {3, -1, 0}

    };

void ejercicio1(){
char cadena[50];
sprintf(cadena, "H<%d%>",getpid());
write(f1,cadena, strlen(cadena));
}

void ejercicio2(){
char cad[100],cad2[100];
read(f2,cad,100);
if( *cad =='E'){
num = *((int*)(cad+1));
sprintf(cad2,"e<%d><%d>",getpid(),num);
write(f1,cad2,strlen(cad2));

}else if( *cad =='R'){
desp = *((int*)(cad+1));
num = *((int*)(dir+desp));
*(int*)(dir+num)= getpid();
sprintf(cad2, "r<%d><%d>",getpid(),num);
write(f1,cad2,strlen(cad2));
}else if( *cad =='+'){
desp = *((int*)(cad+1));
desp1 = *((int*)(cad+1+sizeof(int)));
oper1 = *((int*)(dir+desp));
oper2 = *((int*)(dir+desp1));
oper3 = oper1+oper2;
sprintf(cad2, "+<%d><%d>",getpid(),oper3);
write(f1,cad2,strlen(cad2));
}else if( *cad =='*'){
desp = *((int*)(cad+1));
desp1 = *((int*)(cad+1+sizeof(int)));
semop(idSem, &arrayRestar[3],1);
oper1 = *((int*)(dir+desp));
oper2 = *((int*)(dir+desp1));
oper3 = oper1* oper2;
semop(idSem, &arraySumar[3],1);
sprintf(cad2, "*<%d><%d>",getpid(),oper3);
write(f1,cad2,strlen(cad2));
}
}
int main(int argc, char** argv){

pidM = atoi(argv[1]);
kill(pidM, SIGUSR1);
sleep(1);
f1 = open(F1, O_RDWR);
mkfifo (F2,0666);
f2 = open(F2, O_RDWR);
signal(SIGUSR1, ejercicio1);
idMem = shmget(CLAVE,1024, IPC_CREAT | 0666);
signal(SIGUSR2,ejercicio2);
dir = shmat(idMem,0,0);

idSem = semget(CLAVE , 4, IPC_CREAT | 0666);
for( i= 0; i<4;i++){
semctl(idSem,i,SETVAL,0);
}
pidH1 = fork();
switch(pidH1){
case -1:break;
case 0://pause();
       while(1){
	//pause();   
	semop(idSem, &arrayRestar[1],1);	
	ejercicio2();
       }
       break;
default: pidH2 = fork();
	 switch(pidH2){
	 case -1:break;
	 case 0://pause();
		while(1){
		//pause();	
		semop(idSem, &arrayRestar[2],1); 
		ejercicio2();
		}
		break;
	 default://pause();
		 while(1){
		// pause();
		semop(idSem, &arrayRestar[0],1);  
		ejercicio2();
		 }
		 wait(NULL);
		 wait(NULL);
		getchar();
}

}
return (EXIT_SUCCESS);
}
