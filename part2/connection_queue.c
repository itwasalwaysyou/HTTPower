#include <stdio.h>

#include <string.h>

#include "connection_queue.h"




int connection_queue_init(connection_queue_t *queue) {


    queue->length = 0;

    queue->read_idx = 0;

    queue->write_idx = 0;

    queue->shutdown = 0; 


    //init lock

    //unlock: mutex=1 

    if (pthread_mutex_init(&queue->mutex, NULL) != 0) {

        perror("pthread_mutex_init");

        return -1;

    }


    if (pthread_cond_init(&queue->full, NULL)!=0) {

        perror("pthread_cond_init");

        return -1;

    }


    if (pthread_cond_init(&queue->empty, NULL)!=0) {

        perror("pthread_cond_init");

        return -1;

    }


    return 0;

}





int connection_enqueue(connection_queue_t *queue, int connection_fd) {


    


    // 1 -> 0 lock

    if (pthread_mutex_lock(&queue->mutex) != 0) {

        perror("pthread_mutex_lock");

        return -1;

    }


    while (queue->length == CAPACITY) {

        //block

        //wait for an avialible seat 

        // 0 -> 1 unlock

        if (pthread_cond_wait(&queue->full, &queue->mutex) != 0) {

            perror("pthread_cond_wait");

            return -1;

        }

    }


    // access shared recources: add a now client to queue

    queue->client_fds[queue->write_idx] = connection_fd;

    queue->length++;

    queue->write_idx=((queue->write_idx)+1)%CAPACITY; //[0,4]

    if (pthread_cond_signal(&queue->empty) != 0) {

        perror("pthread_cond_signal");

        return -1;

    }


    // 0 -> 1 unlock

    if (pthread_mutex_unlock(&queue->mutex) != 0) {

        perror("pthread_mutex_unlock");

        return -1;

    }


    return 0;

}




//return socket_fd

int connection_dequeue(connection_queue_t *queue) {


    if (pthread_mutex_lock(&queue->mutex) != 0) {

        perror("pthread_mutex_lock");

        return -1;

    }


    while (queue->length == 0) {

        if (pthread_cond_wait(&queue->empty, &queue->mutex) != 0) {

            perror("pthread_cond_wait");

            return -1;

        }

        if(queue-> shutdown==1){


            if (pthread_mutex_unlock(&queue->mutex) != 0){

                perror("pthread_mutex_unlock");

                return -1;

            }


            return -1;

        }

    }


    int client_fd= queue->client_fds[queue->read_idx];

    queue->read_idx=((queue->read_idx)+1)%CAPACITY; //[0,4]

    queue->length--;


    if (pthread_cond_signal(&queue->full) != 0) {

        perror("pthread_cond_signal");

        return -1;

    }

    if (pthread_mutex_unlock(&queue->mutex) != 0){

        perror("pthread_mutex_unlock");

        return -1;

    }

    return client_fd;

}


int connection_queue_shutdown(connection_queue_t *queue) {

    queue->shutdown = 1;


    if (pthread_cond_broadcast(&queue->full) != 0) {

        perror("pthread_cond_broadcast");

        return -1;

    }

    if (pthread_cond_broadcast(&queue->empty) != 0) {

        perror("pthread_cond_broadcast");

        return -1;

    }


    return 0;

}


int connection_queue_free(connection_queue_t *queue) {


    

    if (pthread_mutex_destroy(&queue->mutex) != 0) {

        perror("pthread_cond_broadcast");

        return -1;

    }

    if (pthread_cond_destroy(&queue->full) != 0) {

        perror("pthread_cond_destroy");

        return -1;

    }

    if (pthread_cond_destroy(&queue->empty) != 0){

        perror("pthread_cond_destroy");

        return -1;

    }


    return 0;


}
