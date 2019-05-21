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
#include <sys/mman.h>
#include "utils.h"
#include "comunicacion.h"

void * p_generico; //puntero que va a ser asignado a la estructura necesaria para la consulta parseada


Instruccion* parser_lql(char*, Procesos); //recibe una consulta en string y desde donde fue llamado, devuelve la consulta parseada o error en un struct instruction.
int cantidad_elementos(char**); //dado un array devuelve la cantidad de elementos que contiene.
int es_numero(char*); //dado un string devuelve 1 si es un numero, 0 en caso contrario.
void print_instruccion_parseada(Instruccion*);
Instruccion* crear_instruccion(Instruction_set, void*, int);
Instruccion* instruccion_error();
bool es_select(char**);
bool es_insert(char**);
bool es_create(char**);
bool es_describe(char**);
bool es_drop(char**);
bool es_add(char**);
bool es_run(char**);
bool es_metrics(char**);
bool es_journal(char**);
bool es_error(char**);
unsigned long int string_to_ulint(char*);
void* leer_por_consola(void (*f) (char*));
void free_consulta(Instruccion*);
uintmax_t get_timestamp();
Instruccion dame_siguiente(char* path, uint16_t numero_instruccion);
