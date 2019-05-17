#include "poolMemory.h"

int main(void) {
	configure_logger();
	configuracion_inicial();


	MAX_VALUE = 10; // esto hay que reemplazarlo por el valor del FS
	inicializar_memoria();
	print_lista_paginas();

	pthread_t consolaKernel;
	pthread_create(&consolaKernel, NULL, (void*) leer_por_consola, retorno_consola);
	conectar_y_crear_hilo(retornarControl,"127.0.0.1", PUERTO_DE_ESCUCHA);

	list_destroy(l_maestro_paginas);
	free(memoria_principal);

}

void configuracion_inicial(void){
	t_config* CONFIG;
	CONFIG = config_create("config.cfg");
	if (!CONFIG) {
		printf("No encuentro el archivo config\n");
		exit_gracefully(EXIT_FAILURE);
	}
	PUERTO_DE_ESCUCHA = config_get_int_value(CONFIG,"PUERTO_DE_ESCUCHA");
	IP_FS = config_get_string_value(CONFIG,"IP_FS");
	PUERTO_FS = config_get_int_value(CONFIG,"PUERTO_FS");
	SIZE_MEM = config_get_int_value(CONFIG,"SIZE_MEM");
	config_destroy(CONFIG);
}

void retorno_consola(char* leido){
	printf("Lo leido es: %s \n",leido);

	/*
	 * METO LO QUE LLEGA POR CONSOLA EN LA PLANIFICACION DE "NEW"
	 *
	 * */

	Instruccion* instruccion_parseada = parser_lql(leido, POOLMEMORY);

	print_instruccion_parseada(instruccion_parseada);

	free_consulta(instruccion_parseada);

}

void retornarControl(Instruction_set instruccion, int socket_cliente){
	printf("ME llego algo y algo deberia hacer");
	//enviar(instruccion, IP_FS, PUERTO_FS);
}

void inicializar_memoria(){

	//resevo memoria para paginas
	memoria_principal = malloc(SIZE_MEM);

	//calculo tamanio de cada pagina KEY-TIMESTAMP-VALUE-MODIF
	int tamanio_pagina = sizeof(uint16_t) + sizeof(uint32_t) +  MAX_VALUE + sizeof(bool);

	//creo la tabla maestra de paginas
	l_maestro_paginas = list_create();

	int memoria_formateada = 0;
	void* comienzo_pagina = memoria_principal;

	int a = 0; //para pruebas




	while (memoria_formateada + tamanio_pagina < SIZE_MEM){

		Pagina_general* nueva_pagina_general = malloc(sizeof(Pagina_general));

		nueva_pagina_general->en_uso = false;
		nueva_pagina_general->pagina = comienzo_pagina;

		list_add(l_maestro_paginas, nueva_pagina_general);

		//lugar de key
		*(uint16_t*) comienzo_pagina = a;
		comienzo_pagina += sizeof(uint16_t);

		//lugar de timestamp
		*(uint32_t*) comienzo_pagina = 0;
		comienzo_pagina += sizeof(uint32_t);

		//lugar de value
		char* palabra = "Lo paginaste todo chinguenguencha";
		memcpy(comienzo_pagina, palabra, MAX_VALUE);
		comienzo_pagina += MAX_VALUE;

		//lugar de modificado
		*(unsigned char*) comienzo_pagina = 0;
		comienzo_pagina += sizeof(unsigned char);

		memoria_formateada += tamanio_pagina;

		a++;

	}

	printf("Memoria incializada de %i bytes con %i paginas de %i bytes cada una. \n",SIZE_MEM,l_maestro_paginas->elements_count, tamanio_pagina);

}

/*
void* get_pagina( char* nombre_tabla, uint16_t key){

}*/


uint16_t* get_key_pagina( void* p_pagina){
	void* p_key =  p_pagina;
	return (uint16_t*) p_key;
}

uint32_t* get_timestamp_pagina( void* p_pagina){
	void* p_timestamp =  p_pagina;
	p_timestamp += sizeof(uint16_t);
	return (uint32_t*) p_timestamp;
}

char* get_value_pagina( void* p_pagina){
	void* p_value =  p_pagina;
	p_value += (sizeof(uint16_t) + sizeof(uint32_t));
	return (char*) p_value;
}

unsigned char* get_modificado_pagina( void* p_pagina){
	void* p_modif = p_pagina;
	p_modif += (sizeof(uint16_t) + sizeof(uint32_t) + MAX_VALUE);
	return (unsigned char*) p_modif;
}

void print_lista_paginas(){
	int nro_pagina = 0;
	int size_lista = l_maestro_paginas->elements_count;
	Pagina_general* pagina_general;
	void* pagina;

	while (nro_pagina < size_lista){
		pagina_general = list_get(l_maestro_paginas, nro_pagina);
		pagina = pagina_general->pagina;

		uint32_t timestamp = *get_timestamp_pagina(pagina);
		uint16_t key = *get_key_pagina(pagina);
		char* value = get_value_pagina(pagina);
		unsigned char modificado = *get_modificado_pagina(pagina);

		printf("Index: %i Pagina: %i Key: %i Timestamp: %lu Valor: %s  \n", nro_pagina, pagina, key, timestamp, value);

		nro_pagina++;
	}
}

