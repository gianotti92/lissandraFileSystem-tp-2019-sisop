#ifndef CLIENTEH
#define CLIENTEH

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
#include "config_poolMemory.h"

#define MAXDATASIZE 100 // máximo número de bytes que se pueden leer de una vez

int conectar_servidor(int puerto , char* ip, char* nombre);

void saludo_inicial_servidor(int fd,char* nombre);


typedef struct Respuesta_para_kernel{
	int id_tipo_respuesta;
	int id_esi;
	char mensaje[100];
	char clave[40];
} __attribute__((packed)) t_respuesta_para_kernel;

#endif
