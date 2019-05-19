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

void retorno_consola(char* leido) {
	sem_wait(&semaforoInicial);

	Proceso * proceso = malloc(sizeof(Proceso));
	Instruccion * instruccion = malloc(sizeof(Instruccion));

	instruccion = parser_lql(leido, KERNEL);

	proceso->instruccion = instruccion;
	proceso->numeroInstruccion = 0;
	proceso->quantumProcesado = 0;
	proceso->file_descriptor = -1;

	pthread_mutex_lock(&mutexEstados);
	list_add(estadoNew, proceso);
	pthread_mutex_unlock(&mutexEstados);

	free(leido);
	free(proceso);
	free(instruccion);

	sem_post(&semaforoNewToReady);
}

void newToReady(){
	while(true){
		sem_wait(&semaforoNewToReady);

		Proceso * proceso = malloc(sizeof(Proceso));

		pthread_mutex_lock(&mutexEstados);
		proceso = list_remove(estadoNew, 0);
		list_add(estadoReady, proceso);
		pthread_mutex_unlock(&mutexEstados);

		sem_post(&semaforoSePuedePlanificar);
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
		sem_wait(&semaforoSePuedePlanificar);

		Proceso * proceso = malloc(sizeof(Proceso));
		Instruccion * instruccion = malloc(sizeof(Instruccion));

		pthread_mutex_lock(&mutexEstados);
		proceso = (Proceso*)list_remove(estadoReady, 0);
		pthread_mutex_unlock(&mutexEstados);
		instruccion = proceso->instruccion;

		while (proceso->quantumProcesado <= QUANTUM) {

			switch(proceso->instruccion->instruccion){
				case RUN:;
					Run * run = malloc(sizeof(Run));
					run = (Run*)instruccion->instruccion_a_realizar;

					char * inst = leer_linea(run->path, proceso->numeroInstruccion);
					Instruccion * instruccionAProcesar = malloc(sizeof(Instruccion));

					if( (int)inst != -1 ){
						instruccionAProcesar = parser_lql(inst, KERNEL);
						proceso->instruccionAProcesar = instruccionAProcesar;

						switch(proceso->instruccionAProcesar->instruccion){
							case CREATE:;
								break;

							case SELECT:;
								Select * select = malloc(sizeof(Select));
								Memoria * mem = malloc(sizeof(Memoria));
								select = (Select*) proceso->instruccionAProcesar->instruccion_a_realizar;

								char * nombreTabla = string_new();
								string_append(&nombreTabla, select->nombre_tabla);

								mem = (Memoria*)dictionary_get(tablasPorConsistencia,nombreTabla);

								int fd = enviar_instruccion(mem->ip, mem->puerto, proceso->instruccionAProcesar->instruccion_a_realizar, KERNEL);

								proceso->file_descriptor = fd;


								free(mem);
								free(select);
								free(nombreTabla);
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

						if(proceso->quantumProcesado == 2){
							pthread_mutex_lock(&mutexEstados);
							list_add(estadoReady, proceso);
							pthread_mutex_unlock(&mutexEstados);
						}

					}else{
						pthread_mutex_lock(&mutexEstados);
						encolar(estadoExit, proceso);
						pthread_mutex_unlock(&mutexEstados);
						return;
					}

					free(inst);
					free(run);
					free(proceso);
					free(instruccion);
					free(instruccionAProcesar);
					break;

				case METRICS:;
					break;

				case SELECT:;
					break;

				case INSERT:;

					break;

				case CREATE:;

					break;

				case DROP:;
					break;

				case ADD:;

					break;

				case DESCRIBE:;
					break;

				case JOURNAL:
					break;

				default:
					break;
			}

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
		Proceso * p = (Proceso *) list_remove(from, posicion);
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
