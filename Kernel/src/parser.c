/*
 ============================================================================
 Name        : parser.c
 Author      : 
 Version     :
 Copyright   : 
 Description :
 ============================================================================
 */

#include "parser.h"

char** parser_lql(char* mensaje, t_log* LOGGER){

	char** ERROR = string_split("ERROR", " ");
	char** CONSULTA_PARSEADA;

	time_t echo_time;
	echo_time = time(NULL);


	if (echo_time == ((time_t)-1)){
		(void) fprintf(stderr, "Fallo al obtener la hora. \n");
		log_error(LOGGER, "Parser: Fallo al obtener la hora.");
		return ERROR;
	}

	char* consulta = string_new();
	consulta = string_substring_until(mensaje, string_length(mensaje)-1);

	consulta = string_from_format("%s %ju", consulta, (uintmax_t)echo_time);
	log_info(LOGGER, "Parser: Consulta - %s", consulta);


	char** consulta_separada = string_split(consulta, " ");

	int length = cantidad_elementos(consulta_separada) - 1;

	if (es_select(consulta_separada)){

		if (length != 3){
			puts("ERROR: La sintaxis correcta es > SELECT [NOMBRE_TABLA] [KEY]");
			log_error(LOGGER, "Parser: Sintaxis incorrecta, chinguengencha!");
			return ERROR;
		}
		else if(!es_numero(consulta_separada[2])|| string_to_ulint(consulta_separada[2])>65535){
			puts("ERROR: La Key debe ser un numero menor a 65.535.");
			log_error(LOGGER, "Parser: La Key es incorrecta.");
			return ERROR;
		}
		else{

			/*puts(consulta_separada[0]);//select
			puts(consulta_separada[1]);//tabla
			puts(consulta_separada[2]);//key
			puts(consulta_separada[3]);//timestamp*/

			string_to_upper(consulta_separada[0]);
			string_to_upper(consulta_separada[1]);

			log_info(LOGGER, "Parser: Consulta aceptada.");
			CONSULTA_PARSEADA = consulta_separada;
		}
	}
	else if (es_insert(consulta_separada)){

		if (length != 4){
			puts("ERROR: La sintaxis correcta es > INSERT [NOMBRE_TABLA] [KEY] ”[VALUE]”");
			log_error(LOGGER, "Parser: Sintaxis incorrecta, chinguengencha!");
			return ERROR;
		}
		else if (!es_numero(consulta_separada[2])|| string_to_ulint(consulta_separada[2])>65535){
			puts("ERROR: La Key debe ser un numero menor a 65.535.");
			log_error(LOGGER, "Parser: La Key es incorrecta.");
			return ERROR;
		}
		else{

			/*puts(consulta_separada[0]);//insert
			puts(consulta_separada[1]);//tabla
			puts(consulta_separada[2]);//key
			puts(consulta_separada[3]);//valor
			puts(consulta_separada[4]);//timestamp*/

			string_to_upper(consulta_separada[0]);
			string_to_upper(consulta_separada[1]);

			log_info(LOGGER, "Parser: Consulta aceptada.");
			CONSULTA_PARSEADA = consulta_separada;

		}
	}
	else if (es_create(consulta_separada)){
		if (length != 5){
			puts("ERROR: La sintaxis correcta es > CREATE [NOMBRE_TABLA] [TIPO_CONSISTENCIA] [NUMERO_PARTICIONES] [COMPACTION_TIME]");
			log_error(LOGGER, "Parser: Sintaxis incorrecta, chinguengencha!");
			return ERROR;
		}
		else if (!es_numero(consulta_separada[3])){
			puts("ERROR: La cantidad de particiones debe ser un numero.");
			log_error(LOGGER, "Parser: La cantidad de particiones debe ser un numero.");
			return ERROR;
		}
		else if (!es_numero(consulta_separada[4])){
			puts("ERROR: El tiempo de compactacion debe ser un numero.");
			log_error(LOGGER, "Parser: El tiempo de compactacion debe ser un numero.");
			return ERROR;
		}
		else{

			/*puts(consulta_separada[0]);//CREATE
			puts(consulta_separada[1]);//tabla
			puts(consulta_separada[2]);//consistencia
			puts(consulta_separada[3]);//particiones
			puts(consulta_separada[4]);//compactacion
			puts(consulta_separada[5]);//timestamp*/

			string_to_upper(consulta_separada[0]);
			string_to_upper(consulta_separada[1]);
			string_to_upper(consulta_separada[2]);

			log_info(LOGGER, "Parser: Consulta aceptada.");
			CONSULTA_PARSEADA = consulta_separada;

		}
	}
	else if (es_describe(consulta_separada)){

		if (length != 2){
			puts("ERROR: La sintaxis correcta es > DESCRIBE [NOMBRE_TABLA]");
			log_error(LOGGER, "Parser: Sintaxis incorrecta, chinguengencha!");
			return ERROR;
		}
		else{

			/*puts(consulta_separada[0]);//DESCRIBE
			puts(consulta_separada[1]);//tabla
			puts(consulta_separada[2]);//timestamp*/

			string_to_upper(consulta_separada[0]);
			string_to_upper(consulta_separada[1]);

			log_info(LOGGER, "Parser: Consulta aceptada.");
			CONSULTA_PARSEADA = consulta_separada;

		}
	}
	else if (es_drop(consulta_separada)){

		if (length != 2){
			puts("ERROR: La sintaxis correcta es > DROP [NOMBRE_TABLA]");
			log_error(LOGGER, "Parser: Sintaxis incorrecta, chinguengencha!");
			return ERROR;
		}
		else{

			/*puts(consulta_separada[0]);//DROP
			puts(consulta_separada[1]);//tabla
			puts(consulta_separada[2]);//timestamp*/

			string_to_upper(consulta_separada[0]);
			string_to_upper(consulta_separada[1]);

			log_info(LOGGER, "Parser: Consulta aceptada.");
			CONSULTA_PARSEADA = consulta_separada;

		}
	}
	else if (string_equals_ignore_case(string_from_format("%s %s", consulta_separada[0], consulta_separada[1]), "ADD MEMORY")){

		if (length != 5){
			puts("ERROR: La sintaxis correcta es > ADD MEMORY [NÚMERO] TO [CRITERIO]");
			log_error(LOGGER, "Parser: Sintaxis incorrecta, chinguengencha!");
			return ERROR;
		}
		else if ((string_equals_ignore_case(consulta_separada[4], "SC"))
				& (string_equals_ignore_case(consulta_separada[4], "SHC"))
				& (string_equals_ignore_case(consulta_separada[4], "EC"))){
			puts("ERROR: El tipo de consistencia es incorrecto. Debe ser uno de [SC, SHC, EC].");
			log_error(LOGGER, "Parser: El tipo de consistencia es incorrecto.");
			return ERROR;
		}
		else{

			/*
			puts(consulta_separada[0]);//ADD
			puts(consulta_separada[1]);//MEMORY
			puts(consulta_separada[2]);//memoria
			puts(consulta_separada[3]);//TO
			puts(consulta_separada[4]);//criterio
			puts(consulta_separada[5]);//timestamp*/

			consulta_separada[0] = string_from_format("%s %s", consulta_separada[0], consulta_separada[1]);
			consulta_separada[1] = consulta_separada[2];
			consulta_separada[2] = consulta_separada[3];
			consulta_separada[3] = consulta_separada[4];
			consulta_separada[4] = consulta_separada[5];
			consulta_separada[5] = NULL;
			free(consulta_separada[6]);

			string_to_upper(consulta_separada[0]);
			string_to_upper(consulta_separada[2]);
			string_to_upper(consulta_separada[3]);

			log_info(LOGGER, "Parser: Consulta aceptada.");
			CONSULTA_PARSEADA = consulta_separada;

		}
	}
	else if (es_run(consulta_separada) ){

		if (length != 2){
			puts("ERROR: La sintaxis correcta es > RUN [ARCHIVO]");
			log_error(LOGGER, "Parser: Sintaxis incorrecta, chinguengencha!");
			return ERROR;
		}
		else{

			/*puts(consulta_separada[0]);//RUN
			puts(consulta_separada[1]);//path archivo
			puts(consulta_separada[2]);//timestamp*/

			string_to_upper(consulta_separada[0]);

			log_info(LOGGER, "Consulta aceptada.");
			CONSULTA_PARSEADA = consulta_separada;

		}
	}
	else if (es_metrics(consulta_separada)){

			if (length != 1){
				puts("ERROR: La sintaxis correcta es > METRICS");
				log_error(LOGGER, "Parser: Sintaxis incorrecta, chinguengencha!");
				return ERROR;
			}
			else{

				/*puts(consulta_separada[0]);//METRICS
				puts(consulta_separada[1]);//timestamp*/

				string_to_upper(consulta_separada[0]);

				log_info(LOGGER, "Parser: Consulta aceptada.");
				CONSULTA_PARSEADA = consulta_separada;

			}
		}
	else if (es_journal(consulta_separada)){

			if (length != 1){
				puts("ERROR: La sintaxis correcta es > JOURNAL");
				log_error(LOGGER, "Parser: Sintaxis incorrecta, chinguengencha!");
				return ERROR;
			}
			else{

				/*puts(consulta_separada[0]);//JOURNAL
				puts(consulta_separada[1]);//timestamp*/

				string_to_upper(consulta_separada[0]);

				log_info(LOGGER, "Parser: Consulta aceptada.");
				CONSULTA_PARSEADA = consulta_separada;

			}
		}
	else {
		puts("ERROR: Las operaciones disponibles son:");
		puts("Desde Kernel      >> [SELECT, INSERT, CREATE, DESCRIBE, DROP, ADD MEMORY, RUN, METRICS]");
		puts("Desde Pool Memory >> [SELECT, INSERT, CREATE, DESCRIBE, DROP, JOURNAL]");
		puts("Desde File System >> [SELECT, INSERT, CREATE, DESCRIBE, DROP]");
		log_error(LOGGER, "Parser: Operacion no reconocida.");
		return ERROR;
	}

	return CONSULTA_PARSEADA;
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

void print_consulta_parseada(char** consulta_parseada){
	int i = 0;

	while (consulta_parseada[i] != NULL){
		printf("%s ", consulta_parseada[i]);
		i++;
	}

	printf("\n");

}



bool es_select(char** consulta_parseada){
	return (string_equals_ignore_case(consulta_parseada[0], "SELECT"));
}

bool es_insert(char** consulta_parseada){
	return (string_equals_ignore_case(consulta_parseada[0], "INSERT"));
}

bool es_create(char** consulta_parseada){
	return (string_equals_ignore_case(consulta_parseada[0], "CREATE"));
}

bool es_describe(char** consulta_parseada){
	return (string_equals_ignore_case(consulta_parseada[0], "DESCRIBE"));
}

bool es_drop(char** consulta_parseada){
	return (string_equals_ignore_case(consulta_parseada[0], "DROP"));
}

bool es_addmemory(char** consulta_parseada){
	return (string_equals_ignore_case(consulta_parseada[0], "ADD MEMORY"));
}

bool es_run(char** consulta_parseada){
	return (string_equals_ignore_case(consulta_parseada[0], "RUN"));
}

bool es_metrics(char** consulta_parseada){
	return (string_equals_ignore_case(consulta_parseada[0], "METRICS"));
}

bool es_journal(char** consulta_parseada){
	return (string_equals_ignore_case(consulta_parseada[0], "JOURNAL"));
}

bool es_error(char** consulta_parseada){
	return (string_equals_ignore_case(consulta_parseada[0], "ERROR"));
}

unsigned short int get_key(char** consulta_parseada){

	if (es_select(consulta_parseada)|es_insert(consulta_parseada)){
		return (unsigned short int) string_to_ulint(consulta_parseada[2]);
	}
	else {
		return -1;
	}
}

unsigned long int get_timestamp(char** consulta_parseada){
	int i = 0;
	while (consulta_parseada[i] != NULL) i++;
	return string_to_ulint(consulta_parseada[i-1]);
}

unsigned long int string_to_ulint(char* string){
	long int dec = 0;
	int i , len;

	len = strlen(string);
	for(i=0; i<len; i++){
		dec = dec * 10 + (string[i] - '0');
	}

	return dec;
}
