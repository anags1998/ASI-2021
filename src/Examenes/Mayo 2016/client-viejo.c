#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>

#define CLAVE 0x45916038L
typedef struct mensaje{
    long canal;
    char cad[100];
}MENSAJE;

typedef struct st_data{
    int state;
    char name[16];
    int speed;
    int rpm;
    int port;
    int sem;
    int semval;
    pid_t pid;
}DATA;

struct sockaddr_in servidor, cliente;
    int ds,tam,idCola,idMem,pos,enc,idSem,salir;
    struct hostent *h;
    char cad[100],*dir;
    MENSAJE m;
    DATA d;
    struct sembuf sumar={0,1,0},restar={0,-1,0};
    
    
/*
 * Al ejecutar este programa pondremo
 * 
 *          ./client nombre
 */

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

void funcion(){
    sendto(ds,cad,strlen(cad),0,(struct sockaddr*)&servidor,sizeof(servidor));
}
void retransmitirMensaje(){
    char cadRecibida[100];
    int fin;
    do{
        signal_EINTR(SIGALRM,funcion);
        cadRecibida[0]='\0';
        alarm(4);
        fin = 0;
        tam = sizeof(servidor);
        recvfrom(ds,cadRecibida,100,0,(struct sockaddr*)&servidor,&tam);
        if(strncmp(cadRecibida,"RTX",3)==0){
            signal_EINTR(SIGALRM,SIG_IGN); 
            funcion();
        }else if(strncmp(cadRecibida,"OK",2)==0){
            signal_EINTR(SIGALRM,SIG_IGN);
            fin=1;
        }
    }while(fin==0);
}

int main(int argc, char** argv) {
    
    
    idCola = msgget(CLAVE,0666);
    idMem = shmget(CLAVE,1024,0666);
    dir = shmat(idMem,0,0);
    idSem = semget(CLAVE,4,0666);
    
    h=gethostbyname("localhost");
    servidor.sin_addr.s_addr = ((struct in_addr*)(h->h_addr))->s_addr;
    servidor.sin_family = AF_INET;
    servidor.sin_port = htons(3001);
    bzero(&(servidor.sin_zero),8);
    
    cliente.sin_addr.s_addr = htonl(INADDR_ANY);
    cliente.sin_family = AF_INET;
    bzero(&(cliente.sin_zero),8);
    cliente.sin_port = 0; //Asignamos el puerto de forma automática
    
    ds = socket(AF_INET,SOCK_DGRAM,0);
    tam = sizeof(cliente);
    bind(ds,(struct sockaddr*)&cliente,sizeof(cliente)); //Publicamos los datos

    getsockname(ds,(struct sockaddr*)&cliente,&tam); //Nos devuelve en cliente, los datos publicados
    
    sprintf(cad,"HELLO %d %s", getpid(),argv[1]);
    sendto(ds,cad,strlen(cad),0,(struct sockaddr*)&servidor,sizeof(servidor));
    /*tam = sizeof(servidor);
    recvfrom(ds,cad,100,0,(struct sockaddr*)&servidor,&tam);
    */
    retransmitirMensaje();
    
    sprintf(cad,"PORT %d", (int)ntohs(cliente.sin_port));
    printf("%d",(int)ntohs(cliente.sin_port));
    sendto(ds,cad,strlen(cad),0,(struct sockaddr*)&servidor,sizeof(servidor));
    /*tam = sizeof(servidor);
    recvfrom(ds,cad,100,0,(struct sockaddr*)&servidor,&tam);
    */
    retransmitirMensaje();
    /*Buscamos en qué posición del array de estructuras de la memoria
     compartida está este cliente*/
    pos=0;
    enc=0;
    while(enc==0 && pos<4){
        d = *((DATA*)(dir+(pos*sizeof(DATA))));
        if(strcmp(d.name,argv[1])==0)
            enc=1;
        else
            pos++;
    }
    salir = 0;
    while(salir==0 && msgrcv(idCola,&m,sizeof(MENSAJE)-sizeof(long),getpid(),0)){
        if(*m.cad=='2'){
            sscanf(m.cad,"2<%d>",&d.speed);
            sprintf(cad,"SPEED %d",d.speed);
            sendto(ds,cad,strlen(cad),0,(struct sockaddr*)&servidor,sizeof(servidor));
          //  retransmitirMensaje();
        }else if(*m.cad=='3'){
            d.rpm = *((int*)(m.cad+1));
            sprintf(cad,"RPM %d", d.rpm);
            sendto(ds,cad,strlen(cad),0,(struct sockaddr*)&servidor,sizeof(servidor));
           // retransmitirMensaje();
        }else if(*m.cad=='4'){
            d.sem = *((int*)(m.cad+1));
            semop(idSem,&restar,1);
            d.semval = semctl(idSem,d.sem,GETVAL,NULL);
            semop(idSem,&sumar,1);
            sprintf(cad,"SEM %d %d",d.sem,d.semval);
            sendto(ds,cad,strlen(cad),0,(struct sockaddr*)&servidor,sizeof(servidor));
            //retransmitirMensaje();
        }else if(*m.cad=='5'){
            salir=1;
      
        }
        *((DATA*)(dir+(pos*sizeof(DATA)))) = d;
    }
    
    sprintf(cad,"BYE");
    sendto(ds,cad,strlen(cad),0,(struct sockaddr*)&servidor,sizeof(servidor));
    
    
    
    close(ds);
    return (EXIT_SUCCESS);
}



