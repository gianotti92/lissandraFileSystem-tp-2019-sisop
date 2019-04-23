#ifndef UTILGUENGUENCHA_COMUNICACION_H_
#define UTILGUENGUENCHA_COMUNICACION_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef enum{
	KERNEL,
	POOLMEMORY,
	FILESYSTEM
}Procesos;

typedef enum{
	EC,
	SC,
	SHC
}Consistencias;

typedef enum{
	SELECT,
	INSERT,
	CREATE,
	DESCRIBE,
	DROP,
	ADD,
	RUN,
	JOURNAL,
	METRICS,
	ERROR
}Instruction_set;

typedef struct{
	char* nombre_tabla;
	uint16_t key;
	uint32_t timestamp;
}Select;

typedef struct{
	char* nombre_tabla;
	uint16_t key;
	char* value;
	uint32_t timestamp_insert;
	uint32_t timestamp;
}Insert;

typedef struct{
	char* nombre_tabla;
	Consistencias consistencia;
	uint8_t particiones;
	uint32_t compactation_time;
	uint32_t timestamp;
}Create;

typedef struct{
	char* nombre_tabla;
	uint32_t timestamp;
}Describe;

typedef struct{
	char* nombre_tabla;
	uint32_t timestamp;
}Drop;

typedef struct{
	uint8_t memoria;
	Consistencias consistencia;
	uint32_t timestamp;
}Add;

typedef struct{
	char* path;
	uint32_t timestamp;
}Run;

typedef struct{
	uint32_t timestamp;
}Journal;

typedef struct{
	uint32_t timestamp;
}Metrics;

typedef struct{
	Instruction_set instruccion;
	void * instruccion_a_realizar;
}Instruccion;



#endif /* UTILGUENGUENCHA_COMUNICACION_H_ */
