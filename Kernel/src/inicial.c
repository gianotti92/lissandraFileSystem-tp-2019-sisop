#include "kernel.h"

void configuracion_inicial(void) {
	t_config* CONFIG;
	CONFIG = config_create("config.cfg");
	if (!CONFIG) {
		log_error(LOGGER, "No encuentro el archivo config");
		exit_gracefully(EXIT_FAILURE);
	}

	LOGGER_METRICS = log_create("logger_metrics.log", "log_metrics", 0, LOG_LEVEL_INFO);

	PUERTO_DE_ESCUCHA = config_get_string_value(CONFIG,"PUERTO_DE_ESCUCHA");
	IP_MEMORIA_PPAL = config_get_string_value(CONFIG,"IP_MEMORIA_PPAL");
	PUERTO_MEMORIA_PPAL = config_get_string_value(CONFIG,"PUERTO_MEMORIA_PPAL");

	QUANTUM = config_get_int_value(CONFIG, "QUANTUM");
	TAMANO_MAXIMO_LECTURA_ARCHIVO = config_get_int_value(CONFIG,
			"TAMANO_MAXIMO_LECTURA_ARCHIVO");
	HILOS_KERNEL = config_get_int_value(CONFIG,
			"HILOS_KERNEL");

	SEGUNDOS_METRICS = config_get_int_value(CONFIG, "TIEMPO_METRICS");
	PREGUNTAR_POR_MEMORIAS = config_get_int_value(CONFIG, "TIEMPO_PREGUNTAR_MEMORIA");
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
	memoriasDisponibles = list_create();
	memoriasAsociadas = list_create();
	memoriasEv = list_create();
	memoriasSc = list_create();
	memoriasHc = list_create();
	list_add_in_index(memoriasAsociadas, EC, memoriasEv);
	list_add_in_index(memoriasAsociadas, SC, memoriasSc);
	list_add_in_index(memoriasAsociadas, SHC, memoriasHc);
	metrics = dictionary_create();

}
