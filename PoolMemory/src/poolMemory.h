#ifndef POOLMEMORY_H_
#define POOLMEMORY_H_

#include <utilguenguencha/comunicacion.h>
#include <utilguenguencha/conexion.h>
#include <utilguenguencha/parser.h>
#include <utilguenguencha/utils.h>



// Funciones del proceso
void configuracion_inicial(void);
void retorno_consola(char* leido);
void retornarControl(Instruction_set instruccion, int socket_cliente);
void inicializar_memoria();
uint32_t* get_timestamp_pagina( void*);
uint16_t* get_key_pagina( void*);
char* get_value_pagina( void*);
unsigned char* get_modificado_pagina( void*);
void print_lista_paginas();



// Variables globales del proceso
int MAX_VALUE;
int PUERTO_DE_ESCUCHA;
char* IP_FS;
int PUERTO_FS;
char** IP_SEEDS;
int** PUERTOS_SEEDS;
uint32_t RETARDO_MEM;
uint32_t RETARDO_FS;
int SIZE_MEM;
uint32_t TIEMPO_JOURNAL;
uint32_t TIEMPO_GOSSIPING;
int NUMERO_MEMORIA;
void* memoria_principal;
t_list* l_maestro_paginas;
t_list* l_segmentos;


//  estructuras
typedef struct{
	char* tabla;
	t_list* paginas;
}Segmento;

typedef struct{
	void* pagina;
	t_flag en_uso;
}Pagina_general;


#endif
