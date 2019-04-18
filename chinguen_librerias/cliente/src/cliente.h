/*
 * cliente.h
 *
 *  Created on: 18 abr. 2019
 *      Author: utnso
 */

#ifndef CLIENTE_H_
#define CLIENTE_H_


#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

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
	METRICS
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
	Select select;
	Insert insert;
	Create create;
	Describe describe;
	Drop drop;
	Add add;
	Run run;
	Journal journal;
	Metrics metrics;
}Instruccion;

#endif /* CLIENTE_H_ */
