/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: virginia
 *
 * Created on 22 de junio de 2021, 14:50
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/shm.h>
#include <sys/signal.h>

#define CLAVE 0x45916038L
#define PORT 3010

struct shm_dev_reg{ //Viene en la plantilla el formato
    int estado;
    int num_dev;
    char descr[16];
    int contador[4];
};

void rutinaAtencion(){
    wait(NULL);
}
/*
 * 
 */
int main(int argc, char** argv) {
    struct sockaddr_in servidor, cliente;
    int sockserv, idMemoria;
    int tam,n, num_dev;
    char cad[100],descr[100], resp[100], *dir;
    struct shm_dev_reg d; //Dispositivo
    
    
    idMemoria=shmget(CLAVE,4096,IPC_CREAT|0666);
    dir=shmat(idMemoria,0,0);
    signal(SIGCHLD,rutinaAtencion);
    //parametros del servidor
    servidor.sin_addr.s_addr=htonl(INADDR_ANY);
    servidor.sin_family=AF_INET;
    servidor.sin_port=htons(PORT);
    bzero(&(servidor.sin_zero),8);
    //creación socket
    sockserv=socket(AF_INET,SOCK_DGRAM,0);
    bind(sockserv,(struct sockaddr*)&servidor,sizeof(servidor));
    while(1){//mientras no se quiera salir
        tam=sizeof(cliente);
        recvfrom(sockserv,cad,100,0,(struct sockaddr*)&cliente,&tam);
        if(*cad=='H'){ //mensaje binario
           num_dev= *((int*)(cad+1));
           sprintf(descr,"%s",cad+1+sizeof(int));
           //la respuesta tb es en binario por tanto no puedo construirla con sprintf
           resp[0]='O';
           *((int*)(resp+1))=num_dev; //ya tengo construida la respuesta        
           sendto(sockserv,resp,5,0,(struct sockaddr*)&cliente,sizeof(cliente));
           d=*((struct shm_dev_reg*) (dir+num_dev*sizeof(struct shm_dev_reg)));//obtengo la estructura del dispositivo desde la memoria
           d.num_dev=num_dev;
           d.estado=1;
           sprintf(d.descr,"%s",descr);
           *((struct shm_dev_reg*) (dir+num_dev*sizeof(struct shm_dev_reg)))=d;//la copio de nuevo con los valores cambiados
           
        }
    }
    return (EXIT_SUCCESS);
}







