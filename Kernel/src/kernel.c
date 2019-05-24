#include "kernel.h"
#include <sys/types.h>

pthread_mutex_t mutexRecursosCompartidos;
sem_t semaforoSePuedePlanificar;
sem_t semaforoNewToReady;

int main(void) {

	pthread_mutex_init(&mutexRecursosCompartidos, NULL);
	sem_init(&semaforoSePuedePlanificar,0,0);
	sem_init(&semaforoNewToReady, 0, 0);

	configure_logger();
	configuracion_inicial();
	iniciarEstados();
	iniciarEstructurasAsociadas();

	pthread_t consolaKernel, memoriasDisponibles, pasarNewToReady;

	pthread_create(&memoriasDisponibles, NULL, (void*) preguntarPorMemoriasDisponibles, NULL);

	pthread_create(&consolaKernel, NULL, (void*) leer_por_consola,
			retorno_consola);

	pthread_create(&pasarNewToReady, NULL, (void*) newToReady, NULL);

	int cantMultiprocesamiento = 0;
	while(cantMultiprocesamiento <  HILOS_KERNEL){
		pthread_t multiProcesamientoKernell;
		pthread_create(&multiProcesamientoKernell, NULL, (void*) planificar,
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
	estadoExec = list_create();
	list_clean(estadoReady);
	list_clean(estadoNew);
	list_clean(estadoExit);
	list_clean(estadoExec);
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

		if(pthread_mutex_trylock(&mutexRecursosCompartidos) == 0){
			list_add(estadoNew, proceso);
			pthread_mutex_unlock(&mutexRecursosCompartidos);
			sem_post(&semaforoNewToReady);
		}else{
			log_error(LOGGER, "ERROR tratando de lockear");
		}



	}
}

void newToReady(){
	while(true){
		sem_wait(&semaforoNewToReady);

		pthread_mutex_lock(&mutexRecursosCompartidos);
		Proceso * proceso = list_remove(estadoNew, 0);
		list_add(estadoReady, proceso);
		pthread_mutex_unlock(&mutexRecursosCompartidos);

		sem_post(&semaforoSePuedePlanificar);
	}

}

void planificar() {
	while(1){
		sem_wait(&semaforoSePuedePlanificar);

		pthread_mutex_lock(&mutexRecursosCompartidos);
		Proceso * proceso = (Proceso*)list_remove(estadoReady, 0);
		pthread_mutex_unlock(&mutexRecursosCompartidos);

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
		pthread_mutex_lock(&mutexRecursosCompartidos);
		encolar(estadoExit, proceso);
		pthread_mutex_unlock(&mutexRecursosCompartidos);
		return;


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
				logicaCreate(instruccionAProcesar);
				break;

			case SELECT:;
				logicaSelect(instruccionAProcesar);
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

	Memoria * mem = (Memoria*) dictionary_get(tablasPorConsistencia, create->nombre_tabla);

}

void logicaAdd(Add * add){
	Memoria * memoria;
	int tamTabla = dictionary_size(memoriasDisponibles);
	int i;

	for(i = 0; i <= tamTabla; i++){
		char * key = string_new();
		sprintf(key, "%d", i);
		memoria = (Memoria*)dictionary_get(memoriasDisponibles, key);
		if(memoria != NULL){
			break;
		}
	}
	asignarConsistenciaAMemoria(memoria->idMemoria, add->consistencia);
}

void logicaSelect(Select * select){

	char * nombreTabla = string_new();
	string_append(&nombreTabla, select->nombre_tabla);

	Memoria * mem = (Memoria*)dictionary_get(tablasPorConsistencia,nombreTabla);

	//int fd = enviar_instruccion(mem->ip, mem->puerto, proceso->instruccionAProcesar->instruccion_a_realizar, KERNEL);
}

bool esFinQuantum(Proceso * p, char * instruccionALeer){
	return instruccionALeer != NULL && p->quantumProcesado == 2;
}

bool esFinLectura(Proceso * p, char * instruccionALeer){
	return instruccionALeer == NULL;
}

void liberarProceso(Proceso * proceso) {
	free(proceso);
}

void encolar(t_list * cola, Proceso * proceso) {
	pthread_mutex_lock(&mutexRecursosCompartidos);
	list_add(cola, proceso);
	pthread_mutex_unlock(&mutexRecursosCompartidos);
}

void pasarProceso(int posicion, t_list *from, t_list *to) {
	if (list_size(from) > 0) {
		pthread_mutex_lock(&mutexRecursosCompartidos);
		Proceso * p = (Proceso *) list_remove(from, posicion);
		list_add(to, p);
		pthread_mutex_unlock(&mutexRecursosCompartidos);
	}
}

void cambiarEstado(Proceso* p, t_list * estado){
	pthread_mutex_lock(&mutexRecursosCompartidos);
	list_add(estado, p);
	pthread_mutex_unlock(&mutexRecursosCompartidos);
}

void asignarConsistenciaAMemoria(uint32_t idMemoria, Consistencias consistencia){
	char * key = string_new();
	sprintf(key, "%d", idMemoria);
	/*probar que el get no saque el elemento de la lista*/
	Memoria * m = malloc(sizeof(Memoria));
	m = (Memoria*) dictionary_get(memoriasDisponibles, key);
	if(m != NULL){
		pthread_mutex_lock(&mutexRecursosCompartidos);
		dictionary_put(memoriasAsociadas, CONSISTENCIAS_STRING[consistencia], m);
		pthread_mutex_unlock(&mutexRecursosCompartidos);
	}else{
		/*Fallo el log*/
		printf("Fallo el ADD");
	}
}

void llenarTablasPorConsistencia(char * nombreTable, char * consistencia){

	pthread_mutex_lock(&mutexRecursosCompartidos);
	Memoria * mem = (Memoria*)dictionary_get(memoriasAsociadas, consistencia);
	pthread_mutex_unlock(&mutexRecursosCompartidos);

	if(mem != NULL){
		pthread_mutex_lock(&mutexRecursosCompartidos);
		dictionary_put(tablasPorConsistencia, nombreTable, consistencia);
		pthread_mutex_unlock(&mutexRecursosCompartidos);
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
		pthread_mutex_lock(&mutexRecursosCompartidos);
		dictionary_put(memoriasDisponibles, key , m);
		pthread_mutex_unlock(&mutexRecursosCompartidos);



		sleep(5);
	}
}
