#include "poolMemory.h"

int main(void) {
	configure_logger();
	configuracion_inicial();
	MAX_VALUE = 10; // esto hay que reemplazarlo por el valor del FS
	inicializar_memoria();

	pthread_t consolaKernel, gossiping;
	pthread_create(&consolaKernel, NULL, (void*) leer_por_consola, retorno_consola);
	pthread_create(&gossiping, NULL, (void*) lanzar_gossiping, NULL);
	servidor_comunicacion(retornarControl, PUERTO_DE_ESCUCHA);

	pthread_join(consolaKernel, NULL);
	pthread_join(gossiping, NULL);

	list_destroy(l_maestro_paginas); //entender el list_destroy_and_destroy_elements()
	list_destroy(l_memorias);
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
	IP_SEEDS = config_get_string_value(CONFIG,"IP_SEEDS");
	PUERTOS_SEEDS = config_get_string_value(CONFIG,"PUERTOS_SEEDS");


}

void retorno_consola(char* leido){
	Instruccion* instruccion_parseada = parser_lql(leido, POOLMEMORY);
	atender_consulta(instruccion_parseada);// tiene que devolver el paquete con la respuesta

}

void retornarControl(Instruccion *instruccion, int cliente){

	printf("Lo que me llego desde KERNEL es:\n");
	print_instruccion_parseada(instruccion);
	printf("El fd de la consulta es %d y no esta cerrado\n", cliente);

	atender_consulta(instruccion); //tiene que devolver el paquete con la respuesta

	//liberar_conexion(cliente); // Para liberar el fd del socket
	//liberar_conexion(fd_proceso); // Para liberar el fd del socket
	free_consulta(instruccion);
}

void inicializar_memoria(){


	memoria_principal = malloc(SIZE_MEM); //resevo memoria para paginar


	if (memoria_principal == NULL){
		log_error(LOGGER, "Memoria: No se pudo malloquear la memoria principal.");
		exit_gracefully(-1);
	}

	int tamanio_pagina = sizeof(uint16_t) + sizeof(uint32_t) +  MAX_VALUE + sizeof(bool); //calculo tamanio de cada pagina KEY-TIMESTAMP-VALUE-MODIF

	l_maestro_paginas = list_create(); //creo la tabla maestra de paginas, es una t_list global

	l_segmentos = list_create(); //creo la tabla de segmentos, es una t_list global

	int memoria_formateada = 0; //inicializo un contador de memoria formateada

	void* desplazamiento = memoria_principal; //pongo al principio el desplazamiento

	while (memoria_formateada + tamanio_pagina < SIZE_MEM){

		Pagina_general* registro_maestra = malloc(sizeof(Pagina_general)); //registro para la tabla maestra de paginas

		if (registro_maestra == NULL){
			log_error(LOGGER, "Memoria: Fallo malloc para Pagina_general.");
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


		char* palabra = "\0";
		memcpy(desplazamiento, palabra, MAX_VALUE);
		desplazamiento += MAX_VALUE;				//lugar de value


		*(t_flag*) desplazamiento = 0;
		desplazamiento += sizeof(t_flag);			//lugar de modificado
		//fin formateo de memoria


		memoria_formateada += tamanio_pagina;


	}

	printf("Memoria incializada de %i bytes con %i paginas de %i bytes cada una. \n",SIZE_MEM,l_maestro_paginas->elements_count, tamanio_pagina);
	log_info(LOGGER, "Memoria incializada de %i bytes con %i paginas de %i bytes cada una.",SIZE_MEM,l_maestro_paginas->elements_count, tamanio_pagina);
}

void atender_consulta (Instruccion* instruccion_parseada){

		int fd_proceso;

		if (instruccion_parseada->instruccion == SELECT){

			Select* instruccion_select = (Select*) instruccion_parseada->instruccion_a_realizar;

			Segmento* segmento = buscar_segmento(instruccion_select->nombre_tabla);
			void* pagina = NULL;

			if (segmento != NULL){
				pagina = buscar_pagina_en_segmento(segmento, instruccion_select->key);
			}

			if(pagina == NULL){
			// no tenemos la tabla-key en memoria
				if((fd_proceso = enviar_instruccion(IP_FS, PUERTO_FS, instruccion_parseada, POOLMEMORY))){
					printf("La consulta fue enviada al fd %d de FILESYSTEM y este sigue abierto\n", fd_proceso);

					//hacer un insert con la respuesta
					//insertar_en_memoria(instruccion_select->nombre_tabla, instruccion_select->key, value, timestamp, false);
				}

			//devolver un paquete value/error
			}
			else{
			// tenemos la tabla-key en memoria
				print_pagina(pagina);
				//devolver un paquete value/error
			}
		}
		else if (instruccion_parseada->instruccion == INSERT){

			Insert* instruccion_insert = (Insert*) instruccion_parseada->instruccion_a_realizar;

			insertar_en_memoria(instruccion_insert->nombre_tabla , instruccion_insert->key, instruccion_insert->value, instruccion_insert->timestamp_insert, true);

		//devolver un ok/error
		}
		else if(instruccion_parseada->instruccion != ERROR &&
				instruccion_parseada->instruccion != METRICS &&
				instruccion_parseada->instruccion != ADD &&
				instruccion_parseada->instruccion != RUN &&
				instruccion_parseada->instruccion != JOURNAL){
			if((fd_proceso = enviar_instruccion(IP_FS, PUERTO_FS, instruccion_parseada, POOLMEMORY))){
				printf("La consulta fue enviada al fd %d de FILESYSTEM y este sigue abierto\n", fd_proceso);

			//captar respuesta y devolver paquete
			}
		}
		else {
			printf("La consulta enviada no puede ser procesada por la memoria. \n");
		}

		//liberar_conexion(fd_proceso); // Para liberar el fd del socket
		free_consulta(instruccion_parseada);
}


void insertar_en_memoria(char* nombre_tabla, t_key key, char* value, t_timestamp timestamp_insert, t_flag modificado){

	Segmento* segmento = buscar_segmento(nombre_tabla);

	if(segmento == NULL){
		//no tenemos el segmento, creo uno
		segmento = crear_segmento(nombre_tabla);
	}

	void* pagina = buscar_pagina_en_segmento(segmento, key);

	if(pagina == NULL){
		//no tenemos la pagina
		pagina = seleccionar_pagina();
		agregar_pagina_en_segmento(segmento, pagina);
	}

	set_key_pagina(pagina, key);
	set_value_pagina(pagina, value);
	set_timestamp_pagina(pagina, timestamp_insert);
	set_modificado_pagina(pagina, modificado);

	//devolver un ok
}

void agregar_pagina_en_segmento(Segmento* segmento, void* pagina){
	list_add(segmento->paginas, pagina);
}

void* buscar_segmento(char* nombre_segmento){

	int posicion = 0;
	Segmento* segmento;
	bool encontrado = false;

	while (posicion < l_segmentos->elements_count && !encontrado){

		segmento = list_get(l_segmentos, posicion);

		if(coincide_segmento(nombre_segmento, segmento)){
			encontrado = true;
		}
		posicion ++;
	}

	if (encontrado) {
		return segmento;
	}

	return NULL;
}

void* buscar_pagina_en_segmento(Segmento* segmento, t_key key){

	t_list* l_paginas = segmento->paginas;

	int posicion = 0;
	void* pagina = list_get(l_paginas, posicion);

	if (pagina != NULL){

		while (!coincide_pagina(key, pagina) && ((posicion + 1) < l_paginas->elements_count)){
			posicion ++;
			pagina = list_get(l_paginas, posicion);
		}

		if (coincide_pagina(key, pagina)) {
			return pagina;
		}
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
	int largo = string_length(value);

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


void lanzar_gossiping(){

	l_memorias = list_create();
	log_info(LOGGER, "Memoria: Se lanza el proceso de gossiping.");

	IP_SEEDS = string_substring(IP_SEEDS, 1, string_length(IP_SEEDS)-2);
	PUERTOS_SEEDS = string_substring(PUERTOS_SEEDS, 1, string_length(PUERTOS_SEEDS)-2);

	char** ips = string_split(IP_SEEDS, ",");
	char** puertos = string_split(PUERTOS_SEEDS, ",");

	int posicion = 0;

	while (ips[posicion] != NULL && puertos[posicion] != NULL){

		char* puerto = puertos[posicion];
		char* ip = ips[posicion];

		Memoria* nueva_memoria = malloc(sizeof(Memoria));
		nueva_memoria->ip = ip;
		nueva_memoria->puerto = puerto;
		nueva_memoria->idMemoria = posicion;

		list_add(l_memorias, nueva_memoria);

		posicion++;
	}
	Instruccion *instruccion = malloc(sizeof(Instruccion));
	Gossip * gossip = malloc(sizeof(Gossip));
	gossip->lista_memorias = l_memorias;
	instruccion->instruccion = GOSSIP;
	instruccion->instruccion_a_realizar = gossip;

	enviar_instruccion(IP_FS,PUERTO_FS,instruccion, POOLMEMORY);
}

void print_memorias (){

	int posicion = 0;
	int size = l_memorias->elements_count;
	Memoria* memoria;

	while (posicion < size){
		memoria = list_get(l_memorias, posicion);

		printf("ID: %i IP: %s PUERTO: %s \n", memoria->idMemoria, memoria->ip, memoria->puerto);
		posicion++;
	}

}

void* seleccionar_pagina (){
	 //aca iria el algoritmo para reemplazar paginas
	int posicion = 0;
	Pagina_general* pagina_general = list_get(l_maestro_paginas, posicion);

	while(pagina_en_uso(pagina_general)){
		posicion++;
		pagina_general = list_get(l_maestro_paginas, posicion);
	}

	pagina_general->en_uso = true;

	return pagina_general->pagina;
}

Segmento* crear_segmento(char* nombre_segmento){

	Segmento* nuevo_segmento = malloc(sizeof(Segmento));
	nuevo_segmento->nombre = nombre_segmento;
	nuevo_segmento->paginas = list_create();
	list_add(l_segmentos, nuevo_segmento);

	return nuevo_segmento;

}

bool pagina_en_uso(Pagina_general* pagina_general){
	return pagina_general->en_uso == true;
}


bool memoria_full(){

	return list_all_satisfy(l_maestro_paginas, pagina_en_uso);

}
