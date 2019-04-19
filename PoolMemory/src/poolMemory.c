#include "poolMemory.h"

void imprimir(char* mensaje){
	printf("El mensaje es %s\n", mensaje);
}

int main(void) {
	configure_logger();
	configuracion_inicial();
	conectar_y_crear_hilo(imprimir, IP_CONFIG, PUERTO_ESCUCHA_CONEXION);
	exit_gracefully(EXIT_SUCCESS);
}

void retornarControl(char ** msj, int cliente){
	printf("mensaje desde pool memory");
	printf("%s", msj);
	enviar(msj, IP, PUERTO_FS);
}


void configuracion_inicial(){
	CONFIG = config_create("config.cfg");
	if (!CONFIG) {
		printf("No encuentro el archivo config\n");
		exit_gracefully(EXIT_FAILURE);
	}
	PUERTO_ESCUCHA_CONEXION = config_get_int_value(config,"PUERTO_ESCUCHA_CONEXION");
	IP_CONFIG = malloc(sizeof(char) * 100);
	strcpy(IP_CONFIG,config_get_string_value(config, "IP_CONFIG"));

	IP_CONFIG_KERNEL=127.0.0.1
	PUERTO_CONFIG_KERNEL=8000
	IP_CONFIG_POOLMEMORY=127.0.0.2
	PUERTO_CONFIG_POOLMEMORY=8001

}
