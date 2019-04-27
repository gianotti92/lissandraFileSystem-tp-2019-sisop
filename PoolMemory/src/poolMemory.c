#include "poolMemory.h"

int main(void) {
	configure_logger();
	configuracion_inicial();
	pthread_t consolaKernel;
	pthread_create(&consolaKernel, NULL, (void*) leer_por_consola, retorno_consola);
	conectar_y_crear_hilo(retornarControl,"127.0.0.1", PUERTO_DE_ESCUCHA);

}

void configuracion_inicial(void){
	t_config* CONFIG;
	CONFIG = config_create("config.cfg");
	if (!CONFIG) {
		printf("No encuentro el archivo config\n");
		exit_gracefully(EXIT_FAILURE);
	}
	PUERTO_DE_ESCUCHA = config_get_int_value(CONFIG,"PUERTO_DE_ESCUCHA");
	IP_FS = config_get_string_value(CONFIG,"IP_FS");
	PUERTO_FS = config_get_int_value(CONFIG,"PUERTO_FS");
	config_destroy(CONFIG);
}

void retorno_consola(char* leido){
	printf("Lo leido es: %s \n",leido);

	/*
	 * METO LO QUE LLEGA POR CONSOLA EN LA PLANIFICACION DE "NEW"
	 *
	 * */

	Instruccion* instruccion_parseada = parser_lql(leido, POOLMEMORY);

	print_instruccion_parseada(instruccion_parseada);

	//free_consulta(instruccion_parseada);

}

void retornarControl(Instruction_set instruccion, int socket_cliente){
	printf("ME llego algo y algo deberia hacer");
	//enviar(instruccion, IP_FS, PUERTO_FS);
}
