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
    int ds,num,addrlen;
    char cad[20],*cad2;
    
    dni=strtok(argv[1],"@");
    dominio=strtok(NULL,":");
    puerto=strtok(NULL,"");
    
    h=gethostbyname(dominio);
    servidor.sin_family=AF_INET;
    servidor.sin_port=htons(atoi(puerto));
    servidor.sin_addr.s_addr=((struct in_addr*)(h->h_addr))->s_addr;
    bzero(&(servidor.sin_zero),8);

    ds=socket(AF_INET,SOCK_DGRAM,0);

    connect(ds,(struct sockaddr*)&servidor,sizeof(servidor));
    
    sprintf(cad,"get clave %s\n",dni);
    sendto(ds,cad,strlen(cad),0,(struct sockaddr*)&servidor,sizeof(servidor));
    printf("TX:%s\n",cad);
    addrlen=sizeof(servidor); //De quien recibimos
    recvfrom(ds,cad,sizeof(cad),0,(struct sockaddr*)&servidor,&addrlen);
    printf("RX:%s\n",cad);
    sscanf(cad,"CLAVE:<%d>\n",&num);
    sprintf(cad,"<%d>\n",num);
    sendto(ds,cad,sizeof(cad),0,(struct sockaddr*)&servidor,sizeof(servidor));
    printf("TX:%s\n",cad);
    
    close(ds);
}
