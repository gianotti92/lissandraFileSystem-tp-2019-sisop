#include "poolMemory.h"

int main(void) {
	pthread_mutex_init(&mutexMarcos, NULL);
	pthread_mutex_init(&mutexSegmentos, NULL);
	pthread_mutex_init(&mutexMemorias, NULL);
	sem_init(&semJournal, 0, 2);

	configure_logger();
	configuracion_inicial();
	MAX_VAL = 10; // esto hay que reemplazarlo por el valor del FS
	inicializar_memoria();

	pthread_t consolaPoolMemory, gossiping, servidorPM, T_confMonitor;
	void *TR_confMonitor;

	pthread_create(&T_confMonitor,NULL,TH_confMonitor,NULL); 										//se encarga de mantener actualizados los valores del config
	pthread_create(&consolaPoolMemory, NULL, (void*) leer_por_consola, retorno_consola);			//consola - sale por retorno_consola()
	pthread_create(&gossiping, NULL, (void*) lanzar_gossiping, NULL);								//crea y mantiene actualizada la lista de memorias - gossiping

	Comunicacion *comunicacion_instrucciones = malloc(sizeof(Comunicacion));
	comunicacion_instrucciones->puerto_servidor = PUERTO_DE_ESCUCHA;
	comunicacion_instrucciones->proceso = POOLMEMORY;
	comunicacion_instrucciones->tipo_comunicacion = T_INSTRUCCION;
	pthread_create(&servidorPM, NULL, (void*) servidor_comunicacion, comunicacion_instrucciones);	//servidor - sale por retorno_control()

	pthread_join(servidorPM, NULL);
	pthread_join(consolaPoolMemory, NULL);
	pthread_join(gossiping, NULL);
	pthread_join(T_confMonitor,&TR_confMonitor);

	if((int)TR_confMonitor != 0) {
		log_error(LOGGER,"Error con el thread de monitoreo de configuracion: %d",(int)TR_confMonitor);
	}

	list_destroy(L_MARCOS); //entender el list_destroy_and_destroy_elements()
	list_destroy(L_MEMORIAS);
	free(MEMORIA_PRINCIPAL);

}

void configuracion_inicial(void){
	t_config* CONFIG;
	CONFIG = config_create("config.cfg");

	if (!CONFIG) {
		printf("Memoria: Archivo de configuracion no encontrado. \n");
		exit_gracefully(EXIT_FAILURE);
	}

	PUERTO_DE_ESCUCHA = config_get_string_value(CONFIG,"PUERTO_DE_ESCUCHA");
	IP_FS = config_get_string_value(CONFIG,"IP_FS");
	PUERTO_FS = config_get_string_value(CONFIG,"PUERTO_FS");
	IP_SEEDS = config_get_string_value(CONFIG,"IP_SEEDS");
	PUERTOS_SEEDS = config_get_string_value(CONFIG,"PUERTOS_SEEDS");
	SIZE_MEM = config_get_int_value(CONFIG,"SIZE_MEM");

	RETARDO_MEM = config_get_int_value(CONFIG,"RETARDO_MEM");
	RETARDO_FS = config_get_int_value(CONFIG,"RETARDO_FS");
	RETARDO_JOURNAL = config_get_int_value(CONFIG,"RETARDO_JOURNAL");
	RETARDO_GOSSIPING = config_get_int_value(CONFIG,"RETARDO_GOSSIPING");
	NUMERO_MEMORIA = config_get_int_value(CONFIG,"NUMERO_MEMORIA");

	//config_destroy(CONFIG);
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

	pthread_mutex_lock(&mutexMarcos);
	pthread_mutex_lock(&mutexSegmentos);

	MEMORIA_PRINCIPAL = malloc(SIZE_MEM); //resevo memoria para paginar
	PAGINAS_MODIFICADAS = 0; //contador de paginas para saber si estoy full

	if (MEMORIA_PRINCIPAL == NULL){
		log_error(LOGGER, "Memoria: No se pudo malloquear la memoria principal.");
		pthread_mutex_unlock(&mutexMarcos);
		pthread_mutex_unlock(&mutexSegmentos);
		exit_gracefully(-1);
	}

	int tamanio_pagina = sizeof(uint16_t) + sizeof(uint32_t) +  MAX_VAL + sizeof(bool); //calculo tamanio de cada pagina KEY-TIMESTAMP-VALUE-MODIF

	L_MARCOS = list_create(); //creo la tabla maestra de paginas, es una t_list global

	L_SEGMENTOS = list_create(); //creo la tabla de segmentos, es una t_list global

	int memoria_formateada = 0; //inicializo un contador de memoria formateada

	void* desplazamiento = MEMORIA_PRINCIPAL; //pongo al principio el desplazamiento

	while (memoria_formateada + tamanio_pagina < SIZE_MEM){

		Marco* registro_maestra = malloc(sizeof(Marco)); //registro para la tabla maestra de paginas

		if (registro_maestra == NULL){
			log_error(LOGGER, "Memoria: Fallo malloc para marco.");
			exit_gracefully(-1);
		}

		registro_maestra->en_uso = false;
		registro_maestra->pagina = desplazamiento;

		list_add(L_MARCOS, registro_maestra); //inserto el registro para una nueva pagina en la tabla maestra de paginas

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


	pthread_mutex_unlock(&mutexMarcos);
	pthread_mutex_unlock(&mutexSegmentos);

	printf("Memoria incializada de %i bytes con %i paginas de %i bytes cada una. \n",SIZE_MEM,L_MARCOS->elements_count, tamanio_pagina);
	log_info(LOGGER, "Memoria incializada de %i bytes con %i paginas de %i bytes cada una.",SIZE_MEM,L_MARCOS->elements_count, tamanio_pagina);
}

Instruccion* atender_consulta (Instruccion* instruccion_parseada){

		sem_wait(&semJournal);

		Instruccion* instruccion_respuesta = malloc(sizeof(Instruccion));

		switch	(instruccion_parseada->instruccion){
		case SELECT:;


			pthread_mutex_lock(&mutexSegmentos);

			Select* instruccion_select = (Select*) instruccion_parseada->instruccion_a_realizar;

			Segmento* segmento = buscar_segmento(instruccion_select->nombre_tabla);
			void* pagina = NULL;

			if (segmento != NULL){
				pagina = buscar_pagina_en_segmento(segmento, instruccion_select->key);
			}

			pthread_mutex_unlock(&mutexSegmentos);

			if(pagina == NULL){
			// no tenemos la tabla-key en memoria
				if((instruccion_respuesta = enviar_instruccion(IP_FS, PUERTO_FS, instruccion_parseada, POOLMEMORY, T_INSTRUCCION))){

					if (instruccion_respuesta->instruccion == RETORNO){
						Retorno_Generico* resp_retorno_generico = instruccion_respuesta->instruccion_a_realizar;

						if (resp_retorno_generico->tipo_retorno == VALOR){
							Retorno_Value* resp_retorno_value = resp_retorno_generico->retorno;
							int result_insert = insertar_en_memoria(instruccion_select->nombre_tabla, instruccion_select->key, resp_retorno_value->value, resp_retorno_value->timestamp, false);

							if(result_insert == -2){
								instruccion_respuesta = respuesta_error(MEMORY_FULL);
							} else if (result_insert < 0){
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

			break;

		case INSERT:;
			Insert* instruccion_insert = (Insert*) instruccion_parseada->instruccion_a_realizar;

			int result_insert = insertar_en_memoria(instruccion_insert->nombre_tabla , instruccion_insert->key, instruccion_insert->value, instruccion_insert->timestamp_insert, true);

			if(result_insert == -2){
				instruccion_respuesta = respuesta_error(MEMORY_FULL);
			} else if (result_insert < 0){
				instruccion_respuesta = respuesta_error(INSERT_FAILURE);
			}

			instruccion_respuesta = respuesta_success();

			break;

		case DROP:;
			Drop* instruccion_drop = (Drop*) instruccion_parseada->instruccion_a_realizar;

			eliminar_de_memoria(instruccion_drop->nombre_tabla);

			instruccion_respuesta = enviar_instruccion(IP_FS, PUERTO_FS, instruccion_parseada, POOLMEMORY, T_INSTRUCCION);

			break;

		case JOURNAL:;

			sem_wait(&semJournal);

			Journal* instruccion_journal = (Journal*) instruccion_parseada->instruccion_a_realizar;

			int result_journal = lanzar_journal(instruccion_journal->timestamp);

			if (result_journal >= 0){
				instruccion_respuesta = respuesta_success();
			} else {
				instruccion_respuesta = respuesta_error(JOURNAL_FAILURE);
			}

			sem_post(&semJournal);
			break;

		case GOSSIP:;
			instruccion_respuesta = respuesta_success();
			break;

		default:;
			if(instruccion_parseada->instruccion != ERROR &&
			   instruccion_parseada->instruccion != METRICS &&
			   instruccion_parseada->instruccion != ADD &&
			   instruccion_parseada->instruccion != RUN &&
			   instruccion_parseada->instruccion != JOURNAL){
				instruccion_respuesta = enviar_instruccion(IP_FS, PUERTO_FS, instruccion_parseada, POOLMEMORY, T_INSTRUCCION);
			} else {
				instruccion_respuesta = respuesta_error(BAD_REQUEST);
			}
		}


		sem_post(&semJournal);
		free_consulta(instruccion_parseada);

		return instruccion_respuesta;
}


int insertar_en_memoria(char* nombre_tabla, t_key key, char* value, t_timestamp timestamp_insert, t_flag modificado){

	pthread_mutex_lock(&mutexSegmentos);

	Segmento* segmento = buscar_segmento(nombre_tabla);
	bool pagina_nueva = false;


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
			return -2; // no hay paginas, la memoria esta FULL
		}
		pagina_nueva = true;
		agregar_pagina_en_segmento(segmento, pagina);
	}

	set_key_pagina(pagina, key);
	set_value_pagina(pagina, value);
	set_timestamp_pagina(pagina, timestamp_insert);
	set_modificado_pagina(pagina, modificado);

	if (pagina_nueva && modificado){
		PAGINAS_MODIFICADAS++;
	}

	pthread_mutex_unlock(&mutexSegmentos);

	return 1;
}

void eliminar_de_memoria(char* nombre_tabla){

	pthread_mutex_lock(&mutexSegmentos);

	Segmento* segmento = buscar_segmento(nombre_tabla);

	if (segmento != NULL){
		t_list* paginas = segmento->paginas;
		int posicion = (paginas->elements_count -1);
		Marco* marco_liberar;
		void* pagina_liberar;



		while (posicion >= 0){


			pagina_liberar = list_get(paginas, posicion);

			pthread_mutex_lock(&mutexMarcos);
			marco_liberar = buscar_marco(pagina_liberar);

			if(marco_liberar != NULL){
				marco_liberar->en_uso = false;
				PAGINAS_MODIFICADAS--;
			}

			pthread_mutex_unlock(&mutexMarcos);

			posicion --;

		}



		list_destroy(paginas);

		int index = index_segmento(nombre_tabla);

		if (index >= 0){
			list_remove(L_SEGMENTOS, index);
		}

	pthread_mutex_unlock(&mutexSegmentos);
	}
}


void agregar_pagina_en_segmento(Segmento* segmento, void* pagina){
	list_add(segmento->paginas, pagina);
}

void* buscar_segmento(char* nombre_segmento){

	int posicion = 0;
	Segmento* segmento;
	bool encontrado = false;

	while (posicion < L_SEGMENTOS->elements_count && !encontrado){

		segmento = list_get(L_SEGMENTOS, posicion);

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

	while (posicion < L_SEGMENTOS->elements_count && !encontrado){

		segmento = list_get(L_SEGMENTOS, posicion);

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

Marco* buscar_marco(void* pagina){

	int posicion = (L_MARCOS->elements_count -1);
	Marco* marco = list_get(L_MARCOS, posicion);
	Marco* marco_encontrado = NULL;

	while (posicion >= 0){
		marco = list_get(L_MARCOS, posicion);

		if(marco->pagina == pagina){
			marco_encontrado = marco;
		}

		posicion --;
	}

	return marco_encontrado;
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
	int size_lista = L_MARCOS->elements_count;
	Marco* marco;
	void* pagina;

	while (nro_pagina < size_lista){
		marco = list_get(L_MARCOS, nro_pagina);
		pagina = marco->pagina;

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

	L_MEMORIAS = list_create();
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

		list_add(L_MEMORIAS, nueva_memoria);

		posicion++;
	}
	Instruccion *instruccion = malloc(sizeof(Instruccion));
	Gossip * gossip = malloc(sizeof(Gossip));
	gossip->lista_memorias = L_MEMORIAS;
	instruccion->instruccion = GOSSIP;
	instruccion->instruccion_a_realizar = gossip;

	// Instruccion* instruccion_respuesta = enviar_instruccion(IP_FS,PUERTO_FS,instruccion, POOLMEMORY, T_GOSSIPING);
}

int lanzar_journal(t_timestamp timestamp_journal){

	int posicion_segmento = (L_SEGMENTOS->elements_count -1);
	int posicion_pagina;
	Segmento* segmento;
	t_list* paginas;
	void* pagina;
	Marco* marco;

	Insert* instruccion_insert = malloc(sizeof(Insert));
	Instruccion* instruccion = malloc(sizeof(Instruccion));
	instruccion->instruccion = INSERT;
	instruccion->instruccion_a_realizar = instruccion_insert;


	while(posicion_segmento >= 0){

		segmento = list_get(L_SEGMENTOS, posicion_segmento);
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

				marco = buscar_marco(pagina);
				marco->en_uso = false;
				PAGINAS_MODIFICADAS--;

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
	int size = L_MEMORIAS->elements_count;
	Memoria* memoria;

	while (posicion < size){
		memoria = list_get(L_MEMORIAS, posicion);

		printf("ID: %i IP: %s PUERTO: %s \n", memoria->idMemoria, memoria->ip, memoria->puerto);
		posicion++;
	}

}

void* seleccionar_pagina (){
	 //aca iria el algoritmo para reemplazar paginas

	if(L_MARCOS->elements_count > PAGINAS_MODIFICADAS){

		int posicion = 0;
		Marco* marco = list_get(L_MARCOS, posicion);

		while(pagina_en_uso(marco)){
			posicion++;
			marco = list_get(L_MARCOS, posicion);
		}

		marco->en_uso = true;

		return marco->pagina;
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
	list_add(L_SEGMENTOS, nuevo_segmento);

	return nuevo_segmento;

}

bool pagina_en_uso(Marco* marco){
	return marco->en_uso == true;
}

bool memoria_full(){
	return list_all_satisfy(L_MARCOS, (void*)pagina_en_uso);
}

/*
	Manejo Monitoreo
*/
void *TH_confMonitor(void * p){

	int confMonitor_cb(void){
		t_config* CONFIG = config_create("config.cfg");

		if(CONFIG == NULL) {
			log_error(LOGGER,"Archivo de configuracion: config.cfg no encontrado");
			return 1;
		}

		RETARDO_MEM = config_get_int_value(CONFIG,"RETARDO_MEM");
		RETARDO_FS = config_get_int_value(CONFIG,"RETARDO_FS");
		RETARDO_JOURNAL = config_get_int_value(CONFIG,"RETARDO_JOURNAL");
		RETARDO_GOSSIPING = config_get_int_value(CONFIG,"RETARDO_GOSSIPING");


		log_info(LOGGER,"Se ha actualizado el archivo de configuracion: RETARDO_MEM: %d, RETARDO_FS: %d, RETARDO_JOURNAL: %d, RETARDO_GOSSIPING: %d", RETARDO_MEM, RETARDO_FS, RETARDO_JOURNAL, RETARDO_GOSSIPING);
		config_destroy(CONFIG);
		return 0;
	}

	int retMon = monitorNode("config.cfg", IN_MODIFY, &confMonitor_cb);
	if(retMon!=0){
		return (void*)1;
	}
	return (void*)0;
}

