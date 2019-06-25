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
				logicaMetrics();
				free(proceso->instruccionAProcesar->instruccion_a_realizar);
				free(proceso->instruccionAProcesar);
				break;

			case JOURNAL:;
				logicaJournal();
				free(proceso->instruccionAProcesar->instruccion_a_realizar);
				free(proceso->instruccionAProcesar);
				break;

			case ADD:;
				logicaAdd(proceso->instruccionAProcesar);
				free(proceso->instruccionAProcesar->instruccion_a_realizar);
				free(proceso->instruccionAProcesar);
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
		free(proximainstruccionChar);

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
	Instruccion * instruccionRespuestaCreate = enviar_instruccion(IP_MEMORIA_PPAL, PUERTO_MEMORIA_PPAL, instruccion, KERNEL, T_INSTRUCCION);
	print_instruccion_parseada(instruccionRespuestaCreate);
}

void logicaAdd(Instruccion * instruccion){
	Memoria *memoria = getMemoria(list_get(memorias, DISP), ((Add*)instruccion->instruccion_a_realizar)->memoria);
	if(memoria == NULL){
		log_error(LOG_OUTPUT, "No corresponde a un id de memoria válido");
		return;
	}
	if(((Add*)instruccion)->consistencia == SHC){
		list_iterate(list_get(memorias, SHC), (void*)enviar_journal);
	}
	asignarConsistenciaAMemoria(memoria, ((Add*)instruccion->instruccion_a_realizar)->consistencia);
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
	t_list *memoriasAsoc = list_get(memorias, consistencia);
	if(consistencia == SC || consistencia == EC || consistencia == SHC){
		if(memoriasAsoc->elements_count != 0){
			Memoria * m = NULL;
			switch(consistencia){
				case SC || EC :;
					int max = list_size(memoriasAsoc);
					int randomId = rand() % max + 1;

					pthread_mutex_lock(&mutexRecursosCompartidos);
					m = list_get(memoriasAsoc, randomId - 1);
					pthread_mutex_unlock(&mutexRecursosCompartidos);
					break;

				default:;
					int indexTabla = generarHash(((Select*)instruccion->instruccion_a_realizar)->nombre_tabla, list_size(memoriasAsoc), ((Select*)instruccion)->key);
					pthread_mutex_lock(&mutexRecursosCompartidos);
					m = list_get(memoriasAsoc, indexTabla);
					pthread_mutex_unlock(&mutexRecursosCompartidos);
					break;
			}
			if(m != NULL){
				Instruccion *instruccionRespuesta = enviar_instruccion(m->ip, m->puerto, instruccion, KERNEL, T_INSTRUCCION);
				print_instruccion_parseada(instruccionRespuesta);
			}
		}else{
			log_error(LOG_ERROR, "No hay memorias asignadas para ese criterio");
		}
	}else{
		log_error(LOG_ERROR, "Esa tabla no existe");
	}
}

void logicaInsert(Instruccion * instruccion){
	Consistencias consistencia = obtenerConsistencia(((Insert*) instruccion->instruccion_a_realizar)->nombre_tabla);
	if(consistencia > 0){
		t_list *memoriasAsoc = list_get(memorias, consistencia);
		int max = list_size(memoriasAsoc);
		int randomId = rand() % max + 1;
		Memoria * mem = NULL;
		pthread_mutex_lock(&mutexRecursosCompartidos);
		mem = list_get(memoriasAsoc, randomId - 1);
		pthread_mutex_unlock(&mutexRecursosCompartidos);

		if(mem != NULL){
			Instruccion * instruccionRespuesta = enviar_instruccion(mem->ip, mem->puerto, instruccion, KERNEL, T_INSTRUCCION);
			print_instruccion_parseada(instruccionRespuesta);
		}else{
			log_error(LOG_OUTPUT, "No hay memorias asignadas a este criterio");
		}
	}else{
		log_error(LOG_OUTPUT, "La tabla no existe");
	}
}

void logicaDrop(Instruccion * instruccion){
	Consistencias consistencia = obtenerConsistencia(((Drop*)instruccion->instruccion_a_realizar)->nombre_tabla);

	if(consistencia > 0){
		t_list *memoriasAsoc = list_get(memorias, consistencia);
		int max = list_size(memoriasAsoc);
		int randomId = rand() % max + 1;
		Memoria * mem = NULL;
		pthread_mutex_lock(&mutexRecursosCompartidos);
		mem = list_get(memoriasAsoc, randomId - 1);
		pthread_mutex_unlock(&mutexRecursosCompartidos);
		if(mem != NULL){
			Instruccion * instruccionRespuesta = enviar_instruccion(mem->ip, mem->puerto, instruccion, KERNEL, T_INSTRUCCION);
			print_instruccion_parseada(instruccionRespuesta);
		}else{
			log_error(LOG_OUTPUT, "No hay memorias asignadas para ese criterio");
		}
	}else{
		log_error(LOG_OUTPUT, "La tabla no existe");
	}
}

void logicaJournal(){
	Consistencias consistencia;
	for(consistencia = EC; consistencia < DISP; consistencia++){
		list_iterate(list_get(memorias, consistencia), (void*)enviar_journal);
	}
}

void logicaDescribe(Instruccion * instruccion){

	if(((Describe*)instruccion->instruccion_a_realizar)->nombre_tabla != NULL){

		Consistencias consistencia = obtenerConsistencia(((Describe*)instruccion->instruccion_a_realizar)->nombre_tabla);

		if(consistencia > 0){

			t_list *memoriasAsoc = list_get(memorias, consistencia);

			if(memoriasAsoc != NULL){
				int max = list_size(memoriasAsoc);
				int randomId = rand() % max + 1;
				Memoria * mem = NULL;
				pthread_mutex_lock(&mutexRecursosCompartidos);
				mem = list_get(memoriasAsoc, randomId - 1);
				pthread_mutex_unlock(&mutexRecursosCompartidos);

				if(mem != NULL){
					Instruccion * intstruccionRespuesta = enviar_instruccion(mem->ip, mem->puerto, instruccion, KERNEL, T_INSTRUCCION);
					print_instruccion_parseada(intstruccionRespuesta);
				}
			}else{
				log_error(LOG_OUTPUT, "No hay memorias asignadas para ese criterio");
			}
		}else{
			log_error(LOG_OUTPUT, "La tabla no existe");
		}
	}else{
		Instruccion * intstruccionRespuesta = enviar_instruccion(IP_MEMORIA_PPAL, PUERTO_MEMORIA_PPAL, instruccion, KERNEL, T_INSTRUCCION);
		print_instruccion_parseada(intstruccionRespuesta);
	}
}

void logicaMetrics(){
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

