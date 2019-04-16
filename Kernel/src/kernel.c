#include "config_kernel.h"


int main(void) {
	get_parametros_config();
	configure_logger();
	puts("Hello Kernel!!"); /* prints  */
	printf("%d \n", PUERTO_ESCUCHA_CONEXION);

	char* leido;
	char** consulta_parseada;

	while (1) {

		leido = leer_consola();
		consulta_parseada = parser_lql(leido, LOGGER);
		if (!es_error(consulta_parseada)) print_consulta_parseada(consulta_parseada);

	}

	return EXIT_SUCCESS;
}

char* leer_consola(){
	char* leido;

	leido = readline(">>");
	add_history(leido);

	log_info(LOGGER, "Leido por consola: %s", leido);

		if (string_equals_ignore_case(leido, "EXIT")){
			puts("Salgo de la consola");
			exit(1);
		}

	return leido;
}
