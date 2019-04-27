#ifndef UTILGUENGUENCHA_COMUNICACION_H_
#define UTILGUENGUENCHA_COMUNICACION_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

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

size_t sizeof_select(Select *select);
size_t sizeof_insert(Insert *insert);
size_t sizeof_create(Create *create);
size_t sizeof_describe(Describe *describe);
size_t sizeof_drop(Drop *drop);
size_t sizeof_add(Add *add);
size_t sizeof_run(Run *run);
size_t sizeof_journal(Journal *journal);
size_t sizeof_metrics(Metrics *metrics);

#endif /* UTILGUENGUENCHA_COMUNICACION_H_ */
