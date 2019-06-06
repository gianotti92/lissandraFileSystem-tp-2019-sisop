
#ifndef UTILGUENGUENCHA_UTILS_H_
#define UTILGUENGUENCHA_UTILS_H_

#include <commons/collections/dictionary.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <commons/log.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

t_log * LOGGER;
/*
typedef struct{
	Instruccion* instruccion;
	Instruccion* instruccionActual;
	int file_descriptor;
	int quantumProcesado;
	int numeroInstruccion;
	int instruccionesTotales;
}Proceso;
*/
void configure_logger();
void exit_gracefully(int);
//Proceso *dame_siguiente(Proceso* proceso);


#endif /* UTILGUENGUENCHA_UTILS_H_ */
