#include "config_poolMemory.h"
#include "conexion.h"

void imprimir(char* mensaje){
	printf("El mensaje es %s\n", mensaje);
}

int main(void) {
	configure_logger();
	get_parametros_config();
	conectar_y_crear_hilo(imprimir, IP, PUERTO_POOL_MEM);
	exit_gracefully(EXIT_SUCCESS);
}

void retornarControl(char * msj, int cliente){
	printf("mensaje desde pool memory");
	printf("%s", msj);
	enviar(msj, IP, PUERTO_FS);
}
