#include "fileSystem.h"

int main(void) {
	configure_logger();
	configuracion_inicial();
	pthread_t consolaFS;
	pthread_create(&consolaFS, NULL, (void*) leer_por_consola, retorno_consola);
	for(;;);
}



void configuracion_inicial(void){
	t_config* CONFIG;
	CONFIG = config_create("config.cfg");
	if (!CONFIG) {
		printf("No encuentro el archivo config\n");
		exit_gracefully(EXIT_FAILURE);
	}
	PUERTO_DE_ESCUCHA = config_get_string_value(CONFIG,"PUERTO_DE_ESCUCHA");
	TAMANIO_VALUE = config_get_string_value(CONFIG,"TAMANIO_VALUE");
	PUNTO_MONTAJE = config_get_string_value(CONFIG, "PUNTO_MONTAJE");
	TIEMPO_DUMP = config_get_int_value(CONFIG, "TIEMPO_DUMP");
	RETARDO = config_get_int_value(CONFIG,"RETARDO");
}

void retorno_consola(char* leido){
	printf("Lo leido es: %s\n",leido);
	/*
	 * METO LO QUE LLEGA POR CONSOLA EN LA MEMTABLE
	 *
	 * */
}

void retornarControl(Instruction_set instruccion, int cliente){
	/*
	 * ACA LLEGA LO QUE LLEGA DESDE POOLMEMORY
	 *
	 * */

	printf("Me llego algo y algo deberia hacer");


}
