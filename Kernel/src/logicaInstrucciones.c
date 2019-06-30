#include "kernel.h"

void acumularMetrics(int id_memoria, Instruction_set instruccion, t_timestamp tiempo);

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
		proceso->fin_proceso = true;
	}
}

void logicaCreate(Proceso * proceso){
	AcumMetrics *nuevoAcum = malloc(sizeof(AcumMetrics));
	nuevoAcum->instruccion = proceso->instruccionAProcesar->instruccion;
	t_timestamp metrics_tiempo = ((Select*) proceso->instruccionAProcesar->instruccion_a_realizar)->timestamp;
	Instruccion * instruccionRespuestaCreate = enviar_instruccion(IP_MEMORIA_PPAL, PUERTO_MEMORIA_PPAL, proceso->instruccionAProcesar, KERNEL, T_INSTRUCCION);
	print_instruccion_parseada(instruccionRespuestaCreate);
	t_timestamp timestamps = get_timestamp();
	nuevoAcum->tiempo = difftime(get_timestamp(), metrics_tiempo);
	nuevoAcum->id_memoria = 1;
	list_add(proceso->metricas, nuevoAcum);
}

void logicaAdd(Proceso * proceso){
	Memoria *memoria = get_memoria(((Add*)proceso->instruccionAProcesar->instruccion_a_realizar)->memoria, DISP);
	if(memoria == NULL){
		free(proceso->instruccionAProcesar->instruccion_a_realizar);
		free(proceso->instruccionAProcesar);
		log_error(LOG_OUTPUT, "No corresponde a un id de memoria válido, las validas son:");
		pthread_mutex_lock(&mutex_disp);
		list_iterate(lista_disp, (void*)mostrar_memoria);
		pthread_mutex_unlock(&mutex_disp);
		return;
	}
	if(((Add*)proceso->instruccionAProcesar->instruccion_a_realizar)->consistencia == SHC){
		pthread_mutex_lock(&mutex_shc);
		list_iterate(lista_shc, (void*)enviar_journal);
		pthread_mutex_unlock(&mutex_shc);
	}
	asignar_memoria_a_consistencia(memoria, ((Add*)proceso->instruccionAProcesar->instruccion_a_realizar)->consistencia);
	eliminar_memoria(memoria);
	free(proceso->instruccionAProcesar->instruccion_a_realizar);
	free(proceso->instruccionAProcesar);
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

void logicaSelect(Proceso * proceso){
	Consistencias consistencia = obtenerConsistencia(((Select*)proceso->instruccionAProcesar->instruccion_a_realizar)->nombre_tabla);
	if(consistencia > 0){
		t_list *memoriasAsoc = dame_lista_de_consistencia(consistencia);
		Memoria *m;
		switch(consistencia){
			case SC || EC :;
				int random = (rand() % (memoriasAsoc->elements_count + 1) - 1);
				m = get_memoria(random, consistencia);
				break;
			default:;
				int indexTabla = generarHash(((Select*)proceso->instruccionAProcesar->instruccion_a_realizar)->nombre_tabla, memoriasAsoc->elements_count, ((Select*)proceso->instruccionAProcesar->instruccion_a_realizar)->key);
				m = get_memoria(indexTabla, consistencia);
				break;
		}
		list_destroy_and_destroy_elements(memoriasAsoc, (void*)eliminar_memoria);
		if(m != NULL){
			Instruccion *instruccionRespuesta = enviar_instruccion(m->ip, m->puerto, proceso->instruccionAProcesar, KERNEL, T_INSTRUCCION);
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
	free(((Select*)proceso->instruccionAProcesar->instruccion_a_realizar)->nombre_tabla);
	free(proceso->instruccionAProcesar->instruccion_a_realizar);
	free(proceso->instruccionAProcesar);
	return;
}

void logicaInsert(Proceso *proceso){
	Consistencias consistencia = obtenerConsistencia(((Insert*) proceso->instruccionAProcesar->instruccion_a_realizar)->nombre_tabla);
	if(consistencia > 0){
		t_list *memoriasAsoc = dame_lista_de_consistencia(consistencia);
		int random = (rand() % (memoriasAsoc->elements_count + 1) - 1);
		Memoria * mem = get_memoria(random, consistencia);
		list_destroy_and_destroy_elements(memoriasAsoc, (void*)eliminar_memoria);
		if(mem != NULL){
			Instruccion * instruccionRespuesta = enviar_instruccion(mem->ip, mem->puerto, proceso->instruccionAProcesar, KERNEL, T_INSTRUCCION);
			print_instruccion_parseada(instruccionRespuesta);
			eliminar_memoria(mem);
			return;
		}else{
			log_error(LOG_OUTPUT, "No hay memorias asignadas a este criterio");
		}
	}else{
		log_error(LOG_OUTPUT, "La tabla no existe");
	}
	free(((Insert*) proceso->instruccionAProcesar->instruccion_a_realizar)->nombre_tabla);
	free(((Insert*) proceso->instruccionAProcesar->instruccion_a_realizar)->value);
	free(proceso->instruccionAProcesar->instruccion_a_realizar);
	free(proceso->instruccionAProcesar);
	return;
}

void logicaDrop(Proceso *proceso){
	Consistencias consistencia = obtenerConsistencia(((Drop*)proceso->instruccionAProcesar->instruccion_a_realizar)->nombre_tabla);
	if(consistencia > 0){
		t_list *memoriasAsoc = dame_lista_de_consistencia(consistencia);
		int random = (rand() % (memoriasAsoc->elements_count + 1) - 1);
		Memoria * mem = get_memoria(random, consistencia);
		list_destroy_and_destroy_elements(memoriasAsoc, (void*)eliminar_memoria);
		if(mem != NULL){
			Instruccion * instruccionRespuesta = enviar_instruccion(mem->ip, mem->puerto, proceso->instruccionAProcesar, KERNEL, T_INSTRUCCION);
			print_instruccion_parseada(instruccionRespuesta);
			eliminar_memoria(mem);
			return;
		}else{
			log_error(LOG_OUTPUT, "No hay memorias asignadas para ese criterio");
		}
	}else{
		log_error(LOG_OUTPUT, "La tabla no existe");
	}
	free(((Drop*)proceso->instruccionAProcesar->instruccion_a_realizar)->nombre_tabla);
	free(proceso->instruccionAProcesar->instruccion_a_realizar);
	free(proceso->instruccionAProcesar);
}

void logicaJournal(Proceso *proceso){
	Consistencias consistencia;
	for(consistencia = EC; consistencia < DISP; consistencia++){
		pthread_mutex_t mutex = dame_mutex_de_consistencia(consistencia);
		pthread_mutex_lock(&mutex);
		t_list *lista = dame_lista_de_consistencia(consistencia);
		list_iterate(lista, (void*)enviar_journal);
		pthread_mutex_unlock(&mutex);
	}
	free(proceso->instruccionAProcesar->instruccion_a_realizar);
	free(proceso->instruccionAProcesar);
}

void logicaDescribe(Proceso *proceso){

	if(((Describe*)proceso->instruccionAProcesar->instruccion_a_realizar)->nombre_tabla != NULL){

		Consistencias consistencia = obtenerConsistencia(((Describe*)proceso->instruccionAProcesar->instruccion_a_realizar)->nombre_tabla);

		if(consistencia > 0){
			t_list *memoriasAsoc = dame_lista_de_consistencia(consistencia);
			int random = (rand() % (memoriasAsoc->elements_count + 1) - 1);
			Memoria * mem = get_memoria(random, consistencia);
			list_destroy_and_destroy_elements(memoriasAsoc, (void*)eliminar_memoria);
			if(mem != NULL){
				Instruccion * intstruccionRespuesta = enviar_instruccion(mem->ip, mem->puerto, proceso->instruccionAProcesar, KERNEL, T_INSTRUCCION);
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
		Instruccion * intstruccionRespuesta = enviar_instruccion(IP_MEMORIA_PPAL, PUERTO_MEMORIA_PPAL, proceso->instruccionAProcesar, KERNEL, T_INSTRUCCION);
		print_instruccion_parseada(intstruccionRespuesta);
		return;
	}
	free(((Describe*)proceso->instruccionAProcesar->instruccion_a_realizar)->nombre_tabla);
	free(proceso->instruccionAProcesar->instruccion_a_realizar);
	free(proceso->instruccionAProcesar);
}

void logicaMetrics(Proceso * proceso){
	pthread_mutex_lock(&mutex_metrics);
	print_metrics();
	pthread_mutex_unlock(&mutex_metrics);
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

void acumularMetrics(int id_memoria, Instruction_set instruccion, t_timestamp tiempo){

	AcumMetrics* nuevoAcum = malloc(sizeof(AcumMetrics));
	nuevoAcum->id_memoria = id_memoria;
	nuevoAcum->instruccion = instruccion;
	nuevoAcum->tiempo = tiempo;

	pthread_mutex_lock(&mutex_metrics);
	list_add(acum30sMetrics, nuevoAcum);
	pthread_mutex_unlock(&mutex_metrics);
}
