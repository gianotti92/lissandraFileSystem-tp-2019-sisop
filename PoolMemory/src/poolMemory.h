#ifndef POOLMEMORY_H_
#define POOLMEMORY_H_

#include "../../utilguenguencha/src/comunicacion.h"
#include "../../utilguenguencha/src/parser.h"
#include "../../utilguenguencha/src/utils.h"


// Variables globales del proceso

char* PUERTO_DE_ESCUCHA;
char* IP_FS;
char* PUERTO_FS;
char** IP_SEEDS;
char** PUERTOS_SEEDS;
uint32_t RETARDO_MEM;
uint32_t RETARDO_FS;

int SIZE_MEM;
t_timestamp RETARDO_MEM;
t_timestamp RETARDO_FS;
t_timestamp RETARDO_JOURNAL;
t_timestamp RETARDO_GOSSIPING;
int NUMERO_MEMORIA;

int MAX_VAL;

void* MEMORIA_PRINCIPAL; //puntero a malloc gigante
int PAGINAS_MODIFICADAS; //contador de paginas en uso, para simplificar el memory_full
t_list* L_MARCOS;		 //lista de "marcos" de la memoria
t_list* L_SEGMENTOS;	 //lista de segmentos, cada segmento tiene su lista de paginas
t_list* L_MEMORIAS;
t_list* L_SEEDS;


pthread_mutex_t mutexMarcos, mutexSegmentos, mutexMemorias;
sem_t semJournal;

pthread_mutex_t mutexListaMemorias;

//  estructuras
typedef struct{
	char* nombre;
	t_list* paginas;
}Segmento;

typedef struct{
	void* pagina;
	t_flag en_uso;
}Marco;


// Funciones del proceso
void configuracion_inicial(void);
void retorno_consola(char* leido);
void inicializar_memoria();
Instruccion* atender_consulta (Instruccion*);
int insertar_en_memoria(char*, t_key, char*, t_timestamp, t_flag);
void agregar_pagina_en_segmento(Segmento*, void*);
void* buscar_segmento(char*);
int index_segmento(char*);
void* buscar_pagina_en_segmento(Segmento*, t_key);
Marco* buscar_marco(void*);
bool coincide_segmento (char*, Segmento*);
bool coincide_pagina (t_key, void*);
void eliminar_de_memoria(char*);  //elimina una tabla de memoria
t_timestamp* get_timestamp_pagina( void*);
t_key* get_key_pagina( void*);
char* get_value_pagina( void*);
t_flag* get_modificado_pagina( void*);
void set_timestamp_pagina( void*, t_timestamp);
void set_key_pagina( void*, t_key);
void set_value_pagina( void*, char*);
void set_modificado_pagina( void*, t_flag);
void print_lista_paginas();
void print_pagina(void*);
void lanzar_gossiping();
int lanzar_journal(t_timestamp);
void print_memorias ();
void* pedir_pagina();
void* seleccionar_pagina ();
Segmento* crear_segmento(char*);
bool pagina_en_uso(Marco*);
bool memoria_full();
Instruccion* crear_error(Error_set);
void lanzar_gossiping();
void gossipear(Memoria *mem);
void add_memory_if_not_exists(Memoria *mem);
bool existe_memoria(Memoria *mem1);
void *TH_confMonitor(void * p);

#endif
