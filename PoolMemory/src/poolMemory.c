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

			// libero instruccion
			free(instruccion_maxValue);

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

	//printf("Lo que me llego desde KERNEL es:\n");
	//print_instruccion_parseada(instruccion);
	//printf("El fd de la consulta es %d y no esta cerrado\n", cliente);

	Instruccion* respuesta = atender_consulta(instruccion); //tiene que devolver el paquete con la respuesta

	responder(cliente, respuesta);
	//free_consulta(respuesta);
	//free_consulta(instruccion);
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

		Instruccion* instruccion_respuesta;

		switch	(instruccion_parseada->instruccion){
		case SELECT:;

			pthread_mutex_lock(&mutexSegmentos);

			Select* instruccion_select = instruccion_parseada->instruccion_a_realizar;

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
						} 
					} 
				}
			// en instruccion_respuesta queda la respuesta del FS que puede ser ERROR o RETORNO
			} else {
			// tenemos la tabla-key en memoria
				instruccion_respuesta = malloc(sizeof(Instruccion));
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
			instruccion_respuesta = malloc(sizeof(Instruccion));
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

			break;

		case GOSSIP:;
			instruccion_respuesta = malloc(sizeof(Instruccion));
			Gossip *gossip = instruccion_parseada->instruccion_a_realizar;
			t_list *lista_gossip = gossip->lista_memorias;
			int aux = 0;
			while(aux < lista_gossip->elements_count){
				Memoria * mem = list_get(lista_gossip, aux);
				add_memory_if_not_exists(mem);
			}
			instruccion_respuesta->instruccion = RETORNO;
			Retorno_Generico * retorno = malloc(sizeof(Retorno_Generico));
			retorno->tipo_retorno = RETORNO_GOSSIP;
			Gossip * gossip_ret = malloc(sizeof(Gossip));
			gossip_ret->lista_memorias = L_MEMORIAS;
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
	}
	pthread_mutex_unlock(&mutexSegmentos);
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

	return strcmp(nombre_segmento, segmento->nombre) == 0;

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
	L_SEEDS = list_create();
	while (IP_SEEDS[posicion] != NULL && PUERTOS_SEEDS[posicion] != NULL){
		char* ip = IP_SEEDS[posicion];
		char* puerto = PUERTOS_SEEDS[posicion];
		Memoria* nueva_memoria = malloc(sizeof(Memoria));
		nueva_memoria->ip = malloc(sizeof(ip));
		nueva_memoria->ip = ip;
		nueva_memoria->puerto = malloc(sizeof(puerto));
		nueva_memoria->puerto = puerto;
		nueva_memoria->idMemoria = posicion;
		list_add(L_SEEDS, nueva_memoria);
		posicion++;
	}
	Memoria * memoria = malloc(sizeof(Memoria));
	memoria->ip = malloc(strlen(get_local_ip())+1);
	strcpy(memoria->ip, get_local_ip());
	memoria->puerto = malloc(strlen(PUERTO_DE_ESCUCHA)+1);
	strcpy(memoria->puerto, PUERTO_DE_ESCUCHA);
	memoria->idMemoria = NUMERO_MEMORIA;
	pthread_mutex_lock(&mutexListaMemorias);
	list_add(L_MEMORIAS, memoria);
	pthread_mutex_unlock(&mutexListaMemorias);
	while(true){
		sleep(10);
		list_iterate(L_SEEDS, (void*)gossipear);
	}
}

void gossipear(Memoria *mem){
	if(chequear_conexion_a(mem->ip, mem->puerto)){
		Instruccion *inst  = malloc(sizeof(Instruccion));
		Gossip * gossip = malloc(sizeof(Gossip));
		gossip->lista_memorias = L_MEMORIAS;
		inst ->instruccion = GOSSIP;
		inst ->instruccion_a_realizar = gossip;
		Instruccion *res = enviar_instruccion(mem->ip, mem->puerto, inst, POOLMEMORY, T_GOSSIPING);
		free(gossip);
		free(inst);
		if(res->instruccion == RETORNO){
			free(res);
			Retorno_Generico *ret = res->instruccion_a_realizar;
			if(ret->tipo_retorno == RETORNO_GOSSIP){
				Gossip *gossip = ret->retorno;
				free(ret);
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
		list_add(L_MEMORIAS, mem);
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
						log_error(LOG_ERROR, "Memoria: Fallo insert de journal"); //loguear mejor los errores posibles, como mal key, mal value, mal table
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

