#include "kernel.h"

Proceso * logicaRun(Proceso * proceso){
	char * proximainstruccionChar = leer_linea(((Run*)proceso->instruccion->instruccion_a_realizar)->path, proceso->numeroInstruccion);

	while(proximainstruccionChar != NULL && proceso->quantumProcesado < QUANTUM){
		proceso->instruccionAProcesar = parser_lql(proximainstruccionChar, KERNEL);
		time_t fin;
		int diff;
		switch(proceso->instruccionAProcesar->instruccion){
			case CREATE:;
				logicaCreate(proceso->instruccionAProcesar);
				break;

			case SELECT:;
				logicaSelect(proceso->instruccionAProcesar);

				fin = get_timestamp();
				diff = difftime(fin ,((Select*)proceso->instruccionAProcesar)->timestamp);
				proceso->segundosQueTardo = diff;
				break;

			case INSERT:;
				logicaInsert(proceso->instruccionAProcesar);
				fin = get_timestamp();
				diff = difftime(fin, ((Insert*)proceso->instruccionAProcesar)->timestamp);
				proceso->segundosQueTardo = diff;
				break;

			case DROP:;
				logicaDrop(proceso->instruccionAProcesar);
				break;

			case DESCRIBE:;
				logicaDescribe(proceso->instruccionAProcesar);
				break;

			case METRICS:;
				logicaMetrics(proceso->instruccionAProcesar);
				break;

			case JOURNAL:;
				logicaJournal(proceso->instruccionAProcesar);
				break;

			case ADD:;
				logicaAdd(proceso->instruccionAProcesar);
				break;
			case ERROR:;
				free(proceso->instruccionAProcesar->instruccion_a_realizar);
				free(proceso->instruccionAProcesar);
				return proceso;

			default:
				break;

		}
		proceso->numeroInstruccion += 1;
		proceso->quantumProcesado += 1;

		proximainstruccionChar = leer_linea(((Run*)proceso->instruccion->instruccion_a_realizar)->path, proceso->numeroInstruccion);
	}

	if(esFinQuantum(proceso, proximainstruccionChar)){
		encolar(estadoReady, proceso);
		return proceso;
	}else if(esFinLectura(proceso, proximainstruccionChar)){
		encolar(estadoExit, proceso);
		return proceso;
	}else{
		encolar(estadoExit, proceso);
		return proceso;
	}
}

void logicaCreate(Instruccion * instruccion){
	//FIXME: Hay que seleccionar una aleatoriamente, no la MEMORIA_PPAL
	Instruccion * instruccionRespuestaCreate = enviar_instruccion(IP_MEMORIA_PPAL, PUERTO_MEMORIA_PPAL, instruccion, KERNEL, T_INSTRUCCION);
	print_instruccion_parseada(instruccionRespuestaCreate);
}

void logicaAdd(Instruccion * instruccion){
	Memoria *memoria = get_memoria(((Add*)instruccion->instruccion_a_realizar)->memoria, DISP);
	if(memoria == NULL){
		free(instruccion->instruccion_a_realizar);
		free(instruccion);
		log_error(LOG_OUTPUT, "No corresponde a un id de memoria válido, las validas son:");
		pthread_mutex_lock(&mutex_disp);
		list_iterate(lista_disp, (void*)mostrar_memoria);
		pthread_mutex_unlock(&mutex_disp);
		return;
	}
	if(((Add*)instruccion)->consistencia == SHC){
		pthread_mutex_lock(&mutex_shc);
		list_iterate(lista_shc, (void*)enviar_journal);
		pthread_mutex_unlock(&mutex_shc);
	}
	asignar_memoria_a_consistencia(memoria, ((Add*)instruccion->instruccion_a_realizar)->consistencia);
	eliminar_memoria(memoria);
	free(instruccion->instruccion_a_realizar);
	free(instruccion);
}

void enviar_journal(Memoria *memoria){
	Instruccion * instruccion = malloc(sizeof(Instruccion));
	instruccion->instruccion = JOURNAL;
	Journal * journal = malloc(sizeof(Journal));
	journal->timestamp = get_timestamp();
	instruccion->instruccion_a_realizar = journal;
	Instruccion * instruccionRespuesta = enviar_instruccion(memoria->ip, memoria->puerto, instruccion, KERNEL, T_INSTRUCCION);
	free_retorno(instruccionRespuesta);
}

void logicaSelect(Instruccion * instruccion){
	Consistencias consistencia = obtenerConsistencia(((Select*)instruccion->instruccion_a_realizar)->nombre_tabla);
	if(consistencia > 0){
		t_list *memoriasAsoc = dame_lista_de_consistencia(consistencia);
		Memoria *m;
		switch(consistencia){
			case SC || EC :;
				int random = (rand() % (memoriasAsoc->elements_count + 1) - 1);
				m = get_memoria(random, consistencia);
				break;

			default:;
				int indexTabla = generarHash(((Select*)instruccion->instruccion_a_realizar)->nombre_tabla, memoriasAsoc->elements_count, ((Select*)instruccion)->key);
				m = get_memoria(indexTabla, consistencia);
				break;
		}
		list_destroy_and_destroy_elements(memoriasAsoc, (void*)eliminar_memoria);
		if(m != NULL){
			Instruccion *instruccionRespuesta = enviar_instruccion(m->ip, m->puerto, instruccion, KERNEL, T_INSTRUCCION);
			print_instruccion_parseada(instruccionRespuesta);
			eliminar_memoria(m);
			return;
		}
		else{
			log_error(LOG_ERROR, "No hay memorias asignadas a este criterio");
		}
	}else{
		log_error(LOG_ERROR, "Esa tabla no existe");

	}
	free(((Select*)instruccion->instruccion_a_realizar)->nombre_tabla);
	free(instruccion->instruccion_a_realizar);
	free(instruccion);
	return;
}

void logicaInsert(Instruccion * instruccion){
	Consistencias consistencia = obtenerConsistencia(((Insert*) instruccion->instruccion_a_realizar)->nombre_tabla);
	if(consistencia > 0){
		t_list *memoriasAsoc = dame_lista_de_consistencia(consistencia);
		Memoria * mem = NULL;
		if(consistencia == SC){
			//SC siempre tiene una sola memoria
			mem = get_memoria(1, consistencia);
		}else if(consistencia == SHC){
			//entrar por hash
			int key = ((Insert*) instruccion->instruccion_a_realizar)->key;

		}else if(consistencia == EC){
			//una random
			int random = (rand() % (memoriasAsoc->elements_count + 1) - 1);
		}

		list_destroy_and_destroy_elements(memoriasAsoc, (void*)eliminar_memoria);
		if(mem != NULL){
			Instruccion * instruccionRespuesta = enviar_instruccion(mem->ip, mem->puerto, instruccion, KERNEL, T_INSTRUCCION);
			print_instruccion_parseada(instruccionRespuesta);
			eliminar_memoria(mem);
			return;
		}else{
			log_error(LOG_OUTPUT, "No hay memorias asignadas a este criterio");
		}
	}else{
		log_error(LOG_OUTPUT, "La tabla no existe");
	}
	free(((Insert*) instruccion->instruccion_a_realizar)->nombre_tabla);
	free(((Insert*) instruccion->instruccion_a_realizar)->value);
	free(instruccion->instruccion_a_realizar);
	free(instruccion);
	return;
}

void logicaDrop(Instruccion * instruccion){
	Consistencias consistencia = obtenerConsistencia(((Drop*)instruccion->instruccion_a_realizar)->nombre_tabla);
	if(consistencia > 0){
		t_list *memoriasAsoc = dame_lista_de_consistencia(consistencia);
		int random = (rand() % (memoriasAsoc->elements_count + 1) - 1);
		Memoria * mem = get_memoria(random, consistencia);
		list_destroy_and_destroy_elements(memoriasAsoc, (void*)eliminar_memoria);
		if(mem != NULL){
			Instruccion * instruccionRespuesta = enviar_instruccion(mem->ip, mem->puerto, instruccion, KERNEL, T_INSTRUCCION);
			print_instruccion_parseada(instruccionRespuesta);
			eliminar_memoria(mem);
			return;
		}else{
			log_error(LOG_OUTPUT, "No hay memorias asignadas para ese criterio");
		}
	}else{
		log_error(LOG_OUTPUT, "La tabla no existe");
	}
	free(((Drop*)instruccion->instruccion_a_realizar)->nombre_tabla);
	free(instruccion->instruccion_a_realizar);
	free(instruccion);
}

void logicaJournal(Instruccion * instruccion){
	Consistencias consistencia;
	for(consistencia = EC; consistencia < DISP; consistencia++){
		pthread_mutex_t mutex = dame_mutex_de_consistencia(consistencia);
		pthread_mutex_lock(&mutex);
		t_list *lista = dame_lista_de_consistencia(consistencia);
		list_iterate(lista, (void*)enviar_journal);
		pthread_mutex_unlock(&mutex);
	}
	free(instruccion->instruccion_a_realizar);
	free(instruccion);
}

void logicaDescribe(Instruccion * instruccion){

	if(((Describe*)instruccion->instruccion_a_realizar)->nombre_tabla != NULL){

		Consistencias consistencia = obtenerConsistencia(((Describe*)instruccion->instruccion_a_realizar)->nombre_tabla);

		if(consistencia > 0){
			t_list *memoriasAsoc = dame_lista_de_consistencia(consistencia);
			int random = (rand() % (memoriasAsoc->elements_count + 1) - 1);
			Memoria * mem = get_memoria(random, consistencia);
			list_destroy_and_destroy_elements(memoriasAsoc, (void*)eliminar_memoria);
			if(mem != NULL){
				Instruccion * intstruccionRespuesta = enviar_instruccion(mem->ip, mem->puerto, instruccion, KERNEL, T_INSTRUCCION);
				print_instruccion_parseada(intstruccionRespuesta);
				eliminar_memoria(mem);
				return;
			}else{
				log_error(LOG_OUTPUT, "No hay memorias asignadas para ese criterio");
			}
		}else{
			log_error(LOG_OUTPUT, "La tabla no existe");
		}
	}else{
		Instruccion * intstruccionRespuesta = enviar_instruccion(IP_MEMORIA_PPAL, PUERTO_MEMORIA_PPAL, instruccion, KERNEL, T_INSTRUCCION);
		print_instruccion_parseada(intstruccionRespuesta);
		return;
	}
	free(((Describe*)instruccion->instruccion_a_realizar)->nombre_tabla);
	free(instruccion->instruccion_a_realizar);
	free(instruccion);
}

void logicaMetrics(Instruccion * instruccion){
	pthread_mutex_lock(&mutexRecursosCompartidos);
	char * campo1 = dictionary_get(metrics, READS);
	char * campo2 = dictionary_get(metrics, WRITES);
	char * campo3 = dictionary_get(metrics, MEM_LOAD);
	char * campo4 = dictionary_get(metrics, WRITE_LAT);
	char * campo5 = dictionary_get(metrics, READ_LAT);
	pthread_mutex_unlock(&mutexRecursosCompartidos);

	log_info(LOG_OUTPUT,"%s\n", campo1);
	log_info(LOG_OUTPUT,"%s\n", campo2);
	log_info(LOG_OUTPUT,"%s\n", campo3);
	log_info(LOG_OUTPUT,"%s\n", campo4);
	log_info(LOG_OUTPUT,"%s\n", campo5);
	free(instruccion->instruccion_a_realizar);
	free(instruccion);
}

Consistencias obtenerConsistencia(char * nombreTabla){
	bool criterioNombre(Table_Metadata* tabla){
		return !strcmp(tabla->tablename,nombreTabla);
	}
	pthread_mutex_lock(&lista_de_tablas_mx);
	Table_Metadata * found = list_find(lista_de_tablas,(void*)criterioNombre);
	pthread_mutex_unlock(&lista_de_tablas_mx);
	if(found == NULL) {
		return -1;
	} else {
		return found->consistencia;
	}
}

int generarHash(char * nombreTabla, int tamLista, int key){
	int hash = 0;
	int i;
	for(i = 0; i < strlen(nombreTabla); i++){
		hash += nombreTabla[i];
	}

	hash += key;

	return hash % tamLista;
}

