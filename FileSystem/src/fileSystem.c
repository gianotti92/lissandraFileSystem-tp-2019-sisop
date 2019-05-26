#include "fileSystem.h"

int main(void) {
	configure_logger();
	configuracion_inicial();
	pthread_t consolaFS;
	pthread_create(&consolaFS, NULL, (void*) leer_por_consola, retorno_consola);
	t_comunicacion *comunicacion;
	comunicacion->puerto_servidor = malloc(strlen(PUERTO_DE_ESCUCHA) * sizeof(char));
	memcpy(comunicacion->puerto_servidor, PUERTO_DE_ESCUCHA, strlen(PUERTO_DE_ESCUCHA) * sizeof(char));
	comunicacion->tipo_comunicacion = T_INSTRUCCION; 
	servidor_comunicacion(comunicacion);
}

void configuracion_inicial(void){
	t_config* CONFIG;
	CONFIG = config_create("config.cfg");
	if (!CONFIG) {
		printf("No encuentro el archivo config\n");
		exit_gracefully(EXIT_FAILURE);
	}
	PUERTO_DE_ESCUCHA = config_get_string_value(CONFIG,"PUERTO_DE_ESCUCHA");
	TAMANIO_VALUE = config_get_int_value(CONFIG,"TAMANIO_VALUE");
	PUNTO_MONTAJE = config_get_string_value(CONFIG, "PUNTO_MONTAJE");
	TIEMPO_DUMP = config_get_int_value(CONFIG, "TIEMPO_DUMP");
	RETARDO = config_get_int_value(CONFIG,"RETARDO");
}

void retorno_consola(char* leido){

	Instruccion* instruccion_parseada = parser_lql(leido, FILESYSTEM);
	// Filesystem solo ve sus consultas
	if(	instruccion_parseada->instruccion != ERROR &&
		instruccion_parseada->instruccion != ADD &&
		instruccion_parseada->instruccion != JOURNAL &&
		instruccion_parseada->instruccion != METRICS &&
		instruccion_parseada->instruccion != RUN){
		printf("Lo que me llego por consola es:\n");
		print_instruccion_parseada(instruccion_parseada);
	}
	free_consulta(instruccion_parseada);

}

void retornarControl(Instruccion * instruccion, int cliente){

	printf("Lo que me llego desde POOLMEMORY es:\n");

	print_instruccion_parseada(instruccion);

	printf("El fd de la consulta es %d y no esta cerrado\n", cliente);
	//liberar_conexion(cliente); // Para liberar el fd del socket

}
