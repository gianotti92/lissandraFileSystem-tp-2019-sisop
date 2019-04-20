#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
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
#include "utils.h"

char** parser_lql(char*, t_log*); //recibe una consulta en string y un logger, devuelve la consulta parseada y utiliza el loguer.
int cantidad_elementos(char**); //dado un array devuelve la cantidad de elementos que contiene.
int es_numero(char*); //dado un string devuelve 1 si es un numero, 0 en caso contrario.
void print_consulta_parseada(char**);
bool es_select(char**);
bool es_insert(char**);
bool es_create(char**);
bool es_describe(char**);
bool es_drop(char**);
bool es_addmemory(char**);
bool es_run(char**);
bool es_metrics(char**);
bool es_journal(char**);
bool es_error(char**);
unsigned long int string_to_ulint(char*);
unsigned short int get_key(char**);
unsigned long int get_timestamp(char**);
void* leer_por_consola(void (*f) (char*));
