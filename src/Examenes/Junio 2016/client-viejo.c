#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/signal.h>
#include <unistd.h>


#define CLAVE 0x45916038L
#define TIME 4			// Temporizador de retransmisión
#define BUFLEN 256	// Tamaño de bufferes genérico

#define SIZE_SHM 1024	//Tamaño del segmento de memoria compartida

#define MAX_DEV	4	//Maximo # dispositivos en la tabla de sesiones


/* El array de semáforos a crear tiene tamaño 4
	El semáforo 0 servirá para controlar el acceso a lecturas de valores de
		los semáforos 1 y 2
	El semáforo 1 para poder modificar su valor en comando SEM
	El semáforo 2 para poder modificar su valor en comando SEM
	el semáforo 3 para controlar los accesos a la tabla de sesiones en 
		fase de registro.
*/
#define MAX_SEM 4	// Número de semáforos en el array

/* Lista de tipos de mensaje recibidos en la cola de mensajes. */

#define COMM_SPEED '2'
#define COMM_RPM '3'
#define COMM_SEM '4'
#define COMM_BYE '5'

#define MAX_COMMAND 6


#define OFF_REG_TBL 0	// Desplazamiento de la tabla de sesiones en SHM

#define ST_FREE 0
#define ST_PID 1
#define ST_DATA 2
#define LEN_NAME 16	//Usar como maximo nombres de 8

#define LEN_FIFO_NAME 32
#define ROOT_NAME_FIFO "/tmp/fifo-"

typedef struct mensaje1{
    long canal;
    int pid;
    char cad[100];
}MENSAJE1;

typedef struct mensaje2{
    long canal;
    char cad[100];
}MENSAJE2;

struct st_reg {
	int state;	// State of register
	char name[LEN_NAME];	// Name of device
	pid_t pid;	// Process identifier
};

#define OFF_DATA_TBL 300	// Desplazamiento de la tabla de datos en SHM

struct st_data {
	float speed;	// Speed parameter as float value 
	int rpm;	// Speed parameter as float value 
	int sem;	// Sem number 
	int semval;	// Sem value 
	
};

int idCola,fifo,salir,idSem,pos,enc,idMem;
MENSAJE1 m1;
MENSAJE2 m2;
char cad[100],*dir;
struct st_data d;
struct st_reg *pReg;
struct st_data *pData;
struct sembuf restar ={0,-1,0}, sumar={0,1,0};

/*************************************************************************/
/* Función a utilizar para sustituir a signal() de la libreria.
Esta función permite programar la recepción de la señál de temporización de
alarm() para que pueda interrumpir una funcion bloqueante.
El alumno debe saber como utilizarla.
*/
int signal_EINTR(int sig,void(*handler)())
{
struct sigaction sa;
	sa.sa_handler = handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	return(sigaction(sig,&sa,NULL));
}
/*************************************************************************/

void enviarComando(){
     msgsnd(idCola,&m1,sizeof(m1)-sizeof(long),0);
}
void retransmitir(){
    int fin=0;
    do{
        signal_EINTR(SIGALRM, enviarComando);
        alarm(4);
        m2.cad[0]='\0';
        msgrcv(idCola,&m2,sizeof(m2)-sizeof(long),getpid(),0);
        printf("Recibido: %s", m2.cad);
        fflush(stdout);
        if(strncmp(m2.cad,"RTX",3)==0){
            signal_EINTR(SIGALRM,SIG_IGN);
            enviarComando();
        }else if(strncmp(m2.cad,"OK",2)==0){
            signal_EINTR(SIGALRM,SIG_IGN);
            fin=1;
        }
    }while(fin==0);
}


int main(int argc, char** argv) {

    char nombreFifo[100];
    sprintf(nombreFifo,"/tmp/fifo-%d",getpid());
    mkfifo(nombreFifo,0666);
    fifo = open(nombreFifo,O_RDWR);
    
    idSem = semget(CLAVE,4,0666);
    idCola = msgget(CLAVE,0666);
    idMem = shmget(CLAVE,1024,0666);
    dir = shmat(idMem,0,0);
    
    m1.canal=1L;
    m1.pid = getpid();
    sprintf(m1.cad,"HELLO %d %s", getpid(),argv[1]);
    msgsnd(idCola,&m1,sizeof(m1)-sizeof(long),0);
    /*msgrcv(idCola,&m2,sizeof(m2)-sizeof(long),getpid(),0);
    printf("Recibido: %s", m2.cad);
    fflush(stdout);
    */
    retransmitir();
    
    pos = 0;
    enc = 0;
    while(enc==0 && pos<MAX_DEV){
        pReg = (struct st_reg*)(dir+pos*sizeof(struct st_reg));
        if(strcmp(pReg->name,argv[1])==0)
            enc=1;
        else
            pos++;
    }
    
    pReg->pid = getpid();
    pData = (struct st_data *)(dir+300+pos*sizeof(struct st_data));
    
    salir=0;
    while(salir==0 && read(fifo,cad,100)>0){
        if(*cad=='2'){
            //d.speed = *((float*)(cad+1));
            pData->speed = *((float*)(cad+1));
            m1.canal = 1L;
            m1.pid = getpid();
            //sprintf(m1.cad,"SPEED %f",d.speed);
            sprintf(m1.cad,"SPEED %f",pData->speed);
            msgsnd(idCola,&m1,sizeof(m1)-sizeof(long),0);
            /*msgrcv(idCola,&m2,sizeof(m2)-sizeof(long),getpid(),0);
            printf("Recibido: %s", m2.cad);
            fflush(stdout);
            */
            retransmitir();
    
        }else if(*cad=='3'){
            //sscanf(cad+1,"%d",&d.rpm);
            sscanf(cad+1,"<%d>",&pData->rpm);
            m1.canal = 1L;
            m1.pid = getpid();
            //sprintf(m1.cad,"RPM %d",d.rpm);
            sprintf(m1.cad,"RPM %d",pData->rpm);
            msgsnd(idCola,&m1,sizeof(m1)-sizeof(long),0);
            /*msgrcv(idCola,&m2,sizeof(m2)-sizeof(long),getpid(),0);
            printf("3-Recibido: %s", m2.cad);
            fflush(stdout);*/
            retransmitir();
        }else if(*cad=='4'){
            /*d.sem = *((int*)(cad+1));
            d.semval = semctl(idSem,d.sem,GETVAL,NULL);
            */
            pData->sem = *((int*)(cad+1));
            semop(idSem,&restar,1);
            pData->semval = semctl(idSem,pData->sem,GETVAL,NULL);
            semop(idSem,&sumar,1);
            m1.canal = 1L;
            m1.pid = getpid();
            //sprintf(m1.cad,"SEM %d %d",d.sem,d.semval);
            sprintf(m1.cad,"SEM %d %d",pData->sem,pData->semval);
            msgsnd(idCola,&m1,sizeof(m1)-sizeof(long),0);
            /*msgrcv(idCola,&m2,sizeof(m2)-sizeof(long),getpid(),0);
            printf("4-Recibido: %s", m2.cad);
            fflush(stdout);*/
            retransmitir();
        }else if(*cad=='5'){
            salir=1;
        }
    }
    
    
    m1.canal = 1L;
    m1.pid = getpid();
    sprintf(m1.cad,"BYE");
    msgsnd(idCola,&m1,sizeof(m1)-sizeof(long),0);
    return (EXIT_SUCCESS);
}


