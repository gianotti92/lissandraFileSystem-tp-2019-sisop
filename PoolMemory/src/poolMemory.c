#include "poolMemory.h"

int main(void) {
	configure_logger();
	configuracion_inicial();
	pthread_t consolaKernel;
	pthread_create(&consolaKernel, NULL, (void*) leer_por_consola, retorno_consola);
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
	IP_FS = config_get_string_value(CONFIG,"IP_FS");
	PUERTO_FS = config_get_string_value(CONFIG,"PUERTO_FS");
}

void retorno_consola(char* leido){

	Instruccion* instruccion_parseada = parser_lql(leido, POOLMEMORY);

	print_instruccion_parseada(instruccion_parseada);

	free_consulta(instruccion_parseada);

}

void retornarControl(Instruction_set instruccion, int socket_cliente){
	printf("Me llego algo y algo deberia hacer");

}
