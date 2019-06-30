#include "kernel.h"

void configuracion_inicial(void) {
	t_config* CONFIG = config_create("config.cfg");
	if (!CONFIG) {
		log_error(LOG_ERROR, "No encuentro el archivo config");
		exit_gracefully(EXIT_FAILURE);
	}

	LOGGER_METRICS = log_create("log_metrics.log", "log_metrics", 0, LOG_LEVEL_INFO);
	
	char * puertoEscucha = config_get_string_value(CONFIG,"PUERTO_DE_ESCUCHA");
	PUERTO_DE_ESCUCHA = malloc(strlen(puertoEscucha) + 1);
	strcpy(PUERTO_DE_ESCUCHA, puertoEscucha);
	
	char * ipMemoriaPrincipal = config_get_string_value(CONFIG,"IP_MEMORIA_PPAL");
	IP_MEMORIA_PPAL = malloc(strlen(ipMemoriaPrincipal) + 1);
	strcpy(IP_MEMORIA_PPAL, ipMemoriaPrincipal);

	char * puertoMemoriaPrincipal = config_get_string_value(CONFIG,"PUERTO_MEMORIA_PPAL");
	PUERTO_MEMORIA_PPAL = malloc(strlen(puertoMemoriaPrincipal) + 1);
	strcpy(PUERTO_MEMORIA_PPAL, puertoMemoriaPrincipal);

	QUANTUM = config_get_int_value(CONFIG, "QUANTUM");
	TAMANO_MAXIMO_LECTURA_ARCHIVO = config_get_int_value(CONFIG,
			"TAMANO_MAXIMO_LECTURA_ARCHIVO");
	HILOS_KERNEL = config_get_int_value(CONFIG,
			"HILOS_KERNEL");

	SEGUNDOS_METRICS = config_get_int_value(CONFIG, "TIEMPO_METRICS");
	PREGUNTAR_POR_MEMORIAS = config_get_int_value(CONFIG, "TIEMPO_PREGUNTAR_MEMORIA");
	TIEMPO_DESCRIBE = config_get_int_value(CONFIG, "TIEMPO_DESCRIBE");

	config_destroy(CONFIG);

	acum30sMetrics = list_create();
}

void actualizar_configuracion(t_config* conf) {
	SEGUNDOS_METRICS = config_get_int_value(conf, "TIEMPO_METRICS");
	PREGUNTAR_POR_MEMORIAS = config_get_int_value(conf, "TIEMPO_PREGUNTAR_MEMORIA");
	TIEMPO_DESCRIBE = config_get_int_value(conf, "TIEMPO_DESCRIBE");
}

void iniciarEstados() {
	estadoReady = list_create();
	estadoNew = list_create();
	estadoExit = list_create();
	list_clean(estadoReady);
	list_clean(estadoNew);
	list_clean(estadoExit);
}

void iniciarEstructurasAsociadas(){
	lista_ec = list_create();
	lista_sc = list_create();
	lista_shc = list_create();
	lista_disp = list_create();
	metrics = dictionary_create();

}
