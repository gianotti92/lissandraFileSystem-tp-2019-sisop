#include "kernel.h"

int main(void) {
	configure_logger();
	configuracion_inicial();
	iniciarEstados();

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
	log_info(LOGGER, "Kernel. Se retorno a consola");
	log_info(LOGGER, leido);

	Instruccion instruccion_parseada = parser_lql(leido, KERNEL);
	if(instruccion_parseada.instruccion == RUN){
		FILE prog = crear_programa();
		aniadir_programa_cola();
		planificar_programas();
	}else{
		log_info(LOGGER, "not implemented");
	}





}

void iniciarEstados(){
	log_info(LOGGER,"Kernel:Se inician estados");
	estadoReady = queue_create();
	estadoNew = queue_create();
	estadoExit = queue_create();
	estadoExec = queue_create();
	queue_clean(estadoReady);
	queue_clean(estadoNew);
	queue_clean(estadoExit);
	queue_clean(estadoExec);
}


void planificar_programas(){

}

void moverAEstado(CategoriaDeMensaje categoria, char** mensaje){
	switch(categoria){
	char * v;
	u_int32_t k;
	case RUN_MESSAGE:

		break;
	case ERROR_MESSAGE:
		log_info(LOGGER,"Kernel:Categoria ERROR. Esta mal escrita la query, no se continua");
		break;
	case QUERY:

	default:
		log_info(LOGGER,"Kernel:Categoria NADA. Esto no deberia pasar nunca ajaj..");
		break;
	}
}

