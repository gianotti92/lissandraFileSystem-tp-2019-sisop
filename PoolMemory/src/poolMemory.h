#ifndef POOLMEMORY_H_
#define POOLMEMORY_H_

#include "../../utilguenguencha/comunicacion.h"
#include "../../utilguenguencha/parser.h"
#include "../../utilguenguencha/utils.h"


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
void retornarControl(Instruccion *instruccion, int cliente);
void inicializar_memoria();
void atender_consulta (Instruccion*);
void* buscar_pagina( char*, t_key);
void* buscar_pagina_en_segmento(Segmento*, t_key);
void* buscar_segmento(char*);
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
void crear_segmento(char*, void*);

#endif
