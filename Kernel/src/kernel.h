#ifndef KERNEL_H_
#define KERNEL_H_


#include "../../utilguenguencha/comunicacion.h"
#include "../../utilguenguencha/kernel_utils.h"
#include "../../utilguenguencha/parser.h"
#include "../../utilguenguencha/utils.h"
#include <commons/collections/dictionary.h>
#include <semaphore.h>
#include <stdlib.h>
#include <sys/types.h>

typedef struct{
	Instruccion* instruccion;
	Instruccion* instruccionAProcesar;
	int file_descriptor;
	int quantumProcesado;
	int numeroInstruccion;
}Proceso;



char *CONSISTENCIAS_STRING[] = {
    "EC", "SC", "SHC",
};

// Funciones del proceso
void configuracion_inicial(void);
void retorno_consola(char* leido);
void retornarControl(Instruction_set instruccion, int socket_cliente);
void iniciarEstados();

void leerArchivo(char * path);
void encolar(t_list * cola, Proceso * proceso);
Proceso* desencolar(t_list * cola);
void putMemorySafe(t_dictionary * dic, char* key, Memoria * value);
void putTablaSafe(t_dictionary * dic, char* key, char * value);
Memoria* getMemoriaSafe(t_dictionary * dic, char*key);
char* getTablasSafe(t_dictionary * dic, char*key);
void planificar();
void iniciarEstructurasAsociadas();
void asignarConsistenciaAMemoria(uint32_t id, Consistencias leido);
void llenarTablasPorConsistencia(char * nombreTable, char * consistencia);
Instruccion * dameSiguiente(char * path, int numeroInstruccion);
void preguntarPorMemoriasDisponibles();
void newToReady();
void logicaCreate(Create * create);
void logicaRun(Run * run, Proceso * proceso);
void logicaSelect(Select * select);
void logicaAdd(Add * add);
bool esFinLectura(Proceso * p, char * instruccionALeer);
bool esFinQuantum(Proceso * p, char * instruccionALeer);


// Variables del proceso
t_list *estadoReady;
t_list *estadoNew;
t_list *estadoExit;

// tablas del proceso
t_dictionary * memoriasDisponibles;
t_dictionary * memoriasAsociadas;
t_dictionary * tablasPorConsistencia;

char* PUERTO_DE_ESCUCHA;

char * IP_MEMORIA_PPAL;
char* PUERTO_MEMORIA_PPAL;
int QUANTUM;
int MULTIPROCESAMIENTO;
uint32_t REFRESH_METADATA;
uint32_t RETARDO;
int TAMANO_MAXIMO_LECTURA_ARCHIVO;
int HILOS_KERNEL;



#endif /* KERNEL_H_ */
