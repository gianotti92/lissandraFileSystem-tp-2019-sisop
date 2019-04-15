#include <stdio.h>
#include <stdlib.h>

#include "conexion.h"
#include "config_kernel.h"

void imprimir(char* mensaje){
	printf("El mensaje es %s\n", mensaje);
}

int main(void) {
	get_parametros_config();
	configure_logger();
	conectar_y_crear_hilo(imprimir, IP, PUERTO_KERNELL);
	exit_gracefully(EXIT_SUCCESS);

}

void retornarControl(char * mensaje, int socketCliente){
	/* logica de cada proceso, iniciar planificador en kernell por ejemplo*/
	printf("%s", mensaje);
	enviar(mensaje, IP, PUERTO_POOL_MEM);
}
