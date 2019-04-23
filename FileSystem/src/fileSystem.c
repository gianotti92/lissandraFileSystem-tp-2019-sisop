#include "fileSystem.h"

int main(void) {
	configure_logger();
	configuracion_inicial();
	pthread_t consolaFS;
	pthread_create(&consolaFS, NULL, (void*) leer_por_consola, retorno_consola);
	conectar_y_crear_hilo(retornarControl, "127.0.0.1", PUERTO_DE_ESCUCHA);
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

void retorno_consola(char* leido){
	printf("Lo leido es: %s \n",leido);

	/*
	 * METO LO QUE LLEGA POR CONSOLA EN LA MEMTABLE
	 *
	 * */

	Instruccion instruccion_parseada = parser_lql(leido, FILESYSTEM);

	switch(instruccion_parseada.instruccion){
		case SELECT: {Select * select = instruccion_parseada.instruccion_a_realizar;
					 printf("Tabla: %s Key: %i TS: %lu \n",select->nombre_tabla, select->key, select->timestamp);
					 break;}
		case INSERT: {Insert * insert = instruccion_parseada.instruccion_a_realizar;
					 printf("Tabla: %s Key: %i Valor: %s TSins: %lu TS: %lu \n",insert->nombre_tabla,insert->key, insert->value, insert->timestamp_insert, insert->timestamp);
					 break;}
		case CREATE: {Create * create = instruccion_parseada.instruccion_a_realizar;
					 printf("Tabla: %s Particiones: %i Compactacion: %lu Consistencia: %i TS: %lu \n",create->nombre_tabla,create->particiones, create->compactation_time, create->consistencia, create->timestamp);
					 break;}
		case DESCRIBE: {Describe * describe = instruccion_parseada.instruccion_a_realizar;
						printf("Tabla: %s TS: %lu\n",describe->nombre_tabla, describe->timestamp);
						break;}
		case DROP: {Drop * drop = instruccion_parseada.instruccion_a_realizar;
					printf("Tabla: %s TS: %lu\n",drop->nombre_tabla, drop->timestamp);
					break;}
		case ERROR: printf("ERROR DE CONSULTA \n");
	}


}

void retornarControl(Instruction_set instruccion, int cliente){
	/*
	 * ACA LLEGA LO QUE LLEGA DESDE POOLMEMORY
	 *
	 * */

	printf("Me llego algo y algo deberia hacer");


}
