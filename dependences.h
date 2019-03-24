//
// Created by MATIAS BUSCO on 2019-03-17.
//

#ifndef SOCKETS_SOCKETS_H
#define SOCKETS_SOCKETS_H

#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/types.h>
#include <commons/log.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>
#define BACKLOG 10 // Conexiones a la espera de un accept

int server_socket, client_socket;
t_queue * proc_queue;
t_log * logger;

typedef struct {
	int client_fd;
	struct sockaddr_in client_dir;
} t_client_proccess;

static t_client_proccess *client_create(int client_fd, struct sockaddr_in client_dir){
	t_client_proccess *new = malloc( sizeof(t_client_proccess) );
	new->client_fd = client_fd;
	new->client_dir = client_dir;
	return new;
}

static void client_destroy(t_client_proccess *self){
	free(self);
}

#endif //SOCKETS_SOCKETS_H
