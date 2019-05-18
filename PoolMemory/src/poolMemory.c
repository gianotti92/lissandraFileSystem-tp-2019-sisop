#include "poolMemory.h"

int main(void) {
	configure_logger();
	configuracion_inicial();


	MAX_VALUE = 10; // esto hay que reemplazarlo por el valor del FS
	inicializar_memoria();
	//print_lista_paginas();

	pthread_t consolaKernel;
	pthread_create(&consolaKernel, NULL, (void*) leer_por_consola, retorno_consola);
	servidor_comunicacion(retornarControl, PUERTO_DE_ESCUCHA);

	pthread_join(consolaKernel, NULL);

	list_destroy(l_maestro_paginas); //entender el list_destroy_and_destroy_elements()
	free(memoria_principal);

}

void configuracion_inicial(void){
	t_config* CONFIG;
	CONFIG = config_create("config.cfg");
	if (!CONFIG) {
		printf("No encuentro el archivo config\n");
		exit_gracefully(EXIT_FAILURE);
	}
	PUERTO_DE_ESCUCHA = config_get_string_value(CONFIG,"PUERTO_DE_ESCUCHA");
	IP_FS = config_get_string_value(CONFIG,"IP_FS");
	PUERTO_FS = config_get_string_value(CONFIG,"PUERTO_FS");
	SIZE_MEM = config_get_int_value(CONFIG,"SIZE_MEM");
	config_destroy(CONFIG);
}

void retorno_consola(char* leido){

	Instruccion* instruccion_parseada = parser_lql(leido, POOLMEMORY);
	int fd_proceso;
	// La memoria no usa las funciones de KERNEL y JOURNAL no lo envia a filesystem
	if(	instruccion_parseada->instruccion != ERROR &&
		instruccion_parseada->instruccion != METRICS &&
		instruccion_parseada->instruccion != ADD &&
		instruccion_parseada->instruccion != RUN &&
		instruccion_parseada->instruccion != JOURNAL){
		if((fd_proceso = enviar_instruccion(IP_FS, PUERTO_FS, instruccion_parseada, POOLMEMORY))){
			printf("La consulta fue enviada al fd %d de FILESYSTEM y este sigue abierto\n", fd_proceso);
		}
	}
	//liberar_conexion(fd_proceso); // Para liberar el fd del socket
	free_consulta(instruccion_parseada);
}

void retornarControl(Instruccion *instruccion, int cliente){

	printf("Lo que me llego desde KERNEL es:\n");
	print_instruccion_parseada(instruccion);
	printf("El fd de la consulta es %d y no esta cerrado\n", cliente);
	int fd_proceso;
	// Al retornar control de poolmemory solo le llegan las que corresponden y metrics no tiene que enviarla
	if(	instruccion->instruccion != ERROR &&
		instruccion->instruccion != JOURNAL){
		if((fd_proceso = enviar_instruccion(IP_FS, PUERTO_FS, instruccion, POOLMEMORY))){
			printf("La consulta fue enviada al fd %d de FILESYSTEM y este sigue abierto\n", fd_proceso);
		}
	}
	//liberar_conexion(cliente); // Para liberar el fd del socket
	//liberar_conexion(fd_proceso); // Para liberar el fd del socket
	free_consulta(instruccion);
}

void inicializar_memoria(){


	memoria_principal = malloc(SIZE_MEM); //resevo memoria para paginar

	int tamanio_pagina = sizeof(uint16_t) + sizeof(uint32_t) +  MAX_VALUE + sizeof(bool); //calculo tamanio de cada pagina KEY-TIMESTAMP-VALUE-MODIF

	l_maestro_paginas = list_create(); //creo la tabla maestra de paginas, es una t_list global

	l_segmentos = list_create(); //creo la tabla de segmentos, es una t_list global

	int memoria_formateada = 0; //inicializo un contador de memoria formateada

	void* desplazamiento = memoria_principal; //pongo al principio el desplazamiento

	while (memoria_formateada + tamanio_pagina < SIZE_MEM){

		Pagina_general* registro_maestra = malloc(sizeof(Pagina_general)); //registro para la tabla maestra de paginas

		if (registro_maestra == NULL){
			exit_gracefully(-1);
		}

		registro_maestra->en_uso = false;
		registro_maestra->pagina = desplazamiento;

		list_add(l_maestro_paginas, registro_maestra); //inserto el registro para una nueva pagina en la tabla maestra de paginas


		//formateo de memoria
		*(t_key*) desplazamiento = 0;
		desplazamiento += sizeof(t_key);			//lugar de key


		*(t_timestamp*) desplazamiento = 0;
		desplazamiento += sizeof(t_timestamp);		//lugar de timestamp


		char* palabra = "Lo paginaste todo chinguenguencha";
		memcpy(desplazamiento, palabra, MAX_VALUE);
		desplazamiento += MAX_VALUE;				//lugar de value


		*(t_flag*) desplazamiento = 0;
		desplazamiento += sizeof(t_flag);			//lugar de modificado
		//fin formateo de memoria


		memoria_formateada += tamanio_pagina;


	}

	printf("Memoria incializada de %i bytes con %i paginas de %i bytes cada una. \n",SIZE_MEM,l_maestro_paginas->elements_count, tamanio_pagina);

}

/*
void* buscar_pagina( char* nombre_tabla, uint16_t key){

}*/


t_key* get_key_pagina( void* p_pagina){
	void* p_key =  p_pagina;
	return (t_key*) p_key;
}

t_timestamp* get_timestamp_pagina( void* p_pagina){
	void* p_timestamp =  p_pagina;
	p_timestamp += sizeof(t_key);
	return (t_timestamp*) p_timestamp;
}

char* get_value_pagina( void* p_pagina){
	void* p_value =  p_pagina;
	p_value += (sizeof(t_key) + sizeof(t_timestamp));
	return (char*) p_value;
}

unsigned char* get_modificado_pagina( void* p_pagina){   //no me funciona el bool*
	void* p_modif = p_pagina;
	p_modif += (sizeof(t_key) + sizeof(t_timestamp) + MAX_VALUE);
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

		t_timestamp timestamp = *get_timestamp_pagina(pagina);
		t_key key = *get_key_pagina(pagina);
		char* value = get_value_pagina(pagina);
		t_flag modificado = *get_modificado_pagina(pagina);

		printf("Index: %i Pagina: %i Key: %i Timestamp: %lu Valor: %s  \n", nro_pagina, pagina, key, timestamp, value);

		nro_pagina++;
	}
}

