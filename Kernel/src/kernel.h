#ifndef KERNEL_H_
#define KERNEL_H_


#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <semaphore.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include "../../utilguenguencha/src/tipos_guenguencha.h"
#include "../../utilguenguencha/src/comunicacion.h"
#include "../../utilguenguencha/src/parser.h"
#include "../../utilguenguencha/src/utils.h"
#include "../../utilguenguencha/src/kernel_utils.h"


int READS, WRITES;
double WRITE_LAT, READ_LAT;
t_list* MEM_LOAD;

t_log * LOGGER_METRICS;

typedef struct{
	Instruccion* instruccion;
	Instruccion* instruccionAProcesar;
	int quantumProcesado;
	int numeroInstruccion;
	int segundosQueTardo;
	bool esProcesoRun;
}Proceso;

typedef struct {
	int id_memoria;
	Instruction_set instruccion;
	t_timestamp tiempo;
}AcumMetrics;

typedef struct {
	int id_memoria;
	int cantidad_ins_sel;
	int cantidad_instrucciones;
}AcumuladorMemoria;

pthread_mutex_t mutexRecursosCompartidos;
sem_t semaforoSePuedePlanificar, semaforoNewToReady;

// Funciones del proceso
void configuracion_inicial(void);
void actualizar_configuracion(t_config* conf);
void retorno_consola(char* leido);
void iniciarEstados();
void leerArchivo(char * path);
void encolar(t_list * cola, Proceso * proceso);
Proceso* desencolar(t_list * cola);
Memoria * desencolarMemoria(t_list * lista, int posicion);
void putTablaSafe(t_dictionary * dic, char* key, char * value);
Memoria *get_memoria(int idMemoria, Consistencias consistencia);
char* getTablasSafe(t_dictionary * dic, char*key);
void ejecutar();
void iniciarEstructurasAsociadas();
Instruccion * dameSiguiente(char * path, int numeroInstruccion);
void lanzar_gossiping();
void newToReady();
void logicaCreate(Instruccion * instruccion);
Proceso * logicaRun(Proceso * proceso);
void logicaDescribe(Instruccion * instruccion);
void logicaJournal(Instruccion * instruccion);
void logicaDrop(Instruccion * instruccion);
void logicaSelect(Instruccion * instruccion);
void logicaAdd(Instruccion * instruccion);
void logicaInsert(Instruccion * instruccion);
void logicaMetrics(Instruccion * instruccion);
bool esFinLectura(Proceso * p, char * instruccionALeer);
bool esFinQuantum(Proceso * p, char * instruccionALeer);
void calculoMetrics();
void inicializarValoresMetrics();
void loguear_metrics();
void print_metrics();
void *TH_confMonitor(void * p);
Consistencias obtenerConsistencia(char * nombreTabla);
int generarHash(char * nombreTabla, int tamLista, int key);
void mostrarId(Memoria * memoria);
void enviar_journal(Memoria *memoria);
bool existe_memoria_en(Memoria *mem1, t_list* lista);
void agregarSiNoExiste(t_list * list, Memoria *m);
t_list *dame_lista_de_consistencia(Consistencias consistencia);
pthread_mutex_t dame_mutex_de_consistencia(Consistencias consistencia);
void asignar_memoria_a_consistencia(Memoria * memoria, Consistencias consistencia);
AcumuladorMemoria* dameAcumulador(int id_memoria, t_list* lista_acumuladores);


// Variables del proceso
t_list *estadoReady;
t_list *estadoNew;
t_list *estadoExit;

// tablas del proceso
t_list *acum30sMetrics;
t_dictionary * metrics;

char* PUERTO_DE_ESCUCHA;

char * IP_MEMORIA_PPAL;
char* PUERTO_MEMORIA_PPAL;
int QUANTUM;
int MULTIPROCESAMIENTO;
uint32_t REFRESH_METADATA;
uint32_t RETARDO;
int TAMANO_MAXIMO_LECTURA_ARCHIVO;
int HILOS_KERNEL;
int SEGUNDOS_METRICS;
int PREGUNTAR_POR_MEMORIAS;
int TIEMPO_DESCRIBE;

t_list *lista_ec;
t_list *lista_sc;
t_list *lista_shc;
t_list *lista_disp;

pthread_mutex_t mutex_disp;
pthread_mutex_t mutex_sc;
pthread_mutex_t mutex_ec;
pthread_mutex_t mutex_shc;

pthread_mutex_t mutex_metrics;

/*
	Describes
*/
typedef struct {
	char * tablename;
	Consistencias consistencia;
}Table_Metadata;



t_list * lista_de_tablas;
pthread_mutex_t lista_de_tablas_mx;
void realizarDescribeGeneral(void);

#endif /* KERNEL_H_ */
