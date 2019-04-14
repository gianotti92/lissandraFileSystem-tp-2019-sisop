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

t_dictionary* parser_lql(char *, t_log*); //recibe una consulta en string y un logger, devuelve la consulta parseada y utiliza el loguer.
int cantidad_elementos(char **); //dado un array devuelve la cantidad de elementos que contiene.
int es_numero(char*); //dado un string devuelve 1 si es un numero, 0 en caso contrario.
void print_dictionary(t_dictionary*);
int es_select(t_dictionary*);
bool es_insert(t_dictionary*);
bool es_create(t_dictionary*);
bool es_describe(t_dictionary*);
bool es_drop(t_dictionary*);
bool es_addmemory(t_dictionary*);
bool es_run(t_dictionary*);
bool es_metrics(t_dictionary*);
bool es_journal(t_dictionary*);
unsigned long int string_to_ulint(char*);
