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
#include <pthread.h>
#include <commons/config.h>
#include "config_kernel.h"

#define BACKLOG 10     // Cuántas conexiones pendientes se mantienen en cola

//#define MYIP "0.0.0.0" para muchas pc (dejar comentado para poder usar localmente)
//#define MYIP "192.168.0.17"

void levantar_servidor_kernel();

void levantar_servidor_status();

void enviar_saludo(int fdCliente);

int recibir_saludo(int fdCliente);

void atender_cliente(void* idSocketCliente);




#endif
