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

int main(int argc, char **argv)
{

    int sockserv, sockcli, tam,idSem,idMem,desp1,desp2,op1,op2,resul;
    char *dir,*cad1,*cad2;
    struct sockaddr_in dir_servidor, dir_cliente;
    char buffer[TAM];
    struct sembuf suma[3] = {{0,1,0},{1,1,0},{2,1,0}},resta[3] = {{0,-1,0},{1,-1,0},{2,-1,0}};

    idSem = semget(CLAVE,3,0666);
    idMem = shmget(CLAVE, 1024,0666);
    dir = shmat(idMem,0,0);
    signal(SIGCHLD, atencion);

    //Declaracion de direcciones:
    dir_servidor.sin_family = AF_INET;
    dir_servidor.sin_port = htons(PORT);
    dir_servidor.sin_addr.s_addr = INADDR_ANY;
    bzero(&(dir_servidor.sin_zero),8);

    //Creacion del socket TCP:
    sockserv = socket(AF_INET, SOCK_STREAM, 0);

    //Asignacion de las direcciones
    bind(sockserv, (struct sockaddr *)&dir_servidor, sizeof(dir_servidor));
    //Escucha
    printf("Esperando...\n");
    listen(sockserv, SOMAXCONN);

    while (1)
    {
        tam = sizeof(dir_cliente);

        sockcli = accept(sockserv, (struct sockaddr *)&dir_cliente, &tam);
        semop(idSem,&suma[0],1);   
	 //Creacion del proceso que se encarga de atender la peticion:
            if (fork() == 0)
        {
           semop(idSem,&resta[1],1);
		 //HIJO
            close(sockserv);
            while (read(sockcli, buffer, sizeof(buffer)) > 0)
            {
                if (*buffer =='h')
                {
                    sprintf(buffer, "OK\n");
                    write(sockcli, buffer, strlen(buffer)+1);
                }else{
		cad1 = strtok(buffer,">");
		cad2 = strtok(NULL,"");
		sscanf(cad1,"<%d",&desp1);
		if (*cad2 == '='){
		sscanf(cad2,"=%d",&resul);
		semop(idSem,&resta[2],1);
		*((int*)(dir+desp1)) = resul;
		semop(idSem,&suma[2],1);
		sprintf(buffer, "\n");
		write(sockcli,buffer,strlen(buffer)+1);
		}else if(*cad2 == '?'){
		semop(idSem,&resta[2],1);
		resul = *((int*)(dir+desp1));
		semop(idSem,&suma[2],1);
		sprintf(buffer,"%d\n",resul);
		write(sockcli,buffer,strlen(buffer)+1);
		}else if(*cad2 =='+'){
		sscanf(cad2,"+<%d>",&desp2);
		semop(idSem,&resta[2],1);
		op1 = *((int*)(dir+desp1));
		semop(idSem,&suma[2],1);
		semop(idSem,&resta[2],1);
		op2 = *((int*)(dir+desp2));
		semop(idSem,&suma[2],1);
		resul = op1+op2;
		sprintf(buffer,"%d\n",resul);
                write(sockcli,buffer,strlen(buffer)+1);
		}
		}
            }
            close(sockcli);
	    semop(idSem,&suma[1],1);
	    semop(idSem,&resta[0],1);
            exit(0); //Cerramos el hijo.
        }
        else
        {
            //PADRE
            close(sockcli);
        }
    }
//close(sockserv);
return(EXIT_SUCCESS);
}

