#include "filesSystem.h"

void imprimir(char* mensaje){
	printf("El mensaje es %s\n", mensaje);
}

int main(void) {
	configure_logger();
	get_parametros_config();
	conectar_y_crear_hilo(imprimir, IP_CONFIG, PUERTO_ESCUCHA_CONEXION);
	exit_gracefully(EXIT_SUCCESS);
}


void retornarControl(char ** msj, int cliente){
	printf("Mensaje desde fileSystem ");
	printf("%s", msj);


}
