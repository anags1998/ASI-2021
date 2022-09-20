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

    int sockserv, sockcli, tam;
    struct sockaddr_in dir_servidor, dir_cliente;
    char buffer[TAM];

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
            //Creacion del proceso que se encarga de atender la peticion:
            if (fork() == 0)
        {
            //HIJO
            close(sockserv);
            while (read(sockcli, buffer, sizeof(buffer)) > 0)
            {
                if (*buffer =='h')
                {
                    sprintf(buffer, "OK\n");
                    write(sockcli, buffer, strlen(buffer)+1);
                }
            }
            close(sockcli);
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
