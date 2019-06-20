#include "kernel.h"

Proceso * logicaRun(Run * run, Proceso * proceso){
	char * proximainstruccionChar = leer_linea(run->path, proceso->numeroInstruccion);

	while(proximainstruccionChar != NULL && proceso->quantumProcesado < QUANTUM){

		Instruccion * instruccionAProcesar = parser_lql((char*)proximainstruccionChar, KERNEL);
		proceso->instruccionAProcesar = instruccionAProcesar;
		Instruction_set tipoInstruccionAProcesar = instruccionAProcesar->instruccion;

		time_t fin;
		int diff;
		switch(tipoInstruccionAProcesar){
			case CREATE:;
				Create * create = (Create *) proceso->instruccionAProcesar->instruccion_a_realizar;
				logicaCreate(create);
				break;

			case SELECT:;
				Select * select = (Select *) proceso->instruccionAProcesar->instruccion_a_realizar;
				logicaSelect(select);

				fin = get_timestamp();
				diff = difftime(fin ,select->timestamp);
				proceso->segundosQueTardo = diff;
				break;

			case INSERT:;
				Insert * insert = (Insert *) proceso->instruccionAProcesar->instruccion_a_realizar;
				logicaInsert(insert);

				fin = get_timestamp();
				diff = difftime(fin, insert->timestamp);
				proceso->segundosQueTardo = diff;
				break;

			case DROP:;
				Drop * drop = (Drop *) proceso->instruccionAProcesar->instruccion_a_realizar;
				logicaDrop(drop);
				break;

			case ERROR:;
				return proceso;

			default:
				break;

		}
		proceso->numeroInstruccion += 1;
		proceso->quantumProcesado += 1;

		proximainstruccionChar = leer_linea(run->path, proceso->numeroInstruccion);
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

void logicaCreate(Create * create){
	Instruccion * i = malloc(sizeof(Instruccion));
	i->instruccion = CREATE;
	i->instruccion_a_realizar = (void*) create;

	Instruccion * instruccionRespuestaCreate = enviar_instruccion(IP_MEMORIA_PPAL, PUERTO_MEMORIA_PPAL, i, KERNEL, T_INSTRUCCION);
	print_instruccion_parseada(instruccionRespuestaCreate);

	free(instruccionRespuestaCreate);
}

void logicaAdd(Add * add){
	Memoria *memoria = getMemoria(list_get(memorias, DISP), add->memoria);
	if(memoria == NULL){
		printf("Las memorias disponibles son: ");
		list_iterate(list_get(memorias, DISP), (void*)mostrarId);
		printf("\n");
		return;
	}
	if(add->consistencia == SHC){
		list_iterate(list_get(memorias, SHC), (void*)enviar_journal);
	}
	asignarConsistenciaAMemoria(memoria, add->consistencia);

}

void enviar_journal(Memoria *memoria){
	Instruccion * instruccion = malloc(sizeof(Instruccion));
	instruccion->instruccion = JOURNAL;
	Journal * journal = malloc(sizeof(Journal));
	journal->timestamp = get_timestamp();
	instruccion->instruccion_a_realizar = journal;
	Instruccion * instruccionRespuesta = enviar_instruccion(memoria->ip, memoria->puerto, instruccion, KERNEL, T_INSTRUCCION);
	free(instruccion->instruccion_a_realizar);
	free(instruccion);
	free(instruccionRespuesta->instruccion_a_realizar);
	free(instruccionRespuesta);
}

void logicaSelect(Select * select){
	Consistencias consistencia = obtenerConsistencia(select->nombre_tabla);
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
					int indexTabla = generarHash(select->nombre_tabla, list_size(memoriasAsoc), select->key);
					pthread_mutex_lock(&mutexRecursosCompartidos);
					m = list_get(memoriasAsoc, indexTabla);
					pthread_mutex_unlock(&mutexRecursosCompartidos);
					break;
			}
			if(m != NULL){
				Instruccion * i = malloc(sizeof(Instruccion));
				i->instruccion_a_realizar = (void *) select;
				i->instruccion = SELECT;
				Instruccion *instruccionRespuesta = enviar_instruccion(m->ip, m->puerto, i, KERNEL, T_INSTRUCCION);
				print_instruccion_parseada(instruccionRespuesta);
			}
		}else{
			log_error(LOG_ERROR, "No hay memorias asignadas para ese criterio");

		}
	}else{
		log_error(LOG_ERROR, "Esa tabla no existe");
	}
}

void logicaInsert(Insert * insert){
	Instruccion * i = malloc(sizeof(Instruccion));
	i->instruccion_a_realizar = (void *) insert;
	i->instruccion = INSERT;

	Consistencias consistencia = obtenerConsistencia(insert->nombre_tabla);
	if(consistencia > 0){
		t_list *memoriasAsoc = list_get(memorias, consistencia);
		int max = list_size(memoriasAsoc);
		int randomId = rand() % max + 1;
		Memoria * mem = NULL;
		pthread_mutex_lock(&mutexRecursosCompartidos);
		mem = list_get(memoriasAsoc, randomId - 1);
		pthread_mutex_unlock(&mutexRecursosCompartidos);

		if(mem != NULL){
			Instruccion * instruccionRespuesta = enviar_instruccion(mem->ip, mem->puerto, i, KERNEL, T_INSTRUCCION);
			print_instruccion_parseada(instruccionRespuesta);

			free(i->instruccion_a_realizar);
			free(i);
			free(instruccionRespuesta->instruccion_a_realizar);
			free(instruccionRespuesta);
		}else{
			log_error(LOGGER, "Kernel. No hay memorias asignadas a este criterio %s\n", consistencia);
			free(i->instruccion_a_realizar);
			free(i);
		}
	}else{
		log_error(LOGGER, "Error al buscar la consistencia");
	}
}

void logicaDrop(Drop * drop){
	Instruccion * i = malloc(sizeof(Instruccion));
	i->instruccion_a_realizar = (void *) drop;
	i->instruccion = DROP;

	Consistencias consistencia = obtenerConsistencia(drop->nombre_tabla);

	if(consistencia > 0){
		t_list *memoriasAsoc = list_get(memorias, consistencia);
		int max = list_size(memoriasAsoc);
		int randomId = rand() % max + 1;
		Memoria * mem = NULL;
		pthread_mutex_lock(&mutexRecursosCompartidos);
		mem = list_get(memoriasAsoc, randomId - 1);
		pthread_mutex_unlock(&mutexRecursosCompartidos);


		if(mem != NULL){
			Instruccion * instruccionRespuesta = enviar_instruccion(mem->ip, mem->puerto, i, KERNEL, T_INSTRUCCION);
			free(i);
			free(mem);
			free(instruccionRespuesta);
		}else{
			log_error(LOGGER, "Kernel. No hay memorias asignadas a este criterio %s\n", consistencia);
			free(i);
			free(mem);
		}
	}else{
		log_error(LOGGER, "Error al buscar la consistencia");
	}
}

void logicaJournal(Journal * journal){
	Consistencias consistencia;
	for(consistencia = EC; consistencia < DISP; consistencia++){
		list_iterate(list_get(memorias, consistencia), (void*)enviar_journal);
	}

}

void logicaDescribe(Describe * describe){
	Instruccion * inst = malloc(sizeof(Instruccion));
	inst->instruccion_a_realizar = (void *) describe;
	inst->instruccion = DESCRIBE;

	if(describe->nombre_tabla != NULL){

		Consistencias consistencia = obtenerConsistencia(describe->nombre_tabla);

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
					Instruccion * intstruccionRespuesta = enviar_instruccion(mem->ip, mem->puerto, inst, KERNEL, T_INSTRUCCION);
					print_instruccion_parseada(intstruccionRespuesta);
					free(intstruccionRespuesta);
				}
			}else{
				log_error(LOGGER, "No hay memorias asignadas a la consistencia correspondiente");
			}
		}
	}else{
		Instruccion * intstruccionRespuesta = enviar_instruccion(IP_MEMORIA_PPAL, PUERTO_MEMORIA_PPAL, inst, KERNEL, T_INSTRUCCION);
		print_instruccion_parseada(intstruccionRespuesta);
		/*liberar con instruccion kevin*/
		free(intstruccionRespuesta);
	}
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

