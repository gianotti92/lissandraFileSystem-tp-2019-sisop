#ifndef SERVIDORH
#define SERVIDORH

#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/fcntl.h>
#include <pthread.h>
#include "utils.h"
#include "comunicacion.h"


#define BACKLOG 10     // Cuántas conexiones pendientes se mantienen en cola


t_queue * listaConexiones;
t_queue * listaConexionesRetorno;

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


/*funciones que DEBERIAN utilizar los procesos*/
void conectar_y_crear_hilo(void (*f) (Instruction_set, int),char* ip, int port);
void enviar(Instruction_set instruccion, char* ip, int puerto);
char* recibir(char* ip, int puerto);

/*funciones hilo handler*/
void atender_cliente(void (*f) (Instruction_set, int));


/*funciones abstraccion comportamiento*/
int iniciar_socket();
void cargar_valores_address(struct sockaddr_in *fileSystemAddres,char*ip, int port);
void evitar_bloqueo_puerto(int socket);
void realizar_bind(int socket, struct sockaddr_in *fileSystemAddres);
void ponerse_a_escuchar(int socket, int cantidadConexiones);
void retornarControl(Instruction_set instruccion, int socket_cliente);
void realizar_conexion(int socketServer, struct sockaddr_in * address);

#endif
