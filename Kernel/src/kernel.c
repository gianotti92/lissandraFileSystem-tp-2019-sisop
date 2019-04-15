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
	levantar_servidor(imprimir); // Le paso la funcion que quiero que ejecute
	exit_gracefully(EXIT_SUCCESS);

}

void retornarControl(char * mensaje){
	/* logica de cada proceso, iniciar planificador en kernell por ejemplo*/
	printf(mensaje);
}
