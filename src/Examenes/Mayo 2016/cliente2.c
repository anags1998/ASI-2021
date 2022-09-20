/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: virginia
 *
 * Created on 23 de junio de 2021, 8:50
 */

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
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/wait.h>

#define SERVER_PORT 3001
#define SERVERIP "127.0.0.1"

#define CLAVE 0x45916038L	// Baliabideetarako gakoa. Zure NAN jarri.


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

void rutinaAtencion(){
    wait(NULL);
}
    
int main(int argc, char** argv) {
    struct sockaddr_in servidor, cliente;
    int sockserv, idMemoria, tamcli,tamserv;
    char cad[100], *dir;
    int i, enc;
    DATA disp;
    
    idMemoria=shmget(CLAVE,1024,0666); //supongo que ya esta creada pq hay que extraer valores de ella desde el principio
    dir=shmat(idMemoria,0,0);
    
    signal(SIGCHLD,rutinaAtencion);
    
    servidor.sin_family=AF_INET;
    servidor.sin_addr.s_addr=htonl(INADDR_ANY);
    servidor.sin_port=htons(SERVER_PORT);
    bzero(&(servidor.sin_zero),8);
    
    cliente.sin_family=AF_INET;
    cliente.sin_addr.s_addr=htonl(INADDR_ANY);
    cliente.sin_port=0; //cualquier puerto
    bzero(&(cliente.sin_zero),8);
    
    sockserv=socket(AF_INET,SOCK_DGRAM,0);
    bind(sockserv,(struct sockaddr*)&cliente,sizeof(cliente));
    
    tamcli=sizeof(cliente);   
    getsockname(sockserv,(struct sockaddr*)&cliente,&tamcli);
    
    sprintf(cad,"HELLO %d %s",getpid(), argv[1]);
    sendto(sockserv,cad,strlen(cad),0,(struct sockaddr*)&servidor,sizeof(servidor));
    tamserv=sizeof(servidor);
    recvfrom(sockserv,cad,100,0,(struct sockaddr*)&servidor,&tamserv);//Recibo la respuesta del servidor  
    sprintf(cad,"PORT %d",(int)ntohs(cliente.sin_port)); //puerto origen de la comunicacion es puerto del cliente y lo tengo que convertir a dire de red y luego a entero
    
    sendto(sockserv,cad,strlen(cad),0,(struct sockaddr*)&servidor,sizeof(servidor));
    recvfrom(sockserv,cad,100,0,(struct sockaddr*)&servidor,&tamserv);//Recibo la respuesta del servidor
    //Voy a buscar si el dispositivo sigue activo
    //1ro 
    enc=0;
    i=0;
    while(enc==0 &&i<4){
        disp=*((DATA*)(dir+(i*sizeof(DATA))));
        if(strcmp(disp.name,argv[1])==0){//si coinciden
            enc=1;
        }else{
            i++;
        }
        
        
    }
    
    sprintf(cad,"BYE");
    sendto(sockserv,cad,strlen(cad),0,(struct sockaddr*)&servidor,sizeof(servidor));
    close(sockserv);
    return (EXIT_SUCCESS);
}








