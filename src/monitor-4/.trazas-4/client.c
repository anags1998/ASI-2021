#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

int main(int argc, char** argv) {

    struct sockaddr_in server, client;
    struct hostent *h;
    int ds, dc, addrlen;
    char cad[5];
    
    /*Ejercicio1*/
    ds=socket(AF_INET, SOCK_STREAM, 0); 
    h=gethostbyname("localhost");
    server.sin_family=AF_INET;
    server.sin_port=htons(3000);
    server.sin_addr.s_addr=((struct in_addr *)(h->h_addr))->s_addr;
    memset(&(server.sin_zero), 0, sizeof(server));
    
    connect(ds,(struct sockaddr *)&server, sizeof(server)); 
    printf("\nFin del ejercicio 1.Pulsa intro.\n");
    getchar();
    
    /*Ejerccicio2*/
    read(ds, cad, 5); 
    
    printf("\nFin del ejercicio 2.Pulsa intro.\n");  
    getchar();
    
    /*Ejercicio3*/
    write(ds, cad, strlen(cad)); 
    read(ds, cad, 5);
    write(ds, cad, strlen(cad));
    
    printf("\nFin del ejercicio 3.Pulsa intro.\n");
    getchar();
    
    /*Ejercicio4*/
    /*****SERVIDOR TCP*****/
    ds=socket(AF_INET, SOCK_STREAM,0);
    
    server.sin_family=AF_INET;
    server.sin_port=htons(3001);
    server.sin_addr.s_addr=htonl(INADDR_ANY);
    memset(&(server.sin_zero), 0, sizeof(server));
    
    bind(ds, (struct sockaddr*)&server, sizeof(server)); 
    
    listen(ds, SOMAXCONN); 
    addrlen=sizeof(client);
    dc=accept(ds, (struct sockaddr*)&client, &addrlen); 
    printf("\nFin del ejercicio 4.Pulsa intro.\n");
    getchar();
    
    /*Ejercicio5*/
    read(dc, cad, 5);
    write(dc, cad, strlen(cad));
    
    printf("\nFin del ejercicio 5.Pulsa intro.\n");
    getchar();
    
    /*Ejercicio6*/
   
    ds=socket(AF_INET, SOCK_DGRAM, 0);
    
    
    h=gethostbyname("localhost");
    server.sin_family=AF_INET;
    server.sin_port=htons(3000);
    server.sin_addr.s_addr=((struct in_addr *)(h->h_addr))->s_addr;
    memset(&(server.sin_zero), 0, sizeof(server));
    
    client.sin_family=AF_INET;
    client.sin_port=htons(3000);
    client.sin_addr.s_addr=htonl(INADDR_ANY);
    memset(&(server.sin_zero), 0, sizeof(server));
    
    bind(ds,(struct sockaddr*)&client, sizeof(client));
    
    sprintf(cad, "HOLA!");
    sendto(ds, cad, strlen(cad), 0, (struct sockaddr*)&server, sizeof(server));
    
    printf("\nFin del ejercicio 6.Pulsa intro.\n");    
    getchar();
    
    /*Ejercicio7*/
    ds=socket(AF_INET,SOCK_DGRAM,0);
    
    
    server.sin_family=AF_INET;
    server.sin_port=htons(3001);
    server.sin_addr.s_addr=htonl(INADDR_ANY);
    memset(&(server.sin_zero), 0, sizeof(server));
    
    bind(ds,(struct sockaddr*)&server,sizeof(server));
    
    addrlen=sizeof(client);
    recvfrom(ds, cad, 5, 0, (struct sockaddr*)&client, &addrlen);
    sendto(ds, cad, strlen(cad), 0, (struct sockaddr*)&client, sizeof(client));
    
    printf("\nFin del ejercicio 7.Pulsa intro.\n");
    getchar();
    
    return (EXIT_SUCCESS);
}

