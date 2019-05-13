#include "poolMemory.h"

int main(void) {
	configure_logger();
	configuracion_inicial();
	pthread_t consolaKernel;
	pthread_create(&consolaKernel, NULL, (void*) leer_por_consola, retorno_consola);
	servidor_comunicacion(retornarControl, PUERTO_DE_ESCUCHA);
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
	int fd_proceso;
	// La memoria no usa las funciones de KERNEL y JOURNAL no lo envia a filesystem
	if(	instruccion_parseada->instruccion != ERROR &&
		instruccion_parseada->instruccion != METRICS &&
		instruccion_parseada->instruccion != ADD &&
		instruccion_parseada->instruccion != RUN &&
		instruccion_parseada->instruccion != JOURNAL){
		if((fd_proceso = enviar_instruccion(IP_FS, PUERTO_FS, instruccion_parseada, POOLMEMORY))){
			printf("La consulta fue enviada al fd %d de FILESYSTEM y este sigue abierto\n", fd_proceso);
		}
	}
	//liberar_conexion(fd_proceso); // Para liberar el fd del socket
	free_consulta(instruccion_parseada);
}

void retornarControl(Instruccion *instruccion, int cliente){

	printf("Lo que me llego desde KERNEL es:\n");
	print_instruccion_parseada(instruccion);
	printf("El fd de la consulta es %d y no esta cerrado\n", cliente);
	int fd_proceso;
	// Al retornar control de poolmemory solo le llegan las que corresponden y metrics no tiene que enviarla
	if(	instruccion->instruccion != ERROR &&
		instruccion->instruccion != JOURNAL){
		if((fd_proceso = enviar_instruccion(IP_FS, PUERTO_FS, instruccion, POOLMEMORY))){
			printf("La consulta fue enviada al fd %d de FILESYSTEM y este sigue abierto\n", fd_proceso);
		}
	}
	//liberar_conexion(cliente); // Para liberar el fd del socket
	//liberar_conexion(fd_proceso); // Para liberar el fd del socket
	free_consulta(instruccion);
}
