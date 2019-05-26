#ifndef POOLMEMORY_H_
#define POOLMEMORY_H_

#include "../../utilguenguencha/src/comunicacion.h"
#include "../../utilguenguencha/src/parser.h"
#include "../../utilguenguencha/src/utils.h"


// Variables globales del proceso
char* PUERTO_DE_ESCUCHA;
int MAX_VALUE;
char* IP_FS;
char* PUERTO_FS;
char* IP_SEEDS;
char* PUERTOS_SEEDS;
uint32_t RETARDO_MEM;
uint32_t RETARDO_FS;
int SIZE_MEM;
uint32_t TIEMPO_JOURNAL;
uint32_t TIEMPO_GOSSIPING;
int NUMERO_MEMORIA;
void* memoria_principal;
t_list* l_maestro_paginas;
t_list* l_segmentos;
t_list* l_memorias;


//  estructuras
typedef struct{
	char* nombre;
	t_list* paginas;
}Segmento;

typedef struct{
	void* pagina;
	t_flag en_uso;
}Pagina_general;


// Funciones del proceso
void configuracion_inicial(void);
void retorno_consola(char* leido);
void inicializar_memoria();
void atender_consulta (Instruccion*);
void insertar_en_memoria(char*, t_key, char*, t_timestamp, t_flag);
void agregar_pagina_en_segmento(Segmento*, void*);
void* buscar_segmento(char*);
int index_segmento(char*);
void* buscar_pagina_en_segmento(Segmento*, t_key);
Pagina_general* buscar_pagina_general(void*);
bool coincide_segmento (char*, Segmento*);
bool coincide_pagina (t_key, void*);
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
void print_memorias ();
void* pedir_pagina();
void* seleccionar_pagina ();
Segmento* crear_segmento(char*);
bool pagina_en_uso(Pagina_general*);
bool memoria_full();

#endif
