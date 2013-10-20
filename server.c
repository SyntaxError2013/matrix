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
#include <sys/select.h>


#define DEFAULT_SERVER_PORT 5432
#define MAX_MSG_SIZE 1024
#define MAX_CLIENTS 100


void exit_client(int fd, fd_set *readset, int fd_array[], int *num_clients);

int main(int argc, char *argv[])
{
    int port = DEFAULT_SERVER_PORT;
    int server_sockfd, client_sockfd;
    struct sockaddr_in server_addr, client_addr;
    int addrlen = sizeof(struct sockaddr_in);

    char message[MAX_MSG_SIZE + 1],
         kbd_input[MAX_MSG_SIZE + 1];

    fd_set readset, tempset;
    int num_clients = 0;
    int fd_array[MAX_CLIENTS];
    int i, fd;

    //parse command line arguments
    if(argc == 3){
        if(!strcmp("-p", argv[1]))
            sscanf(argv[1], "%d", &port);
        else{
            printf("Usage: %s [-p PORT]\n", argv[0]);
            exit(0);
        }
    }else if(argc != 1){
        printf("Usage: %s [-p PORT]\n", argv[0]);
        exit(0);
    }

    printf("*** Server waiting ***\n");

    bzero(&server_addr, addrlen);
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);

    //bind
    bind(server_sockfd, (struct sockaddr *) &server_addr, addrlen);

    //listen
    listen(server_sockfd, 10);

    FD_ZERO(&readset);
    FD_SET(server_sockfd, &readset);
    FD_SET(fileno(stdin), &readset);
    //wait for clients
    while(1){

        tempset = readset;
        select(FD_SETSIZE, &readset, NULL, NULL, NULL);

        for(fd = 0; fd < FD_SETSIZE; fd++){
            if(FD_ISSET(fd, &readset)){
                if(fd == server_sockfd){
                    // accept new client connection request
                    client_sockfd = accept(server_sockfd, (struct sockaddr *) &client_addr, &addrlen);

                    if(num_clients < MAX_CLIENTS){
                        FD_SET(client_sockfd, &readset);
                        fd_array[num_clients] = client_sockfd;

                        //welcome client
                        sprintf(message, "Welcome client xyz\n");
                        write(client_sockfd, message, strlen(message));
                        ++num_clients;
                        printf(">> ");
                        fflush(stdout);
                    }else{
                        sprintf(message, "Sorry, too many clients connected. Try again later.\n");
                        write(client_sockfd, message, strlen(message));
                        close(client_sockfd);
                    }
                }else if(fd == fileno(stdin)){
                    printf(">> ");
                    fgets(kbd_input, MAX_MSG_SIZE, stdin);

                    if(!strcmp("quit\n", kbd_input)){
                        sprintf(message, "Server shutting down!\n");

                        for(fd = 0; fd < num_clients; fd++){
                            write(fd_array[fd], message, strlen(message));
                            close(fd_array[fd]);
                        }

                        close(server_sockfd);
                        exit(0);
                    }else{
                        for(fd = 0; fd < num_clients; fd++){
                            write(fd_array[fd], kbd_input, strlen(kbd_input));
                        }
                    }
                }else if(fd){//client socket
                    //client will only send EOF
                    //so remove that client from fd_array
                    exit_client(fd, &readset, fd_array, &num_clients);
                }
            }
        }
    }
    return 0;
}


void exit_client(int fd, fd_set *readset, int fd_array[], int *num_clients)
{
    close(fd);
    FD_CLR(fd, readset);
    int i;
    for(i = 0; i < (*num_clients) - 1; i++)
        if(fd_array[i] == fd)
            break;
    for(; i < (*num_clients) - 1; i++)
        fd_array[i] = fd_array[i + 1];
    (*num_clients)--;
}
