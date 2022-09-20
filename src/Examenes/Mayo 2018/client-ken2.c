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

#define SERVER_PORT 3010
#define SERVERIP "127.0.0.1"

#define CLAVE 0x45916038L // Clave de los recursos. Sustituir por DNI.

#define SIZE_SHM 4096 // Tamaño del segmento de memoria compartida

#define MAX_DEV 4 // Máximo # dispositivos # [1..4]

#define DEVOFFSET 200 // Offset entre dispositivos

/* El dispositivo 1 comienza en la posición 0 de la memoria
	El dispositivo 2 en la posición 0+OFFSET
	El dispositivo 3 en OFFSET*2
	El dispositivo 4 en OFFSET*3
*/

/* Formato de un registro de dispositivo */

struct shm_dev_reg
{
    int estado;     /* 1 activo, cualquier otra cosa libre */
    int num_dev;    /* numero de dispositivo #[1..4] */
    char descr[15]; /* descripción del dispositivo*/
    int n_cont;     /* número de contadores para el dispositivo*/
};

/* Detrás del registro del dispositivo (+sizeof(struct shm_dev_reg)) 
   se almacenan uno tras otro sus valores en formato (int)
*/

/* El array de semáforos a crear tiene tamaño 5
	El semáforo 0 no se utilizará
	El semáforo 1 controla el acceso al registro del dispositivo 1 
	El semáforo # controla el acceso al registro del dispositivo # [1..4]
*/
#define MAX_SEM 5 // Número de semáforos en el array

/* Lista de mensajes UDP */
#define HELLO "HLO"
#define OK "OK"
#define WRITE 'W'

/* Comandos cola de mensajes */
#define DUMP 'D'

/* Constantes del desarrollo del ejercicio*/
#define SIZE 256

/* Estructura para la lectura de comando + dispositivo: Total 8 bytes */

struct msgq_input
{
    char cmd;    /* Comando */
    int num_dev; /* numero de dispositivo */
};

/*Intentaremos con estructuras y memorias*/

#ifdef STRUCT /*Estructura para la recepcion del comando hlo y param*/
typedef struct hello
{
    char[3] command;
    char num;
    char cont;
    char descr[15];
} HLO;

#endif

char *getInput(char *msg)
{
    char static buff[SIZE];

    printf("%s", msg);
    fflush(stdout);
    fgets(buff, SIZE, stdin);
    return (buff);
}

int main(int argc, char *argv[])
{
    struct sockaddr_in dirServ, dirClnt;
    int ids, idm, idc, sockSrv, sockClnt, tam, num, n, cont, ind, val, estd = 1;
    char buff[SIZE], des[15], *mem, comamd;
    struct shm_dev_reg *reg;

    idc = msgget(CLAVE, IPC_CREAT | 0666);
    ids = semget(CLAVE, MAX_SEM, 0);
    idm = shmget(CLAVE, SIZE_SHM, IPC_CREAT | 0666);
    mem = shmat(idm, NULL, 0);

    getInput("Pulsa para empezar");
    /*Abrimos el socket*/

    if ((sockSrv = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("Problemas en la creacion del socket\n");
        exit(1);
    }

    /*Asignacion de parametros de memoria*/

    dirServ.sin_family = AF_INET;
    dirServ.sin_port = htons(SERVER_PORT);
    dirServ.sin_addr.s_addr = INADDR_ANY;

    /*Enlazado de socket y direcciones*/
    tam = sizeof(dirServ);
    bind(sockSrv, (struct sockaddr *)&dirServ, tam);

    /*Intercambio de datos*/
    while ((n = recvfrom(sockSrv, buff, SIZE, 0, (struct sockaddr *)&dirServ, &tam)) > 0)
    {
        /*Forma un poco cutre
    num = atoi(buff+5);
    cont = atoi(buff+9);*/
        buff[n] = '\0';
        comamd = *buff;
        printf("Comando : %c\n", comamd);
        if (*buff == 'H')
        {
        sscanf(buff, "HLO <%d> <%d> %s", &num, &cont, des);
        printf("Valores obtenidos: cmd:%c, num:%d, cont:%d\n", comamd, num, cont);

        reg = (struct shm_dev_reg *)(mem + DEVOFFSET * (num - 1));
        reg->num_dev = num;
        reg->estado = estd;
        sprintf(reg->descr, "%s", des);
        }
       else  if (*buff == 'W')
        {
            num = *(int *)(buff + 1);
            ind = *(int *)(buff + 1 + sizeof(int));
            val = *(int *)(buff + 1 + 2 * sizeof(int));

            printf("Almacenando valores: %d %d %d\n", num, ind, val);

            *((int *)(mem + (DEVOFFSET * (num - 1)) + sizeof(struct shm_dev_reg) + sizeof(int) * ind)) = val;

        } /*else{
            printf("Comando desconocido\n");
            exit(1);
        }*/

        sprintf(buff, "OK <%d>", num);
        sendto(sockSrv, buff, SIZE, 0, (struct sockaddr *)&dirServ, tam);
    }

    exit(0);
}
