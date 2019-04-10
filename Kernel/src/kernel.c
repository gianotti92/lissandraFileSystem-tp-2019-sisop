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

void leer_consola();
void parser_lql(char *);

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

	puts("");
	leido = readline(">>");

		while (strncmp(leido, "EXIT", 4) != 0){

			//loguear valor leido
			parser_lql(leido);
			//puts(leido);
			free(leido);
			leido = readline(">>");
		}

	puts(leido);
	free(leido);
	puts("Salgo de la consola");

}

void parser_lql(char * consulta){

	string_to_upper(consulta);

	if (string_starts_with(consulta, "SELECT")){
		puts("Es un SELECT");
	}
	else if (string_starts_with(consulta, "INSERT")){
		puts("Es un INSERT");
	}
	else if (string_starts_with(consulta, "CREATE")){
		puts("Es un CREATE");
	}
	else if (string_starts_with(consulta, "DESCRIBE")){
		puts("Es un DESCRIBE");
	}
	else if (string_starts_with(consulta, "DROP")){
		puts("Es un DROP");
	}
	else if (string_starts_with(consulta, "ADD MEMORY")){
		puts("Es un ADD MEMORY");
	}
	else {
		puts("ERROR: La consulta debe comenzar con alguno de [SELECT, INSERT, CREATE, DESCRIBE, DROP, ADD MEMORY]");
	}
}
