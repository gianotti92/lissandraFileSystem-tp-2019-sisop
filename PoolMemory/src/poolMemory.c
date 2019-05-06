#include "poolMemory.h"

int main(void) {
	configure_logger();
	configuracion_inicial();

	MAX_VALUE = 10; // esto hay que reemplazarlo por el valor del FS
	inicializar_memoria();

	pthread_t consolaKernel;
	pthread_create(&consolaKernel, NULL, (void*) leer_por_consola, retorno_consola);
	conectar_y_crear_hilo(retornarControl,"127.0.0.1", PUERTO_DE_ESCUCHA);

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


	//defino estructuras para las paginas
	typedef struct{
		uint32_t timestamp;
		uint16_t key;
		char value[MAX_VALUE];
		unsigned char modificado; //es el tipo de dato que menos bytes ocupa
	}Pagina;

	typedef struct{
		Pagina* pagina;
		unsigned char en_uso;
	}Pagina_aux;

	//resevo memoria
	void* memoria_principal = malloc(SIZE_MEM);

	//formateo de memoria
	t_list* l_maestro_paginas = list_create();

	Pagina* nueva_pagina = (Pagina*) memoria_principal;
	Pagina_aux pagina_aux;
	Pagina_aux* nueva_pagina_aux = &pagina_aux;

	int memoria_formateada = 0;

	while (memoria_formateada + sizeof(Pagina) < SIZE_MEM){

		//inicializo una pagina en blanco
		nueva_pagina->key = 0;
		nueva_pagina->modificado = 0;
		nueva_pagina->timestamp = 0;
		nueva_pagina->value[0] = '\0';

		//inicializo una pagina auxiliar para la nueva pagina
		nueva_pagina_aux->en_uso = 0;
		nueva_pagina_aux->pagina = nueva_pagina;

		list_add(l_maestro_paginas, nueva_pagina_aux); //add se encarga de crear un nuevo elemento dentro de la lista, por eso puedo usar siempre la misma pagina

		nueva_pagina += sizeof(Pagina); //muevo el puntero a la pagina siguiente

		memoria_formateada += sizeof(Pagina);

	}

	printf("Memoria incializada de %i bytes con %i paginas de %i bytes cada una. \n",SIZE_MEM,l_maestro_paginas->elements_count, sizeof(Pagina));
	list_destroy(l_maestro_paginas);
	free(memoria_principal);
}

/*
Pagina* get_pagina( char* nombre_tabla, uint16_t key){

}*/
