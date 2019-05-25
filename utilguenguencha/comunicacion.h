#ifndef UTILGUENGUENCHA_COMUNICACION_H_
#define UTILGUENGUENCHA_COMUNICACION_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <error.h>
#include "utils.h"

#define BACKLOG 10     // Cuántas conexiones pendientes se mantienen en cola

//nuestros tipos de datos
#define t_key uint16_t
#define t_timestamp uint32_t
#define t_flag bool

typedef enum {
	EC, SC, SHC
} Consistencias;

typedef enum {
	SELECT, INSERT, CREATE, DESCRIBE, DROP, ADD, RUN, JOURNAL, METRICS, ERROR, GOSSIP
} Instruction_set;

typedef enum {
	BAD_KEY, MISSING_TABLE, UNKNOWN, BAD_REQUEST, MISSING_FILE
} Error_set;

typedef enum {
	KERNEL, FILESYSTEM, POOLMEMORY
} Procesos;


typedef struct{
	char* nombre_tabla;
	t_key key;
	t_timestamp timestamp;
} Select;

typedef struct {
	char* nombre_tabla;
	t_key key;
	char* value;
	t_timestamp timestamp_insert;
	t_timestamp timestamp;
} Insert;

typedef struct {
	char* nombre_tabla;
	Consistencias consistencia;
	uint8_t particiones;
	t_timestamp compactation_time;
	t_timestamp timestamp;
} Create;

typedef struct {
	char* nombre_tabla;
	t timestamp;
} Describe;

typedef struct {
	char* nombre_tabla;
	t_timestamp timestamp;
} Drop;

typedef struct {
	uint8_t memoria;
	Consistencias consistencia;
	t_timestamp timestamp;
} Add;

typedef struct {
	char* path;
	t_timestamp timestamp;
} Run;

typedef struct {
	t_timestamp timestamp;
} Journal;

typedef struct {
	t_timestamp timestamp;
} Metrics;

typedef struct {
	Instruction_set instruccion;
	void * instruccion_a_realizar;
} Instruccion;

typedef struct{
	Error_set error;
} Error;

typedef struct {
	size_t size;
	void* stream;
} t_buffer;

typedef struct {
	Procesos source;
	Instruction_set header;
	t_buffer* buffer;
} t_paquete;

typedef struct {
	char* ip;
	char* puerto;
	int idMemoria;
} Memoria;

typedef struct {
	t_list *lista_memorias;
} Gossip;

void servidor_comunicacion(void (*funcion_retorno)(Instruccion*, int),
		char* puerto_servidor);
int iniciar_servidor(char* puerto);
int crear_conexion(char* ip, char* puerto);
t_paquete* crear_paquete(Procesos proceso_del_que_envio,
		Instruccion* instruccion);
int enviar_instruccion(char* ip, char* puerto, Instruccion *instruccion,
		Procesos proceso_del_que_envio);
bool enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
void empaquetar_select(t_paquete *paquete, Select *select);
void empaquetar_insert(t_paquete *paquete, Insert *insert);
void empaquetar_create(t_paquete * paquete, Create *create);
void empaquetar_describe(t_paquete * paquete, Describe *describe);
void empaquetar_drop(t_paquete * paquete, Drop * drop);
void empaquetar_journal(t_paquete * paquete, Journal * journal);
void empaquetar_gossip(t_paquete * paquete, Gossip * gossip);
void* serializar_paquete(t_paquete* paquete, int bytes);
bool recibir_buffer(int aux1, Instruction_set inst_op, Instruccion *instruccion);
Select *desempaquetar_select(void* stream);
Insert *desempaquetar_insert(void* stream);
Create *desempaquetar_create(void* stream);
Describe *desempaquetar_describe(void* stream);
Drop *desempaquetar_drop(void* stream);
Journal *desempaquetar_journal(void* stream);
Gossip *desempaquetar_gossip(void* stream);


#endif /* UTILGUENGUENCHA_COMUNICACION_H_ */
