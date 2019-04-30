#ifndef KERNEL_H_
#define KERNEL_H_

#include <utilguenguencha/comunicacion.h>
#include <utilguenguencha/conexion.h>
#include <utilguenguencha/parser.h>
#include <utilguenguencha/utils.h>



typedef struct{
	t_list * instrucciones;
}Proceso;


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
void planificar();
void pasarProcesoAReady();


// Variables del proceso
t_list *estadoReady;
t_list *estadoNew;
t_list *estadoExit;
t_list *estadoExec;
int PUERTO_DE_ESCUCHA;
char * IP_MEMORIA_PPAL;
int PUERTO_MEMORIA_PPAL;
int QUANTUM;
int MULTIPROCESAMIENTO;
uint32_t REFRESH_METADATA;
uint32_t RETARDO;
int TAMANO_MAXIMO_LECTURA_ARCHIVO;
int HILOS_KERNEL;

#endif /* KERNEL_H_ */
