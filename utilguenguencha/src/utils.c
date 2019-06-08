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

int monitorNode(char * node,int mode,int(*callback)(void)){
	char buffer[EVENT_BUF_LEN];
	int infd = inotify_init();
	if (infd < 0) {
		log_error(LOGGER,"Problemas al crear el inotify para %s, %s",node,strerror(errno));
		return 1;
	}
	int wfd = inotify_add_watch(infd,node,mode);
	while(1){
		int length = read(infd,buffer,EVENT_BUF_LEN);
		if (length < 0) {
			log_error(LOGGER,"Problemas al leer el inotify de %s, %s",node,strerror(errno));
			inotify_rm_watch(infd,wfd);
			close(infd);
			return 1;
		}
		if((*callback)()!=0){
			inotify_rm_watch(infd,wfd);
			close(infd);
			return 1;
		}
		inotify_rm_watch(infd,wfd);
		wfd = inotify_add_watch(infd,node,mode);
	}
	inotify_rm_watch(infd,wfd);
	close(infd);
	return 0;
}
