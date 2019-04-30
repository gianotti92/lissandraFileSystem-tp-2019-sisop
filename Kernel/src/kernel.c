#include "kernel.h"
#include <sys/types.h>

pthread_mutex_t mutex;

int main(void) {
	pthread_mutex_init(&mutex, NULL);
	configure_logger();
	configuracion_inicial();
	iniciarEstados();

	pthread_t consolaKernel;
	pthread_create(&consolaKernel, NULL, (void*) leer_por_consola,
			retorno_consola);

//	pthread_t multiProcesamientoKernell;
//	pthread_create(&multiProcesamientoKernell, NULL, (void*) planificar,
//					NULL);

	pthread_join(consolaKernel, NULL);
//	pthread_join(&multiProcesamientoKernell, NULL);
}

void configuracion_inicial(void) {
	t_config* CONFIG;
	CONFIG = config_create("config.cfg");
	if (!CONFIG) {
		log_error(LOGGER, "No encuentro el archivo config");
		exit_gracefully(EXIT_FAILURE);
	}
	PUERTO_DE_ESCUCHA = config_get_int_value(CONFIG, "PUERTO_DE_ESCUCHA");
	IP_MEMORIA_PPAL = config_get_string_value(CONFIG, "IP_MEMORIA_PPAL");
	PUERTO_MEMORIA_PPAL = config_get_int_value(CONFIG, "PUERTO_MEMORIA_PPAL");
	QUANTUM = config_get_int_value(CONFIG, "QUANTUM");
	TAMANO_MAXIMO_LECTURA_ARCHIVO = config_get_int_value(CONFIG,
			"TAMANO_MAXIMO_LECTURA_ARCHIVO");
	HILOS_KERNEL = config_get_int_value(CONFIG,
			"TAMANO_MAXIMO_LECTURA_ARCHIVO");
	config_destroy(CONFIG);
}

void retorno_consola(char* leido) {
	time_t echo_time;
	echo_time = time(NULL);

	log_info(LOGGER, "Kernel. Se retorno a consola");
	log_info(LOGGER, leido);
	if (echo_time == ((time_t)-1)){
		puts ("ERROR: Fallo al obtener la hora.");
		log_error(LOGGER, "Kernel: Fallo al obtener la hora.");
	exit_gracefully(0);
	}

	leido = string_from_format("%s %ju", leido, (uintmax_t)echo_time);
	char ** leidoSplit = string_split(leido, " ");

	Proceso * proceso = asignar_instrucciones(leidoSplit);
	encolar(estadoNew, proceso);

	liberarProceso(proceso);

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

Proceso * crear_proceso() {
	Proceso * proceso = malloc(sizeof(Proceso));
	proceso->instrucciones = list_create();

	return proceso;
}

Proceso * asignar_instrucciones(char ** leidoSplit) {

	time_t echo_time;
	echo_time = time(NULL);

	if (echo_time == ((time_t)-1)){
		puts ("ERROR: Fallo al obtener la hora.");
		log_error(LOGGER, "Kernel: Fallo al obtener la hora.");
	exit_gracefully(0);
	}
	Proceso * proceso = crear_proceso();
	if (es_run(leidoSplit)) {
		FILE * f = fopen(leidoSplit[1], "r");
		char buffer[TAMANO_MAXIMO_LECTURA_ARCHIVO];
		if (f == NULL) {
			perror("No se puede abrir archivo!");
			log_error(LOGGER, "Kernel. error al leer el archivo");
			exit_gracefully(1);
		}
		while (fgets(buffer, sizeof(buffer), f) != NULL) {
			char * line = string_new();
			line = strtok(buffer, "\n");
			line = string_from_format("%s %ju", line, (uintmax_t)echo_time);
			pthread_mutex_lock(&mutex);
			list_add(proceso->instrucciones, &line);
			pthread_mutex_unlock(&mutex);
		}
		fclose(f);

	} else {
		pthread_mutex_lock(&mutex);
		list_add(proceso->instrucciones, &leidoSplit);
		pthread_mutex_unlock(&mutex);
	}

	int cantidad = cantidad_elementos(leidoSplit);
	int i = 0;
	while (i < cantidad) {
		free(leidoSplit[i]);
		i++;
	}
	return proceso;
}

void planificar() {
	//para no hacer espera activa hay que poner un semanaforo, preguntar como..
	while(1){
		pasarProcesoAReady();
		Proceso * p;
		if(list_size(estadoReady) > 0){
			pthread_mutex_lock(&mutex);
			p = list_get(estadoReady, 0);
			pthread_mutex_unlock(&mutex);

			int scriptProcesado = 0;
			while (scriptProcesado <= QUANTUM) {

				pthread_mutex_lock(&mutex);
				int cantidadDeInstrucciones = list_size(p->instrucciones);
				pthread_mutex_unlock(&mutex);

				if(cantidadDeInstrucciones == 1){
					//cuando tengo un proceso con una sola intruccion, se reinicia el quantum?
					Instruccion * i =  list_get(p->instrucciones, 0);

				}else{

				}
			}
		}
	}
}

void liberarProceso(Proceso * proceso) {
	free(proceso);
}

void encolar(t_list * cola, Proceso * proceso) {
	list_add(cola, proceso);
}

void pasarProcesoAReady() {
	if (list_size(estadoNew) > 0) {
		Proceso * p = (Proceso *) list_get(estadoNew, 0);
		list_add(estadoReady, p);
	}
}
