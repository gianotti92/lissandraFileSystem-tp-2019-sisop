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

	leido = readline(">>");

	string_to_upper(leido);

		while (strncmp(leido, "EXIT", 4) != 0){

			//loguear valor leido
			parser_lql(leido);
			free(leido);
			leido = readline(">>");
			string_to_upper(leido);
		}

	free(leido);
	puts("Salgo de la consola");

}

void parser_lql(char * consulta){
	//falla cuando escribis una consulta sin todas sus partes, porque hace un puts de un elemento del array que no existe, verificar eso.

	char** consulta_parseada = string_split(consulta, " "); //agregar timestamp al final

	if (string_starts_with(consulta_parseada[0], "SELECT")){

		puts("Es un SELECT");
		puts(consulta_parseada[0]);//select
		puts(consulta_parseada[1]);//tabla
		puts(consulta_parseada[2]);//key
	}
	else if (string_starts_with(consulta_parseada[0], "INSERT")){

		puts("Es un INSERT");
		puts(consulta_parseada[0]);//insert
		puts(consulta_parseada[1]);//tabla
		puts(consulta_parseada[2]);//key
		puts(consulta_parseada[3]);//valor
		puts(consulta_parseada[4]);//timestamp
	}
	else if (string_starts_with(consulta_parseada[0], "CREATE")){

		puts("Es un CREATE");
		puts(consulta_parseada[0]);//CREATE
		puts(consulta_parseada[1]);//tabla
		puts(consulta_parseada[2]);//consistencia
		puts(consulta_parseada[3]);//particiones
		puts(consulta_parseada[4]);//compactacion
		puts(consulta_parseada[5]);//timestamp

	}
	else if (string_starts_with(consulta_parseada[0], "DESCRIBE")){

		puts("Es un DESCRIBE");
		puts(consulta_parseada[0]);//DESCRIBE
		puts(consulta_parseada[1]);//tabla
		puts(consulta_parseada[2]);//timestamp
	}
	else if (string_starts_with(consulta_parseada[0], "DROP")){

		puts("Es un DROP");
		puts(consulta_parseada[0]);//DROP
		puts(consulta_parseada[1]);//tabla
		puts(consulta_parseada[2]);//timestamp
	}
	else if (string_starts_with(consulta_parseada[0], "ADD MEMORY")){

		puts("Es un ADD MEMORY");
		puts(consulta_parseada[0]);//ADD
		puts(consulta_parseada[1]);//MEMORY
		puts(consulta_parseada[2]);//memoria
		puts(consulta_parseada[3]);//timestamp
	}
	else if (string_starts_with(consulta_parseada[0], "RUN")){

		puts("Es un RUN");
		puts(consulta_parseada[0]);//RUN
		puts(consulta_parseada[1]);//archivo
		puts(consulta_parseada[2]);//timestamp
	}
	else {
		puts("ERROR: La consulta debe comenzar con alguno de [SELECT, INSERT, CREATE, DESCRIBE, DROP, ADD MEMORY]");
	}
}
