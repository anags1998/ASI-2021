#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>

int main(int argc, char **argv){
    char *dni,*dominio,*puerto;
    struct sockaddr_in servidor;
    struct hostent *h;
    int ds,num;
    char cad[20],*cad2;
    
    dni=strtok(argv[1],"@");
    dominio=strtok(NULL,":");
    puerto=strtok(NULL,"");
    
    h=gethostbyname(dominio);
    servidor.sin_family=AF_INET;
    servidor.sin_port=htons(atoi(puerto));
    servidor.sin_addr.s_addr=((struct in_addr*)(h->h_addr))->s_addr;
    bzero(&(servidor.sin_zero),8);
    ds=socket(AF_INET,SOCK_STREAM,0);
    connect(ds,(struct sockaddr*)&servidor,sizeof(servidor));
    sprintf(cad,"user %s\n",dni);
    write(ds,cad,strlen(cad));
    printf("TX:%s\n",cad);
    read(ds,cad,20);
    printf("RX:%s\n",cad);
    sprintf(cad,"get clave\n");
    write(ds,cad,strlen(cad));
    printf("TX:%s\n",cad);
    read(ds,cad,20);
    printf("RX:%s\n",cad);
    sscanf(cad,"CLAVE:<%d>\n",&num);
    sprintf(cad,"<%d>\n",num);
    write(ds,cad,strlen(cad));
    printf("TX:%s\n",cad);
    read(ds,cad,20);
    printf("RX:%s\n",cad);
    close(ds);
}
