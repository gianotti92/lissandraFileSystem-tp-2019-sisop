#include "config_poolMemory.h"


void get_parametros_config(){

	t_config* config = config_create("config.cfg");
	if (!config) {
		printf("No encuentro el archivo config\n");
		//MUERO
		exit_gracefully(EXIT_FAILURE);
	}

	PUERTO_CONFIG_KERNEL = config_get_int_value(config,"PUERTO_CONFIG_KERNEL");
	IP_CONFIG_KERNEL = malloc(sizeof(char) * 100);
	strcpy(IP_CONFIG_KERNEL,config_get_string_value(config, "IP_CONFIG_KERNEL"));

	PUERTO_CONFIG_POOLMEMORY = config_get_int_value(config,"PUERTO_CONFIG_POOLMEMORY");
	IP_CONFIG_POOLMEMORY = malloc(sizeof(char) * 100);
	strcpy(IP_CONFIG_POOLMEMORY,config_get_string_value(config, "IP_CONFIG_POOLMEMORY"));



	config_destroy(config);
}

void configure_logger() {

	LOGGER = log_create("poolMemory.log","tp-lissandra",1,LOG_LEVEL_DEBUG);
	log_info(LOGGER, "Inicia poolMemory");
}

void exit_gracefully(int exit_code){
	if(exit_code == EXIT_FAILURE){
		log_error(LOGGER,"Proceso termino en error");
	}
	else{
		log_info(LOGGER,"Proceso termino correctamente");
	}
	log_destroy(LOGGER);
	exit(exit_code);
}
