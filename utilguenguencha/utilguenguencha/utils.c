#include "utils.h"

void configure_logger() {
	LOGGER = log_create("logger.log","tp-lissandra",1,LOG_LEVEL_DEBUG);
	log_info(LOGGER, "Inicia Proceso");
}

void exit_gracefully(int exit_code){
	if(exit_code == EXIT_FAILURE){
		log_error(LOGGER,strerror(errno));
	}
	else{
		log_info(LOGGER,"Proceso termino correctamente");
	}
	log_destroy(LOGGER);
	exit(exit_code);
}
