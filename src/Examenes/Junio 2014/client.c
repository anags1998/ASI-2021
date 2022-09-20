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

int pidM,pidH1,pidH2,pidP,f1,f2,num,num1,num2,idMem,idSem,desp,desp1;
char cad1[100],cad2[100],cad3[100],*dir;
struct sembuf sumar[4]={{0,1,0},{1,1,0},{2,1,0},{3,1,0}},restar[4]={{0,-1,0},{1,-1,0},{2,-1,0},{3,-1,0}};

void ejer1(){
sprintf(cad1,"H<%d>",getpid());//se hace con sprintf porque es ASCII
write(f1,cad1,strlen(cad1));
}

void ejer2(){
read(f2,cad2,100);
if(*cad2 == 'E'){
num = *((int*)(cad2+1));//se hace asi porque es binario
sprintf(cad3,"e<%d><%d>",getpid(),num);//pide formato ASCII
write(f1,cad3,strlen(cad3));
}else if( *cad2 == 'R'){
desp = *((int*)(cad2+1));//por ser binario
num =  *((int*)(dir+desp));//nos situamos en el sitio del dato a leer
*((int*)(dir+num)) = getpid();//num es la posicion de la clave,pide que sea en formato binario
sprintf(cad3,"r<%d><%d>",getpid(),num);//pide formato ASCII
write(f1,cad3,strlen(cad3));
}else if(*cad2 == '+'){
desp = *((int*)(cad2+1));//por ser binario
desp1 = *((int*)(cad2 + 1 + sizeof(int)));
num1 = *((int*)(dir+desp));//el primer numero en la direccion mas un desplazamiento
num2  = *((int*)(dir+desp1));
num = num1+num2;
sprintf(cad3,"+<%d><%d>",getpid(),num);//pide formato ASCII
write(f1,cad3,strlen(cad3));
}else if(*cad2 == '*'){
desp = *((int*)(cad2+1));//por ser binario
desp1 = *((int*)(cad2 + 1 + sizeof(int)));
semop(idSem,&restar[3],1);//en este caso nos dice que hay que despues habilitar el paso a la memoria compartida, por eso luego se hace suma
num1 = *((int*)(dir+desp));
num2  = *((int*)(dir+desp1));
semop(idSem,&sumar[3],1);
num = num1*num2;
sprintf(cad3,"*<%d><%d>",getpid(),num);//pide formato ASCII
write(f1,cad3,strlen(cad3));

}
}

int main(int argc, char** argv){
	
	pidM = atoi(argv[1]);
	kill(pidM, SIGUSR1);//mando señal a monitor
	sleep(2);//para que le de tiempo a comprobar
	f1 = open(F1, O_RDWR);//abro fifo escritura
	signal(SIGUSR1,ejer1);//recibimos señal y ejecutamos ejercicio 1
	mkfifo(F2, 0666);//creamos fifo lectura
	f2 = open(F2,O_RDWR);//abrimos fifo lectura
	idMem = shmget(CLAVE,1024, IPC_CREAT | 0666);//creamos memoria
	dir = shmat(idMem,0,0);//cogemos direccion, cuidado no se puede machacar
	idSem =semget(CLAVE,4,IPC_CREAT | 0666);//creamos semaforos
	semctl(idSem,0,SETVAL,0);//inicializamos semaforos, se podria con un bucle for
	semctl(idSem,1,SETVAL,0); 
	semctl(idSem,2,SETVAL,0); 
	semctl(idSem,3,SETVAL,0); 
 
	pidH1 = fork();
	switch(pidH1){
	case -1: break;
	case 0:while(1){
	       semop(idSem,&restar[1],1);//solo restamos porque el enunciado lo dice asi
	       ejer2();//hay que comentar la siguiente linea porque ya no se usan señales ej4
	       //signal(SIGUSR2,ejer2);//recibimos señal y hacemos ejercicio 2
	       }break;
	       
	default: pidH2 = fork();
		 switch (pidH2){
		 case -1: break;
		 case 0:while(1){
			semop(idSem,&restar[2],1);
			ejer2();
			//signal(SIGUSR2,ejer2);//recibimos señal y hacemos ejercicio 2
			}
			break;
		 default:while(1){
			 semop(idSem,&restar[0],1);
		         ejer2();
			 //signal(SIGUSR2,ejer2);//recibimos señal y hacemos ejercicio 2
			 }
     			 wait(NULL);
			 wait(NULL);
			 getchar();
			 break;
		 }	
	}


return (EXIT_SUCCESS);
}
