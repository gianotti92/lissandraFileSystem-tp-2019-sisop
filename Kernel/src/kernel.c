#include "kernel.h"
#include <sys/types.h>

int main(void) {
	configure_logger();
	configuracion_inicial();
	iniciarEstados();

	pthread_t consolaKernel;
	pthread_create(&consolaKernel, NULL, (void*) leer_por_consola,
			retorno_consola);
	for (;;) {
	} // Para que no muera
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
	TAMANO_MAXIMO_LECTURA_ARCHIVO = config_get_int_value(CONFIG, "TAMANO_MAXIMO_LECTURA_ARCHIVO");
	HILOS_KERNEL= config_get_int_value(CONFIG, "TAMANO_MAXIMO_LECTURA_ARCHIVO");
	config_destroy(CONFIG);
}

void retorno_consola(char* leido) {
	log_info(LOGGER, "Kernel. Se retorno a consola");
	log_info(LOGGER, leido);

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
			list_add(proceso->instrucciones, &line);
		}
		fclose(f);

	} else {
		list_add(proceso->instrucciones, &leidoSplit);
	}

	int cantidad = cantidad_elementos(leidoSplit);
	int i = 0;
	while (i < cantidad) {
		free(leidoSplit[i]);
		i++;
	}
	return proceso;
}

void liberarProceso(Proceso * proceso){
	free(proceso);
}

void encolar(t_list * cola, Proceso * proceso){
	list_add(cola, proceso);
}
