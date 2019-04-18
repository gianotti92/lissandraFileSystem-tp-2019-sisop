#include <stdio.h>
#include <stdlib.h>
#include "config_fileSystem.h"
#include "conexion.h"

void imprimir(char* mensaje){
	printf("El mensaje es %s\n", mensaje);
}

int main(void) {
	configure_logger();
	get_parametros_config();
	printf("%d\n", PUERTO_ESCUCHA_CONEXION);
	conectar_y_crear_hilo(imprimir, IP, PUERTO_FS);
	exit_gracefully(EXIT_SUCCESS);
}


void retornarControl(char * msj, int cliente){
	printf("Mensaje desde fileSystem ");
	printf("%s", msj);


}
