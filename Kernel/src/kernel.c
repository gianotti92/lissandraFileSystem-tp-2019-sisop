#include "kernel.h"
#include <sys/types.h>

pthread_mutex_t mutexEstados;
sem_t semaforoSePuedePlanificar;
sem_t semaforoInicial;
sem_t semaforoNewToReady;

int main(void) {
	pthread_mutex_init(&mutexEstados, NULL);
	sem_init(&semaforoInicial, 0, 1);
	sem_init(&semaforoSePuedePlanificar,0,0);
	sem_init(&semaforoNewToReady, 0 ,0);

	configure_logger();
	configuracion_inicial();
	iniciarEstados();
	iniciarEstructurasAsociadas();

	pthread_t consolaKernel, memoriasDisponibles;

	pthread_create(&memoriasDisponibles, NULL, (void*) preguntarPorMemoriasDisponibles, NULL);

	pthread_create(&consolaKernel, NULL, (void*) leer_por_consola,
			retorno_consola);

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

void retorno_consola(char* leido) {
	sem_wait(&semaforoInicial);

	Instruccion * instruccion = malloc(sizeof(Instruccion));
	Proceso * proceso = malloc(sizeof(Proceso));

	instruccion = parser_lql(leido, KERNEL);

	proceso->quantumProcesado = 0;
	proceso->numeroInstruccion = 0;
	proceso->instruccionActual = instruccion;



	switch(instruccion->instruccion){
		case ADD:;
			Add * add = malloc(sizeof(Add));
			add = (Add*)instruccion->instruccion_a_realizar;
			asignarConsistenciaAMemoria(add->memoria, add->consistencia);
			free(instruccion);
			free(add);
			free(proceso);
			break;

		case JOURNAL:
			break;

		case RUN:;
			Run * run = malloc(sizeof(Run));
			run = (Run*)instruccion->instruccion_a_realizar;
			char * inst = leer_linea(run->path, proceso->numeroInstruccion);
			if( (int)inst != -1 ){
				instruccion = parser_lql(inst, KERNEL);
			}else{
				//TODO: logica de error
			}

			proceso->instruccionActual = instruccion->instruccion_a_realizar;
			encolar(estadoNew, proceso);
			free(inst);
			free(run);
			free(proceso);
			free(instruccion);
			break;

		case METRICS:;
			break;

		case SELECT:;
			Select * select = malloc(sizeof(Select));
			proceso->instruccionActual = instruccion->instruccion_a_realizar;
			encolar(estadoNew, proceso);
			free(select);
			free(proceso);
			free(instruccion);
		break;

		case INSERT:;
			Insert * insert = malloc(sizeof(Insert));
			proceso->instruccionActual = instruccion->instruccion_a_realizar;
			encolar(estadoNew, proceso);
			free(insert);
			free(proceso);
			free(instruccion);
			break;

		case CREATE:;
			Create * create = malloc(sizeof(Create));
			create = (Create*) instruccion->instruccion_a_realizar;
			llenarTablasPorConsistencia(create->nombre_tabla, CONSISTENCIAS_STRING[create->consistencia]);
			proceso->instruccionActual = instruccion->instruccion_a_realizar;
			encolar(estadoNew, proceso);
			free(create);
			free(proceso);
			free(instruccion);
			break;

		case DESCRIBE:;
			break;

		case DROP:;
			break;


		default:

			break;

	}

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
	memoriasDisponibles = list_create();
}


void planificar() {
	while(1){
		int posicion = 0;
		sem_wait(&semaforoSePuedePlanificar);
		pasarProceso(posicion, estadoReady, estadoExec);
		Proceso * proceso = malloc(sizeof(Proceso));

		pthread_mutex_lock(&mutexEstados);
		proceso = (Proceso*)list_remove(estadoExec, 0);
		pthread_mutex_unlock(&mutexEstados);

		while (proceso->quantumProcesado <= QUANTUM) {
			switch(proceso->instruccionActual->instruccion){
				case SELECT:;
					Select * select = malloc(sizeof(Select));
					select = (Select*) proceso->instruccionActual->instruccion_a_realizar;

					pthread_mutex_lock(&mutexEstados);
					Consistencias consistencia =(Consistencias) dictionary_get(tablasPorConsistencia, select->nombre_tabla);
					Memoria * m = (Memoria*)dictionary_get(memoriasAsociadas, CONSISTENCIAS_STRING[consistencia]);
					pthread_mutex_unlock(&mutexEstados);
					int fileDescriptor = enviar_instruccion(m->ip,m->puerto, proceso->instruccionActual->instruccion_a_realizar, KERNEL);
					proceso->file_descriptor = fileDescriptor;
					free(select);
					free(m);

					break;

				case INSERT:;


					break;

				case CREATE:;


					break;

				case DROP:;
					break;

				default:
					break;
			}

			/* pedir siguiente instruccion */
			proceso->numeroInstruccion += 1;
			proceso->quantumProcesado += 1;
			//TODO: logica de salojo o fin de proceso o error
		}
		sem_post(&semaforoInicial);
		free(proceso);
	}
}

void liberarProceso(Proceso * proceso) {
	free(proceso);
}

void encolar(t_list * cola, Proceso * proceso) {
	pthread_mutex_lock(&mutexEstados);
	list_add(cola, proceso);
	pthread_mutex_unlock(&mutexEstados);
}

void pasarProceso(int posicion, t_list *from, t_list *to) {
	if (list_size(from) > 0) {
		pthread_mutex_lock(&mutexEstados);
		Proceso * p = (Proceso *) list_get(from, posicion);
		list_add(to, p);
		pthread_mutex_unlock(&mutexEstados);
	}
}

void cambiarEstado(Proceso* p, t_list * estado){
	pthread_mutex_lock(&mutexEstados);
	list_add(estado, p);
	pthread_mutex_unlock(&mutexEstados);
}

void asignarConsistenciaAMemoria(uint32_t idMemoria, Consistencias consistencia){
	Memoria * m = malloc(sizeof(Memoria));
	/*probar que el get no saque el elemento de la lista*/
	m = (Memoria*) list_get(memoriasDisponibles, idMemoria);
	if(m != NULL){
		pthread_mutex_lock(&mutexEstados);
		dictionary_put(memoriasAsociadas, CONSISTENCIAS_STRING[consistencia], m);
		pthread_mutex_unlock(&mutexEstados);
	}else{
		/*Fallo el log*/
		printf("Fallo el ADD");
	}
	free(m);
}

void llenarTablasPorConsistencia(char * nombreTable, char * consistencia){
	Memoria *mem = malloc(sizeof(Memoria));
	pthread_mutex_lock(&mutexEstados);
	mem = (Memoria*)dictionary_get(memoriasAsociadas, consistencia);
	pthread_mutex_unlock(&mutexEstados);

	if(mem != NULL){
		pthread_mutex_lock(&mutexEstados);
		dictionary_put(tablasPorConsistencia, nombreTable, consistencia);
		pthread_mutex_unlock(&mutexEstados);
	}else{
		printf("fallo create, no existe memoria con dicha consistencia");
	}
}

// mock
int enviarX(Instruccion * i, char * ip, char *  puerto){
	return 4;
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
		free(ip);
		free(puerto);
		/* funcion de conexiones que me devuelve memoria disponible */
		pthread_mutex_lock(&mutexEstados);
		list_add_in_index(memoriasDisponibles, m->idMemoria, m);
		pthread_mutex_unlock(&mutexEstados);

		sleep(5);
	}
}
