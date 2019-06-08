#include "poolMemory.h"

int main(void) {
	configure_logger();
	configuracion_inicial();
	MAX_VAL = 10; // esto hay que reemplazarlo por el valor del FS
	inicializar_memoria();

	pthread_t consolaPoolMemory, gossiping, servidorPM;
	pthread_create(&consolaPoolMemory, NULL, (void*) leer_por_consola, retorno_consola);
	pthread_create(&gossiping, NULL, (void*) lanzar_gossiping, NULL);

	Comunicacion *comunicacion_instrucciones = malloc(sizeof(Comunicacion));
	comunicacion_instrucciones->puerto_servidor = PUERTO_DE_ESCUCHA;
	comunicacion_instrucciones->proceso = POOLMEMORY;
	pthread_create(&servidorPM, NULL, (void*) servidor_comunicacion, comunicacion_instrucciones);

	pthread_join(servidorPM, NULL);
	pthread_join(consolaPoolMemory, NULL);
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
	Instruccion* respuesta = atender_consulta(instruccion_parseada);// tiene que devolver el paquete con la respuesta

	print_instruccion_parseada(respuesta);

}

void retornarControl(Instruccion *instruccion, int cliente){

	printf("Lo que me llego desde KERNEL es:\n");
	print_instruccion_parseada(instruccion);
	printf("El fd de la consulta es %d y no esta cerrado\n", cliente);

	Instruccion* respuesta = atender_consulta(instruccion); //tiene que devolver el paquete con la respuesta






	responder(cliente, respuesta);

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

	int tamanio_pagina = sizeof(uint16_t) + sizeof(uint32_t) +  MAX_VAL + sizeof(bool); //calculo tamanio de cada pagina KEY-TIMESTAMP-VALUE-MODIF

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
		memcpy(desplazamiento, palabra, MAX_VAL);
		desplazamiento += MAX_VAL;				//lugar de value


		*(t_flag*) desplazamiento = 0;
		desplazamiento += sizeof(t_flag);			//lugar de modificado
		//fin formateo de memoria


		memoria_formateada += tamanio_pagina;


	}

	printf("Memoria incializada de %i bytes con %i paginas de %i bytes cada una. \n",SIZE_MEM,l_maestro_paginas->elements_count, tamanio_pagina);
	log_info(LOGGER, "Memoria incializada de %i bytes con %i paginas de %i bytes cada una.",SIZE_MEM,l_maestro_paginas->elements_count, tamanio_pagina);
}

Instruccion* atender_consulta (Instruccion* instruccion_parseada){

		Instruccion* instruccion_respuesta = malloc(sizeof(Instruccion));

		if (instruccion_parseada->instruccion == SELECT){

			Select* instruccion_select = (Select*) instruccion_parseada->instruccion_a_realizar;

			Segmento* segmento = buscar_segmento(instruccion_select->nombre_tabla);
			void* pagina = NULL;

			if (segmento != NULL){
				pagina = buscar_pagina_en_segmento(segmento, instruccion_select->key);
			}

			if(pagina == NULL){
			// no tenemos la tabla-key en memoria
				if((instruccion_respuesta = enviar_instruccion(IP_FS, PUERTO_FS, instruccion_parseada, POOLMEMORY, T_INSTRUCCION))){

					if (instruccion_respuesta->instruccion == RETORNO){
						Retorno_Generico* resp_retorno_generico = instruccion_respuesta->instruccion_a_realizar;

						if (resp_retorno_generico->tipo_retorno == VALOR){
							Retorno_Value* resp_retorno_value = resp_retorno_generico->retorno;

							int result_insert = insertar_en_memoria(instruccion_select->nombre_tabla, instruccion_select->key, resp_retorno_value->value, resp_retorno_value->timestamp, false);

							if (result_insert < 0){
								instruccion_respuesta = respuesta_error(INSERT_FAILURE);
							}

						} else {
							instruccion_respuesta = respuesta_error(BAD_RESPONSE);
						}
					}
				}
			// en instruccion_respuesta queda la respuesta del FS que puede ser ERROR o RETORNO
			} else {
			// tenemos la tabla-key en memoria

				instruccion_respuesta->instruccion = RETORNO;
				Retorno_Generico* p_retorno_generico = malloc(sizeof(Retorno_Generico));
				Retorno_Value* p_retorno_valor = malloc(sizeof(Retorno_Value));

				p_retorno_valor->timestamp = *get_timestamp_pagina(pagina);
				p_retorno_valor->value = get_value_pagina(pagina);

				p_retorno_generico->tipo_retorno = VALOR;
				p_retorno_generico->retorno = p_retorno_valor;

				instruccion_respuesta->instruccion_a_realizar = p_retorno_generico;
			}
		}
		else if (instruccion_parseada->instruccion == INSERT){

			Insert* instruccion_insert = (Insert*) instruccion_parseada->instruccion_a_realizar;

			int result = insertar_en_memoria(instruccion_insert->nombre_tabla , instruccion_insert->key, instruccion_insert->value, instruccion_insert->timestamp_insert, true);

			if (result >= 0){
				instruccion_respuesta = respuesta_success();
			} else {
				instruccion_respuesta = respuesta_error(INSERT_FAILURE);
			}
		}
		else if (instruccion_parseada->instruccion == DROP){

			Drop* instruccion_drop = (Drop*) instruccion_parseada->instruccion_a_realizar;

			eliminar_de_memoria(instruccion_drop->nombre_tabla);

			instruccion_respuesta = enviar_instruccion(IP_FS, PUERTO_FS, instruccion_parseada, POOLMEMORY, T_INSTRUCCION);

		}
		else if (instruccion_parseada->instruccion == JOURNAL){

			Journal* instruccion_journal = (Journal*) instruccion_parseada->instruccion_a_realizar;

			int result = lanzar_journal(instruccion_journal->timestamp);

			if (result >= 0){
				instruccion_respuesta = respuesta_success();
			} else {
				instruccion_respuesta = respuesta_error(JOURNAL_FAILURE);
			}
		}
		else if(instruccion_parseada->instruccion != ERROR &&
				instruccion_parseada->instruccion != METRICS &&
				instruccion_parseada->instruccion != ADD &&
				instruccion_parseada->instruccion != RUN &&
				instruccion_parseada->instruccion != JOURNAL){
			instruccion_respuesta = enviar_instruccion(IP_FS, PUERTO_FS, instruccion_parseada, POOLMEMORY, T_INSTRUCCION);
		}
		else {
			instruccion_respuesta = respuesta_error(BAD_REQUEST);
		}

		free_consulta(instruccion_parseada);
		return instruccion_respuesta;
}


int insertar_en_memoria(char* nombre_tabla, t_key key, char* value, t_timestamp timestamp_insert, t_flag modificado){

	Segmento* segmento = buscar_segmento(nombre_tabla);

	if(segmento == NULL){
		//no tenemos el segmento, creo uno
		segmento = crear_segmento(nombre_tabla);

		if(segmento == NULL){
			return -1;
		}

	}

	void* pagina = buscar_pagina_en_segmento(segmento, key);

	if(pagina == NULL){
		//no tenemos la pagina
		pagina = seleccionar_pagina();

		if(pagina == NULL){
			return -2;
		}

		agregar_pagina_en_segmento(segmento, pagina);
	}

	set_key_pagina(pagina, key);
	set_value_pagina(pagina, value);
	set_timestamp_pagina(pagina, timestamp_insert);
	set_modificado_pagina(pagina, modificado);

	return 1;
}

void eliminar_de_memoria(char* nombre_tabla){
	Segmento* segmento = buscar_segmento(nombre_tabla);

	if (segmento != NULL){
		t_list* paginas = segmento->paginas;
		int posicion = (paginas->elements_count -1);
		Pagina_general* pagina_general_liberar;
		void* pagina_liberar;

		while (posicion >= 0){
			pagina_liberar = list_get(paginas, posicion);
			pagina_general_liberar = buscar_pagina_general(pagina_liberar);

			if(pagina_general_liberar != NULL){
				pagina_general_liberar->en_uso = false;
			}

			posicion --;

		}

		list_destroy(paginas);

		int index = index_segmento(nombre_tabla);

		if (index >= 0){
			list_remove(l_segmentos, index);
		}
	}
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

int index_segmento(char* nombre_segmento){

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
		return (posicion -1);
	}

	return -1;
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

Pagina_general* buscar_pagina_general(void* pagina){

	int posicion = (l_maestro_paginas->elements_count -1);
	Pagina_general* pagina_general = list_get(l_maestro_paginas, posicion);
	Pagina_general* pagina_encontrada = NULL;

	while (posicion >= 0){
		pagina_general = list_get(l_maestro_paginas, posicion);

		if(pagina_general->pagina == pagina){
			pagina_encontrada = pagina_general;
		}

		posicion --;
	}

	return pagina_encontrada;
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
	p_modif += (sizeof(t_key) + sizeof(t_timestamp) + MAX_VAL);
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
	memcpy(p_value, value, MAX_VAL);

}

void set_modificado_pagina( void* p_pagina, t_flag estado){
	void* p_modif = p_pagina;
	p_modif += (sizeof(t_key) + sizeof(t_timestamp) + MAX_VAL);
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

		printf("Index: %i Pagina: %i Key: %i Timestamp: %zu Valor: %s Modificado: %i \n", nro_pagina, (int)pagina, key, timestamp, value, (int) modificado);

		nro_pagina++;
	}
}

void print_pagina(void* pagina){

		t_timestamp timestamp = *get_timestamp_pagina(pagina);
		t_key key = *get_key_pagina(pagina);
		char* value = get_value_pagina(pagina);
		t_flag modificado = *get_modificado_pagina(pagina);

		printf("Pagina: %i Key: %i Timestamp: %zu Valor: %s Modificado: %i  \n",(int)pagina, key, timestamp, value, (int) modificado);

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

	// Instruccion* instruccion_respuesta = enviar_instruccion(IP_FS,PUERTO_FS,instruccion, POOLMEMORY, T_GOSSIPING);
}

int lanzar_journal(t_timestamp timestamp_journal){

	int posicion_segmento = (l_segmentos->elements_count -1);
	int posicion_pagina;
	Segmento* segmento;
	t_list* paginas;
	void* pagina;
	Pagina_general* pagina_general;

	Insert* instruccion_insert = malloc(sizeof(Insert));
	Instruccion* instruccion = malloc(sizeof(Instruccion));
	instruccion->instruccion = INSERT;
	instruccion->instruccion_a_realizar = instruccion_insert;


	while(posicion_segmento >= 0){

		segmento = list_get(l_segmentos, posicion_segmento);
		paginas = segmento->paginas;
		posicion_pagina = (paginas->elements_count -1);

		while (posicion_pagina >= 0){
			pagina = list_get(paginas, posicion_pagina);

			if (*get_modificado_pagina(pagina)){
				instruccion_insert->key = *get_key_pagina(pagina);
				instruccion_insert->nombre_tabla = segmento->nombre;
				instruccion_insert->timestamp_insert = *get_timestamp_pagina(pagina);
				instruccion_insert->value = get_value_pagina(pagina);
				instruccion_insert->timestamp = timestamp_journal;

				Instruccion* instruccion_respuesta;
				if((instruccion_respuesta = enviar_instruccion(IP_FS, PUERTO_FS, instruccion, POOLMEMORY, T_INSTRUCCION))){

					if(instruccion_respuesta->instruccion == ERROR){
						free_consulta(instruccion);
						return -1;
					}
				}

				pagina_general = buscar_pagina_general(pagina);
				pagina_general->en_uso = false;

			}
			posicion_pagina--;
		}
		eliminar_de_memoria(segmento->nombre);
		posicion_segmento--;
	}

	free_consulta(instruccion);
	return 1;
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

	if(!memoria_full()){

		int posicion = 0;
		Pagina_general* pagina_general = list_get(l_maestro_paginas, posicion);

		while(pagina_en_uso(pagina_general)){
			posicion++;
			pagina_general = list_get(l_maestro_paginas, posicion);
		}

		pagina_general->en_uso = true;

		return pagina_general->pagina;
	}
	else {
		printf("No hay mas memoria chinguenguencha!");
		log_info(LOGGER,"Memoria - La memoria esta FULL.");
		return (void*) NULL;
	}
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
	return list_all_satisfy(l_maestro_paginas, (void*)pagina_en_uso);
}
