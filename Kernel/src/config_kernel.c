#include "config_kernel.h"


void get_parametros_config(){

	t_config* config = config_create("config.cfg");
	if (!config) {
		printf("No encuentro el archivo config\n");
		//MUERO
		exit(1);
	}

	PUERTO_ESCUCHA_CONEXION = config_get_int_value(config,"PUERTO_ESCUCHA_CONEXION");
	IP_CONFIG_MIO = malloc(sizeof(char) * 100);
	strcpy(IP_CONFIG_MIO,config_get_string_value(config, "IP_CONFIG_MIO"));

	config_destroy(config);
}

void configure_logger() {
  LOGGER = log_create("log de operaciones.log","tp-lissandra",0,LOG_LEVEL_INFO);
  log_info(LOGGER, "Empezamos.....");

}
