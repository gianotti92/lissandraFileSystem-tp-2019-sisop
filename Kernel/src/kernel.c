#include <stdio.h>
#include <stdlib.h>
#include "config_kernel.h"
#include "servidor.h"

void imprimir(char* mensaje){
	printf("El mensaje es %s\n", mensaje);
}

int main(void) {
	get_parametros_config();
	configure_logger();
	Instruccion *instruccion = malloc(sizeof(Instruccion));
	instruccion->funcion = imprimir;
	instruccion->args = NULL;
	levantar_servidor(&instruccion); // Le paso la funcion que quiero que ejecute
	exit_gracefully(EXIT_SUCCESS);
}
