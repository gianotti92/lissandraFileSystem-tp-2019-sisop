#ifndef KERNEL_H_
#define KERNEL_H_


#include "../../utilguenguencha/src/comunicacion.h"
#include "../../utilguenguencha/src/kernel_utils.h"
#include "../../utilguenguencha/src/parser.h"
#include "../../utilguenguencha/src/utils.h"
#include <commons/collections/dictionary.h>
#include <semaphore.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>

typedef struct{
	Instruccion* instruccion;
	Instruccion* instruccionAProcesar;
	int file_descriptor;
	int quantumProcesado;
	int numeroInstruccion;
	int segundosQueTardo;
}Proceso;

char *CONSISTENCIAS_STRING[] = {
    "EC", "SC", "SHC",
};

// Funciones del proceso
void configuracion_inicial(void);
void retorno_consola(char* leido);
void iniciarEstados();
void leerArchivo(char * path);
void encolar(t_list * cola, Proceso * proceso);
Proceso* desencolar(t_list * cola);
void putMemorySafe(t_dictionary * dic, char* key, Memoria * value);
void putTablaSafe(t_dictionary * dic, char* key, char * value);
Memoria* getMemoriaSafe(t_dictionary * dic, char*key);
char* getTablasSafe(t_dictionary * dic, char*key);
void ejecutar();
void iniciarEstructurasAsociadas();
void asignarConsistenciaAMemoria(uint32_t id, Consistencias leido);
void llenarTablasPorConsistencia(char * nombreTable, char * consistencia);
Instruccion * dameSiguiente(char * path, int numeroInstruccion);
void preguntarPorMemoriasDisponibles();
void newToReady();
int logicaCreate(Create * create);
void logicaRun(Run * run, Proceso * proceso);
int logicaDrop(Drop * drop);
int logicaSelect(Select * select);
void logicaAdd(Add * add);
int logicaInsert(Insert * insert);
bool esFinLectura(Proceso * p, char * instruccionALeer);
bool esFinQuantum(Proceso * p, char * instruccionALeer);
void calculoMetrics();
void inicializarValoresMetrics();
void graficar(int contadorInsert, int contadorSelect, int contadorSelectInsert, int operacionesTotales, int tiempoPromedioInsert, int tiempoPromedioSelect);
int enviarInstruccionLuqui(char* ip, char* puerto, Instruccion *instruccion,
		Procesos proceso_del_que_envio);
// Hay que definirla ya que no tiene definición en kernel y sino rompería, de todos modos no se usa
void retornarControl(Instruccion *instruccion, int socket_cliente){};


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
