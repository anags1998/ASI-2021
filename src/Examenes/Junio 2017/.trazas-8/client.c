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
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <fcntl.h>
#include <netdb.h>

#define FIFOW "/tmp/fifo_wr"
#define FIFOR "/tmp/fifo_rd"
#define CLAVE 0x45916038L

int cont,pidMonitor,pidPadre,pid1,pid2,pid3,fifow,fifor,contKA,tam,idMem,num,fin,ds,dc;
char cad[100],*dir;
struct sockaddr_in servidor, cliente;
struct hostent *h;

void cambiarFin(){
    fin=1;
}
void incrementarCont(){
    cont++;
}

void terminar(){
    kill(pid1,SIGKILL);
    kill(pid2,SIGKILL);
    kill(pid3,SIGKILL);
    exit(0);
}
int main(int argc, char** argv) {
    
    printf("Pulsa INTRO después de seleccionar la OPCIÓN 1 en el monitor");
    getchar();
    pidMonitor=atoi(argv[1]);
    pidPadre = getpid();
    idMem = shmget(CLAVE,1024,0666);
    dir=shmat(idMem,0,0);
    pid1=fork();
    switch(pid1){
        case -1: break;
        case 0: fifor = open(FIFOR,O_RDWR);
                fifow = open(FIFOW,O_RDWR);
                contKA = 0;
                sprintf(cad,"hello %d",getpid());
                write(fifow,cad,strlen(cad));
                fin=0;
                while(fin==0 && read(fifor,cad,100)>0){
                    if(*cad=='K'){
                        contKA++;
                        *((int*)(dir+10))=contKA;
                        sprintf(cad,"ka %d",contKA);
                        write(fifow,cad,strlen(cad));
                    }else if(*cad=='S'){
                        sscanf(cad,"SECRET %d",&num);
                        sprintf(cad,"key 2 %d",num);
                        write(fifow,cad,strlen(cad));
                    }else if(*cad=='Q'){
                        sprintf(cad,"quit");
                        write(fifow,cad,strlen(cad));
                        fin=1;
                    }
                }
                pause();
                break;
        default: pid2=fork();
                switch(pid2){
                    case -1: break;
                    case 0: h = gethostbyname("localhost");
                            servidor.sin_addr.s_addr = ((struct in_addr*)(h->h_addr))->s_addr;
                            servidor.sin_port = htons(8000 + pidPadre);
                            servidor.sin_family = AF_INET;
                            bzero(&(servidor.sin_zero),8);
                            cliente.sin_addr.s_addr = htonl(INADDR_ANY);
                            cliente.sin_port = htons(8000 + pidPadre);
                            cliente.sin_family = AF_INET;
                            bzero(&(cliente.sin_zero),8);
                            ds = socket(AF_INET,SOCK_STREAM,0);
                            connect(ds,(struct sockaddr*)&servidor,sizeof(servidor));
                            contKA = 0;
                            sprintf(cad,"hello %d",getpid());
                            write(ds,cad,strlen(cad));
                            fin=0;
                            while(fin==0 && read(ds,cad,100)>0){
                                if(*cad=='K'){
                                    contKA++;
                                    *((int*)(dir+10+sizeof(int)))=contKA;
                                    sprintf(cad,"ka %d",contKA);
                                    write(ds,cad,strlen(cad));
                                }else if(*cad=='S'){
                                    sscanf(cad,"SECRET %d",&num);
                                    sprintf(cad,"key 5 %d",num);
                                    write(ds,cad,strlen(cad));
                                }else if(*cad=='Q'){
                                    sprintf(cad,"quit");
                                    write(ds,cad,strlen(cad));
                                    fin=1;
                                }
                            }
                            close(ds);
                            pause();
                            break;
                    default: pid3=fork();
                            switch(pid3){
                                case -1: break;
                                case 0: h = gethostbyname("localhost");
                                        servidor.sin_addr.s_addr = ((struct in_addr*)(h->h_addr))->s_addr;
                                        servidor.sin_port = htons(8000 + pidPadre);
                                        servidor.sin_family = AF_INET;
                                        bzero(&(servidor.sin_zero),8);
                                        cliente.sin_addr.s_addr = htonl(INADDR_ANY);
                                        cliente.sin_port = htons(8000 + pidPadre);
                                        cliente.sin_family = AF_INET;
                                        bzero(&(cliente.sin_zero),8);
                                        ds = socket(AF_INET,SOCK_DGRAM,0);
                                        bind(ds,(struct sockaddr*)&cliente,sizeof(cliente));
                                        contKA = 0;
                                        sprintf(cad,"hello %d",getpid());
                                        sendto(ds,cad,strlen(cad),0,(struct sockaddr*)&servidor,sizeof(servidor));
                                        fin=0;
                                        tam = sizeof(servidor);
                                        while(fin==0 && recvfrom(ds,cad,100,0,(struct sockaddr*)&servidor,&tam)>0){
                                            if(*cad=='K'){
                                                contKA++;
                                                *((int*)(dir+10+2*sizeof(int)))=contKA;
                                                sprintf(cad,"ka %d",contKA);
                                                sendto(ds,cad,strlen(cad),0,(struct sockaddr*)&servidor,sizeof(servidor));
                                            }else if(*cad=='S'){
                                                sscanf(cad,"SECRET %d",&num);
                                                sprintf(cad,"key 8 %d",num);
                                                sendto(ds,cad,strlen(cad),0,(struct sockaddr*)&servidor,sizeof(servidor));
                                            }else if(*cad=='Q'){
                                                sprintf(cad,"quit");
                                                sendto(ds,cad,strlen(cad),0,(struct sockaddr*)&servidor,sizeof(servidor));
                                                fin=1;
                                            }
                                        }
                                        close(ds);
                                        pause();
                                         break;
                                default: kill(pidMonitor,SIGUSR1);
                                         signal(SIGQUIT,terminar);
                                         signal(SIGUSR1,cambiarFin);
                                         fin=0;
                                         while(fin==0){
                                             pause();
                                         }
                                         cont=0;
                                         fin=0;
                                         signal(SIGUSR2,incrementarCont);
                                         while(fin==0){
                                             pause();
                                         }
                                         kill(pidMonitor,SIGUSR2);
                                         *((int*)(dir+10+3*sizeof(int))) = cont;
                                         while(1){
                                             pause();
                                         }
                            }
                }
    }
   

    return (EXIT_SUCCESS);
}
