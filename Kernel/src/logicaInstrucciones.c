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
				proceso->file_descriptor = logicaCreate(create);
				break;

			case SELECT:;
				Select * select = (Select *) proceso->instruccionAProcesar->instruccion_a_realizar;
				proceso->file_descriptor = logicaSelect(select);
				fin = get_timestamp();
				diff = difftime(fin ,select->timestamp);
				proceso->segundosQueTardo = diff;
				break;

			case INSERT:;
				Insert * insert = (Insert *) proceso->instruccionAProcesar->instruccion_a_realizar;
				proceso->file_descriptor = logicaInsert(insert);
				fin = get_timestamp();
				diff = difftime(fin, insert->timestamp);
				proceso->segundosQueTardo = diff;
				break;

			case DROP:;
				Drop * drop = (Drop *) proceso->instruccionAProcesar->instruccion_a_realizar;
				proceso->file_descriptor = logicaDrop(drop);
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

int logicaCreate(Create * create){
	Instruccion * i = malloc(sizeof(Instruccion));
	i->instruccion = CREATE;
	i->instruccion_a_realizar = (void*) create;

	Instruccion * instruccionRespuestaCreate = enviar_instruccion(IP_MEMORIA_PPAL, PUERTO_MEMORIA_PPAL, i, KERNEL, T_INSTRUCCION);
	print_instruccion_parseada(instruccionRespuestaCreate);

	free(instruccionRespuestaCreate);
	return -100;
}

void logicaAdd(Add * add){
	Memoria * memoria = NULL;
	int tamTabla = dictionary_size(memoriasDisponibles);
	int i;

	for(i = 0; i <= tamTabla; i++){
		char * key = string_new();
		sprintf(key, "%d", i);
		memoria = (Memoria*)getMemoriaSafe(memoriasDisponibles, key);
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

int logicaSelect(Select * select){
	Instruccion * i = malloc(sizeof(Instruccion));
	i->instruccion_a_realizar = (void *) select;
	i->instruccion = SELECT;

	char * consistencia = obtenerConsistencia(select->nombre_tabla);
	Memoria * m = NULL;
	t_list *memoriasAsoc = memoriasAsoc = getMemoriasAsociadasSafe(memoriasAsociadas, consistencia);

	switch(string2consistencia(consistencia)){
		case SC || EC :;
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
			log_error(LOGGER, "Kernel.No deberia haber llegado aca chinguenguencha");
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
		return 1;
	}else{
		log_error(LOGGER, "Kernel. No hay memorias asignadas a este criterio %s", consistencia);
		free(i->instruccion_a_realizar);
		free(i);
		free(consistencia);
		return -100;
	}
}

int logicaInsert(Insert * insert){
	Instruccion * i = malloc(sizeof(Instruccion));
	i->instruccion_a_realizar = (void *) insert;
	i->instruccion = INSERT;

	char * consistencia = obtenerConsistencia(insert->nombre_tabla);

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
		return 1;
	}else{
		log_error(LOGGER, "Kernel. No hay memorias asignadas a este criterio %s", consistencia);
		free(i->instruccion_a_realizar);
		free(consistencia);
		free(i);
	}

	return -100;
}

int logicaDrop(Drop * drop){
	Instruccion * i = malloc(sizeof(Instruccion));
	i->instruccion_a_realizar = (void *) drop;
	i->instruccion = DROP;

	char * consistencia = obtenerConsistencia(drop->nombre_tabla);

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
		return 19;
	}else{
		log_error(LOGGER, "Kernel. No hay memorias asignadas a este criterio %s", consistencia);
		free(i);
		free(mem);
		free(consistencia);
	}
	return -100;
}

int logicaJournal(Journal * journal){
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
	return 1;

}

int logicaDescribe(Describe * describe){
	Instruccion * inst = malloc(sizeof(Instruccion));
	inst->instruccion_a_realizar = (void *) describe;
	inst->instruccion = DESCRIBE;

	if(describe->nombre_tabla != NULL){

		char * consistencia = obtenerConsistencia(describe->nombre_tabla);

		t_list *memoriasAsoc = getMemoriasAsociadasSafe(memoriasAsociadas, consistencia);
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
		free(inst->instruccion_a_realizar);
		free(inst);
		return 1;

	}else{
		int i;
		for(i = 0; i < dictionary_size(memoriasDisponibles); i++){
			char * key = string_new();
			sprintf(key, "%d", i);
			Memoria * m = getMemoriaSafe(memoriasDisponibles, key);
			if(m != NULL){
				Instruccion * intstruccionRespuesta = enviar_instruccion(m->ip, m->puerto, inst, KERNEL, T_INSTRUCCION);
				print_instruccion_parseada(intstruccionRespuesta);
				free(intstruccionRespuesta);
			}
		}
		free(inst);
		return 1;
	}
}

char * obtenerConsistencia(char * nombreTabla){
	Instruccion * instruccionDescribe = malloc(sizeof(Instruccion));
	Describe * describe = malloc(sizeof(Describe));

	describe->nombre_tabla = nombreTabla;

	instruccionDescribe->instruccion = DESCRIBE;
	instruccionDescribe->instruccion_a_realizar = (void *) describe;

	char * consistencia = malloc(3 * sizeof(char));

	//fixme: siempre se debe preguntar el describe a la memoria principal? que pasa si tengo varios procesos en exec y todos hacen describe?
	Instruccion * describeResponse = enviar_instruccion(IP_MEMORIA_PPAL,PUERTO_MEMORIA_PPAL,instruccionDescribe, KERNEL, T_INSTRUCCION);

	switch (describeResponse->instruccion) {
		case RETORNO:;
			Retorno_Generico * retornoGenerico = (Retorno_Generico*)  describeResponse->instruccion_a_realizar;

			switch (retornoGenerico->tipo_retorno) {
				case DATOS_DESCRIBE:;
					Retorno_Describe * retornoDescribe = (Retorno_Describe *) retornoGenerico->retorno;
					consistencia = consistencia2string(retornoDescribe->consistencia);
					break;
				default:
					break;
			}

			break;
		case ERROR:

			break;


		default:
			break;
	}

	free(describe->nombre_tabla);
	free(describe);
	free(instruccionDescribe->instruccion_a_realizar);
	free(instruccionDescribe);
	free(describeResponse->instruccion_a_realizar);
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
