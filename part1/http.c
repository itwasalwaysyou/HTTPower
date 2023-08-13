#include <assert.h>

#include <errno.h>

#include <fcntl.h>

#include <stdio.h>

#include <sys/stat.h>

#include <string.h>

#include <unistd.h>

#include "http.h"


#include <stdlib.h>


#define BUFSIZE 512


const char *get_mime_type(const char *file_extension) {

    if (strcmp(".txt", file_extension) == 0) {

        return "text/plain";

    } else if (strcmp(".html", file_extension) == 0) {

        return "text/html";

    } else if (strcmp(".jpg", file_extension) == 0) {

        return "image/jpeg";

    } else if (strcmp(".png", file_extension) == 0) {

        return "image/png";

    } else if (strcmp(".pdf", file_extension) == 0) {

        return "application/pdf";

    }


    return NULL;

}



// read request--> fd(for communicate with client)

int read_http_request(int fd, char *resource_name) {

    // allocate a new fd that refer to the same socket fd 

    int sock_fd_copy = dup(fd);

    if (sock_fd_copy == -1) {

        perror("dup");

        return -1;

    }


    // open the sock as stream

    FILE *socket_stream = fdopen(sock_fd_copy, "r");

    if (socket_stream == NULL) {

        perror("fdopen");

        close(fd);

        return -1;

    }

    char buf[BUFSIZE];


    //no buffering, read date from Stream directly! 

    if (setvbuf(socket_stream, buf, _IONBF, BUFSIZE) != 0) { 

        perror("setvbuf");

        fclose(socket_stream);

        return -1;

    }


    //read path name, let main() know, be ready for responds

    char path[BUFSIZE];

    fscanf(socket_stream,"GET %s HEEP/1.0\r\n", path);

    strcpy(resource_name, path);


    // clean socket stream after use

    if (fclose(socket_stream) != 0) {

        perror("fclose");

        return -1;

    }

    close(sock_fd_copy);


    return 0;

}


int write_http_response(int fd, const char *resource_path) {


    int sock_fd_copy = dup(fd);

    if (sock_fd_copy == -1) {

        perror("dup");

        return -1;

    }



    char message[BUFSIZE];


    struct stat st;


    //check if the file exist

    int ret=stat(resource_path,&st);


    if(ret==-1){ //404 NOT FOUND

        strcpy(message, "HTTP/1.0 404 Not Found\r\nContent-Length: 0\r\n\r\n");


        // message => socket

        if (write(sock_fd_copy, message, strlen(message)) == -1) {

            perror("write");

            return -1;

        }


        if(errno!=ENOENT){

            //other invaild errors  

            perror("NOT ENOENT");

            return -1;

        }

        return -1;


    }



    // 200 OK

    int file = open(resource_path, O_RDONLY);

    strcpy(message, "HTTP/1.0 200 OK\r\n");

    strcat(message,"Content-Type: "); 

    char *type= strrchr(resource_path,'.');

    const char* what_type = get_mime_type(type);

    strcat(message, what_type);

    strcat(message, "\r\n");

    strcat(message,"Content-Length: "); 

    char str[BUFSIZE];

    sprintf(str,"%ld",st.st_size);

    strcat(message, str);

    strcat(message, "\r\n");

    strcat(message, "\r\n");


    //send header to socket:

    write(sock_fd_copy,message,strlen(message));

    char buffer[BUFSIZE];


    //send content to socket:

    int r;

    while((r=read(file, buffer, BUFSIZE))>0){

        //buffer => socket

        if(write(fd,buffer,r)==-1){

            perror("write");

            return -1;

        }

    }


    if (close(file) != 0) {

        perror("close");

        return -1;

    }

    return 0;

            

}


    
