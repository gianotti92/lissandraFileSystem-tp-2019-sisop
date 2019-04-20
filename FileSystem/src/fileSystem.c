#include "filesSystem.h"S

int main(void) {
	configure_logger();
	configuracion_inicial();
	conectar_y_crear_hilo(retornarControl, "127.0.0.1", PUERTO_DE_ESCUCHA);
}


void retornarControl(Instruction_set instruccion, int cliente){
	printf("Me llego algo y algo deberia hacer");


}

void configuracion_inicial(void){
	t_config* CONFIG;
	CONFIG = config_create("config.cfg");
	if (!CONFIG) {
		printf("No encuentro el archivo config\n");
		exit_gracefully(EXIT_FAILURE);
	}
	PUERTO_DE_ESCUCHA = config_get_int_value(CONFIG,"PUERTO_DE_ESCUCHA");
	TAMANIO_VALUE = config_get_string_value(CONFIG,"TAMANIO_VALUE");
	PUNTO_MONTAJE = config_get_string_value(CONFIG, "PUNTO_MONTAJE");
	TIEMPO_DUMP = config_get_int_value(CONFIG, "TIEMPO_DUMP");
	RETARDO = config_get_int_value(CONFIG,"RETARDO");
	config_destroy(CONFIG);
}
