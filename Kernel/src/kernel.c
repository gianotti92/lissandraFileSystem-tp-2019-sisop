#include "kernel.h"

int main(void) {
	configure_logger();
	configuracion_inicial();
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
	IP_MEMORIA_PPAL = config_get_string_value(CONFIG,"IP_MEMORIA_PPAL");
	PUERTO_MEMORIA_PPAL = config_get_int_value(CONFIG,"PUERTO_MEMORIA_PPAL");
	QUANTUM = config_get_int_value(CONFIG, "QUANTUM");
	config_destroy(CONFIG);
}

void retornarControl(Instruction_set instruccion, int socket_cliente){
	printf("Me llego algo y algo deberia hacer");
	/*
	log_info(LOGGER,"Kernel:Se retorna a kernell");
	iniciarEstados();
	CategoriaDeMensaje categoriaMsj = categoria(mensaje);
	moverAEstado(categoriaMsj, mensaje);
	printf("%s", mensaje[0]);
	*/

	/*el enviar de kernel tiene una logica previa para definir a
	 * que memoria deber{a enviar segun el tipo de tabla a la cual
	 * sea necesario acceder segun su consistencia
	 * */
	enviar(instruccion, IP_MEMORIA_PPAL, PUERTO_MEMORIA_PPAL);

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
	case ERROR:
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

