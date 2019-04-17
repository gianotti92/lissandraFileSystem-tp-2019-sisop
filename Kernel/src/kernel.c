#include "conexion.h"
#include "config_kernel.h"
#include "parser.h"
#include "kernel.h"

t_dictionary *estadoReady;
t_dictionary *estadoNew;
t_dictionary *estadoExit;
t_dictionary *estadoExec;

int main(void) {
	get_parametros_config();
	configure_logger();
	log_info(LOGGER, "Hello Kernel!!");
	printf("%d \n", PUERTO_ESCUCHA_CONEXION);
	conectar_y_crear_hilo(parser_lql, IP, PUERTO_KERNELL);

	exit_gracefully(EXIT_SUCCESS);
}


void retornarControl(char ** mensaje, int socketCliente){
	log_info(LOGGER,"Kernel:Se retorna a kernell");
	iniciarEstados();
	CategoriaDeMensaje categoriaMsj = categoria(mensaje);
	moverAEstado(categoriaMsj, mensaje);
	printf("%s", mensaje[0]);
	enviar(mensaje[0], IP, PUERTO_POOL_MEM);
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
		return RUN;
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
	case RUN:
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
