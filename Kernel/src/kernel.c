#include "conexion.h"
#include "config_kernel.h"
#include "kernel.h"

t_dictionary *estadoReady;
t_dictionary *estadoNew;
t_dictionary *estadoExit;
t_dictionary *estadoExec;

void imprimir(char* mensaje){
	printf("El mensaje es %s\n", mensaje);
}

int main(void) {
	get_parametros_config();
	configure_logger();
	conectar_y_crear_hilo(imprimir, IP, PUERTO_KERNELL);
	exit_gracefully(EXIT_SUCCESS);

}

void retornarControl(char * mensaje, int socketCliente){
	log_info(LOGGER,"Kernel:Se inicia proceso kernell");
	iniciarEstados();
	CategoriaDeMensaje categoria = categoria(mensaje);
	moverAEstado(categoria, mensaje);


	printf("%s", mensaje);
	enviar(mensaje, IP, PUERTO_POOL_MEM);
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

CategoriaDeMensaje categoria(char * mensaje){
	log_info(LOGGER,"Kernel:Se asigna categoria del mensaje");
	if(string_contains(mensaje,"run")){
		log_info(LOGGER,"Kernel:Categoria RUN. Se asigna categoria del mensaje");
		return RUN;
	}else if(string_contains(mensaje,"error")){
		log_info(LOGGER,"Kernel:Categoria ERROR. Se asigna categoria del mensaje");
		return ERROR;
	}else{
		log_info(LOGGER,"Kernel:Categoria QUERY. Se asigna categoria del mensaje");
		return QUERY;
	}
}

void moverAEstado(CategoriaDeMensaje categoria, char* mensaje){
	switch(categoria){
	case RUN:
			log_info(LOGGER,"Kernel:Categoria RUN. Se mueve el mensaje a nuevos");
			u_int32_t key = 1;
			char * value = new_string();
			string_append(&value, mensaje);
			dictionary_put(estadoNew, key, value);
			free(value);
		break;
	case ERROR:
		log_info(LOGGER,"Kernel:Categoria ERROR. Esta mal escrita la query, no se continua");
		break;
	case QUERY:
		log_info(LOGGER,"Kernel:Categoria QUERY. Se mueve el mensaje a listos");
		u_int32_t key = 1;
		char * value = new_string();
		string_append(&value, mensaje);
		dictionary_put(estadoNew, key, value);
		free(value);
		break;
	default:
		log_info(LOGGER,"Kernel:Categoria NADA. Esto no deberia pasar nunca ajaj..");
		break;
	}
}
