#include "kernel.h"

void logicaRun(Proceso * proceso){
	char * proximainstruccionChar = leer_linea(((Run*)proceso->instruccion->instruccion_a_realizar)->path, proceso->numeroInstruccion);

	if(proximainstruccionChar != NULL){
		proceso->instruccionAProcesar = parser_lql(proximainstruccionChar, KERNEL);
		switch(proceso->instruccionAProcesar->instruccion){
			case CREATE:;
				logicaCreate(proceso);
				break;

			case SELECT:;
				logicaSelect(proceso);
				break;

			case INSERT:;
				logicaInsert(proceso);
				break;

			case DROP:;
				logicaDrop(proceso);
				break;

			case DESCRIBE:;
				logicaDescribe(proceso);
				break;

			case METRICS:;
				logicaMetrics(proceso);
				break;

			case JOURNAL:;
				logicaJournal(proceso);
				break;

			case ADD:;
				logicaAdd(proceso);
				break;
			case ERROR:;
				log_error(LOG_ERROR, "Error en la linea: %d del script %s", (proceso->numeroInstruccion + 1), ((Run*)proceso->instruccion->instruccion_a_realizar)->path);
				free(proceso->instruccionAProcesar->instruccion_a_realizar);
				free(proceso->instruccionAProcesar);
				break;
			default:
				break;
		}
		proceso->numeroInstruccion += 1;
		proceso->quantumProcesado += 1;
	}else{
		free(((Run*)proceso->instruccion->instruccion_a_realizar)->path);
		free(proceso->instruccion->instruccion_a_realizar);
		free(proceso->instruccion);
		proceso->fin_proceso = true;
	}
}

void logicaCreate(Proceso * proceso){
	t_list *memoriasAsoc = dame_lista_de_consistencia(((Create*) proceso->instruccionAProcesar->instruccion_a_realizar)->consistencia);
	if(memoriasAsoc->elements_count > 0){
		Instruction_set inst = proceso->instruccionAProcesar->instruccion;
		t_timestamp metrics_tiempo = ((Create*) proceso->instruccionAProcesar->instruccion_a_realizar)->timestamp;
		t_list * memoriasAsoc = list_duplicate_all(lista_disp, (void*)duplicar_memoria, mutex_disp);
		int random = rand() % memoriasAsoc->elements_count;
		Memoria * mem = duplicar_memoria((Memoria*)list_get(memoriasAsoc,random));
		Instruccion * instruccionRespuesta = enviar_instruccion(IP_MEMORIA_PPAL, PUERTO_MEMORIA_PPAL, proceso->instruccionAProcesar, KERNEL, T_INSTRUCCION);
		if(instruccionRespuesta->instruccion != ERROR){
			AcumMetrics *nuevoAcum = malloc(sizeof(AcumMetrics));
			nuevoAcum->tiempo = difftime(get_timestamp(), metrics_tiempo);
			nuevoAcum->instruccion = inst;
			nuevoAcum->id_memoria = mem->idMemoria;
			list_add(proceso->metricas, nuevoAcum);
			realizarDescribeGeneral();
		}
		print_instruccion_parseada(instruccionRespuesta);
		list_destroy_and_destroy_elements(memoriasAsoc, (void*)eliminar_memoria);
		eliminar_memoria(mem);
	}else{
		log_error(LOG_ERROR, "No hay memorias asignadas a este criterio");
	}
}

void logicaAdd(Proceso * proceso){
	Memoria *memoria = get_memoria(((Add*)proceso->instruccionAProcesar->instruccion_a_realizar)->memoria, DISP);
	if(memoria == NULL){
		free(proceso->instruccionAProcesar->instruccion_a_realizar);
		free(proceso->instruccionAProcesar);
		log_error(LOG_DEBUG, "No corresponde a un id de memoria válido, las validas son:");
		pthread_mutex_lock(&mutex_disp);
		list_iterate(lista_disp, (void*)mostrar_memoria);
		pthread_mutex_unlock(&mutex_disp);
		return;
	}
	if(((Add*)proceso->instruccionAProcesar->instruccion_a_realizar)->consistencia == SHC){
		pthread_mutex_lock(&mutex_shc);
		Memoria * memoria = NULL;
		int aux = 0;
		memoria = list_get(lista_shc, aux);
		t_timestamp initial_time;
		Instruccion * instruccionRespuesta;
		while(memoria != NULL){
			Instruccion * instruccion = malloc(sizeof(Instruccion));
			instruccion->instruccion = JOURNAL;
			Journal * journal = malloc(sizeof(Journal));
			journal->timestamp = get_timestamp();
			instruccion->instruccion_a_realizar = journal;
			initial_time = get_timestamp();
			instruccionRespuesta = enviar_instruccion(memoria->ip, memoria->puerto, instruccion, KERNEL, T_INSTRUCCION);
			if(instruccionRespuesta->instruccion != ERROR){
				AcumMetrics *nuevoAcum = malloc(sizeof(AcumMetrics));
				nuevoAcum->tiempo = difftime(get_timestamp(), initial_time);
				nuevoAcum->instruccion = JOURNAL;
				nuevoAcum->id_memoria = memoria->idMemoria;
				list_add(proceso->metricas, nuevoAcum);
			}
			free_retorno(instruccionRespuesta);
			aux++;
		}
		pthread_mutex_unlock(&mutex_shc);
	}
	asignar_memoria_a_consistencia(memoria, ((Add*)proceso->instruccionAProcesar->instruccion_a_realizar)->consistencia);
	eliminar_memoria(memoria);
	free(proceso->instruccionAProcesar->instruccion_a_realizar);
	free(proceso->instruccionAProcesar);
}

void logicaSelect(Proceso * proceso){
	Consistencias consistencia = obtenerConsistencia(((Select*)proceso->instruccionAProcesar->instruccion_a_realizar)->nombre_tabla);
	if(consistencia != NOT_FOUND){
		t_list *memoriasAsoc = dame_lista_de_consistencia(consistencia);
		Memoria * mem = NULL;
		if(memoriasAsoc->elements_count > 0){
			switch(consistencia){
				case SC || EC :;
					int random = rand() % memoriasAsoc->elements_count;
					mem = duplicar_memoria((Memoria*)list_get(memoriasAsoc,random));
					break;

				default:;
					int indexTabla = generarHash(((Select*)proceso->instruccionAProcesar->instruccion_a_realizar)->nombre_tabla, memoriasAsoc->elements_count);
					mem = duplicar_memoria((Memoria*)list_get(memoriasAsoc, indexTabla));
					break;
			}
		}
		list_destroy_and_destroy_elements(memoriasAsoc, (void*)eliminar_memoria);
		if(mem != NULL){
			Instruction_set inst = proceso->instruccionAProcesar->instruccion;
			t_timestamp metrics_tiempo = ((Select*) proceso->instruccionAProcesar->instruccion_a_realizar)->timestamp;
			Instruccion *instruccionRespuesta = enviar_instruccion(mem->ip, mem->puerto, proceso->instruccionAProcesar, KERNEL, T_INSTRUCCION);
			if(instruccionRespuesta->instruccion != ERROR){
				AcumMetrics *nuevoAcum = malloc(sizeof(AcumMetrics));
				nuevoAcum->tiempo = difftime(get_timestamp(), metrics_tiempo);
				nuevoAcum->instruccion = inst;
				nuevoAcum->id_memoria = mem->idMemoria;
				list_add(proceso->metricas, nuevoAcum);
			}
			print_instruccion_parseada(instruccionRespuesta);
			eliminar_memoria(mem);
			return;
		}
		else{
			log_error(LOG_ERROR, "No hay memorias asignadas a este criterio");
		}
	}else{
		log_error(LOG_ERROR, "Esa tabla no existe");
	}
	free(((Select*)proceso->instruccionAProcesar->instruccion_a_realizar)->nombre_tabla);
	free(proceso->instruccionAProcesar->instruccion_a_realizar);
	free(proceso->instruccionAProcesar);
	return;
}

void logicaInsert(Proceso *proceso){
	Consistencias consistencia = obtenerConsistencia(((Insert*) proceso->instruccionAProcesar->instruccion_a_realizar)->nombre_tabla);
	if(consistencia != NOT_FOUND){
		t_list *memoriasAsoc = dame_lista_de_consistencia(consistencia);
		Memoria * mem = NULL;
		if(memoriasAsoc->elements_count > 0){
			switch(consistencia){
				case SC || EC :;
					int random = rand() % memoriasAsoc->elements_count;
					mem = duplicar_memoria((Memoria*)list_get(memoriasAsoc,random));
					break;
				default:;
					int indexTabla = generarHash(((Insert*)proceso->instruccionAProcesar->instruccion_a_realizar)->nombre_tabla, memoriasAsoc->elements_count);
					mem = duplicar_memoria((Memoria*)list_get(memoriasAsoc, indexTabla));
					break;
			}
		}
		list_destroy_and_destroy_elements(memoriasAsoc, (void*)eliminar_memoria);
		if(mem != NULL){
			Instruction_set inst = proceso->instruccionAProcesar->instruccion;
			t_timestamp metrics_tiempo = ((Insert*) proceso->instruccionAProcesar->instruccion_a_realizar)->timestamp;
			Instruccion * instruccionRespuesta = enviar_instruccion(mem->ip, mem->puerto, proceso->instruccionAProcesar, KERNEL, T_INSTRUCCION);
			if(instruccionRespuesta->instruccion != ERROR){
				AcumMetrics *nuevoAcum = malloc(sizeof(AcumMetrics));
				nuevoAcum->tiempo = difftime(get_timestamp(), metrics_tiempo);
				nuevoAcum->instruccion = inst;
				nuevoAcum->id_memoria = mem->idMemoria;
				list_add(proceso->metricas, nuevoAcum);
			}
			print_instruccion_parseada(instruccionRespuesta);
			eliminar_memoria(mem);
			return;
		}else{
			log_error(LOG_ERROR, "No hay memorias asignadas a este criterio");
		}
	}else{
		log_error(LOG_ERROR, "La tabla no existe");
	}
	free(((Insert*) proceso->instruccionAProcesar->instruccion_a_realizar)->nombre_tabla);
	free(((Insert*) proceso->instruccionAProcesar->instruccion_a_realizar)->value);
	free(proceso->instruccionAProcesar->instruccion_a_realizar);
	free(proceso->instruccionAProcesar);
	return;
}

void logicaDrop(Proceso *proceso){
	Consistencias consistencia = obtenerConsistencia(((Drop*)proceso->instruccionAProcesar->instruccion_a_realizar)->nombre_tabla);
	if(consistencia != NOT_FOUND){
		t_list *memoriasAsoc = dame_lista_de_consistencia(consistencia);
		Memoria * mem = NULL;
		if(memoriasAsoc->elements_count > 0){
			switch(consistencia){
				case SC || EC :;
					int random = rand() % memoriasAsoc->elements_count;
					mem = duplicar_memoria((Memoria*)list_get(memoriasAsoc,random));
					break;
				default:;
					int indexTabla = generarHash(((Drop*)proceso->instruccionAProcesar->instruccion_a_realizar)->nombre_tabla, memoriasAsoc->elements_count);
					mem = duplicar_memoria((Memoria*)list_get(memoriasAsoc, indexTabla));
					break;
			}
		}
		list_destroy_and_destroy_elements(memoriasAsoc, (void*)eliminar_memoria);
		if(mem != NULL){
			Instruction_set inst = proceso->instruccionAProcesar->instruccion;
			t_timestamp metrics_tiempo = ((Drop*) proceso->instruccionAProcesar->instruccion_a_realizar)->timestamp;
			Instruccion * instruccionRespuesta = enviar_instruccion(mem->ip, mem->puerto, proceso->instruccionAProcesar, KERNEL, T_INSTRUCCION);
			if(instruccionRespuesta->instruccion != ERROR){
				AcumMetrics *nuevoAcum = malloc(sizeof(AcumMetrics));
				nuevoAcum->tiempo = difftime(get_timestamp(), metrics_tiempo);
				nuevoAcum->instruccion = inst;
				nuevoAcum->id_memoria = mem->idMemoria;
				list_add(proceso->metricas, nuevoAcum);
			}
			print_instruccion_parseada(instruccionRespuesta);
			eliminar_memoria(mem);
			return;
		}else{
			log_error(LOG_ERROR, "No hay memorias asignadas para ese criterio");
		}
	}else{
		log_error(LOG_ERROR, "La tabla no existe");
	}
	free(((Drop*)proceso->instruccionAProcesar->instruccion_a_realizar)->nombre_tabla);
	free(proceso->instruccionAProcesar->instruccion_a_realizar);
	free(proceso->instruccionAProcesar);
}

void logicaJournal(Proceso *proceso){
	Memoria *memoria = NULL;
	int aux = 0;
	Consistencias consistencia;
	t_list * lista;
	pthread_mutex_t mutex;
	t_timestamp initial_time;
	Instruccion * instruccionRespuesta;
	for(consistencia = EC; consistencia < DISP; consistencia ++){
		mutex = dame_mutex_de_consistencia(consistencia);
		pthread_mutex_lock(&mutex);
		lista = dame_lista_de_consistencia(consistencia);
		memoria = list_get(lista, aux);
		while(memoria != NULL){
			Instruccion * instruccion = malloc(sizeof(Instruccion));
			instruccion->instruccion = JOURNAL;
			Journal * journal = malloc(sizeof(Journal));
			journal->timestamp = get_timestamp();
			instruccion->instruccion_a_realizar = journal;
			initial_time = get_timestamp();
			instruccionRespuesta = enviar_instruccion(memoria->ip, memoria->puerto, instruccion, KERNEL, T_INSTRUCCION);
			if(instruccionRespuesta->instruccion != ERROR){
				AcumMetrics *nuevoAcum = malloc(sizeof(AcumMetrics));
				nuevoAcum->tiempo = difftime(get_timestamp(), initial_time);
				nuevoAcum->instruccion = proceso->instruccionAProcesar->instruccion;
				nuevoAcum->id_memoria = memoria->idMemoria;
				list_add(proceso->metricas, nuevoAcum);
			}
			free_retorno(instruccionRespuesta);
			aux++;
		}
		pthread_mutex_unlock(&mutex);
		list_destroy_and_destroy_elements(lista, (void*)eliminar_memoria);
		aux = 0;
	}
	free(proceso->instruccionAProcesar->instruccion_a_realizar);
	free(proceso->instruccionAProcesar);
}

void logicaDescribe(Proceso *proceso){
	if(((Describe*)proceso->instruccionAProcesar->instruccion_a_realizar)->nombre_tabla != NULL){
		Consistencias consistencia = obtenerConsistencia(((Describe*)proceso->instruccionAProcesar->instruccion_a_realizar)->nombre_tabla);
		Memoria * mem = NULL;
		if(consistencia != NOT_FOUND){
			t_list *memoriasAsoc = dame_lista_de_consistencia(consistencia);
			if(memoriasAsoc->elements_count > 0){
				switch(consistencia){
					case SC || EC :;
						int random = rand() % memoriasAsoc->elements_count;
						mem = duplicar_memoria((Memoria*)list_get(memoriasAsoc,random));
						break;
					default:;
						int indexTabla = generarHash(((Select*)proceso->instruccionAProcesar->instruccion_a_realizar)->nombre_tabla, memoriasAsoc->elements_count);
						mem = duplicar_memoria((Memoria*)list_get(memoriasAsoc, indexTabla));
						break;
				}
			}
			list_destroy_and_destroy_elements(memoriasAsoc, (void*)eliminar_memoria);
			if(mem != NULL){
				Instruction_set inst = proceso->instruccionAProcesar->instruccion;
				t_timestamp metrics_tiempo = ((Describe*) proceso->instruccionAProcesar->instruccion_a_realizar)->timestamp;
				Instruccion * instruccionRespuesta = enviar_instruccion(mem->ip, mem->puerto, proceso->instruccionAProcesar, KERNEL, T_INSTRUCCION);
				if(instruccionRespuesta->instruccion != ERROR){
					AcumMetrics *nuevoAcum = malloc(sizeof(AcumMetrics));
					nuevoAcum->tiempo = difftime(get_timestamp(), metrics_tiempo);
					nuevoAcum->instruccion = inst;
					nuevoAcum->id_memoria = mem->idMemoria;
					list_add(proceso->metricas, nuevoAcum);
				}
				print_instruccion_parseada(instruccionRespuesta);
				eliminar_memoria(mem);
				return;
			}else{
				log_error(LOG_ERROR, "No hay memorias asignadas para ese criterio");
			}
		}else{
			log_error(LOG_ERROR, "La tabla no existe");
		}
	}else{
		Instruction_set inst = proceso->instruccionAProcesar->instruccion;
		t_timestamp metrics_tiempo = ((Describe*) proceso->instruccionAProcesar->instruccion_a_realizar)->timestamp;
		Instruccion * instruccionRespuesta = enviar_instruccion(IP_MEMORIA_PPAL, PUERTO_MEMORIA_PPAL, proceso->instruccionAProcesar, KERNEL, T_INSTRUCCION);
		if(instruccionRespuesta->instruccion != ERROR){
			AcumMetrics *nuevoAcum = malloc(sizeof(AcumMetrics));
			nuevoAcum->tiempo = difftime(get_timestamp(), metrics_tiempo);
			nuevoAcum->instruccion = inst;
			nuevoAcum->id_memoria = 1;
			list_add(proceso->metricas, nuevoAcum);
		}
		print_instruccion_parseada(instruccionRespuesta);
		return;
	}
	free(((Describe*)proceso->instruccionAProcesar->instruccion_a_realizar)->nombre_tabla);
	free(proceso->instruccionAProcesar->instruccion_a_realizar);
	free(proceso->instruccionAProcesar);
}

void logicaMetrics(Proceso * proceso){
	loguear_metrics(LOG_OUTPUT);
	free(proceso->instruccionAProcesar->instruccion_a_realizar);
	free(proceso->instruccionAProcesar);

}

Consistencias obtenerConsistencia(char * nombreTabla){
	bool criterioNombre(Table_Metadata* tabla){
		return !strcmp(tabla->tablename,nombreTabla);
	}
	pthread_mutex_lock(&lista_de_tablas_mx);
	Table_Metadata * found = list_find(lista_de_tablas,(void*)criterioNombre);
	pthread_mutex_unlock(&lista_de_tablas_mx);
	if(found == NULL) {
		return NOT_FOUND;
	} else {
		return found->consistencia;
	}
}

int generarHash(char * nombreTabla, int tamLista){
	int hash = 0;
	int i;
	for(i = 0; i < strlen(nombreTabla); i++){
		hash += nombreTabla[i];
	}
	return hash % tamLista;
}
