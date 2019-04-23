#ifndef KERNEL_H_
#define KERNEL_H_

#include <utilguenguencha/comunicacion.h>
#include <utilguenguencha/conexion.h>
#include <utilguenguencha/parser.h>
#include <utilguenguencha/utils.h>

// Tipos del proceso
typedef enum categoriaDeMensaje{
	ERROR_MESSAGE,
	RUN_MESSAGE,
	QUERY
} CategoriaDeMensaje;

// Funciones del proceso
void configuracion_inicial(void);
void retorno_consola(char* leido);
void retornarControl(Instruction_set instruccion, int socket_cliente);
void iniciarEstados();

CategoriaDeMensaje categoria(char ** mensaje);
void moverAEstado(CategoriaDeMensaje categoria, char ** mensaje);
void leerArchivo(char * path);


// Variables del proceso
t_dictionary *estadoReady;
t_dictionary *estadoNew;
t_dictionary *estadoExit;
t_dictionary *estadoExec;
int PUERTO_DE_ESCUCHA;
char * IP_MEMORIA_PPAL;
int PUERTO_MEMORIA_PPAL;
int QUANTUM;
int MULTIPROCESAMIENTO;
uint32_t REFRESH_METADATA;
uint32_t RETARDO;

#endif /* KERNEL_H_ */
