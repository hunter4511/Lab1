#include<stdio.h> 
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h> 
#include <stdlib.h>
#include<netdb.h> 
#include <unistd.h>
 
int to_ip(char * hostname , char* ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;
         
    if ( (he = gethostbyname( hostname ) ) == NULL) 
    {
        puts("gethostbyname");
        return 1;
    }
 
    addr_list = (struct in_addr **) he->h_addr_list;
     
    for(i = 0; addr_list[i] != NULL; i++) 
    {
        strcpy(ip , inet_ntoa(*addr_list[i]) );
        return 0;
    }
     
    return 1;
}

int main(int argc , char *argv[])
{
    int sock;
    struct sockaddr_in server;
    char server_reply[2000], ip[30];

    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Cannot create socket");
        return 1;
    }
    puts("Socket successfully created");
    
    if(argc == 2) {       
        to_ip(argv[1], ip);
        printf("IP: %s \n", ip);

        server.sin_addr.s_addr = inet_addr(ip);
        printf("Address: %s \n", argv[1]);
    }else{
        server.sin_addr.s_addr = inet_addr("127.0.0.1");        
    }

    server.sin_family = AF_INET;
    server.sin_port = htons( 6379 );
 
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("Connection denied");
        return 1;
    }
     
    char *commands[4] = {
        "SET 'ключ' 'тест'\n",
        "GET 'ключ'\n"
    };

    for(int i=0;i<2;i++)
    {
        printf("MSG:\n%s\n", commands[i]);
         
        if( send(sock , commands[i] , strlen(commands[i]) , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
         
        if( recv(sock , server_reply , 2000 , 0) < 0)
        {
            puts("recv failed");
            break;
        }
         
        puts("RESPONSE :\n");
        puts(server_reply);
    }
     
    close(sock);
    return 0;
}