#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#pragma comment(lib,"ws2_32.lib")//linking winsock library
#define SERVER_IP "127.0.0.1"//localhost  server ip
#define SERVER_PORT 8080 //server port
#define BUFFER_SIZE 1024//max size of msg
#define NAME_LEN 50 //max length of name

SOCKET client_socket;
struct sockaddr_in server_addr;
int running=1;
char username[NAME_LEN];

void receive_messages(void *arg){//to receive messages
    char buffer[BUFFER_SIZE];
    while(running){
        memset(buffer,0,BUFFER_SIZE);
        int recv_len=recvfrom(client_socket, buffer, BUFFER_SIZE, 0, NULL, NULL);
        if(recv_len==SOCKET_ERROR){
            int error=WSAGetLastError();
            if(error!=WSAEWOULDBLOCK){
                printf("Receive error: %d\n",error);
            }
            continue;
        }
        printf("\n%s\nYou: ",buffer);//print received message
        fflush(stdout);
    }
}

int main(){
    WSADATA wsa;
    char buffer[BUFFER_SIZE];
    //initialising winsock
    if(WSAStartup(MAKEWORD(2,2),&wsa)!=0){
        printf("Winsock init failed\n");
        return 1;
    }
    //creating udp socket
    client_socket=socket(AF_INET,SOCK_DGRAM,0);
    if(client_socket==INVALID_SOCKET){
        printf("Socket creation failed\n");
        return 1;
    }
    //configuring server address
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(SERVER_PORT);
    server_addr.sin_addr.s_addr=inet_addr(SERVER_IP);
    printf("Enter your username: ");
    fgets(username,NAME_LEN,stdin);
    username[strcspn(username,"\n")]='\0';
    //registering with server
    char reg_msg[BUFFER_SIZE];
    snprintf(reg_msg,BUFFER_SIZE,"Register:%s",username);
    sendto(client_socket, reg_msg, strlen(reg_msg),0,(struct sockaddr*)&server_addr, sizeof(server_addr));
    printf("\nWelcome %s. Commands: \n",username);
    printf("/list: Show online users\n");
    printf("/private[ID][msg]:Send private message\n");
    printf("/exit: Quit\n");
    _beginthread(receive_messages,0,NULL);//start receive thread
    while(1){
        printf("You: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer,"\n")]='\0';
        if(strcmp(buffer,"/exit")==0){
            running=0;//stop receive thread
            break;
        }
        sendto(client_socket, buffer, strlen(buffer),0,(struct sockaddr*)&server_addr, sizeof(server_addr));
    }
    closesocket(client_socket);
    WSACleanup();
    return 0;
}
