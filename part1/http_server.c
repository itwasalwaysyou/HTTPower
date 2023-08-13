#include <errno.h>

#include <netdb.h>

#include <signal.h>

#include <stdio.h>

#include <string.h>

#include <sys/stat.h>

#include <sys/types.h>

#include <sys/socket.h>

#include <unistd.h>


#include "http.h"


#define BUFSIZE 512

#define LISTEN_QUEUE_LEN 5


int keep_going = 1;


void handle_sigint(int signo) {

    keep_going = 0;

}


int main(int argc, char **argv) {

    // First command is directory to serve, second command is port

    if (argc != 3) {

        printf("Usage: %s <directory> <port>\n", argv[0]);

        return 1;

    }


    // Uncomment the lines below to use these definitions:

    const char *serve_dir = argv[1];

    const char *port = argv[2];


    // hints setup

    struct addrinfo hints;

    memset(&hints,0,sizeof(hints));

    hints.ai_family= AF_UNSPEC;

    hints.ai_socktype=SOCK_STREAM;

    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *server;


   // Set up address info for socket() and connect()

    int ret_val = getaddrinfo(NULL, port, &hints, &server);

    if (ret_val != 0) {

        fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(ret_val));

        return -1;

    }


    //create a socket to listen

    int sock_fd= socket(server->ai_family,server->ai_socktype,server->ai_protocol);

    if(sock_fd==-1){

        perror("socket");

        freeaddrinfo(server);

        return -1;

    }


     // Bind for listen

    if (bind(sock_fd, server->ai_addr, server->ai_addrlen) == -1) {

        perror("bind");

        freeaddrinfo(server);

        close(sock_fd);

        return -1;

    }


    // listen for client

    if (listen(sock_fd, LISTEN_QUEUE_LEN) == -1) {

        perror("listen");

        close(sock_fd);

        return 1;

    }


    // create a new socket to commmunicate with client! 

    while(keep_going){


        int client_fd = accept(sock_fd, NULL, NULL);

            if (client_fd == -1) {

                // check if accept() is interrupted by a signal

                if (errno != EINTR) {

                    perror("accept");

                    close(sock_fd);

                    return 1;

                } else {

                    break;

                }

        }


        // read request

        char file_name[BUFSIZE];

        if (read_http_request(client_fd, file_name)==-1){

            perror("read");

            close(client_fd);

            return -1;

        }



        char path_name[BUFSIZE];

        strcpy(path_name, serve_dir);

        strcat(path_name, file_name);

        if(write_http_response(client_fd, path_name)==-1){

            perror("write");

            close(client_fd);

            return -1;

        }

        close(client_fd);      

    }


    close(sock_fd);

    return 0;

}
