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
void espera(){
	wait(NULL);	
	return;
}

//FUNCION PRINCIPAL-------------------------------------------------

int main(int argc, char** argv) {

	struct sockaddr_in servidor,cliente;
	int dc,ds,tam,intentos,bien,out,num,idMem,idSem;
	char cad[100],cad1[100],*dir,usuario[100],pass[100];
	struct  sembuf sumar[3] = {{0,1,0},{1,1,0},{2,1,0}};
	struct  sembuf restar[3] = {{0,-1,0},{1,-1,0},{2,-1,0}};

	signal(SIGCHLD,espera);
	idMem = shmget(CLAVE,1024,0666);
	dir = shmat(idMem,0,0);
	idSem = semget(CLAVE,3,0666);

	servidor.sin_addr.s_addr = htonl(INADDR_ANY);
        servidor.sin_port = htons(3010);
	servidor.sin_family = AF_INET;
    	bzero(&(servidor.sin_zero),8);

    	ds = socket(AF_INET, SOCK_STREAM, 0);
    	bind (ds, (struct sockaddr*)&servidor, sizeof(servidor));
    	listen(ds,SOMAXCONN);
    
	while(1){
	tam = sizeof(cliente);
	dc = accept(ds,(struct sockaddr *)&cliente, &tam);
	semop(idSem,&restar[2],1);
	*((int*)(dir)) = *((int*)(dir))+1;
	semop(idSem,&sumar[2],1);
	if(fork() == 0){
		close(ds);
		semop(idSem,&restar[2],1);
		*((int*)(dir+3*sizeof(int))) = *((int*)(dir+3*sizeof(int)))+1;
		semop(idSem,&sumar[2],1);
		intentos = 0;
		bien = 0;
		do{
			sprintf(cad,"login:");
			write(dc,cad,strlen(cad));
			read(dc,usuario,100);
			sprintf(cad,"passwd:");
			write(dc,cad,strlen(cad));
			read(dc,pass,100);
			intentos ++;
			sprintf(usuario,"%s",strtok(usuario,"\n"));
			sprintf(pass,"%s",strtok(pass,"\n"));
			if(strcmp(usuario,"anonymous") == 0 && strcmp(pass,"guest")==0){
				bien = 1;
				semop(idSem,&restar[2],1);
				*((int*)(dir+sizeof(int))) = *((int*)(dir+sizeof(int)))+1;
				semop(idSem,&sumar[2],1);
			}else{
			semop(idSem,&restar[2],1);
			*((int*)(dir+2*sizeof(int))) = *((int*)(dir+2*sizeof(int)))+1;
			semop(idSem,&sumar[2],1);
			}

		}while(intentos < 3 && bien == 0);
	if(bien == 1){
		out = 0;
		sprintf(cad,"%s:$",usuario);
		write(dc,cad,strlen(cad));
		semop(idSem,&sumar[0],1);
		semop(idSem,&restar[1],1);
		while(out == 0 && read(dc,cad,100)>0){
		if(*cad == 'w'){
			sprintf(cad,"%s\n",usuario);
			write(dc,cad,strlen(cad));
			sprintf(cad,"%s:$",usuario);
                	write(dc,cad,strlen(cad));
		}else if(*cad == 'p'){
			sprintf(cad,"%d\n",getpid());
                        write(dc,cad,strlen(cad));
                        sprintf(cad,"%s:$",usuario);
                        write(dc,cad,strlen(cad));
		}else if(*cad == 'q'){
			out = 1;
		}else if(*cad == 'b'){
			sprintf(cad,"Error\n");
                        write(dc,cad,strlen(cad));
                        sprintf(cad,"%s:$",usuario);
                        write(dc,cad,strlen(cad));
		}else if(*cad == 'a'){
			semop(idSem,&restar[2],1);
			num = *((int*)(dir+3*sizeof(int)));
			semop(idSem,&sumar[2],1);
			sprintf(cad,"%d\n",num);
                        write(dc,cad,strlen(cad));
                        sprintf(cad,"%s:$",usuario);
                        write(dc,cad,strlen(cad));
		}else if(*cad == 'n'){
			sprintf(cad,"%s",strtok(cad,"\n"));
			if(strcmp(cad,"ncnx")== 0){
				semop(idSem,&restar[2],1);
				num = *((int*)(dir));
				semop(idSem,&sumar[2],1);
	                        sprintf(cad,"%d\n",num);
        	                write(dc,cad,strlen(cad));
                	        sprintf(cad,"%s:$",usuario);
                       		write(dc,cad,strlen(cad));

			}else if(strcmp(cad,"nsess")== 0){
				semop(idSem,&restar[2],1);
				num = *((int*)(dir+sizeof(int)));
                        	semop(idSem,&sumar[2],1);
				sprintf(cad,"%d\n",num);
                        	write(dc,cad,strlen(cad));
                	        sprintf(cad,"%s:$",usuario);
        	                write(dc,cad,strlen(cad));
	
                        }else if(strcmp(cad,"nok")== 0){
				semop(idSem,&restar[2],1);
				num = *((int*)(dir+2*sizeof(int)));
				semop(idSem,&sumar[2],1);
	                        sprintf(cad,"%d\n",num);
        	                write(dc,cad,strlen(cad));
                	        sprintf(cad,"%s:$",usuario);
                        	write(dc,cad,strlen(cad));

                 }       }
		}
		semop(idSem,&restar[0],1);
		semop(idSem,&sumar[1],1);

	}
		semop(idSem,&restar[2],1);
		*((int*)(dir+3*sizeof(int))) = *((int*)(dir+3*sizeof(int)))-1;
		semop(idSem,&sumar[2],1);
		close(dc);

		exit(0);
	

	}else{
		close(dc);
	}

	}
	
 
	return (EXIT_SUCCESS);
	}
