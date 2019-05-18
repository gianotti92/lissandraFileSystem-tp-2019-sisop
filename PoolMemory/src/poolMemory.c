#include "poolMemory.h"

int main(void) {
	configure_logger();
	configuracion_inicial();


	MAX_VALUE = 10; // esto hay que reemplazarlo por el valor del FS
	inicializar_memoria();

	/* para testear
	t_list* lista_paginas = list_create();

	Pagina_general* pagina = list_get(l_maestro_paginas,1);
	pagina = pagina->pagina;
	set_key_pagina(pagina, 153);
	list_add(lista_paginas, pagina);

	Pagina_general* pagina2 = list_get(l_maestro_paginas,3);
	pagina2 = pagina2->pagina;
	set_key_pagina(pagina2, 298);
	list_add(lista_paginas, pagina2);

	Segmento* segmento;
	segmento->paginas = lista_paginas;
	segmento->nombre = "tb_papita";
	list_add(l_segmentos, segmento);

	void* pagina_r = buscar_pagina("tb_papita", 153);

	print_pagina(pagina_r);
	*/

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


void* buscar_pagina( char* nombre_segmento, t_key key){

	//busco el segmento
	//void *list_find(l_segmentos, bool(*closure)(void*)); --necesito aplicacion parcial para la condicion y no puedo

	Segmento* segmento_encontrado = buscar_segmento(nombre_segmento);

	if (segmento_encontrado == NULL){
		return NULL;
	}

	void* pagina = buscar_pagina_en_segmento(segmento_encontrado, key);

	if (pagina == NULL){
		return NULL;
	}

	return pagina;

}

void* buscar_segmento(char* nombre_segmento){

	int posicion = 0;
	Segmento* segmento = list_get(l_segmentos, posicion);

	while (!coincide_segmento(nombre_segmento, segmento) && (posicion < l_segmentos->elements_count)){
		segmento = list_get(l_segmentos, posicion);
		posicion ++;
	}

	if (coincide_segmento(nombre_segmento, segmento)) {
		return segmento;
	}

	return NULL;
}

void* buscar_pagina_en_segmento(Segmento* segmento, t_key key){

	t_list* l_paginas = segmento->paginas;

	int posicion = 0;
	void* pagina = list_get(l_paginas, posicion);

	while (!coincide_pagina(key, pagina) && ((posicion + 1) < l_paginas->elements_count)){
		posicion ++;
		pagina = list_get(l_paginas, posicion);
	}

	if (coincide_pagina(key, pagina)) {
		return pagina;
	}

	return NULL;

}

bool coincide_segmento (char* nombre_segmento, Segmento* segmento){
	int resultado = strcmp(nombre_segmento, segmento->nombre);
	return resultado == 0;

}

bool coincide_pagina (t_key key, void* pagina){
	t_key key_en_pagina = *get_key_pagina(pagina);
	return key == key_en_pagina;
}


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

t_flag* get_modificado_pagina( void* p_pagina){
	void* p_modif = p_pagina;
	p_modif += (sizeof(t_key) + sizeof(t_timestamp) + MAX_VALUE);
	return (t_flag*) p_modif;
}


void set_key_pagina( void* p_pagina, t_key key){
	void* p_key =  p_pagina;
	*(t_key*) p_key = key;
}

void set_timestamp_pagina( void* p_pagina, t_timestamp timestamp){
	void* p_timestamp =  p_pagina;
	p_timestamp += sizeof(t_key);
	*(t_timestamp*) p_timestamp = timestamp;
}

void set_value_pagina( void* p_pagina, char* value){
	void* p_value =  p_pagina;
	p_value += (sizeof(t_key) + sizeof(t_timestamp));
	memcpy(p_value, value, MAX_VALUE);
}

void set_modificado_pagina( void* p_pagina, t_flag estado){
	void* p_modif = p_pagina;
	p_modif += (sizeof(t_key) + sizeof(t_timestamp) + MAX_VALUE);
	*(t_flag*) p_modif = estado;
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

void print_pagina(void* pagina){

		t_timestamp timestamp = *get_timestamp_pagina(pagina);
		t_key key = *get_key_pagina(pagina);
		char* value = get_value_pagina(pagina);
		t_flag modificado = *get_modificado_pagina(pagina);

		printf("Pagina: %i Key: %i Timestamp: %lu Valor: %s  \n", pagina, key, timestamp, value);

}

