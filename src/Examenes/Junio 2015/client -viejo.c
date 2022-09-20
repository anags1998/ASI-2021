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

//Funciones varias-------------------------------------------------
void atencion()
{
    wait(NULL);
    return;
}

//FUNCION PRINCIPAL-------------------------------------------------

int main(int argc, char** argv) {
    
    struct sockaddr_in servidor, cliente;
    int tam, ds, dc, intentos, correcto, salir,idMem,idSem;
    char cad[100],usu[100],pass[100],*dir;
    struct sembuf sumar[3]={{0,1,0},{1,1,0},{2,1,0}}, restar[3]={{0,-1,0},{1,-1,0},{2,-1,0}};
    
    signal(SIGCHLD,atencion);
    
    idMem = shmget(CLAVE,1024,0666);
    dir=shmat(idMem,0,0);
    
    idSem = semget(CLAVE, 3 , 0666);
    
    servidor.sin_addr.s_addr = htonl(INADDR_ANY);
    servidor.sin_port = htons(3010);
    servidor.sin_family = AF_INET;
    bzero(&(servidor.sin_zero),8);

    ds = socket(AF_INET, SOCK_STREAM, 0);
    bind (ds, (struct sockaddr*)&servidor, sizeof(servidor));
    listen(ds,SOMAXCONN);
    
    while(1){
        tam = sizeof(cliente);
        dc = accept(ds,(struct sockaddr*)&cliente, &tam);
        semop(idSem,&restar[2],1);
        *((int*)dir) = *((int*)dir) + 1;
        semop(idSem,&sumar[2],1);
        if(fork()==0){
            close(ds);
            //atiende al cliente
            semop(idSem,&restar[2],1);
            *((int*)(dir+3*sizeof(int))) = *((int*)(dir+3*sizeof(int))) + 1;
            semop(idSem,&sumar[2],1);   
            
            intentos = 0;
            correcto = 0;
            do{
                sprintf(cad,"login:");
                write(dc,cad,strlen(cad));
                read(dc,usu,100);
                sprintf(cad,"passwd:");
                write(dc,cad,strlen(cad));
                read(dc,pass,100);
                intentos++;
                sprintf(usu,"%s", strtok(usu,"\n")); //sprintf añade \0 a la cadena
                sprintf(pass,"%s",strtok(pass,"\n"));
                if(strcmp(usu,"anonymous")==0 && strcmp(pass,"guest")==0){
                    correcto=1;
                    semop(idSem,&restar[2],1);
                    *((int*)(dir+sizeof(int))) = *((int*)(dir+sizeof(int))) + 1;
                    semop(idSem,&sumar[2],1);
                }else{
                    semop(idSem,&restar[2],1);
                    *((int*)(dir+2*sizeof(int))) = *((int*)(dir+2* sizeof(int))) + 1;
                    semop(idSem,&sumar[2],1);
                }
            }while(intentos<3 && correcto==0);
            
            if(correcto==1){
                
                salir = 0;

                sprintf(cad,"%s:$",usu);
                write(dc,cad,strlen(cad));
                semop(idSem,&sumar[0],1);
                semop(idSem,&restar[1],1);
                
                while(salir==0 && read(dc,cad,100)>0){
                    sprintf(cad,"%s",strtok(cad,"\n"));
                    if(strcmp(cad,"whoami")==0){
                        sprintf(cad,"%s\n",usu);
                        write(dc,cad,strlen(cad));
                        sprintf(cad,"%s:$",usu);
                        write(dc,cad,strlen(cad));
                    }else if(strcmp(cad,"pid")==0){
                        sprintf(cad,"%d\n",getpid());
                        write(dc,cad,strlen(cad));
                        sprintf(cad,"%s:$",usu);
                        write(dc,cad,strlen(cad));
                    }else if(strcmp(cad,"quit")==0){
                        salir = 1;
                    }else if(strcmp(cad,"bad command")==0){
                        sprintf(cad,"Error\n");
                        write(dc,cad,strlen(cad));
                        sprintf(cad,"%s:$",usu);
                        write(dc,cad,strlen(cad));
                    }else if(strcmp(cad,"ncnx")==0){
                        semop(idSem,&restar[2],1);
                        sprintf(cad,"%d\n", *((int*)dir));
                        semop(idSem,&sumar[2],1);
                        write(dc,cad,strlen(cad));
                        sprintf(cad,"%s:$",usu);
                        write(dc,cad,strlen(cad));
                    }else if(strcmp(cad,"nsess")==0){
                        semop(idSem,&restar[2],1);
                        sprintf(cad,"%d\n", *((int*)(dir+sizeof(int))));
                        semop(idSem,&sumar[2],1);
                        write(dc,cad,strlen(cad));
                        sprintf(cad,"%s:$",usu);
                        write(dc,cad,strlen(cad));
                    }else if(strcmp(cad,"nok")==0){
                        semop(idSem,&restar[2],1);
                        sprintf(cad,"%d\n", *((int*)(dir+2*sizeof(int))));
                        semop(idSem,&sumar[2],1);
                        write(dc,cad,strlen(cad));
                        sprintf(cad,"%s:$",usu);
                        write(dc,cad,strlen(cad));
                    }else if(strcmp(cad,"active session")==0){
                        semop(idSem,&restar[2],1);
                        sprintf(cad,"%d\n", *((int*)(dir+3*sizeof(int))));
                        semop(idSem,&sumar[2],1);
                        write(dc,cad,strlen(cad));
                        sprintf(cad,"%s:$",usu);
                        write(dc,cad,strlen(cad));
                    }
                }
                semop(idSem,&restar[0],1);
                semop(idSem,&sumar[1],1);
            }
            semop(idSem,&restar[2],1);
            *((int*)(dir+3*sizeof(int))) = *((int*)(dir+3*sizeof(int))) - 1;
            semop(idSem,&sumar[2],1);
            close(dc);
            
            exit(0);
        }else
            close(dc);
    }
    return (EXIT_SUCCESS);
}












