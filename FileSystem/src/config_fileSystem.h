#ifndef CONFIG_FILESYSTEM_H_
#define CONFIG_FILESYSTEM_H_

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

t_log * LOGGER;
int PUERTO_ESCUCHA_CONEXION;

//Cargo los parametros desde el archivo config y los libero conforme deje de usarlos
void funcion_para_testear_valgrind(void);
void get_parametros_config();
void configure_logger();
void exit_gracefully(int exit_code);

#endif
