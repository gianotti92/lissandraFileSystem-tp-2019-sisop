#include "fileSystem.h"

int main(void) {
	configure_logger();
	configuracion_inicial();
	pthread_t consolaFS;
	pthread_create(&consolaFS, NULL, (void*) leer_por_consola, retorno_consola);
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

void retornarControl(Instruccion * instruccion, int cliente){
	switch(instruccion->instruccion){
		case SELECT:
			printf("SELECT\n");
			Select *select;
			select = instruccion->instruccion_a_realizar;
			printf("El nombre de la tabla en el select es: %s\n", select->nombre_tabla);
			break;
		case INSERT:
			printf("INSERT\n");
			break;
		case CREATE:
			printf("CREATE\n");
			break;
		case DESCRIBE:
			printf("DESCRIBE\n");
			break;
		case DROP:
			printf("DROP\n");
			break;
		case JOURNAL:
			printf("JOURNAL\n");
			break;
		default:
			printf("Se supone que nunca deberia entrar aqui\n");
			break;
		}
	printf("El fd del source de la instruccion es %d y no esta cerrado\n", cliente);

}
