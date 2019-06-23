#include "poolMemory.h"

int main(void) {

	pthread_mutex_init(&mutexMarcos, NULL);
	pthread_mutex_init(&mutexSegmentos, NULL);
	pthread_mutex_init(&mutexMemorias, NULL);
	sem_init(&semJournal, 0, 2);

	print_guenguencha();

	configure_logger();
	configuracion_inicial();
	inicializar_memoria();

	pthread_mutex_init(&mutexListaMemorias, NULL);
	L_MEMORIAS = list_create();
	pthread_t consolaPoolMemory, gossiping, servidorPM, T_confMonitor;
	void *TR_confMonitor;

	pthread_create(&T_confMonitor,NULL,TH_confMonitor,NULL); 										//se encarga de mantener actualizados los valores del config
	pthread_create(&consolaPoolMemory, NULL, (void*) leer_por_consola, retorno_consola);			//consola - sale por retorno_consola()
	pthread_create(&gossiping, NULL, (void*) lanzar_gossiping, NULL);								//crea y mantiene actualizada la lista de memorias - gossiping


	Comunicacion *comunicacion_instrucciones = malloc(sizeof(Comunicacion));
	comunicacion_instrucciones->puerto_servidor = malloc(strlen(PUERTO_DE_ESCUCHA)+1);
	strcpy(comunicacion_instrucciones->puerto_servidor, PUERTO_DE_ESCUCHA);
	comunicacion_instrucciones->proceso = POOLMEMORY;

	pthread_create(&servidorPM, NULL, (void*) servidor_comunicacion, comunicacion_instrucciones);

	pthread_join(servidorPM, NULL);
	pthread_join(consolaPoolMemory, NULL);
	pthread_join(gossiping, NULL);
	pthread_join(T_confMonitor,&TR_confMonitor);

	if((int)TR_confMonitor != 0) {
		log_error(LOG_ERROR,"Error con el thread de monitoreo de configuracion: %d",(int)TR_confMonitor);
	}

	list_destroy(L_MARCOS); //entender el list_destroy_and_destroy_elements()
	list_destroy(L_MEMORIAS);

	pthread_mutex_destroy(&mutexMarcos);
	pthread_mutex_destroy(&mutexSegmentos);
	pthread_mutex_destroy(&mutexMemorias);
	sem_destroy(&semJournal);


	free(MEMORIA_PRINCIPAL);

}

void configuracion_inicial(void){
	t_config* CONFIG;
	CONFIG = config_create("config.cfg");

	if (!CONFIG) {
		printf("Memoria: Archivo de configuracion no encontrado. \n");
		exit_gracefully(EXIT_FAILURE);
	}
	char* config_puerto_escucha = config_get_string_value(CONFIG,"PUERTO_DE_ESCUCHA");
	PUERTO_DE_ESCUCHA = malloc(strlen(config_puerto_escucha) + 1);
	strcpy(PUERTO_DE_ESCUCHA, config_puerto_escucha);
	char* config_ip_fs = config_get_string_value(CONFIG,"IP_FS");
	IP_FS = malloc(strlen(config_ip_fs)+1);
	strcpy(IP_FS, config_ip_fs);
	char* config_puerto_fs = config_get_string_value(CONFIG,"PUERTO_FS");
	PUERTO_FS = malloc(strlen(config_puerto_fs) + 1);
	strcpy(PUERTO_FS, config_puerto_fs);
	SIZE_MEM = config_get_int_value(CONFIG,"SIZE_MEM");
	char** config_ip_seeds = config_get_array_value(CONFIG,"IP_SEEDS");
	int i = 0;

	while(config_ip_seeds[i]!= NULL){
		IP_SEEDS = realloc(IP_SEEDS, sizeof(char*) * (i+1));
		IP_SEEDS[i] = string_duplicate(config_ip_seeds[i]);
		free(config_ip_seeds[i]);
		i++;
	}
	free(config_ip_seeds);
	IP_SEEDS = realloc(IP_SEEDS, sizeof(char*) * (i+1));
	IP_SEEDS[i] = NULL;


	char** config_puertos_seeds = config_get_array_value(CONFIG,"PUERTOS_SEEDS");
	i = 0;
	while(config_puertos_seeds[i]!= NULL){
		PUERTOS_SEEDS= realloc(PUERTOS_SEEDS, sizeof(char*) * (i+1));
		PUERTOS_SEEDS[i] = string_duplicate(config_puertos_seeds[i]);
		free(config_puertos_seeds[i]);
		i++;
	}
	free(config_puertos_seeds);
	PUERTOS_SEEDS = realloc(PUERTOS_SEEDS, sizeof(char*) * (i+1));
	PUERTOS_SEEDS[i] = NULL;

	RETARDO_MEM = config_get_int_value(CONFIG,"RETARDO_MEM");
	RETARDO_FS = config_get_int_value(CONFIG,"RETARDO_FS");
	RETARDO_JOURNAL = config_get_int_value(CONFIG,"RETARDO_JOURNAL");
	RETARDO_GOSSIPING = config_get_int_value(CONFIG,"RETARDO_GOSSIPING");
	NUMERO_MEMORIA = config_get_int_value(CONFIG,"NUMERO_MEMORIA");
	config_destroy(CONFIG);
	Instruccion* instruccion_maxValue = malloc(sizeof(Instruccion));
	instruccion_maxValue->instruccion = MAX_VALUE;

	Instruccion* respuesta = enviar_instruccion(IP_FS,PUERTO_FS, instruccion_maxValue, POOLMEMORY, T_VALUE);

	if (respuesta->instruccion == RETORNO) {
		Retorno_Generico* retorno_generico = respuesta->instruccion_a_realizar;

		if (retorno_generico->tipo_retorno == TAMANIO_VALOR_MAXIMO) {
			Retorno_Max_Value* retorno_maxValue = retorno_generico->retorno;
			MAX_VAL = retorno_maxValue->value_size;
			free_retorno(respuesta);
			log_info(LOG_INFO, "MAX_VALUE obtenido del FileSystem: %d.", MAX_VAL);

		} else {
			print_instruccion_parseada(respuesta);
			log_error(LOG_ERROR, "Memoria: No se obtuvo un MAX_VALUE al pedir el MAX_VALUE al FileSystem.");
			exit_gracefully(EXIT_FAILURE);
		}

	} else {
		print_instruccion_parseada(respuesta);
		log_error(LOG_ERROR, "Se obtuvo un ERROR al pedir el MAX_VALUE al FileSystem.");
		exit_gracefully(EXIT_FAILURE);
	}


}

void retorno_consola(char* leido){
	Instruccion* instruccion_parseada = parser_lql(leido, POOLMEMORY);
	Instruccion* respuesta = atender_consulta(instruccion_parseada);
	print_instruccion_parseada(respuesta);
}

void retornarControl(Instruccion *instruccion, int cliente){
	Instruccion* respuesta = atender_consulta(instruccion);
	responder(cliente, respuesta);
	free_retorno(respuesta);
}

void inicializar_memoria(){

	pthread_mutex_lock(&mutexMarcos);
	pthread_mutex_lock(&mutexSegmentos);

	MEMORIA_PRINCIPAL = malloc(SIZE_MEM); //resevo memoria para paginar
	PAGINAS_USADAS = 0; //contador de paginas en uso
	PAGINAS_MODIFICADAS = 0; //contador de paginas para saber si estoy full

	if (MEMORIA_PRINCIPAL == NULL){
		log_error(LOG_ERROR, "Memoria: No se pudo malloquear la memoria principal.");
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
			log_error(LOG_ERROR, "Memoria: Fallo malloc para marco.");
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

	
	log_info(LOG_INFO, "Memoria incializada de %i bytes con %i paginas de %i bytes cada una.",SIZE_MEM,L_MARCOS->elements_count, tamanio_pagina);
}

Instruccion* atender_consulta (Instruccion* instruccion_parseada){


		usleep(RETARDO_MEM);

		sem_wait(&semJournal);

		Instruccion* instruccion_respuesta;

		switch	(instruccion_parseada->instruccion){
		case SELECT:;

			if(L_MARCOS->elements_count == PAGINAS_MODIFICADAS){
				//la memoria esta full, lanzo journal
				t_timestamp timestamp = get_timestamp();
				lanzar_journal(timestamp);
			}

			pthread_mutex_lock(&mutexSegmentos);

			Select* instruccion_select = instruccion_parseada->instruccion_a_realizar;
			char* nombre_tabla_select = malloc(strlen(instruccion_select->nombre_tabla)+1);
			strcpy(nombre_tabla_select, instruccion_select->nombre_tabla);
			t_key key_select = instruccion_select->key;

			Segmento* segmento = buscar_segmento(nombre_tabla_select);
			int id_pagina = -1;

			if (segmento != NULL){
				id_pagina = buscar_pagina_en_segmento(segmento, key_select);
			}

			pthread_mutex_unlock(&mutexSegmentos);

			if(id_pagina < 0){
			// no tenemos la tabla-key en memoria

				if((instruccion_respuesta = enviar_instruccion(IP_FS, PUERTO_FS, instruccion_parseada, POOLMEMORY, T_INSTRUCCION))){
					usleep(RETARDO_FS);

					if (instruccion_respuesta->instruccion == RETORNO){
						Retorno_Generico* resp_retorno_generico = instruccion_respuesta->instruccion_a_realizar;

						if (resp_retorno_generico->tipo_retorno == VALOR){
							Retorno_Value* resp_retorno_value = resp_retorno_generico->retorno;

							int result_insert = insertar_en_memoria(nombre_tabla_select, key_select, resp_retorno_value->value, resp_retorno_value->timestamp, false);

							if(result_insert == -2){
								instruccion_respuesta = respuesta_error(MEMORY_FULL);
							} else if (result_insert < 0){
								instruccion_respuesta = respuesta_error(INSERT_FAILURE);
							}
						} 
					} 
				}
			// en instruccion_respuesta queda la respuesta del FS que puede ser ERROR o RETORNO
			}else {
			// tenemos la tabla-key en memoria
				instruccion_respuesta = malloc(sizeof(Instruccion));
				instruccion_respuesta->instruccion = RETORNO;
				Retorno_Generico* p_retorno_generico = malloc(sizeof(Retorno_Generico));
				Retorno_Value* p_retorno_valor = malloc(sizeof(Retorno_Value));

				void* pagina = get_pagina(id_pagina);
				p_retorno_valor->timestamp = *get_timestamp_pagina(pagina);
				char* value = get_value_pagina(pagina);
				p_retorno_valor->value = malloc(strlen(value) + 1);
				strcpy(p_retorno_valor->value, value);
				p_retorno_generico->tipo_retorno = VALOR;
				p_retorno_generico->retorno = p_retorno_valor;

				instruccion_respuesta->instruccion_a_realizar = p_retorno_generico;

				marcar_ultimo_uso(id_pagina);

				free(instruccion_select->nombre_tabla);
				free(instruccion_select);
				free(instruccion_parseada);

			}
			free(nombre_tabla_select);
			break;

		case INSERT:;

			if(L_MARCOS->elements_count == PAGINAS_MODIFICADAS){
				//la memoria esta full, lanzo journal
				t_timestamp timestamp = get_timestamp();
				lanzar_journal(timestamp);
			}

			Insert* instruccion_insert = (Insert*) instruccion_parseada->instruccion_a_realizar;

			char* nombre_tabla_insert = malloc(strlen(instruccion_insert->nombre_tabla)+1);
			strcpy(nombre_tabla_insert, instruccion_insert->nombre_tabla);
			char* value_insert = malloc(strlen(instruccion_insert->value)+1);
			strcpy(value_insert, instruccion_insert->value);
			t_key key_insert = instruccion_insert->key;

			if (string_length(instruccion_insert->value) > MAX_VAL) {
				instruccion_respuesta = respuesta_error(LARGE_VALUE);
				break;
			}

			int result_insert = insertar_en_memoria(nombre_tabla_insert , key_insert, value_insert, instruccion_insert->timestamp_insert, true);

			if(result_insert == -2){
				instruccion_respuesta = respuesta_error(MEMORY_FULL);
			} else if (result_insert < 0){
				instruccion_respuesta = respuesta_error(INSERT_FAILURE);
			}

			instruccion_respuesta = respuesta_success();

			free(instruccion_insert->nombre_tabla);
			free(instruccion_insert->value);
			free(instruccion_insert);
			free(instruccion_parseada);
			free(nombre_tabla_insert);
			free(value_insert);

			break;

		case DROP:;
			Drop* instruccion_drop = (Drop*) instruccion_parseada->instruccion_a_realizar;

			char* nombre_tabla_drop = malloc(strlen(instruccion_drop->nombre_tabla)+1);
			strcpy(nombre_tabla_drop, instruccion_drop->nombre_tabla);

			eliminar_de_memoria(nombre_tabla_drop);

			instruccion_respuesta = enviar_instruccion(IP_FS, PUERTO_FS, instruccion_parseada, POOLMEMORY, T_INSTRUCCION);
			usleep(RETARDO_FS);

			free(nombre_tabla_drop);

			break;

		case JOURNAL:;
			Journal* instruccion_journal = (Journal*) instruccion_parseada->instruccion_a_realizar;

			int result_journal = lanzar_journal(instruccion_journal->timestamp);

			if (result_journal >= 0){
				instruccion_respuesta = respuesta_success();
			} else {
				instruccion_respuesta = respuesta_error(JOURNAL_FAILURE);
			}

			free(instruccion_journal);
			free(instruccion_parseada);

			break;

		case GOSSIP:;
			Gossip *gossip = instruccion_parseada->instruccion_a_realizar;
			t_list *lista_gossip = gossip->lista_memorias;
			int aux = 0;
			while(aux < lista_gossip->elements_count){
				Memoria * mem = list_get(lista_gossip, aux);
				add_memory_if_not_exists(mem);
				eliminar_memoria(mem);
			}
			list_destroy(lista_gossip);
			instruccion_respuesta = malloc(sizeof(Instruccion));
			instruccion_respuesta->instruccion = RETORNO;
			Retorno_Generico * retorno = malloc(sizeof(Retorno_Generico));
			retorno->tipo_retorno = RETORNO_GOSSIP;
			Gossip * gossip_ret = malloc(sizeof(Gossip));
			t_list *lista = filtrar_memorias();
			t_list *a_enviar = list_duplicate_all(lista, (void*)duplicar_memoria);
			list_destroy(lista);
			gossip_ret->lista_memorias = a_enviar;
			retorno->retorno = gossip_ret;
			instruccion_respuesta->instruccion_a_realizar = retorno;
			break;

		default:;
			if(instruccion_parseada->instruccion != ERROR &&
			   instruccion_parseada->instruccion != METRICS &&
			   instruccion_parseada->instruccion != ADD &&
			   instruccion_parseada->instruccion != RUN &&
			   instruccion_parseada->instruccion != JOURNAL){
				instruccion_respuesta = enviar_instruccion(IP_FS, PUERTO_FS, instruccion_parseada, POOLMEMORY, T_INSTRUCCION);
				usleep(RETARDO_FS);
			} else {
				instruccion_respuesta = respuesta_error(BAD_REQUEST);
			}
		}


		sem_post(&semJournal);

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

	pthread_mutex_unlock(&mutexSegmentos);

	int id_pagina = buscar_pagina_en_segmento(segmento, key);

	if(id_pagina < 0){
		//no tenemos la pagina
		id_pagina = seleccionar_marco();

		if(id_pagina < 0){
			return -2; // no hay paginas, la memoria esta FULL
		}
		pagina_nueva = true;

		agregar_pagina_en_segmento(segmento, id_pagina);
	}

	void* pagina = get_pagina(id_pagina);

	set_key_pagina(pagina, key);
	set_value_pagina(pagina, value);
	set_timestamp_pagina(pagina, timestamp_insert);
	set_modificado_pagina(pagina, modificado);

	marcar_ultimo_uso(id_pagina);

	if (pagina_nueva && modificado && (PAGINAS_MODIFICADAS < L_MARCOS->elements_count)){
		PAGINAS_MODIFICADAS++;
	}

	return 1;
}

void* get_pagina(int id_pagina){

	if(id_pagina <= (L_MARCOS->elements_count -1)){
		Marco* marco = list_get(L_MARCOS, id_pagina);
		return marco->pagina;
	} else {
		return (void*) NULL;
	}
}

void eliminar_de_memoria(char* nombre_tabla){

	pthread_mutex_lock(&mutexSegmentos);

	Segmento* segmento = buscar_segmento(nombre_tabla);

	if (segmento != NULL){
		t_list* paginas = segmento->paginas;
		int posicion = (paginas->elements_count -1);
		Marco* marco_liberar;
		int id_pagina_liberar;

		while (posicion >= 0){

			id_pagina_liberar = (int) list_get(paginas, posicion);

			pthread_mutex_lock(&mutexMarcos);
			marco_liberar = list_get(L_MARCOS, id_pagina_liberar);

			if(marco_liberar != NULL){
				marco_liberar->en_uso = false;
				PAGINAS_USADAS--;
			}

			pthread_mutex_unlock(&mutexMarcos);

			posicion --;

		}


		list_destroy(paginas);
		int index = index_segmento(nombre_tabla);

		if (index >= 0){
			Segmento* segmento_liberar = list_get(L_SEGMENTOS, index);
			list_remove(L_SEGMENTOS, index);
			free(segmento_liberar->nombre);
			free(segmento_liberar);

		}
	}
	pthread_mutex_unlock(&mutexSegmentos);
}


void agregar_pagina_en_segmento(Segmento* segmento, int id_pagina){
	pthread_mutex_lock(&mutexSegmentos);
	list_add(segmento->paginas, (void*) id_pagina);
	pthread_mutex_unlock(&mutexSegmentos);
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

int buscar_pagina_en_segmento(Segmento* segmento, t_key key){

	t_list* l_paginas = segmento->paginas;

	int posicion = (l_paginas->elements_count -1);
	int id_pagina;

	while (posicion >= 0){
		id_pagina = (int) list_get(l_paginas, posicion);

		if (coincide_pagina(key, id_pagina)) {
			return id_pagina;
		}

		posicion--;
	}

	return -1;
}


bool coincide_segmento (char* nombre_segmento, Segmento* segmento){

	return strcmp(nombre_segmento, segmento->nombre) == 0;

}

bool coincide_pagina (t_key key, int id_pagina){

	void* pagina = get_pagina(id_pagina);

	if(pagina != NULL) {

		t_key key_en_pagina = *get_key_pagina(pagina);
		return key == key_en_pagina;
	} else {
		return false;
	}
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
	
	memcpy(p_pagina + sizeof(t_key) + sizeof(t_timestamp), value, strlen(value)+1);

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
	int posicion = 0;
	while (IP_SEEDS[posicion] != NULL && PUERTOS_SEEDS[posicion] != NULL){
		char* ip = IP_SEEDS[posicion];
		char* puerto = PUERTOS_SEEDS[posicion];
		Memoria* nueva_memoria = malloc(sizeof(Memoria));
		nueva_memoria->ip = malloc(strlen(ip) +1);
		strcpy(nueva_memoria->ip, ip);
		nueva_memoria->puerto = malloc(strlen(puerto)+1);
		strcpy(nueva_memoria->puerto ,puerto);
		nueva_memoria->idMemoria = -1;
		free(ip);
		free(puerto);
		list_add(L_MEMORIAS, nueva_memoria);
		posicion++;
	}
	free(IP_SEEDS);
	free(PUERTOS_SEEDS);
	Memoria * memoria = malloc(sizeof(Memoria));
	char* local_ip = get_local_ip();
	memoria->ip = malloc(strlen(local_ip)+1);
	strcpy(memoria->ip, local_ip);
	memoria->puerto = malloc(strlen(PUERTO_DE_ESCUCHA)+1);
	strcpy(memoria->puerto, PUERTO_DE_ESCUCHA);
	memoria->idMemoria = NUMERO_MEMORIA;
	pthread_mutex_lock(&mutexListaMemorias);
	list_add(L_MEMORIAS, memoria);
	pthread_mutex_unlock(&mutexListaMemorias);
	while(true){
		usleep(RETARDO_GOSSIPING);
		t_list *lista = filtrar_memorias();
		list_iterate(lista, (void*)gossipear);
		list_destroy(lista);
	}
}

t_list* filtrar_memorias(){
	int aux = 0;
	t_list *lista_filtrada = list_create();
	Memoria *m1 = list_get(L_MEMORIAS, aux);
	bool filtrar(Memoria *m2){
		return strcmp(m1->ip, m2->ip) == 0 && strcmp(m1->puerto, m2->puerto) == 0;
	}
	while(m1 != NULL){
		if(list_count_satisfying(L_MEMORIAS, (void*)filtrar) > 0){
			if(m1->idMemoria != -1){
				list_add(lista_filtrada, m1);
			}
		}

		aux++;
		m1 = list_get(L_MEMORIAS, aux);
	}
	return lista_filtrada;
}

void gossipear(Memoria *mem){
	if(mem->idMemoria != NUMERO_MEMORIA){
		if(chequear_conexion_a(mem->ip, mem->puerto)){
			Instruccion *inst  = malloc(sizeof(Instruccion));
			Gossip * gossip = malloc(sizeof(Gossip));
			t_list *lista = filtrar_memorias();
			t_list *a_enviar = list_duplicate_all(lista, (void*)duplicar_memoria);
			list_destroy(lista);
			gossip->lista_memorias = a_enviar;
			inst->instruccion = GOSSIP;
			inst ->instruccion_a_realizar = gossip;
			Instruccion *res = enviar_instruccion(mem->ip, mem->puerto, inst, POOLMEMORY, T_GOSSIPING);
			if(res->instruccion == RETORNO){
				Retorno_Generico *ret = res->instruccion_a_realizar;
				if(ret->tipo_retorno == RETORNO_GOSSIP){
					Gossip *gossip = ret->retorno;
					t_list *lista_retorno = gossip->lista_memorias;
					list_iterate(lista_retorno, (void*)add_memory_if_not_exists);
				}
			}
			free_retorno(res);
		}else{
			pthread_mutex_lock(&mutexListaMemorias);
			bool remove_condition_gossip(Memoria *mem2){
				if(mem2->idMemoria != NUMERO_MEMORIA && mem2->idMemoria != -1){
					return (strcmp(mem2->ip, mem->ip) == 0 && strcmp(mem2->puerto, mem->puerto) == 0);
				}
				return false;
			}
			list_remove_and_destroy_by_condition(L_MEMORIAS, (void*)remove_condition_gossip, (void*)eliminar_de_memoria);
			pthread_mutex_unlock(&mutexListaMemorias);
		}

	}
}

void add_memory_if_not_exists(Memoria *mem){
	if(!existe_memoria(L_MEMORIAS, mem)){
		Memoria *a_agregar = duplicar_memoria(mem);
		pthread_mutex_lock(&mutexListaMemorias);
		list_add(L_MEMORIAS, a_agregar);
		pthread_mutex_unlock(&mutexListaMemorias);
	}
}

bool existe_memoria(t_list *lista, Memoria *mem1){
	int cantidad = list_size(L_MEMORIAS);
	while(cantidad > 0){
		Memoria * mem2 = list_get(lista, cantidad);
		if(strcmp(mem1->ip, mem2->ip) == 0 && strcmp(mem1->puerto, mem2->puerto) == 0) {
			return true;
		}
		cantidad --;
	}
	return false;
}

int lanzar_journal(t_timestamp timestamp_journal){
	usleep(RETARDO_JOURNAL);

	sem_wait(&semJournal);

	int posicion_segmento = (L_SEGMENTOS->elements_count -1);
	int posicion_pagina;
	Segmento* segmento;
	t_list* paginas;
	int id_pagina;
	void* pagina;
	Marco* marco;

	while(posicion_segmento >= 0){

		segmento = list_get(L_SEGMENTOS, posicion_segmento);
		paginas = segmento->paginas;
		posicion_pagina = (paginas->elements_count -1);


		while (posicion_pagina >= 0){
			id_pagina = (int) list_get(paginas, posicion_pagina);
			pagina = get_pagina(id_pagina);

			if (*get_modificado_pagina(pagina)){

				Instruccion* instruccion = malloc(sizeof(Instruccion));
				instruccion->instruccion = INSERT;
				Insert* instruccion_insert = malloc(sizeof(Insert));
				instruccion->instruccion_a_realizar = instruccion_insert;

				instruccion_insert->key = *get_key_pagina(pagina);

				instruccion_insert->nombre_tabla = malloc(strlen(segmento->nombre)+1);
				strcpy(instruccion_insert->nombre_tabla, segmento->nombre);

				instruccion_insert->timestamp_insert = *get_timestamp_pagina(pagina);

				char* value = get_value_pagina(pagina);
				instruccion_insert->value = malloc(strlen(value) + 1);
				strcpy(instruccion_insert->value, value);
				instruccion_insert->timestamp = timestamp_journal;
				Instruccion* instruccion_respuesta = enviar_instruccion(IP_FS, PUERTO_FS, instruccion, POOLMEMORY, T_INSTRUCCION);
				usleep(RETARDO_FS);
				free_retorno(instruccion_respuesta);
				marco = list_get(L_MARCOS, id_pagina);
				marco->en_uso = false;
				PAGINAS_MODIFICADAS--;
			}

			posicion_pagina--;
		}
		eliminar_de_memoria(segmento->nombre);
		posicion_segmento--;
	}

	sem_post(&semJournal);
	return 1;
}

int seleccionar_marco(){

	if(L_MARCOS->elements_count > PAGINAS_USADAS){
		// hay paginas vacias
		int posicion = 0;
		Marco* marco = list_get(L_MARCOS, posicion);

		if(marco < 0){
			return -1;
		}

		while(pagina_en_uso(marco)){
			posicion++;
			marco = list_get(L_MARCOS, posicion);

			if(marco < 0){
				return -1;
			}
		}

		marco->en_uso = true;
		PAGINAS_USADAS++;

		return posicion;
	} else {

		int id_pagina = marco_por_LRU(); //algoritmo para reemplazar paginas
		if (id_pagina < 0){
			eliminar_referencia(id_pagina);
			return id_pagina;
		}

		printf("No hay mas memoria chinguenguencha!");
		log_info(LOG_INFO,"Memoria - LRU no selecciono una memoria.");
		return -1;
	}
}

Segmento* crear_segmento(char* nombre_segmento){

	Segmento* nuevo_segmento = malloc(sizeof(Segmento));
	nuevo_segmento->nombre = malloc(strlen(nombre_segmento)+1);
	strcpy(nuevo_segmento->nombre, nombre_segmento);
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
			log_error(LOG_ERROR,"Archivo de configuracion: config.cfg no encontrado");
			return 1;
		}

		RETARDO_MEM = config_get_int_value(CONFIG,"RETARDO_MEM");
		RETARDO_FS = config_get_int_value(CONFIG,"RETARDO_FS");
		RETARDO_JOURNAL = config_get_int_value(CONFIG,"RETARDO_JOURNAL");
		RETARDO_GOSSIPING = config_get_int_value(CONFIG,"RETARDO_GOSSIPING");


		log_info(LOG_ERROR,"Se ha actualizado el archivo de configuracion: RETARDO_MEM: %d, RETARDO_FS: %d, RETARDO_JOURNAL: %d, RETARDO_GOSSIPING: %d", RETARDO_MEM, RETARDO_FS, RETARDO_JOURNAL, RETARDO_GOSSIPING);
		config_destroy(CONFIG);
		return 0;
	}

	int retMon = monitorNode("config.cfg", IN_MODIFY, &confMonitor_cb);
	if(retMon!=0){
		return (void*)1;
	}
	return (void*)0;
}

void marcar_ultimo_uso(int id_pagina){

	Marco* marco = list_get(L_MARCOS, id_pagina);

	if(marco != NULL){
		marco->ultimo_uso = get_timestamp();
	}
}

int marco_por_LRU(){

	Marco *marco, *marco_seleccionado;
	int posicion = L_MARCOS->elements_count -1;
	int id_seleccionado;

	marco_seleccionado = list_get(L_MARCOS, posicion);
	id_seleccionado = posicion;
	posicion--;

		while (posicion >= 0){

			marco = list_get(L_MARCOS, posicion);

			if(marco->ultimo_uso < marco_seleccionado->ultimo_uso){
				marco_seleccionado = marco;
				id_seleccionado = posicion;
			}

			posicion--;
		}

	return id_seleccionado;
}

void eliminar_referencia(int id_pagina_eliminar){

	Segmento* segmento;
	int pos_segmento = L_SEGMENTOS->elements_count -1;

	t_list* paginas_de_segmento;
	int pos_paginas;

	int id_pagina;

	bool marco_eliminado = false;

	while (pos_segmento >= 0 && !marco_eliminado) {
		segmento = list_get(L_SEGMENTOS, pos_segmento);
		paginas_de_segmento = segmento->paginas;

		pos_paginas = paginas_de_segmento->elements_count -1;

		while (pos_paginas >= 0 && !marco_eliminado){

			id_pagina = (int) list_get(paginas_de_segmento, pos_paginas);

			if (id_pagina == id_pagina_eliminar){
				list_remove(paginas_de_segmento, pos_paginas);
				marco_eliminado = true;
			}

			pos_paginas--;
		}

		pos_segmento--;

	}

}
