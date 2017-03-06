#ifdef _WIN32
#include<winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define BUFFLEN 1024
#define MAXCLIENTS 10

int findemptyuser(int c_sockets[]){
    int i;
    for (i = 0; i <  MAXCLIENTS; i++){
        if (c_sockets[i] == -1){
            return i;
        }
    }
    return -1;
}

int arandomnumber (){
	return rand() % 10;
}

int moreorless (int usernumber, int systemnumber){
	if (usernumber > systemnumber)
		return 1;
	else if (usernumber < systemnumber)
		return 2;
	else
		return 3;
}

int isnumber (int number){
	if (number >= 0 && number <= 9)
		return 1;
	else 
		return 0;
}

int main(int argc, char *argv[]){
#ifdef _WIN32
    WSADATA data;
#endif 
    unsigned int port;
    unsigned int clientaddrlen;
    int l_socket;
    int c_sockets[MAXCLIENTS];
    fd_set read_set;

    struct sockaddr_in servaddr;
    struct sockaddr_in clientaddr;

    int maxfd = 0;
    int i;
	
	//trying to apply some logic lol
	int number[MAXCLIENTS];

    char buffer[BUFFLEN];
	
	int n = 0;
	for (n; n < MAXCLIENTS; n++)
	{
		number[n] = arandomnumber();
	}
	
#ifdef _WIN32
    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2,2),&data) != 0)
    {
        printf("Failed. Error Code : %d",WSAGetLastError());
        exit(EXIT_FAILURE);
    }
     
    printf("Initialised.\n");   
#endif

    if (argc != 2){
        fprintf(stderr, "USAGE: %s <port>\n", argv[0]);
        return -1;
    }


    port = atoi(argv[1]);
    if ((port < 1) || (port > 65535)){
        fprintf(stderr, "ERROR #1: invalid port specified.\n");
        return -1;
    }

    if ((l_socket = socket(AF_INET, SOCK_STREAM,0)) < 0){
        fprintf(stderr, "ERROR #2: cannot create listening socket.\n");
        return -1;
    }

    memset(&servaddr,0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(port);

    if (bind (l_socket, (struct sockaddr *)&servaddr,sizeof(servaddr))<0){
        fprintf(stderr,"ERROR #3: bind listening socket.\n");
        return -1;
    }
	else printf("Binding successfull.\n");
	
	printf("Waiting for clients! :)\n");

    if (listen(l_socket, 5) <0){
        fprintf(stderr,"ERROR #4: error in listen().\n");
        return -1;
    }                           

    for (i = 0; i < MAXCLIENTS; i++){
        c_sockets[i] = -1;
    }


    for (;;){
        FD_ZERO(&read_set);
        for (i = 0; i < MAXCLIENTS; i++){
            if (c_sockets[i] != -1){
                FD_SET(c_sockets[i], &read_set);
                if (c_sockets[i] > maxfd){
                    maxfd = c_sockets[i];
                }
            }
        }

        FD_SET(l_socket, &read_set);
        if (l_socket > maxfd){
            maxfd = l_socket;
        }
        
        select(maxfd+1, &read_set, NULL , NULL, NULL);

        if (FD_ISSET(l_socket, &read_set)){
            int client_id = findemptyuser(c_sockets);
            if (client_id != -1){
                clientaddrlen = sizeof(clientaddr);
                memset(&clientaddr, 0, clientaddrlen);
                c_sockets[client_id] = accept(l_socket, (struct sockaddr*)&clientaddr, &clientaddrlen);
				//send(c_sockets[client_id], message, strlen(message), 0);
                printf("Connected:  %s\n",inet_ntoa(clientaddr.sin_addr));
            }
        }
				//go through all clients
        for (i = 0; i < MAXCLIENTS; i++){
            if (c_sockets[i] != -1){
				
                if (FD_ISSET(c_sockets[i], &read_set)){
                    //cleans buffer before receiving data
					memset(&buffer,0,BUFFLEN);
					//receives data
				#ifdef _WIN32
					int r_len = recv(c_sockets[i], buffer, BUFFLEN, 0);
				#else
               int r_len = recv(c_sockets[i],buffer,BUFFLEN,0);
				#endif
				
				int nr = atoi(buffer);
				memset(&buffer[0], 0, BUFFLEN);
				if (isnumber(nr) == 0)
				{
					strncpy(buffer, "That was not a number or not between 0 and 9.\n\0", BUFFLEN);
				}
				else
				{
					if (moreorless(nr, number[i]) == 1)
					{
						strncpy(buffer, "less\n\0", BUFFLEN);
					}
					else if (moreorless(nr, number[i]) == 2)
					{
						strncpy(buffer, "more\n\0", BUFFLEN);
					}
					else
					{
						strncpy(buffer, "Congratz! You guessed right!\nA new number was assigned.\n\0", BUFFLEN);
						number[i] = arandomnumber();
					}
				}
				#ifdef _WIN32

                        int w_len = send(c_sockets[i], buffer, strlen(buffer),0);
                        if (w_len <= 0){
                        	closesocket(c_sockets[i]);
									c_sockets[i] = -1;
									printf("Client %s signed out\n", inet_ntoa(clientaddr.sin_addr));
                           }    
             #else
                    int j;
                    for (j = 0; j < MAXCLIENTS; j++){
                        if (c_sockets[j] != -1){
                            int w_len = send(c_sockets[j], buffer, strlen(buffer),0);
                            if (w_len <= 0){
                            	  printf("Client %s signed out\n", inet_ntoa(clientaddr.sin_addr));
                                close(c_sockets[j]);
                                c_sockets[j] = -1;

                            }
                        }
                    }//

            #endif
                       }          
                }
            }
        }

    

    return 0;
}
