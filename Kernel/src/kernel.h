#ifndef KERNEL_H_
#define KERNEL_H_

#include <utilguenguencha/comunicacion.h>
#include <utilguenguencha/parser.h>
#include <utilguenguencha/utils.h>
#include <semaphore.h>
#include <commons/collections/dictionary.h>


typedef struct{
	char * ip;
	char * puerto;
	int idMemoria;
}Memoria;

typedef struct{
	Instruccion* instruccionActual;
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
void liberarProceso(Proceso * proceso);
void encolar(t_list * cola, Proceso * proceso);
void planificar();
void pasarProceso(int posicion, t_list *from, t_list *to);
void cambiarEstado(Proceso* p, t_list * estado);
void iniciarEstructurasAsociadas();
void asignarConsistenciaAMemoria(uint32_t id, Consistencias leido);
void llenarTablasPorConsistencia(char * nombreTable, char * consistencia);
Instruccion * dameSiguiente(char * path, int numeroInstruccion);
void preguntarPorMemoriasDisponibles();



// Variables del proceso
t_list *estadoReady;
t_list *estadoNew;
t_list *estadoExit;
t_list *estadoExec;

// tablas del proceso
t_list * memoriasDisponibles;
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
