#ifndef CONFIG_KERNEL_H_
#define CONFIG_KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/readline.h>
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
#include <time.h>

t_log * LOGGER;
int PUERTO_ESCUCHA_CONEXION;

//Cargo los parametros desde el archivo config y los libero conforme deje de usarlos
void get_parametros_config();
void configure_logger();
void leer_consola();
void parser_lql(char *);
int cantidad_elementos(char **); //dado un array devuelve la cantidad de elementos que contiene.
int es_numero(char*); //dado un string devuelve 1 si es un numero, 0 en caso contrario.

#endif
