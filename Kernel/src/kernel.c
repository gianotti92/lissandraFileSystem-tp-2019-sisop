#include "kernel.h"
#include <sys/types.h>

pthread_mutex_t mutexRecursosCompartidos;
sem_t semaforoSePuedePlanificar, semaforoNewToReady;//, semaforoMetrics;

int main(void) {

	pthread_mutex_init(&mutexRecursosCompartidos, NULL);
	sem_init(&semaforoSePuedePlanificar,0,0);
	sem_init(&semaforoNewToReady, 0, 0);
//	sem_init(&semaforoMetrics, 0, 0);

	configure_logger();
	configuracion_inicial();
	iniciarEstados();
	iniciarEstructurasAsociadas();

	pthread_t consolaKernel, memoriasDisponibles, pasarNewToReady, calcularMetrics;

	pthread_create(&memoriasDisponibles, NULL, (void*) preguntarPorMemoriasDisponibles, NULL);

	pthread_create(&consolaKernel, NULL, (void*) leer_por_consola,
			retorno_consola);

	pthread_create(&pasarNewToReady, NULL, (void*) newToReady, NULL);

	pthread_create(&calcularMetrics, NULL, (void*) calculoMetrics, NULL);


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

		switch(proceso->instruccion->instruccion){
			case RUN:;
				Run * run = (Run *) proceso->instruccion->instruccion_a_realizar;
				logicaRun(run, proceso);
				break;

			case METRICS:;
				break;

			case SELECT:;
				Select * select = (Select *) proceso->instruccion->instruccion_a_realizar;
				logicaSelect(select);
				break;

			case INSERT:;
				Insert * insert = (Insert *) proceso->instruccion->instruccion_a_realizar;
				logicaInsert(insert);
				break;

			case CREATE:;
				Create * create = (Create *) proceso->instruccion->instruccion_a_realizar;
				logicaCreate(create);
				break;

			case DROP:;
				break;

			case ADD:;
				Add * add = (Add *) proceso->instruccion->instruccion_a_realizar;
				logicaAdd(add);
				break;

			case DESCRIBE:;
				break;

			case JOURNAL:
				break;

			default:
				break;
		}
		encolar(estadoExit, proceso);
//		sem_post(&semaforoMetrics);

	}
}

void logicaRun(Run * run, Proceso * proceso){
	char * proximainstruccionChar = leer_linea(run->path, proceso->numeroInstruccion);

	while(proximainstruccionChar != NULL && proceso->quantumProcesado < QUANTUM){

		Instruccion * instruccionAProcesar = parser_lql((char*)proximainstruccionChar, KERNEL);
		proceso->instruccionAProcesar = instruccionAProcesar;
		Instruction_set tipoInstruccionAProcesar = instruccionAProcesar->instruccion;

		switch(tipoInstruccionAProcesar){
			case CREATE:;
//				logicaCreate(instruccionAProcesar);
				break;

			case SELECT:;
//				logicaSelect(instruccionAProcesar);
				break;

			case INSERT:;
				break;

			case DROP:;
				break;

			case ERROR:;
				break;

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

void logicaCreate(Create * create){
	llenarTablasPorConsistencia(create->nombre_tabla, CONSISTENCIAS_STRING[create->consistencia]);
	Memoria * mem = (Memoria*) getMemoriaSafe(tablasPorConsistencia, create->nombre_tabla);

}

void logicaAdd(Add * add){
	Memoria * memoria;
	int tamTabla = dictionary_size(memoriasDisponibles);
	int i;

	for(i = 0; i <= tamTabla; i++){
		char * key = string_new();
		sprintf(key, "%d", i);
		memoria = (Memoria*)getMemoriaSafe(memoriasDisponibles, key);
		if(memoria != NULL){
			break;
		}
	}
	asignarConsistenciaAMemoria(memoria->idMemoria, add->consistencia);
}

void logicaSelect(Select * select){

	char * nombreTabla = string_new();
	string_append(&nombreTabla, select->nombre_tabla);

	Memoria * mem = (Memoria*)getMemoriaSafe(tablasPorConsistencia,nombreTabla);

	//int fd = enviar_instruccion(mem->ip, mem->puerto, proceso->instruccionAProcesar->instruccion_a_realizar, KERNEL);

	free(nombreTabla);
}

void logicaInsert(Insert * insert){
	char * nombreTabla = string_new();
	string_append(&nombreTabla, insert->nombre_tabla);

	Memoria * mem = (Memoria*)getMemoriaSafe(tablasPorConsistencia, nombreTabla);

	free(nombreTabla);

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

void putTablaSafe(t_dictionary * dic, char* key, char * value){
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
char* getTablasSafe(t_dictionary * dic, char*key){
	pthread_mutex_lock(&mutexRecursosCompartidos);
	char * nombre = (char*)dictionary_get(dic, key);
	pthread_mutex_unlock(&mutexRecursosCompartidos);
	return nombre;
}

void asignarConsistenciaAMemoria(uint32_t idMemoria, Consistencias consistencia){
	char * key = string_new();
	sprintf(key, "%d", idMemoria);
	Memoria * m = malloc(sizeof(Memoria));
	m = getMemoriaSafe(memoriasDisponibles, key);
	if(m != NULL){
		putMemorySafe(memoriasAsociadas, CONSISTENCIAS_STRING[consistencia], m);
	}else{
		/*Fallo el log*/
		printf("Fallo el ADD");
	}
}

void llenarTablasPorConsistencia(char * nombreTable, char * consistencia){
	Memoria * mem = (Memoria*)getMemoriaSafe(memoriasAsociadas, consistencia);
	if(mem != NULL){
		putTablaSafe(tablasPorConsistencia, nombreTable, consistencia);
	}else{
		printf("fallo create, no existe memoria con dicha consistencia");
	}
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

void calculoMetrics(){
	int contadorInsert = 0;
	int contadorSelect = 0;
	time_t inicio, fin;
	double diferencia;
	inicio = get_timestamp();
	fin = get_timestamp();
	while(1){
//		sem_wait(&semaforoMetrics);

		Proceso * proceso = desencolar(estadoExit);
		if(proceso != NULL){
			switch(proceso->instruccion->instruccion){
				case INSERT:;
					contadorInsert++;
					free(proceso);
					break;
				case SELECT:;
					contadorSelect++;
					free(proceso);
					break;

				default:
					free(proceso);
					break;
			}
		}
		diferencia = difftime(fin,inicio);

		if((int)diferencia >= 30){

			graficar(contadorInsert, contadorSelect);
			contadorInsert = 0;
			contadorSelect = 0;
			inicio = get_timestamp();
		}
		fin = get_timestamp();
	}
}

void graficar(int contadorInsert, int contadorSelect){
	char * reads = string_new();
	char * writes = string_new();

	string_append(&reads, "Cantidad de reads: ");
	string_append(&writes, "Cantidad de writres: ");

	double r = (double)contadorSelect / 30;
	double w = (double)contadorInsert / 30;

	char * rChar = string_new();
	char * wChar = string_new();

	sprintf(rChar, "%f", r);
	sprintf(wChar, "%f", w);

	string_append(&reads, rChar);
	string_append(&writes, wChar);

	log_info(LOGGER, reads);
	log_info(LOGGER, writes);

	free(reads);
	free(writes);
	free(rChar);
	free(wChar);
}


