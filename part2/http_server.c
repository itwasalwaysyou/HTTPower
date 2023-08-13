#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "connection_queue.h"
#include "http.h"

#define BUFSIZE 512
#define LISTEN_QUEUE_LEN 5
#define N_THREADS 5


int keep_going = 1;
connection_queue_t client_line;




const char *serve_dir;

void handle_sigint(int signo) {
    keep_going = 0;

    //set shutdown to 1;
    if(connection_queue_shutdown(&client_line)==-1){
        perror("connection_queue_shutdown");
    };


}

void *listenForConnect(void *line){
    //note: client line is a shared rescource!
    connection_queue_t *client_line= (connection_queue_t*)line;

    while(1){
    //keep listen...

        char file_name[BUFSIZE];

        //remove:  curr client_fd <- client line
        int curr_thread_client_fd;
        if ((curr_thread_client_fd = connection_dequeue(client_line)) == -1) {
            return NULL;
        }

        if(client_line->shutdown==1){
            return NULL;
        }

        if (read_http_request(curr_thread_client_fd, file_name)==-1){
            perror("read");
            close(curr_thread_client_fd);
            return NULL;
        }

        char path_name[BUFSIZE];
        strcpy(path_name, serve_dir);
        strcat(path_name, file_name);

        if(write_http_response(curr_thread_client_fd, path_name)==-1){
            perror("write");
            close(curr_thread_client_fd);
            return NULL;
        }

        if(close(curr_thread_client_fd)==-1){
            perror("close1");
            return NULL;
        };

    }
    return NULL;
}






int main(int argc, char **argv) {

    if (argc != 3) {
        printf("Usage: %s <directory> <port>\n", argv[0]);
        return 1;
    }

    serve_dir = argv[1];
    const char *port = argv[2];

    struct sigaction act;     

    //catch ctrl+c
    act.sa_flags=0;
    if(sigemptyset(&act.sa_mask)==-1){
        perror("sigemptyset");
        return -1;
    };
    act.sa_handler=handle_sigint;

    if(sigaction(SIGINT,&act,NULL)==-1){
        perror("sigaction");
        return -1;
    };

    //block all possible signals befor cread threads
    sigset_t oldmask;
    if(sigprocmask(SIG_BLOCK,&act.sa_mask,&oldmask)==-1){
        perror("sigprocmask");
        return -1;
    };

    // addrinfo
    struct addrinfo hints;
    memset(&hints,0,sizeof(hints));
    hints.ai_family= AF_UNSPEC;
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    struct addrinfo *server;

   // Set up address info for socket() and connect()
   // return address(linked list) -> &server
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
    freeaddrinfo(server);   

    //listen socket start listen! 
    if (listen(sock_fd, LISTEN_QUEUE_LEN) == -1) {
        perror("listen");
        close(sock_fd);
        return 1;
    }

    if (connection_queue_init(&client_line) != 0) {
        close(sock_fd);
        return -1;
    }

    // create multi- thread, open to accept
    pthread_t threads[N_THREADS];

    for(int i= 0;i<N_THREADS;i++){
    // create n threads for listen   
        //success return 0; fail return errno
        int curr_result= pthread_create(&threads[i], NULL, listenForConnect, (void*)&client_line);
        if(curr_result!=0){
            printf("%s\n",strerror(curr_result));
            return -1;
        }

    }

    //restore old mask
    if(sigprocmask(SIG_SETMASK,&oldmask,NULL)==-1){
        perror("sigprocmask");
        return -1;

    };

    // keep creating new sockets to commmunicate with clients! 
    while(keep_going){
    //jump loop and clean-up by CTRL+C
        // every accept() create a new cliend_fd
        int client_fd = accept(sock_fd, NULL, NULL);
            if (client_fd == -1) {
                if (errno != EINTR) {
                    perror("accept");
                    close(sock_fd);
                    return 1;
                } else {
                    break;
                }
        }
        //once accept() : a new client_fd -> client_line 
        if (connection_enqueue(&client_line, client_fd) != 0) {
            continue;
        }
    }


    // wait child threads:
    for(int i= 0;i<N_THREADS;i++){
        void* ptr=NULL;
        
        int curr_result= pthread_join(threads[i], &ptr);
        if(curr_result!=0){
            printf("%s\n",strerror(curr_result));
            return -1;

        };

    }

    // queue free
    if(connection_queue_free(&client_line)==-1){
        perror("connection_queue_free");
        return -1;

    };
    if(close(sock_fd)==-1){
        perror("close");
        return -1;
    };

    return 0;
}




