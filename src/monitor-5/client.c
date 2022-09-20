#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>


void *funcion(){
    wait(NULL);
}

int main()
{
    struct sockaddr_in servidor,cliente;
    struct hostent *h;
    int ds,num,addrlen,dc;
    char cad[5];
    
    //Ejercicio1
    h=gethostbyname("localhost");
    servidor.sin_family=AF_INET;
    servidor.sin_port=htons(3000);
    servidor.sin_addr.s_addr=((struct in_addr*)(h->h_addr))->s_addr;
    bzero(&(servidor.sin_zero),8);
    ds=socket(AF_INET,SOCK_STREAM,0);
    connect(ds,(struct sockaddr*)&servidor,sizeof(servidor));
    read(ds,&num,sizeof(int));
    sprintf(cad,"<%d>",num);
    write(ds,cad,strlen(cad));
    close(ds);
    printf("\nFin del ejercicio 1.");
    getchar();
    
    if(fork()==0)
    {
        servidor.sin_family=AF_INET;
        servidor.sin_port=htons(3001);
        servidor.sin_addr.s_addr=htonl(INADDR_ANY);
        bzero(&(servidor.sin_zero),8);
        ds=socket(AF_INET,SOCK_STREAM,0);
        bind(ds,(struct sockaddr*)&servidor,sizeof(servidor));
        listen(ds,SOMAXCONN);
        signal(SIGCHLD,funcion);
        while(1)
        {
            addrlen=sizeof(cliente);
            dc=accept(ds,(struct sockaddr*)&cliente,&addrlen);
            if(fork()==0)
            {
                close(ds);
                while(read(dc,&num,sizeof(int))>0)
                {
                    sprintf(cad,"<%d>",num);
                    write(dc,cad,strlen(cad));
                }
                close(dc);
                exit(0);
            }
            else
                close(dc);
            
        }
    }
    printf("\nFin del ejercicio 2.");
    getchar();
    
    //Ejercicio3
    h=gethostbyname("localhost");
    servidor.sin_family=AF_INET;
    servidor.sin_port=htons(3000);
    servidor.sin_addr.s_addr=((struct in_addr*)(h->h_addr))->s_addr;
    bzero(&(servidor.sin_zero),8);
    cliente.sin_family=AF_INET;
    cliente.sin_port=htons(3000);
    cliente.sin_addr.s_addr=htonl(INADDR_ANY);
    bzero(&(cliente.sin_zero),8);
    ds=socket(AF_INET,SOCK_DGRAM,0);
    bind(ds,(struct sockaddr*)&cliente,sizeof(cliente));
    sprintf(cad,"<???>");
    sendto(ds,cad,strlen(cad),0,(struct sockaddr*)&servidor,sizeof(servidor));
    addrlen=sizeof(servidor); //De quien recibimos
    recvfrom(ds,cad,5,0,(struct sockaddr*)&servidor,&addrlen);
    sscanf(cad,"<%3d>",&num);
    sendto(ds,&num,sizeof(int),0,(struct sockaddr*)&servidor,sizeof(servidor));
    printf("\nFin del ejercicio 3.");
    getchar();
    
    //Ejercicio4
    servidor.sin_family=AF_INET;
    servidor.sin_port=htons(3001);
    servidor.sin_addr.s_addr=htonl(INADDR_ANY);
    bzero(&(servidor.sin_zero),8);
    ds=socket(AF_INET,SOCK_DGRAM,0);
    bind(ds,(struct sockaddr*)&servidor,sizeof(servidor));
    addrlen=sizeof(cliente);
    recvfrom(ds,&num,sizeof(int),0,(struct sockaddr*)&cliente,&addrlen);
    sprintf(cad,"<%d>",num);
    sendto(ds,cad,strlen(cad),0,(struct sockaddr*)&cliente,sizeof(cliente));
    while(recvfrom(ds,&num,sizeof(int),0,(struct sockaddr*)&cliente,&addrlen)>0)
    {
        sprintf(cad,"<%d>",num);
        sendto(ds,cad,strlen(cad),0,(struct sockaddr*)&cliente,sizeof(cliente));
    }
    printf("\nFin del ejercicio 4.");
    getchar();
    

}
