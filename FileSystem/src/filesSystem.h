
#ifndef FILESSYSTEM_H_
#define FILESSYSTEM_H_
#define PATH_TABLE="TABLE"

#include <utilguenguencha/comunicacion.h>
#include <utilguenguencha/conexion.h>
#include <utilguenguencha/parser.h>
#include <utilguenguencha/utils.h>


// Funciones del proceso
void configuracion_inicial(void);

// Variables del proceso
int PUERTO_DE_ESCUCHA;
char* PUNTO_MONTAJE;
int TAMANIO_VALUE;
uint32_t TIEMPO_DUMP;
uint32_t RETARDO;

#endif /* FILESSYSTEM_H_ */

int BLOCK_SIZE;
int BLOCKS;
char * MAGIC_NUMBER;
