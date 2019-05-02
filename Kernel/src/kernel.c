#include "kernel.h"
#include <sys/types.h>

pthread_mutex_t mutex;
sem_t productor;
sem_t consumidor;

//averiguar bien diferencia entre estado ready y estado new
//averigar si la funcion parser lql ya me devuelve time stamp
int main(void) {
	pthread_mutex_init(&mutex, NULL);
	sem_init(&productor,0,1);
	sem_init(&consumidor,0,0);
	configure_logger();
	configuracion_inicial();
	iniciarEstados();

	pthread_t consolaKernel;
	pthread_create(&consolaKernel, NULL, (void*) leer_por_consola,
			retorno_consola);

	int cantMultiprocesamiento = 0;
	while(cantMultiprocesamiento <=  HILOS_KERNEL){
		pthread_t multiProcesamientoKernell;
		pthread_create(&multiProcesamientoKernell, NULL, (void*) planificar,
						NULL);
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
	encolar(estadoReady, proceso);

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
			sem_wait(&consumidor);
			pthread_mutex_unlock(&mutex);
			sem_post(&productor);
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
	while(1){
		sem_wait(&productor);
		pasarPrimerProceso(estadoReady, estadoExec);
		Proceso * p;
		pthread_mutex_lock(&mutex);
		p = list_get(estadoExec, 0);
		pthread_mutex_unlock(&mutex);

		p->quantumProcesado = 0;
		while (p->quantumProcesado <= QUANTUM) {
			char * instructionString = list_get(p->instrucciones, 0);
			Instruccion *i; // = (Instruccion *) parser_lql(instructionString, KERNEL); se supone que mati t ya lo tiene
			int fileDescriptor = enviarX(i, "127.0.0.1", 60000);
			p->file_descriptor = fileDescriptor;
			p->quantumProcesado +=1;

			if(p->quantumProcesado == QUANTUM && list_size(p->instrucciones) != 0){
				pthread_mutex_lock(&mutex);
				cambiarEstado(p, estadoReady);
				pthread_mutex_unlock(&mutex);
			}
			if(list_size(p->instrucciones) == 0){
				pthread_mutex_lock(&mutex);
				cambiarEstado(p, estadoExit);
				pthread_mutex_unlock(&mutex);
			}

			free(i);
			free(instructionString);


		}
		sem_post(&consumidor);
		free(p);
	}
}

void liberarProceso(Proceso * proceso) {
	free(proceso);
}

void encolar(t_list * cola, Proceso * proceso) {
	list_add(cola, proceso);
}

void pasarPrimerProceso(t_list *from, t_list *to) {
	if (list_size(from) > 0) {
		Proceso * p = (Proceso *) list_get(from, 0);
		list_add(to, p);
	}
}

void cambiarEstado(Proceso* p, t_list * estado){
	list_add(estado, p);
}

// mock
int enviarX(Instruccion * i, char*ip, int puerto){
	return 4;
}
