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

	char* leido = 	leer_consola();
	char** consulta_parseada =   parser_lql(leido,  LOGGER);

	if (!es_error(consulta_parseada)) print_consulta_parseada(consulta_parseada);

	conectar_y_crear_hilo(imprimir, IP, PUERTO_KERNELL);
	exit_gracefully(EXIT_SUCCESS);

}

void retornarControl(char * mensaje, int socketCliente){
	/* logica de cada proceso, iniciar planificador en kernell por ejemplo*/
	printf("%s", mensaje);
	enviar(mensaje, IP, PUERTO_POOL_MEM);
}

char*  leer_consola(){
	char* leido;

	leido = readline(">>");
	add_history(leido);

		if (!string_equals_ignore_case(leido, "EXIT")){
			log_info(LOGGER, "Leido por consola: %s", leido);
			return leido;
		}
	free(leido);
}
