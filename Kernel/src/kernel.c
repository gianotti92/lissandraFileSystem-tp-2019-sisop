/*
 ============================================================================
 Name        : kernel.c
 Author      : 
 Version     :
 Copyright   : 
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "config_kernel.h"


int main(void) {
	get_parametros_config();
	configure_logger();
	puts("Hello Kernel!!"); /* prints  */
	printf("%d \n", PUERTO_ESCUCHA_CONEXION);

	leer_consola();

	return EXIT_SUCCESS;
}

void leer_consola(){
	char* leido;
	t_dictionary* consulta_parseada;

	leido = readline(">>");
	add_history(leido);

	string_to_upper(leido);

		while (strncmp(leido, "EXIT", 4) != 0){

			log_info(LOGGER, "Leido por consola: %s", leido);
			consulta_parseada = parser_lql(leido, LOGGER);

			if (!dictionary_is_empty(consulta_parseada)) print_dictionary(consulta_parseada);

			free(leido);
			leido = readline(">>");
			add_history(leido);
			string_to_upper(leido);
		}

	free(leido);
	puts("Salgo de la consola");
}
