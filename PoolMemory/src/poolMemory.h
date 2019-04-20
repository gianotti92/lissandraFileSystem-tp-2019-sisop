#ifndef POOLMEMORY_H_
#define POOLMEMORY_H_

#include <utilguenguencha/comunicacion.h>
#include <utilguenguencha/conexion.h>
#include <utilguenguencha/parser.h>
#include <utilguenguencha/utils.h>

// Funciones del proceso
void configuracion_inicial(void);

// Variables globales del proceso
int PUERTO_DE_ESCUCHA;
char* IP_FS;
int PUERTO_FS;
char** IP_SEEDS;
int** PUERTOS_SEEDS;
uint32_t RETARDO_MEM;
uint32_t RETARDO_FS;
int SIZE_MEM;
uint32_t TIEMPO_JOURNAL;
uint32_t TIEMPO_GOSSIPING;
int NUMERO_MEMORIA;

#endif
