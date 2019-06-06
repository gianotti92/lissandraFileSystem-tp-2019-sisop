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
	char* consistencia = consistencia2string(create->consistencia);
	llenarTablasPorConsistencia(create->nombre_tabla,consistencia);
	t_list * listaMemoriasAsoc = getMemoriasAsociadasSafe(memoriasAsociadas,consistencia);
	free(consistencia);
	if(listaMemoriasAsoc != NULL && list_size(listaMemoriasAsoc) > 0){

		Memoria * m = (Memoria * ) desencolarMemoria(listaMemoriasAsoc);
		if(m != NULL){
			int fd = enviarInstruccionLuqui(m->ip, m->puerto, i, KERNEL);
			free(i);
			return fd;
		}else{
			log_error(LOGGER, "Error al extraer memorias asociadas");
			free(i);
			return -100;
		}
	}
	free(i);
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
	asignarConsistenciaAMemoria(memoria, add->consistencia);
}

int logicaSelect(Select * select){

	Instruccion * i = malloc(sizeof(Instruccion));
	i->instruccion_a_realizar = (void *) select;
	i->instruccion = SELECT;

	char * consistencia = (char*)getTablasSafe(tablasPorConsistencia, select->nombre_tabla);

	t_list *memoriasAsoc = getMemoriasAsociadasSafe(memoriasAsociadas, consistencia);
	int max = list_size(memoriasAsoc);
	int randomId = rand() % max + 1;
	Memoria * m = NULL;
	pthread_mutex_lock(&mutexRecursosCompartidos);
	m = list_get(memoriasAsoc, randomId - 1);
	pthread_mutex_unlock(&mutexRecursosCompartidos);


	if(m != NULL){
		int fd = enviarInstruccionLuqui(m->ip, m->puerto,
					i,
					KERNEL);
		free(i);

		return fd;
	}
	free(i);
	return -100;
}

int logicaInsert(Insert * insert){
	Instruccion * i = malloc(sizeof(Instruccion));
	i->instruccion_a_realizar = (void *) insert;
	i->instruccion = INSERT;

	char * consistencia = (char*)getTablasSafe(tablasPorConsistencia, insert->nombre_tabla);

	t_list *memoriasAsoc = getMemoriasAsociadasSafe(memoriasAsociadas, consistencia);
	int max = list_size(memoriasAsoc);
	int randomId = rand() % max + 1;
	Memoria * mem = NULL;
	pthread_mutex_lock(&mutexRecursosCompartidos);
	mem = list_get(memoriasAsoc, randomId - 1);
	pthread_mutex_unlock(&mutexRecursosCompartidos);

	if(mem != NULL){
		int fd = enviarInstruccionLuqui(mem->ip, mem->puerto, i, KERNEL);

		free(i);
		return fd;
	}
	free(i);
	return -100;
}

int logicaDrop(Drop * drop){
	Instruccion * i = malloc(sizeof(Instruccion));
	i->instruccion_a_realizar = (void *) drop;
	i->instruccion = DROP;

	pthread_mutex_lock(&mutexRecursosCompartidos);
	char * consistencia = (char*)dictionary_remove(tablasPorConsistencia, drop->nombre_tabla);
	pthread_mutex_unlock(&mutexRecursosCompartidos);

	t_list *memoriasAsoc = getMemoriasAsociadasSafe(memoriasAsociadas, consistencia);
	int max = list_size(memoriasAsoc);
	int randomId = rand() % max + 1;
	Memoria * mem = NULL;
	pthread_mutex_lock(&mutexRecursosCompartidos);
	mem = list_get(memoriasAsoc, randomId - 1);
	pthread_mutex_unlock(&mutexRecursosCompartidos);


	if(mem != NULL){
		int fd = enviarInstruccionLuqui(mem->ip, mem->puerto, i, KERNEL);
		free(i);
		free(mem);
		return fd;
	}

	free(i);
	return -100;

}

int logicaJournal(Journal * journal){
	Instruccion * i = malloc(sizeof(Instruccion));
	i->instruccion_a_realizar = (void *) journal;
	i->instruccion = JOURNAL;

	t_list * memoriasSc = getMemoriasAsociadasSafe(memoriasAsociadas, "SC");
	t_list * memoriasHc = getMemoriasAsociadasSafe(memoriasAsociadas, "HC");
	t_list * memoriasEc = getMemoriasAsociadasSafe(memoriasAsociadas, "EC");

	Memoria * m = desencolarMemoria(memoriasEc);
	while(m != NULL){
		m = desencolarMemoria(memoriasEc);

		enviarInstruccionLuqui(m->ip, m->puerto, i, KERNEL);
	}

	m = desencolarMemoria(memoriasHc);
	while(m != NULL){
		enviarInstruccionLuqui(m->ip, m->puerto, i, KERNEL);
		m = desencolarMemoria(memoriasHc);
	}

	m = desencolarMemoria(memoriasSc);
	while(m != NULL){
		enviarInstruccionLuqui(m->ip, m->puerto, i, KERNEL);
		m = desencolarMemoria(memoriasSc);
	}
	free(i);
	return 1;

}

int logicaDescribe(Describe * describe){
	Instruccion * inst = malloc(sizeof(Instruccion));
	inst->instruccion_a_realizar = (void *) describe;
	inst->instruccion = DESCRIBE;

	if(describe->nombre_tabla != NULL){

		char * consistencia = (char*)getTablasSafe(tablasPorConsistencia, describe->nombre_tabla);

		t_list *memoriasAsoc = getMemoriasAsociadasSafe(memoriasAsociadas, consistencia);
		int max = list_size(memoriasAsoc);
		int randomId = rand() % max + 1;
		Memoria * mem = NULL;
		pthread_mutex_lock(&mutexRecursosCompartidos);
		mem = list_get(memoriasAsoc, randomId - 1);
		pthread_mutex_unlock(&mutexRecursosCompartidos);

		if(mem != NULL){
			enviarInstruccionLuqui(mem->ip, mem->puerto, inst, KERNEL);
		}
		free(inst);
		return 1;

	}else{
		int i;
		for(i = 0; i < dictionary_size(memoriasDisponibles); i++){
			char * key = string_new();
			sprintf(key, "%d", i);
			Memoria * m = getMemoriaSafe(memoriasDisponibles, key);
			if(m != NULL){
				enviarInstruccionLuqui(m->ip, m->puerto, inst, KERNEL);
			}
		}
		free(inst);
		return 1;
	}
}