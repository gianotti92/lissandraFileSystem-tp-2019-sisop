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
#include "../../utilguenguencha/src/kernel_utils.h"
#include "../../utilguenguencha/src/parser.h"
#include "../../utilguenguencha/src/utils.h"


#define key1 "0"
#define key2 "1"
#define key3 "2"
#define key4 "3"
#define key5 "4"

t_log * LOGGER_METRICS;

typedef struct{
	Instruccion* instruccion;
	Instruccion* instruccionAProcesar;
	int quantumProcesado;
	int numeroInstruccion;
	int segundosQueTardo;
	bool esProcesoRun;
}Proceso;

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
Memoria *getMemoria(t_list *lista_memorias, int idMemoria);
char* getTablasSafe(t_dictionary * dic, char*key);
void ejecutar();
void iniciarEstructurasAsociadas();
void asignarConsistenciaAMemoria(Memoria * memoria, Consistencias consistencia);
Instruccion * dameSiguiente(char * path, int numeroInstruccion);
void lanzar_gossiping();
void newToReady();
void logicaCreate(Create * create);
Proceso * logicaRun(Run * run, Proceso * proceso);
void logicaDescribe(Describe * describe);
void logicaJournal(Journal * journal);
void logicaDrop(Drop * drop);
void logicaSelect(Select * select);
void logicaAdd(Add * add);
void logicaInsert(Insert * insert);
bool esFinLectura(Proceso * p, char * instruccionALeer);
bool esFinQuantum(Proceso * p, char * instruccionALeer);
void calculoMetrics();
void inicializarValoresMetrics();
void graficar(int contadorInsert, int contadorSelect, int contadorSelectInsert, int operacionesTotales, int tiempoPromedioInsert, int tiempoPromedioSelect);
void *TH_confMonitor(void * p);
Consistencias obtenerConsistencia(char * nombreTabla);
int generarHash(char * nombreTabla, int tamLista, int key);
void mostrarId(Memoria * memoria);
void enviar_journal(Memoria *memoria);
void eliminar_memoria(Memoria * memoria);
int existe_memoria_en(Memoria *mem1, t_list* lista);
void agregarMemoria(Memoria * m);
void agregarSiNoExiste(t_list * list, Memoria *m);

// Variables del proceso
t_list *estadoReady;
t_list *estadoNew;
t_list *estadoExit;

// tablas del proceso
t_list * memorias;

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
