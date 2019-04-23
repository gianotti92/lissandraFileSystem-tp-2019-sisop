#include "kernel.h"

int main(void) {
	configure_logger();
	configuracion_inicial();

	pthread_t consolaKernel;
	pthread_create(&consolaKernel, NULL, (void*) leer_por_consola, retorno_consola);
	for(;;){} // Para que no muera
}

void configuracion_inicial(void){
	t_config* CONFIG;
	CONFIG = config_create("config.cfg");
	if (!CONFIG) {
		log_error(LOGGER,"No encuentro el archivo config");
		exit_gracefully(EXIT_FAILURE);
	}
	PUERTO_DE_ESCUCHA = config_get_int_value(CONFIG,"PUERTO_DE_ESCUCHA");
	IP_MEMORIA_PPAL = config_get_string_value(CONFIG,"IP_MEMORIA_PPAL");
	PUERTO_MEMORIA_PPAL = config_get_int_value(CONFIG,"PUERTO_MEMORIA_PPAL");
	QUANTUM = config_get_int_value(CONFIG, "QUANTUM");
	config_destroy(CONFIG);
}

void retorno_consola(char* leido){
	printf("Lo leido es: %s \n",leido);

	/* KERNEL SOLO RECIBE COSAS POR CONSOLA POR LO CUAL NO TIENE SENTIDO QUE ESTE ESCUCHANDO
	 * EN NINGUN PUERTO, SOLO RECIBE Y ACCIONA
	 *
	 * */

	Instruccion instruccion_parseada = parser_lql(leido, KERNEL);

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

void iniciarEstados(){
	log_info(LOGGER,"Kernel:Se inician estados");
	estadoReady = dictionary_create();
	estadoNew = dictionary_create();
	estadoExit = dictionary_create();
	estadoExec = dictionary_create();
	dictionary_clean(estadoReady);
	dictionary_clean(estadoNew);
	dictionary_clean(estadoExit);
	dictionary_clean(estadoExec);
}

CategoriaDeMensaje categoria(char ** mensaje){
	log_info(LOGGER,"Kernel:Se asigna categoria del mensaje");
	char * msj = string_new();
	strcpy(mensaje[0], msj);
	if(string_contains(msj,"run")){
		log_info(LOGGER,"Kernel:Categoria RUN. Se asigna categoria del mensaje");
		return RUN_MESSAGE;
	}else if(string_contains(msj,"error")){
		log_info(LOGGER,"Kernel:Categoria ERROR. Se asigna categoria del mensaje");
		return ERROR;
	}else{
		log_info(LOGGER,"Kernel:Categoria QUERY. Se asigna categoria del mensaje");
		return QUERY;
	}
}

void moverAEstado(CategoriaDeMensaje categoria, char** mensaje){
	switch(categoria){
	char * v;
	u_int32_t k;
	case RUN_MESSAGE:
			log_info(LOGGER,"Kernel:Categoria RUN. Se mueve el mensaje a nuevos");
			v = string_new();
			k = 1;
			string_append(&v, mensaje);
			dictionary_put(estadoNew, k, v);
			free(v);
		break;
	case ERROR_MESSAGE:
		log_info(LOGGER,"Kernel:Categoria ERROR. Esta mal escrita la query, no se continua");
		break;
	case QUERY:
		log_info(LOGGER,"Kernel:Categoria QUERY. Se mueve el mensaje a listos");
		v = string_new();
		k = 1;
		string_append(&v, mensaje);
		dictionary_put(estadoNew, k, v);
		free(v);
		break;
	default:
		log_info(LOGGER,"Kernel:Categoria NADA. Esto no deberia pasar nunca ajaj..");
		break;
	}
}
