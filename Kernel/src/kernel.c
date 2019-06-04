#include "kernel.h"

pthread_mutex_t mutexRecursosCompartidos;
sem_t semaforoSePuedePlanificar, semaforoNewToReady;

int main(void) {

	pthread_mutex_init(&mutexRecursosCompartidos, NULL);
	sem_init(&semaforoSePuedePlanificar,0,0);
	sem_init(&semaforoNewToReady, 0, 0);

	configure_logger();
	configuracion_inicial();
	iniciarEstados();
	iniciarEstructurasAsociadas();

	pthread_t consolaKernel, memoriasDisponibles, pasarNewToReady, calcularMetrics;

	pthread_create(&memoriasDisponibles, NULL, (void*) preguntarPorMemoriasDisponibles, NULL);

	pthread_create(&consolaKernel, NULL, (void*) leer_por_consola,
			retorno_consola);

	pthread_create(&pasarNewToReady, NULL, (void*) newToReady, NULL);

	bool dormir = false;

	pthread_create(&calcularMetrics, NULL, (void*) calculoMetrics, (void*)&dormir);


	int cantMultiprocesamiento = 0;
	while(cantMultiprocesamiento <  HILOS_KERNEL){
		pthread_t multiProcesamientoKernell;
		pthread_create(&multiProcesamientoKernell, NULL, (void*) ejecutar,
						NULL);
		cantMultiprocesamiento++;
}
	pthread_join(consolaKernel, NULL);
}

void configuracion_inicial(void) {
	t_config* CONFIG;
	CONFIG = config_create("config.cfg");
	if (!CONFIG) {
		log_error(LOGGER, "No encuentro el archivo config");
		exit_gracefully(EXIT_FAILURE);
	}

	PUERTO_DE_ESCUCHA = config_get_string_value(CONFIG,"PUERTO_DE_ESCUCHA");
	IP_MEMORIA_PPAL = config_get_string_value(CONFIG,"IP_MEMORIA_PPAL");
	PUERTO_MEMORIA_PPAL = config_get_string_value(CONFIG,"PUERTO_MEMORIA_PPAL");

	QUANTUM = config_get_int_value(CONFIG, "QUANTUM");
	TAMANO_MAXIMO_LECTURA_ARCHIVO = config_get_int_value(CONFIG,
			"TAMANO_MAXIMO_LECTURA_ARCHIVO");
	HILOS_KERNEL = config_get_int_value(CONFIG,
			"HILOS_KERNEL");
}

void iniciarEstados() {
	log_info(LOGGER, "Kernel:Se inician estados");
	estadoReady = list_create();
	estadoNew = list_create();
	estadoExit = list_create();
	list_clean(estadoReady);
	list_clean(estadoNew);
	list_clean(estadoExit);
}

void iniciarEstructurasAsociadas(){
	memoriasAsociadas = dictionary_create();
	tablasPorConsistencia = dictionary_create();
	memoriasDisponibles = dictionary_create();

	memoriasSc = list_create();
	memoriasHc = list_create();
	memoriasEv = list_create();
}

void retorno_consola(char* leido) {

	Proceso * proceso = malloc(sizeof(Proceso));
	Instruccion * instruccion = malloc(sizeof(Instruccion));

	instruccion = parser_lql(leido, KERNEL);


	if(instruccion->instruccion != ERROR){
		proceso->instruccion = instruccion;
		proceso->numeroInstruccion = 0;
		proceso->quantumProcesado = 0;
		proceso->file_descriptor = -1;
		encolar(estadoNew, proceso);
		sem_post(&semaforoNewToReady);
	}
}

void newToReady(){
	while(true){
		sem_wait(&semaforoNewToReady);
		Proceso * proceso = desencolar(estadoNew);
		encolar(estadoReady, proceso);
		sem_post(&semaforoSePuedePlanificar);
	}
}

void ejecutar() {
	while(1){
		sem_wait(&semaforoSePuedePlanificar);
		Proceso * proceso = desencolar(estadoReady);
		time_t fin;
		int diff;
		switch(proceso->instruccion->instruccion){
			case RUN:;
				Run * run = (Run *) proceso->instruccion->instruccion_a_realizar;
				logicaRun(run, proceso);
				break;

			case METRICS:;
				bool dormir = true;
				calculoMetrics((void*)&dormir);
				break;

			case SELECT:;
				Select * select = (Select *) proceso->instruccion->instruccion_a_realizar;
				proceso->file_descriptor = logicaSelect(select);
				fin = get_timestamp();
				diff = difftime(fin ,select->timestamp);
				proceso->segundosQueTardo = diff;
				break;

			case INSERT:;
				Insert * insert = (Insert *) proceso->instruccion->instruccion_a_realizar;
				proceso->file_descriptor = logicaInsert(insert);
				fin = get_timestamp();
				diff = difftime(fin, insert->timestamp);
				proceso->segundosQueTardo = diff;
				break;

			case CREATE:;
				Create * create = (Create *) proceso->instruccion->instruccion_a_realizar;
				proceso->file_descriptor = logicaCreate(create);
				break;

			case DROP:;
				Drop * drop = (Drop *) proceso->instruccion->instruccion_a_realizar;
				proceso->file_descriptor = logicaDrop(drop);
				break;

			case ADD:;
				Add * add = (Add *) proceso->instruccion->instruccion_a_realizar;
				logicaAdd(add);
				break;

			case DESCRIBE:;
				Describe * describe = (Describe*) proceso->instruccion->instruccion_a_realizar;
				logicaDescribe(describe);
				break;

			case JOURNAL:;
				Journal * journal = (Journal*) proceso->instruccion->instruccion_a_realizar;
				logicaJournal(journal);
				break;

			default:
				break;
		}
		encolar(estadoExit, proceso);
	}
}

void logicaRun(Run * run, Proceso * proceso){
	char * proximainstruccionChar = leer_linea(run->path, proceso->numeroInstruccion);

	while(proximainstruccionChar != NULL && proceso->quantumProcesado < QUANTUM){

		Instruccion * instruccionAProcesar = parser_lql((char*)proximainstruccionChar, KERNEL);
		proceso->instruccionAProcesar = instruccionAProcesar;
		Instruction_set tipoInstruccionAProcesar = instruccionAProcesar->instruccion;

		time_t fin;
		int diff;
		switch(tipoInstruccionAProcesar){
			case CREATE:;
				Create * create = (Create *) proceso->instruccion->instruccion_a_realizar;
				proceso->file_descriptor = logicaCreate(create);
				break;

			case SELECT:;
				Select * select = (Select *) proceso->instruccion->instruccion_a_realizar;
				proceso->file_descriptor = logicaSelect(select);
				fin = get_timestamp();
				diff = difftime(fin ,select->timestamp);
				proceso->segundosQueTardo = diff;
				break;

			case INSERT:;
				Insert * insert = (Insert *) proceso->instruccion->instruccion_a_realizar;
				proceso->file_descriptor = logicaInsert(insert);
				fin = get_timestamp();
				diff = difftime(fin, insert->timestamp);
				proceso->segundosQueTardo = diff;
				break;

			case DROP:;
				Drop * drop = (Drop *) proceso->instruccion->instruccion_a_realizar;
				proceso->file_descriptor = logicaDrop(drop);
				break;

			case ERROR:;
				return;

			default:
				break;

		}
		proceso->numeroInstruccion += 1;
		proceso->quantumProcesado += 1;

		proximainstruccionChar = leer_linea(run->path, proceso->numeroInstruccion);
	}

	if(esFinQuantum(proceso, proximainstruccionChar)){
		encolar(estadoReady, proceso);
		return;
	}else if(esFinLectura(proceso, proximainstruccionChar)){
		encolar(estadoExit, proceso);
		return;
	}else{
		encolar(estadoExit, proceso);
		return;
	}
}

int logicaCreate(Create * create){
	Instruccion * i = malloc(sizeof(Instruccion));
	llenarTablasPorConsistencia(create->nombre_tabla, CONSISTENCIAS_STRING[create->consistencia]);
	t_list * listaMemoriasAsoc = getMemoriasAsociadasSafe(memoriasAsociadas, CONSISTENCIAS_STRING[create->consistencia]);
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

bool esFinQuantum(Proceso * p, char * instruccionALeer){
	return instruccionALeer != NULL && p->quantumProcesado == 2;
}

bool esFinLectura(Proceso * p, char * instruccionALeer){
	return instruccionALeer == NULL;
}

void encolar(t_list * cola, Proceso * proceso) {
	pthread_mutex_lock(&mutexRecursosCompartidos);
	list_add(cola, proceso);
	pthread_mutex_unlock(&mutexRecursosCompartidos);
}

Proceso * desencolar(t_list * cola){
	pthread_mutex_lock(&mutexRecursosCompartidos);
	Proceso * p = list_remove(cola, 0);
	pthread_mutex_unlock(&mutexRecursosCompartidos);
	return p;
}

Memoria * desencolarMemoria(t_list * lista){
	pthread_mutex_lock(&mutexRecursosCompartidos);
	Memoria * m = list_get(lista, 0);
	pthread_mutex_unlock(&mutexRecursosCompartidos);
	return m;
}

void putTablaSafe(t_dictionary * dic, char* key, char * value){
	pthread_mutex_lock(&mutexRecursosCompartidos);
	dictionary_put(dic, key, value);
	pthread_mutex_unlock(&mutexRecursosCompartidos);
}

void putMemoryListSafe(t_dictionary * dic, char* key, t_list * value){
	pthread_mutex_lock(&mutexRecursosCompartidos);
	dictionary_put(dic, key, value);
	pthread_mutex_unlock(&mutexRecursosCompartidos);
}

void putMemorySafe(t_dictionary * dic, char* key, Memoria * value){
	pthread_mutex_lock(&mutexRecursosCompartidos);
	dictionary_put(dic, key, value);
	pthread_mutex_unlock(&mutexRecursosCompartidos);
}

Memoria * getMemoriaSafe(t_dictionary * dic, char*key){
	pthread_mutex_lock(&mutexRecursosCompartidos);
	Memoria * m = (Memoria*)dictionary_get(dic, key);
	pthread_mutex_unlock(&mutexRecursosCompartidos);
	return m;
}

t_list * getMemoriasAsociadasSafe(t_dictionary * dic, char*key){
	pthread_mutex_lock(&mutexRecursosCompartidos);
	t_list * listaMemorias = dictionary_get(dic, key);
	pthread_mutex_unlock(&mutexRecursosCompartidos);
	return listaMemorias;
}


char* getTablasSafe(t_dictionary * dic, char*key){
	pthread_mutex_lock(&mutexRecursosCompartidos);
	char * nombre = (char*)dictionary_get(dic, key);
	pthread_mutex_unlock(&mutexRecursosCompartidos);
	return nombre;
}

void asignarConsistenciaAMemoria(Memoria * memoria, Consistencias consistencia){
	Memoria * m = malloc(sizeof(Memoria));
	m = memoria;

	if(m != NULL){
		if( consistencia == SC){
			list_add(memoriasSc,m);
			putMemoryListSafe(memoriasAsociadas, CONSISTENCIAS_STRING[consistencia], memoriasSc);
		}else if(consistencia == EC){
			list_add(memoriasEv, m);
			putMemoryListSafe(memoriasAsociadas, CONSISTENCIAS_STRING[consistencia], memoriasEv);
		}else{
			list_add(memoriasHc, m);
			putMemoryListSafe(memoriasAsociadas, CONSISTENCIAS_STRING[consistencia], memoriasHc);
		}

	}else{
		log_error(LOGGER, "Error al asignar una memoria");
	}

}

void llenarTablasPorConsistencia(char * nombreTable, char * consistencia){
	putTablaSafe(tablasPorConsistencia, nombreTable, consistencia);
}

void preguntarPorMemoriasDisponibles(){
	while(true){
		Memoria * m = malloc(sizeof(Memoria));

		/* funcion de conexiones que me devuelve memoria disponible */
		m->idMemoria = 1;
		char * ip = string_new();
		char * puerto = string_new();
		string_append(&ip, "127.0.0.1");
		string_append(&puerto, "8080");
		m->puerto = puerto;
		m->ip = ip;
		/* funcion de conexiones que me devuelve memoria disponible */

		char * key = string_new();
		sprintf(key, "%d", m->idMemoria);
		putMemorySafe(memoriasDisponibles, key , m);

		sleep(5);
	}
}

void calculoMetrics(bool * direct){
	int contadorInsert = 0;
	int contadorSelect = 0;
	int contadorSelectInsert = 0;
	int operacionesTotales = 0;
	int tiempoPromedioSelect = 0;
	int tiempoPromedioInsert = 0;
	while(1){
		if(!(*direct)){
			sleep(30);
		}
		Proceso * proceso = desencolar(estadoExit);
		while(proceso != NULL){
			operacionesTotales++;
			switch(proceso->instruccion->instruccion){
				case INSERT:;
					tiempoPromedioInsert += proceso->segundosQueTardo;
					contadorInsert++;
					contadorSelectInsert++;
					free(proceso);
					break;
				case SELECT:;
					tiempoPromedioSelect += proceso->segundosQueTardo;
					contadorSelect++;
					contadorSelectInsert++;
					free(proceso);
					break;

				default:
					free(proceso);
					break;
			}
			proceso = desencolar(estadoExit);
		}
		if(contadorInsert == 0){
			contadorInsert = 1;
		}

		if(contadorSelect == 0){
			contadorSelect = 1;
		}
		tiempoPromedioInsert = tiempoPromedioInsert / contadorInsert;
		tiempoPromedioSelect = tiempoPromedioSelect / contadorSelect;

		graficar(contadorInsert, contadorSelect, contadorSelectInsert, operacionesTotales,
				tiempoPromedioInsert, tiempoPromedioSelect);
		contadorInsert = 0;
		contadorSelect = 0;
		contadorSelectInsert = 0;
		if(*direct){
			return;
		}

	}
}

void graficar(int contadorInsert, int contadorSelect, int contadorSelectInsert,
		int operacionesTotales, int tiempoPromedioInsert, int tiempoPromedioSelect){
	char * reads = string_new();
	char * writes = string_new();
	char * memLoad = string_new();
	char * readLatency = string_new();
	char * writeLatency = string_new();
	//FIXME: fijarse porque al principio me devuelve un cantidad read 0.003333 ya que deberia estar vacia lista exit
	string_append(&reads, "Cantidad de reads: ");
	string_append(&writes, "Cantidad de writres: ");
	string_append(&memLoad, "Memory load: ");
	string_append(&readLatency, "Read latency: ");
	string_append(&writeLatency, "Write Latency: ");

	double r = (double)contadorSelect / 30;
	double w = (double)contadorInsert / 30;
	double ml = (double)contadorSelectInsert / operacionesTotales;
	double rl = (double)tiempoPromedioSelect / contadorSelect;
	double wl = (double)tiempoPromedioInsert / contadorSelectInsert;

	char * rChar = string_new();
	char * wChar = string_new();
	char * mlChar = string_new();
	char * rlChar = string_new();
	char * wlChar = string_new();

	sprintf(rChar, "%f", r);
	sprintf(wChar, "%f", w);
	sprintf(mlChar, "%f", ml);
	sprintf(rlChar, "%f",rl);
	sprintf(wlChar, "%f", wl);

	string_append(&reads, rChar);
	string_append(&writes, wChar);
	string_append(&memLoad, mlChar);
	string_append(&readLatency, rlChar);
	string_append(&writeLatency,wlChar);

	log_info(LOGGER, reads);
	log_info(LOGGER, writes);
	log_info(LOGGER, memLoad);
	log_info(LOGGER, writeLatency);
	log_info(LOGGER, readLatency);

	free(reads);
	free(writes);
	free(rChar);
	free(wChar);
	free(memLoad);
	free(mlChar);
	free(writeLatency);
	free(readLatency);
	free(rlChar);
	free(wlChar);
}

/* MOCK */
int enviarInstruccionLuqui(char* ip, char* puerto, Instruccion *instruccion,
		Procesos proceso_del_que_envio){
	if(ip == NULL){
		return -1;
	}
	if(puerto == NULL){
		return -2;
	}
	if(instruccion == NULL){
		return -3;
	}

	sleep(10);

	return 6;
}
/* MOCK */
