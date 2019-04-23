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

	Instruccion instruccion_parseada = parser_lql(leido, POOLMEMORY);

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
		case ADD: {Add * add = instruccion_parseada.instruccion_a_realizar;
				   printf("Memoria: %i Consistencia: %i TS: %lu\n",add->memoria, add->consistencia, add->timestamp);
				   break;}
		case RUN: {Run * run = instruccion_parseada.instruccion_a_realizar;
				   printf("Path: %s TS: %lu\n",run->path, run->timestamp);
				   break;}
		case DROP: {Drop * drop = instruccion_parseada.instruccion_a_realizar;
					printf("Tabla: %s TS: %lu\n",drop->nombre_tabla, drop->timestamp);
					break;}
		case JOURNAL: {Journal * journal = instruccion_parseada.instruccion_a_realizar;
					   printf("TS: %lu \n",journal->timestamp);
					   break;}
		case METRICS: {Metrics * metrics = instruccion_parseada.instruccion_a_realizar;
					   printf("TS: %lu \n",metrics->timestamp);
					   break;}
		case ERROR: printf("ERROR DE CONSULTA \n");
	}

}

void retornarControl(Instruction_set instruccion, int socket_cliente){
	printf("ME llego algo y algo deberia hacer");
	//enviar(instruccion, IP_FS, PUERTO_FS);
}
