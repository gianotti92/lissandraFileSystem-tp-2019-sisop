/*
 * conexion.h
 *
 *  Created on: 11 abr. 2019
 *      Author: luqui
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include <commons/string.h>

#ifndef CONEXION_H_
#define CONEXION_H_

#define IP "127.0.0.1"
#define PUERTO_POOL_MEM 8081
#define PUERTO_FS 8082
#define CANTIDAD_CONEXIONES 100

void inicial_servidor();
int iniciar_socket();
void cargar_valores_address(struct sockaddr_in *fileSystemAddres);
void evitar_bloqueo_puerto(int socket);
void realizar_bind(int socket, struct sockaddr_in *fileSystemAddres);
void ponerse_a_escuchar(int socket, int cantidadConexiones);

#endif /* CONEXION_H_ */
