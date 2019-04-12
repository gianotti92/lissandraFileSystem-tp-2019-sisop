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
#include <time.h>

void leer_consola();
void parser_lql(char *);
int cantidad_elementos(char **);
int es_numero(char*);

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
	time_t echo_time;
	echo_time = time(NULL);

	if (echo_time == ((time_t)-1)){
		(void) fprintf(stderr, "Fallo al obtener la hora. \n");
		 //ERROR: lanzar error por no por leer la hora
	}

	consulta = string_from_format("%s %ju", consulta, (uintmax_t)echo_time);

	char** consulta_parseada = string_split(consulta, " ");

	int length = cantidad_elementos(consulta_parseada) - 1;

	if (string_starts_with(consulta_parseada[0], "SELECT")){

		if (length != 3){
			puts("ERROR: La sintaxis correcta es > SELECT [NOMBRE_TABLA] [KEY]");
		}
		else if(!es_numero(consulta_parseada[2])){
			puts("ERROR: La Key debe ser un numero.");
		}
		else{
			puts(consulta_parseada[0]);//select
			puts(consulta_parseada[1]);//tabla
			puts(consulta_parseada[2]);//key
			puts(consulta_parseada[3]);//timestamp

		}
	}
	else if (string_starts_with(consulta_parseada[0], "INSERT")){

		if (length != 4){
			puts("ERROR: La sintaxis correcta es > INSERT [NOMBRE_TABLA] [KEY] \“[VALUE]\”");
		}
		else if (!es_numero(consulta_parseada[2])){
			puts("ERROR: La Key debe ser un numero.");
		}
		else{
			puts(consulta_parseada[0]);//insert
			puts(consulta_parseada[1]);//tabla
			puts(consulta_parseada[2]);//key
			puts(consulta_parseada[3]);//valor
			puts(consulta_parseada[4]);//timestamp
		}
	}
	else if (string_starts_with(consulta_parseada[0], "CREATE")){
		if (length != 5){
			puts("ERROR: La sintaxis correcta es > CREATE [NOMBRE_TABLA] [TIPO_CONSISTENCIA] [NUMERO_PARTICIONES] [COMPACTION_TIME]");
		}
		else if (!es_numero(consulta_parseada[3])){
			puts("ERROR: La cantidad de particiones debe ser un numero.");
		}
		else if (!es_numero(consulta_parseada[4])){
			puts("ERROR: El tiempo de compactacion debe ser un numero.");
		}
		else{
			puts(consulta_parseada[0]);//CREATE
			puts(consulta_parseada[1]);//tabla
			puts(consulta_parseada[2]);//consistencia
			puts(consulta_parseada[3]);//particiones
			puts(consulta_parseada[4]);//compactacion
			puts(consulta_parseada[5]);//timestamp
		}
	}
	else if (string_starts_with(consulta_parseada[0], "DESCRIBE")){

		if (length != 2){
			puts("ERROR: La sintaxis correcta es > DESCRIBE [NOMBRE_TABLA]");
		}
		else{
			puts(consulta_parseada[0]);//DESCRIBE
			puts(consulta_parseada[1]);//tabla
			puts(consulta_parseada[2]);//timestamp
		}
	}
	else if (string_starts_with(consulta_parseada[0], "DROP")){

		if (length != 2){
			puts("ERROR: La sintaxis correcta es > DROP [NOMBRE_TABLA]");
		}
		else{
			puts(consulta_parseada[0]);//DROP
			puts(consulta_parseada[1]);//tabla
			puts(consulta_parseada[2]);//timestamp
		}
	}
	else if (string_starts_with(consulta_parseada[0], "ADD MEMORY")){

		if (length != 5){
			puts("ERROR: La sintaxis correcta es > ADD MEMORY [NÚMERO] TO [CRITERIO]");
		}
		else{
			puts(consulta_parseada[0]);//ADD
			puts(consulta_parseada[1]);//MEMORY
			puts(consulta_parseada[2]);//memoria
			puts(consulta_parseada[3]);//TO
			puts(consulta_parseada[4]);//criterio
			puts(consulta_parseada[5]);//timestamp
		}
	}
	else if (string_starts_with(consulta_parseada[0], "RUN")){

		if (length != 2){
			puts("ERROR: La sintaxis correcta es > RUN [ARCHIVO]");
		}
		else{
			puts(consulta_parseada[0]);//RUN
			puts(consulta_parseada[1]);//archivo
			puts(consulta_parseada[2]);//timestamp
		}
	}
	else {
		puts("ERROR: Las consultas disponibles son [SELECT, INSERT, CREATE, DESCRIBE, DROP, ADD MEMORY]");
	}
}


int cantidad_elementos(char ** array){
	int i = 0;

	while (array[i] != '\0'){
		i++;
	}

	return i;

}

int es_numero(char* palabra){
	int length = string_length(palabra);

	for (int i=0; i<length; i++){
		if(!isdigit(palabra[i])){
			return 0;
		}
	}

	return 1;
}
