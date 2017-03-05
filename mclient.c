/*
 * Echo klientas
 * 
 * Author: Kęstutis Mizara
 * Description: Išsiunčia serveriui pranešimą ir jį gauna
 */
#ifdef _WIN32
#include<winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <arpa/inet.h>
#endif
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFLEN 1024

int main(int argc, char *argv[]){
#ifdef _WIN32
    WSADATA data;
#endif 
    unsigned int port;
    int s_socket;
    struct sockaddr_in servaddr; // Serverio adreso struktūra
    fd_set read_set;
#ifdef _WIN32
    WSAStartup(MAKEWORD(2,2),&data);    
#endif

    char recvbuffer[BUFFLEN];
    char sendbuffer[BUFFLEN];
	char buffer[BUFFLEN];

    int i;

    if (argc != 3){
        fprintf(stderr,"USAGE: %s <ip> <port>\n",argv[0]);
        exit(1);
    }

    port = atoi(argv[2]);

    if ((port < 1) || (port > 65535)){
        printf("ERROR #1: invalid port specified.\n");
        exit(1);
    }

    /*
     * Sukuriamas socket'as
     */
    if ((s_socket = socket(AF_INET, SOCK_STREAM,0))< 0){
        fprintf(stderr,"ERROR #2: cannot create socket.\n");
        exit(1);
    }
                                
   /*
    * Išvaloma ir užpildoma serverio struktūra
    */
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family = AF_INET; // nurodomas protokolas (IP)
    servaddr.sin_port = htons(port); // nurodomas portas
    
    /*
     * Išverčiamas simbolių eilutėje užrašytas ip į skaitinę formą ir
     * nustatomas serverio adreso struktūroje
	 inet_aton(argv[1], &servaddr.sin_addr) <= 0
     */
    if ((servaddr.sin_addr.s_addr = inet_addr(argv[1])) <= 0) {
        fprintf(stderr,"ERROR #3: Invalid remote IP address.\n");
        exit(1);
    }

    /* 
     * Prisijungiama prie serverio
     */
    if (connect(s_socket,(struct sockaddr*)&servaddr,sizeof(servaddr))<0){
        fprintf(stderr,"ERROR #4: error in connect().\n");
        exit(1);
    }
    memset(&sendbuffer,0,BUFFLEN);
#ifdef _WIN32
#else
    fcntl(0,F_SETFL,fcntl(0,F_GETFL,0)|O_NONBLOCK);
#endif
    while (1){
        FD_ZERO(&read_set);
        FD_SET(s_socket,&read_set);
        FD_SET(0,&read_set);

        select(s_socket+1,&read_set,NULL,NULL,NULL);

        if (FD_ISSET(s_socket, &read_set)){
            memset(&recvbuffer,0,BUFFLEN);
            i = read(s_socket, &recvbuffer, BUFFLEN);
            printf("%s\n",recvbuffer);
        }
        else if (FD_ISSET(0,&read_set)) {
            i = read(0,&sendbuffer,1);
            write(s_socket, sendbuffer,i);
        }
		    printf("Enter the message: ");
    fgets(buffer, BUFFLEN, stdin);
    /*
     * Išsiunčiamas pranešimas serveriui
     */
    send(s_socket,buffer,strlen(buffer),0);

   memset(&buffer,0,BUFFLEN);
    /*
     * Pranešimas gaunamas iš serverio
     */
#ifdef _WIN32
   recv(s_socket,buffer,BUFFLEN,0);
#else
   recv(s_socket,&buffer,BUFFLEN,0);
#endif
   printf("Server sent: %s\n", buffer);
    }



    /*
     * Socket'as uždaromas
     */
#ifdef _WIN32
	closesocket(s_socket);
	WSACleanup();
#else
	close(s_socket);
#endif
    return 0;
}
