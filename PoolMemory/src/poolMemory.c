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
	comunicacion_instrucciones->puerto_servidor = PUERTO_DE_ESCUCHA;
	comunicacion_instrucciones->proceso = POOLMEMORY;

	pthread_create(&servidorPM, NULL, (void*) servidor_comunicacion, comunicacion_instrucciones);

	pthread_join(servidorPM, NULL);
	pthread_join(consolaPoolMemory, NULL);
	pthread_join(gossiping, NULL);
	pthread_join(T_confMonitor,&TR_confMonitor);

	if((int)TR_confMonitor != 0) {
		log_error(LOGGER,"Error con el thread de monitoreo de configuracion: %d",(int)TR_confMonitor);
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

	PUERTO_DE_ESCUCHA = config_get_string_value(CONFIG,"PUERTO_DE_ESCUCHA");
	IP_FS = config_get_string_value(CONFIG,"IP_FS");
	PUERTO_FS = config_get_string_value(CONFIG,"PUERTO_FS");

	SIZE_MEM = config_get_int_value(CONFIG,"SIZE_MEM");
	IP_SEEDS = config_get_array_value(CONFIG,"IP_SEEDS");
	PUERTOS_SEEDS = config_get_array_value(CONFIG,"PUERTOS_SEEDS");

	RETARDO_MEM = config_get_int_value(CONFIG,"RETARDO_MEM");
	RETARDO_FS = config_get_int_value(CONFIG,"RETARDO_FS");
	RETARDO_JOURNAL = config_get_int_value(CONFIG,"RETARDO_JOURNAL");
	RETARDO_GOSSIPING = config_get_int_value(CONFIG,"RETARDO_GOSSIPING");
	NUMERO_MEMORIA = config_get_int_value(CONFIG,"NUMERO_MEMORIA");

	//config_destroy(CONFIG);

	Instruccion* instruccion_maxValue = malloc(sizeof(Instruccion));
	instruccion_maxValue->instruccion = MAX_VALUE;

	Instruccion* respuesta = enviar_instruccion(IP_FS,PUERTO_FS, instruccion_maxValue, POOLMEMORY, T_VALUE);

	if (respuesta->instruccion == RETORNO) {
		Retorno_Generico* retorno_generico = respuesta->instruccion_a_realizar;

		if (retorno_generico->tipo_retorno == TAMANIO_VALOR_MAXIMO) {
			Retorno_Max_Value* retorno_maxValue = retorno_generico->retorno;
			MAX_VAL = retorno_maxValue->value_size;

			log_info(LOGGER, "MAX_VALUE obtenido del FileSystem: %d.", MAX_VAL);

		} else {
			print_instruccion_parseada(respuesta);
			log_error(LOGGER, "Memoria: No se obtuvo un MAX_VALUE al pedir el MAX_VALUE al FileSystem.");
			exit_gracefully(-1);
		}

	} else {

		print_instruccion_parseada(respuesta);
		log_error(LOGGER, "Se obtuvo un ERROR al pedir el MAX_VALUE al FileSystem.");
		exit_gracefully(-1);
	}


}

void retorno_consola(char* leido){
	Instruccion* instruccion_parseada = parser_lql(leido, POOLMEMORY);
	Instruccion* respuesta = atender_consulta(instruccion_parseada);// tiene que devolver el paquete con la respuesta
	print_instruccion_parseada(respuesta);

	free_consulta(respuesta);
}

void retornarControl(Instruccion *instruccion, int cliente){

	printf("Lo que me llego desde KERNEL es:\n");
	print_instruccion_parseada(instruccion);
	printf("El fd de la consulta es %d y no esta cerrado\n", cliente);

	Instruccion* respuesta = atender_consulta(instruccion); //tiene que devolver el paquete con la respuesta

	responder(cliente, respuesta);

	free_consulta(respuesta);
}

void inicializar_memoria(){

	pthread_mutex_lock(&mutexMarcos);
	pthread_mutex_lock(&mutexSegmentos);

	MEMORIA_PRINCIPAL = malloc(SIZE_MEM); //resevo memoria para paginar
	PAGINAS_USADAS = 0; //contador de paginas en uso
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

		Instruccion* instruccion_respuesta; //instruccion_respuesta = malloc(sizeof(Instruccion));

		switch	(instruccion_parseada->instruccion){
		case SELECT:;

			pthread_mutex_lock(&mutexSegmentos);

			Select* instruccion_select = instruccion_parseada->instruccion_a_realizar;

			Segmento* segmento = buscar_segmento(instruccion_select->nombre_tabla);
			int id_pagina = -1;

			if (segmento != NULL){
				id_pagina = buscar_pagina_en_segmento(segmento, instruccion_select->key);
			}

			pthread_mutex_unlock(&mutexSegmentos);

			if(id_pagina < 0){
			// no tenemos la tabla-key en memoria
				char* o_nombre_tabla = malloc(sizeof(instruccion_select->nombre_tabla)+1);
				strcpy(o_nombre_tabla, instruccion_select->nombre_tabla);

				t_key o_key = instruccion_select->key;

				if((instruccion_respuesta = enviar_instruccion(IP_FS, PUERTO_FS, instruccion_parseada, POOLMEMORY, T_INSTRUCCION))){

					if (instruccion_respuesta->instruccion == RETORNO){
						Retorno_Generico* resp_retorno_generico = instruccion_respuesta->instruccion_a_realizar;

						if (resp_retorno_generico->tipo_retorno == VALOR){
							Retorno_Value* resp_retorno_value = resp_retorno_generico->retorno;
							int result_insert = insertar_en_memoria(o_nombre_tabla, o_key, resp_retorno_value->value, resp_retorno_value->timestamp, false);

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
				instruccion_respuesta = malloc(sizeof(Instruccion));
				instruccion_respuesta->instruccion = RETORNO;
				Retorno_Generico* p_retorno_generico = malloc(sizeof(Retorno_Generico));
				p_retorno_generico->tipo_retorno = VALOR;
				Retorno_Value* p_retorno_valor = malloc(sizeof(Retorno_Value));
				p_retorno_generico->retorno = p_retorno_valor;

				void* pagina = get_pagina(id_pagina);
				p_retorno_valor->timestamp = *get_timestamp_pagina(pagina);
				p_retorno_valor->value = get_value_pagina(pagina);

				instruccion_respuesta->instruccion_a_realizar = p_retorno_generico;

				marcar_ultimo_uso(id_pagina);

				free_consulta(instruccion_parseada);

			}

			break;

		case INSERT:;
			Insert* instruccion_insert = (Insert*) instruccion_parseada->instruccion_a_realizar;

			if (string_length(instruccion_insert->value) > MAX_VAL) {
				instruccion_respuesta = respuesta_error(LARGE_VALUE);
				break;
			}

			int result_insert = insertar_en_memoria(instruccion_insert->nombre_tabla , instruccion_insert->key, instruccion_insert->value, instruccion_insert->timestamp_insert, true);

			if(result_insert == -2){
				instruccion_respuesta = respuesta_error(MEMORY_FULL);
			} else if (result_insert < 0){
				instruccion_respuesta = respuesta_error(INSERT_FAILURE);
			}

			instruccion_respuesta = respuesta_success();
			free_consulta(instruccion_parseada);

			break;

		case DROP:;
			Drop* instruccion_drop = (Drop*) instruccion_parseada->instruccion_a_realizar;

			eliminar_de_memoria(instruccion_drop->nombre_tabla);

			instruccion_respuesta = enviar_instruccion(IP_FS, PUERTO_FS, instruccion_parseada, POOLMEMORY, T_INSTRUCCION);

			break;

		case JOURNAL:;
			Journal* instruccion_journal = (Journal*) instruccion_parseada->instruccion_a_realizar;

			int result_journal = lanzar_journal(instruccion_journal->timestamp);

			if (result_journal >= 0){
				instruccion_respuesta = respuesta_success();
			} else {
				instruccion_respuesta = respuesta_error(JOURNAL_FAILURE);
			}

			free_consulta(instruccion_parseada);

			break;

		case GOSSIP:;
			instruccion_respuesta = respuesta_success();
			free_consulta(instruccion_parseada);

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
			list_remove_and_destroy_element(L_SEGMENTOS, index, (void*) free);
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

	// ME FALTA EL CASO EN ATENDER CONSULTA EN EL QUE AGREGO MEMORIAS A L_MEMORIAS CUANDO LLEGAN

	int posicion = 0;
	while (IP_SEEDS[posicion] != NULL && PUERTOS_SEEDS[posicion] != NULL){
		char* ip = IP_SEEDS[posicion];
		char* puerto = PUERTOS_SEEDS[posicion];
		Memoria* nueva_memoria = malloc(sizeof(Memoria));
		nueva_memoria->ip = malloc(sizeof(ip));
		nueva_memoria->ip = ip;
		nueva_memoria->puerto = malloc(sizeof(puerto));
		nueva_memoria->puerto = puerto;
		nueva_memoria->idMemoria = posicion;
		pthread_mutex_lock(&mutexListaMemorias);
		list_add(L_MEMORIAS, nueva_memoria);
		pthread_mutex_unlock(&mutexListaMemorias);
		posicion++;
	}
	while(true){
		sleep(10);
		list_iterate(L_MEMORIAS, (void*)gossipear);
	}
}

void gossipear(Memoria *mem){
	if(chequear_conexion_a(mem->ip, mem->puerto)){
		Instruccion *inst  = malloc(sizeof(Instruccion));
		Gossip * gossip = malloc(sizeof(Gossip));
		gossip->lista_memorias = list_filter(L_MEMORIAS, (void*)chequear_conexion_a);
		inst ->instruccion = GOSSIP;
		inst ->instruccion_a_realizar = gossip;
		Instruccion *res = enviar_instruccion(mem->ip, mem->puerto, inst, POOLMEMORY, T_GOSSIPING);
		free(gossip);
		free(inst);
		if(res->instruccion == RETORNO){
			free(res);
			Retorno_Generico *ret = res->instruccion_a_realizar;
			if(ret->tipo_retorno == RETORNO_GOSSIP){
				free(ret);
				Gossip *gossip = ret->retorno;
				t_list *lista_retorno = gossip->lista_memorias;
				free(gossip);
				list_iterate(lista_retorno, (void*)add_memory_if_not_exists);
				list_destroy(lista_retorno);
			}
		}
	}
}

void add_memory_if_not_exists(Memoria *mem){
	if(!existe_memoria(mem)){
		pthread_mutex_lock(&mutexListaMemorias);
		list_add_in_index(L_MEMORIAS, L_MEMORIAS->elements_count, mem);
		pthread_mutex_unlock(&mutexListaMemorias);
	}else{
		free(mem->ip);
		free(mem->puerto);
		free(mem);
	}
}

bool existe_memoria(Memoria *mem1){
	int cantidad = list_size(L_MEMORIAS);
	while(cantidad > 0){
		Memoria * mem2 = list_get(L_MEMORIAS, cantidad);
		if(strcmp(mem1->ip, mem2->ip) == 0 &&
		   strcmp(mem1->puerto, mem2->puerto) == 0 &&
		   mem1->idMemoria == mem2->idMemoria){
			return true;
		}
		cantidad --;
	}
	return false;
}

int lanzar_journal(t_timestamp timestamp_journal){

	sem_wait(&semJournal);

	int posicion_segmento = (L_SEGMENTOS->elements_count -1);
	int posicion_pagina;
	Segmento* segmento;
	t_list* paginas;
	int id_pagina;
	void* pagina;

	Instruccion* instruccion_respuesta;


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

				instruccion_insert->value = malloc(strlen(get_value_pagina(pagina))+1);
				strcpy( instruccion_insert->value, get_value_pagina(pagina));

				instruccion_insert->timestamp = timestamp_journal;

				if((instruccion_respuesta = enviar_instruccion(IP_FS, PUERTO_FS, instruccion, POOLMEMORY, T_INSTRUCCION))){

					if(instruccion_respuesta->instruccion == ERROR){
						log_error(LOG_ERROR, "Memoria: Fallo insert de journal"); //loguear mejor los errores posibles, como mal key, mal value, mal table
					}

				free_consulta(instruccion_respuesta);

				PAGINAS_MODIFICADAS--;
				}
			}

			posicion_pagina--;
		}
		eliminar_de_memoria(segmento->nombre);
		posicion_segmento--;
	}

	sem_post(&semJournal);
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

int seleccionar_marco(){
	if(L_MARCOS->elements_count == PAGINAS_MODIFICADAS){
		//la memoria esta full, lanzo journal
		t_timestamp timestamp = get_timestamp();
		lanzar_journal(timestamp);
	}

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
		log_info(LOGGER,"Memoria - LRU no selecciono una memoria.");
		return -1;
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
