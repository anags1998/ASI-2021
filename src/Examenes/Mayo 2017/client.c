#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>


#define SIZE_SHM 4096

#define OFF_FISRT_KEY 2
#define OFF_DATA_SEM 10
#define OFF_TBL_KEY 16
#define OFF_REG_KEY 48

#define NSEMS 3
#define CLAVE 0x45916038L

extern int connect_MONITOR(char *);
extern int check_key(int, int );
extern int do_child_work(int fildes[2]);


int f[2],clave3;
char cad[100];

int main(int argc, char** argv){

printf("\nDale al intro una vez pulsado 1 en Monitor\n");
getchar();

connect_MONITOR("45916038B");

pipe(f);

if(fork()==0){
do_child_work(f);
exit(0);
}else{
sprintf(cad,"HELLO");
write(f[1],cad,strlen(cad));
sleep(1);
read(f[0],&clave3,sizeof(int));
check_key(3,clave3);
}



return(EXIT_SUCCESS);
}
