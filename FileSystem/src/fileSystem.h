#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <utilguenguencha/comunicacion.h>
#include <utilguenguencha/parser.h>
#include <utilguenguencha/utils.h>

// Funciones del proceso
void configuracion_inicial(void);
void retorno_consola(char* leido);
void retornarControl(Instruccion * instruccion, int cliente);

// Variables del proceso
char* PUERTO_DE_ESCUCHA;
char* PUNTO_MONTAJE;
int TAMANIO_VALUE;
uint32_t TIEMPO_DUMP;
uint32_t RETARDO;

#endif /* FILESYSTEM_H_ */
