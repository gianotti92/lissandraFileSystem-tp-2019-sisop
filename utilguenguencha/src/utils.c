#include "utils.h"

void configure_logger() {
	LOGGER = log_create("logger.log","tp-lissandra",1,LOG_LEVEL_DEBUG);
	LOGGER_METRICS = log_create("logger_metrics.log", "log_metrics", 0, LOG_LEVEL_DEBUG);
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
char *consistencia2string(Consistencias consistencia){
	char* str=malloc(4);
	switch(consistencia){
		case EC:
			strcpy(str,"EC");
		break;
		case SC:
			strcpy(str,"SC");
		break;
		case SHC:
			strcpy(str,"SHC");
		break;
	}
	return str;
}
int string2consistencia(char* consistencia){
	if(strcmp(consistencia,"EC")==0){
		return EC;
	}
	if(strcmp(consistencia,"SC")==0){
		return SC;
	}
	return SHC;
}
