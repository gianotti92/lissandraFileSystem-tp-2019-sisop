#ifndef KERNEL_H_
#define KERNEL_H_

#include <utilguenguencha/comunicacion.h>
#include <utilguenguencha/parser.h>
#include <utilguenguencha/utils.h>
#include <semaphore.h>
#include <commons/collections/dictionary.h>



typedef struct{
	t_list * instrucciones;
	int file_descriptor;
	int quantumProcesado;
}Proceso;

typedef struct{
	char * ip;
	char * puerto;
	int idMemoria;
}Memoria;

char *CONSISTENCIAS_STRING[] = {
    "EC", "SC", "SHC",
};



// Funciones del proceso
void configuracion_inicial(void);
void retorno_consola(char* leido);
void retornarControl(Instruction_set instruccion, int socket_cliente);
void iniciarEstados();

void leerArchivo(char * path);
Proceso * crear_proceso();
Proceso * asignar_instrucciones(char ** leidoSplit);
void liberarProceso(Proceso * proceso);
void encolar(t_list * cola, Proceso * proceso);
void encolarNew(t_list*lista, char ** split);
void planificar();
void pasarPrimerProceso(t_list *from, t_list *to);
void cambiarEstado(Proceso* p, t_list * estado);
void ponerProcesosEneady();
void iniciarEstructurasAsociadas();
void preguntarPorMemorias();
void seleccionarMemoriaPorConsistencia(char * leido);

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
