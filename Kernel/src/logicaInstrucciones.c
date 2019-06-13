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
	Memoria * memoria = NULL;
	int tamTabla = dictionary_size(memoriasDisponibles);
	int i;

	for(i = 0; i <= tamTabla; i++){
		char * key = malloc(sizeof(char) * 4);
		sprintf(key, "%d", i);
		memoria = (Memoria*)getMemoriaSafe(memoriasDisponibles, key);
		free(key);
		if(memoria != NULL && memoria->idMemoria == add->memoria){
			break;
		}
	}

	if(add->consistencia == SHC){
		int i;
		for(i =0; i < list_size(memoriasHc); i++){

			Memoria * m = desencolarMemoria(memoriasHc, i);
			Instruccion * instruccionJournal = malloc(sizeof(Instruccion));
			instruccionJournal->instruccion = JOURNAL;
			Journal * j = malloc(sizeof(Journal));
			j->timestamp = get_timestamp();
			instruccionJournal->instruccion_a_realizar = (void *) j;

			Instruccion * instruccionRespuesta = enviar_instruccion(m->ip, m->puerto, instruccionJournal, KERNEL, T_INSTRUCCION);
			print_instruccion_parseada(instruccionRespuesta);
			free(instruccionJournal->instruccion_a_realizar);
			free(instruccionJournal);
			free(j);
		}
	}
	asignarConsistenciaAMemoria(memoria, add->consistencia);

}

void logicaSelect(Select * select){
	Instruccion * i = malloc(sizeof(Instruccion));
	i->instruccion_a_realizar = (void *) select;
	i->instruccion = SELECT;


	char * consistencia = obtenerConsistencia(select->nombre_tabla);
	if(consistencia != NULL){
		Memoria * m = NULL;
		t_list *memoriasAsoc = getMemoriasAsociadasSafe(memoriasAsociadas, consistencia);

		switch(string2consistencia(consistencia)){
			case SC || EC :;
				if(memoriasAsoc == NULL) {
					log_error(LOGGER, "No Existen Memorias asociadas\n");
					break;
				}
				int max = list_size(memoriasAsoc);
				int randomId = rand() % max + 1;

				pthread_mutex_lock(&mutexRecursosCompartidos);
				m = list_get(memoriasAsoc, randomId - 1);
				pthread_mutex_unlock(&mutexRecursosCompartidos);
				break;

			case SHC:;
				int indexTabla = generarHash(select->nombre_tabla, list_size(memoriasAsoc), select->key);
				pthread_mutex_lock(&mutexRecursosCompartidos);
				m = list_get(memoriasAsoc, indexTabla);
				pthread_mutex_unlock(&mutexRecursosCompartidos);
				break;

			default:;
				log_error(LOGGER, "Kernel.No deberia haber llegado aca chinguenguencha\n");
				break;
		}

		if(m != NULL){
			Instruccion * instruccionRespuesta = malloc(sizeof(Instruccion));
			instruccionRespuesta = enviar_instruccion(m->ip, m->puerto, i, KERNEL, T_INSTRUCCION);
			print_instruccion_parseada(instruccionRespuesta);

			free(instruccionRespuesta->instruccion_a_realizar);
			free(instruccionRespuesta);
			free(i->instruccion_a_realizar);
			free(i);
			free(consistencia);
		}else{
			log_error(LOGGER, "Kernel. No hay memorias asignadas a este criterio %s \n", consistencia);
			free(i->instruccion_a_realizar);
			free(i);
			free(consistencia);
		}
	}else{
		log_error(LOGGER, "Error al buscar la consistencia");
	}
}

void logicaInsert(Insert * insert){
	Instruccion * i = malloc(sizeof(Instruccion));
	i->instruccion_a_realizar = (void *) insert;
	i->instruccion = INSERT;

	char * consistencia = obtenerConsistencia(insert->nombre_tabla);
	if(consistencia != NULL){
		t_list *memoriasAsoc = getMemoriasAsociadasSafe(memoriasAsociadas, consistencia);
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
			free(consistencia);
		}else{
			log_error(LOGGER, "Kernel. No hay memorias asignadas a este criterio %s\n", consistencia);
			free(i->instruccion_a_realizar);
			free(consistencia);
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

	char * consistencia = obtenerConsistencia(drop->nombre_tabla);

	if(consistencia != NULL){
		t_list *memoriasAsoc = getMemoriasAsociadasSafe(memoriasAsociadas, consistencia);
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
			free(consistencia);
			free(instruccionRespuesta);
		}else{
			log_error(LOGGER, "Kernel. No hay memorias asignadas a este criterio %s\n", consistencia);
			free(i);
			free(mem);
			free(consistencia);
		}
	}else{
		log_error(LOGGER, "Error al buscar la consistencia");
	}
}

void logicaJournal(Journal * journal){
	Instruccion * inst = malloc(sizeof(Instruccion));
	inst->instruccion_a_realizar = (void *) journal;
	inst->instruccion = JOURNAL;

	t_list * memoriasSc = getMemoriasAsociadasSafe(memoriasAsociadas, "SC");
	t_list * memoriasHc = getMemoriasAsociadasSafe(memoriasAsociadas, "HC");
	t_list * memoriasEc = getMemoriasAsociadasSafe(memoriasAsociadas, "EC");

	int i;
	for(i = 0; i < list_size(memoriasEc); i++){
		Memoria * m = desencolarMemoria(memoriasEc, i);
		Instruccion * respuestaJournal = enviar_instruccion(m->ip, m->puerto, inst, KERNEL, T_INSTRUCCION);
		print_instruccion_parseada(respuestaJournal);
		free(respuestaJournal);
	}

	for(i = 0; i < list_size(memoriasSc); i++){
		Memoria * m = desencolarMemoria(memoriasSc, i);
		Instruccion * respuestaJournal = enviar_instruccion(m->ip, m->puerto, inst, KERNEL, T_INSTRUCCION);
		print_instruccion_parseada(respuestaJournal);
		free(respuestaJournal);

	}

	for(i = 0; i < list_size(memoriasHc); i++){
		Memoria * m = desencolarMemoria(memoriasHc, i);
		Instruccion * respuestaJournal = enviar_instruccion(m->ip, m->puerto, inst, KERNEL, T_INSTRUCCION);
		print_instruccion_parseada(respuestaJournal);
		free(respuestaJournal);
	}

	free(inst->instruccion_a_realizar);
	free(inst);

}

void logicaDescribe(Describe * describe){
	Instruccion * inst = malloc(sizeof(Instruccion));
	inst->instruccion_a_realizar = (void *) describe;
	inst->instruccion = DESCRIBE;

	if(describe->nombre_tabla != NULL){

		char * consistencia = obtenerConsistencia(describe->nombre_tabla);

		if(consistencia != NULL){

			t_list *memoriasAsoc = getMemoriasAsociadasSafe(memoriasAsociadas, consistencia);

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
				free(consistencia);
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
	free(inst->instruccion_a_realizar);
	free(inst);
}

char * obtenerConsistencia(char * nombreTabla){
	Instruccion * instruccionDescribe = malloc(sizeof(Instruccion));
	Describe * describe = malloc(sizeof(Describe));

	describe->nombre_tabla = nombreTabla;

	instruccionDescribe->instruccion = DESCRIBE;
	instruccionDescribe->instruccion_a_realizar = (void *) describe;
	//fixme: siempre se debe preguntar el describe a la memoria principal? que pasa si tengo varios procesos en exec y todos hacen describe?
	Instruccion * describeResponse = enviar_instruccion(IP_MEMORIA_PPAL,PUERTO_MEMORIA_PPAL,instruccionDescribe, KERNEL, T_INSTRUCCION);

	char * consistencia = NULL;

	switch(describeResponse->instruccion){
		case RETORNO:
			switch(((Retorno_Generico*)describeResponse->instruccion_a_realizar)->tipo_retorno){
				case SUCCESS:

				break;
				case DATOS_DESCRIBE:;
					t_list * describes = ((Describes*)((Retorno_Generico*)describeResponse->instruccion_a_realizar)->retorno)->lista_describes;
					pthread_mutex_lock(&mutexRecursosCompartidos);
					Retorno_Describe *describe = (Retorno_Describe *)list_get(describes, 0);
					pthread_mutex_unlock(&mutexRecursosCompartidos);

					consistencia = consistencia2string(describe->consistencia);
				break;
				case VALOR:;
				break;
				default:
					log_error(LOGGER, "Error procesar la consulta desde consola\n");
				break;
			}
		break;
		default:
			// vemos
		break;
	}


	//TODO: kevin esta haciendo una funcion para liberar, reemplazar aqui.
	free(describe);
	free(instruccionDescribe);
	return consistencia;
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
