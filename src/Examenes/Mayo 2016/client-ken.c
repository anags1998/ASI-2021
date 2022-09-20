#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <unistd.h>

#define SERVER_PORT 3001
#define DESTIP "127.0.0.1"

#define CLAVE 0x45916038L // Clave de los recursos. Sustituir por DNI.
#define TIME 4            // Temporizador de retransmisión
#define BUFLEN 256        // Tamaño de bufferes genérico

#define SIZE_SHM 1024 //Tamaño del segmento de memoria compartida

#define MAX_DEV 4 //Maximo # dispositivos en la tabla de sesiones

/* El array de semáforos a crear tiene tamaño 4
	El semáforo 0 servirá para controlar el acceso a lecturas de valores de
		los semáforos 1 y 2
	El semáforo 1 para poder modificar su valor en comando SEM
	El semáforo 2 para poder modificar su valor en comando SEM
	el semáforo 3 para controlar los accesos a la tabla de sesiones en 
		fase de registro.
*/
#define MAX_SEM 4 // Número de semáforos en el array

/* Lista de tipos de mensaje recibidos en la cola de mensajes. */

#define COMM_SPEED '2'
#define COMM_RPM '3'
#define COMM_SEM '4'
#define COMM_BYE '5'

#define MAX_COMMAND 6

char *udp_cmd[] = {
    "HELLO", "PORT", "SPEED", "RPM", "SEM", "BYE", ""};

#define OFF_DATA_TBL 0 // Desplazamiento de la tabla de sesiones en SHM

#define ST_FREE 0
#define ST_PID 1
#define ST_DATA 2
#define LEN_NAME 16 //Usar como maximo nombres de 8

struct st_data
{
    int state;           // State of register
    char name[LEN_NAME]; // Name of device
    int speed;           // velocidad
    int rpm;             // revoluciones por minuto
    int port;            // Original port
    int sem;             // Sem number
    int semval;          // Sem value
    pid_t pid;           // Process identifier
};

/*************************************************************************/
/* Función a utilizar para sustituir a signal() de la libreria.
Esta función permite programar la recepción de la señál de temporización de
alarm() para que pueda interrumpir una funcion bloqueante.
El alumno debe saber como utilizarla.
*/
int signal_EINTR(int sig, void (*handler)())
{
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    return (sigaction(sig, &sa, NULL));
}
/*********************************Funciones****************************************/
int crearSck()
{
    int idsk;

    if ((idsk = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("Problemas en la creacion del socket\n");
        exit(1);
    }
    printf("Socket creado con exito\n");

    return idsk;
}
char *getInput(char *msg)
{
    char static buff[BUFLEN];

    printf("%s", msg);
    fflush(stdout); //Limpiamos el buffer de entrada?? o salida
    fgets(buff, BUFLEN, stdin);
    return (buff);
}

int crearColaMj()
{
    int idc;
    if ((idc = msgget(CLAVE, IPC_CREAT|0666)) == -1)
    {
        perror("Problemas en la creacion de la cola de mensajes\n");
        exit(1);
    }
    return idc;
}

void sendParam(struct st_data dt, int idsk)
{
    char buf[BUFLEN];
}

typedef struct mensaje{
    long canal;
    char cad[100];
}MENSAJE;

int main(int argc, char *argv[])
{
    struct sockaddr_in dirServ, dirClnt;
    int socketServ, sockeClnt, ids, idc, puerto, idm, pid, semnum, semval;
    int n, tam, tipo;
    char buff[BUFLEN], *devc;
    struct st_data dat;
	MENSAJE men;

    devc = argv[1];
    pid = getpid();

    printf("EL nombre seleccionado es: <%s>\n", devc);
    /*Ejercicio 1*/
    getInput("Pulsa para empezar\n");

    socketServ = crearSck();
    idc =  msgget(CLAVE, 0666);
    idm = shmget(CLAVE, SIZE_SHM, 0);
    ids = shmget(CLAVE, MAX_SEM, 0);

    /*REALIZAMOS LA ASIGNACION DE DIRECCIONES EN MEMORIA*/
    dirServ.sin_family = AF_INET;
    dirServ.sin_addr.s_addr = INADDR_ANY;
    dirServ.sin_port = htons(SERVER_PORT);

    tam = sizeof(dirServ);
    bind(socketServ, (struct sockaddr *)&dirServ, tam);
    printf("Iniciando conversacion...\n");

    sprintf(buff, "HELLO %d %s\n", pid, devc);
    printf("TX: <%s>\n",buff);
    sendto(socketServ, buff, strlen(buff), 0, (struct sockaddr *)&dirServ, tam);
    recvfrom(socketServ, buff, strlen(buff), 0, (struct sockaddr *)&dirServ, &tam);
    printf("RX: <%s>\n", buff);

    getsockname(socketServ, (struct sockaddr *)&dirClnt, &tam);
    puerto = ntohs(dirClnt.sin_port);

    sprintf(buff, "PORT %d\n", puerto);
    printf("TX: <%s>\n",buff);
    sendto(socketServ, buff, strlen(buff), 0, (struct sockaddr *)&dirServ, tam);
    recvfrom(socketServ, buff, strlen(buff), 0, (struct sockaddr *)&dirServ, &tam);
    printf("RX: <%s>\n", buff);
   
    while (1)
    {
        /*Ejercicio 2*: obtencion de datos a partir de una cola de msj*/
        msgrcv(idc, &men, sizeof(men)- sizeof(long), getpid(),0);
       


        if (*men.cad  == COMM_SPEED)
        {
	printf("ENTRO");
            dat.speed = atoi(buff + sizeof(long) + sizeof(int));
            sprintf(buff, "SPEED %d", dat.speed);
            sendto(socketServ, buff, strlen(buff), 0, (struct sockaddr *)&dirServ, tam);
        }
        else if (tipo == COMM_RPM)
        {
            dat.rpm = *(int*)(buff + sizeof(long) + sizeof(int));
            sprintf(buff, "RPM %d", dat.rpm);
            sendto(socketServ, buff, strlen(buff), 0, (struct sockaddr *)&dirServ, tam);
        }
        else if (tipo == COMM_SEM)
        {
            dat.sem = *(int*)(buff + sizeof(long) + sizeof(int));
            dat.semval = semctl(ids, semnum, GETVAL, 0);
            sprintf(buff, "SEM %d %d", dat.sem, dat.semval);
            sendto(socketServ, buff, strlen(buff), 0, (struct sockaddr *)&dirServ, tam);
        }
        else if (tipo == COMM_BYE)
        {
            printf("Recividos todos los comandos\n");
            sprintf(buff, "BYE");
            sendto(socketServ, buff, strlen(buff), 0, (struct sockaddr *)&dirServ, tam);
            printf("Terminada comunicacion/n");
        } 
}

    
    
            
    
}
