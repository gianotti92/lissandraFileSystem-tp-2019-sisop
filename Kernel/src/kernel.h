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
	Memoria * memoriaAsignada;
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
void pasarPrimerProceso(t_list *from, t_list *to);
void cambiarEstado(Proceso* p, t_list * estado);
void ponerProcesosEneady();
void iniciarEstructurasAsociadas();
void preguntarPorMemorias();
Memoria* seleccionarMemoriaPorConsistencia(Consistencias leido);
Memoria* llenarTablasPorConsistencia(char * nombreTable, char * consistencia);
Instruccion * dameSiguiente(char * path, int numeroInstruccion);

//MOCK value
int enviarX(Instruccion * i, char*ip, int puerto);


// Variables del proceso
t_list *estadoReady;
t_list *estadoNew;
t_list *estadoExit;
t_list *estadoExec;
char* PUERTO_DE_ESCUCHA;

char * IP_MEMORIA_PPAL;
char* PUERTO_MEMORIA_PPAL;
int QUANTUM;
int MULTIPROCESAMIENTO;
uint32_t REFRESH_METADATA;
uint32_t RETARDO;
int TAMANO_MAXIMO_LECTURA_ARCHIVO;
int HILOS_KERNEL;

t_dictionary * memoriasAsociadas;
t_dictionary * tablasPorConsistencia;

#endif /* KERNEL_H_ */
