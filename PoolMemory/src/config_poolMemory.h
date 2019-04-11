#ifndef CONFIG_POOLMEMORY_H_
#define CONFIG_POOLMEMORY_H_

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
#include <math.h>
#include <commons/collections/queue.h>

#define POL_MEM_PORT 8081
#define KERNEL_PORT 8080
#define IP "127.0.0.1"

/* id = 1 responder */
/* id = 2 recibir */


typedef struct{
    int id;
    int value;
}  __attribute__((packed)) TypeOfCommunication;

typedef struct{
    int socketServidor;
    struct sockaddr_in cliente;
    unsigned int tamanoDireccion;
}  __attribute__((packed)) DatosCliente;

t_log * LOGGER;
char* IP_CONFIG_KERNEL;

int PUERTO_CONFIG_KERNEL;

char* IP_CONFIG_POOLMEMORY;

int PUERTO_CONFIG_POOLMEMORY;

//Cargo los parametros desde el archivo config y los libero conforme deje de usarlos
void get_parametros_config();
void configure_logger();
char* IP_CONFIG_MIO;
void exit_gracefully(int exit_code);

#endif
