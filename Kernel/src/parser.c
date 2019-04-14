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

t_dictionary* parser_lql(char* consulta, t_log* LOGGER){

	t_dictionary* consulta_parseada = dictionary_create();//en caso de no poder parsear la consulta, devuelve diccionario vacio

	time_t echo_time;
	echo_time = time(NULL);

	if (echo_time == ((time_t)-1)){
		(void) fprintf(stderr, "Fallo al obtener la hora. \n");
		log_error(LOGGER, "Fallo al obtener la hora.");
		return (consulta_parseada);
	}

	consulta = string_from_format("%s %ju", consulta, (uintmax_t)echo_time);
	log_info(LOGGER, "Consulta: %s", consulta);

	char** consulta_separada = string_split(consulta, " ");

	int length = cantidad_elementos(consulta_separada) - 1;


	if (strncmp(consulta_separada[0], "SELECT", 6) == 0){

		if (length != 3){
			puts("ERROR: La sintaxis correcta es > SELECT [NOMBRE_TABLA] [KEY]");
			log_error(LOGGER, "Sintaxis incorrecta, chinguengencha!");
			return (consulta_parseada);
		}
		else if(!es_numero(consulta_separada[2])|| string_to_ulint(consulta_separada[2])>65535){
			puts("ERROR: La Key debe ser un numero menor a 65.535.");
			log_error(LOGGER, "La Key es incorrecta.");
			return (consulta_parseada);
		}
		else{

			unsigned int key = (short int) string_to_ulint(consulta_separada[2]);
			unsigned long int timestamp = string_to_ulint(consulta_separada[3]);

			dictionary_put(consulta_parseada, "operacion", consulta_separada[0]);
			dictionary_put(consulta_parseada, "tabla", consulta_separada[1]);
			dictionary_put(consulta_parseada, "key", key);
			dictionary_put(consulta_parseada, "timestamp", timestamp);

			/*puts(consulta_separada[0]);//select
			puts(consulta_separada[1]);//tabla
			puts(consulta_separada[2]);//key
			puts(consulta_separada[3]);//timestamp*/

			log_info(LOGGER, "Consulta aceptada.");
		}
	}
	else if (strncmp(consulta_separada[0], "INSERT", 6) == 0){

		if (length != 4){
			puts("ERROR: La sintaxis correcta es > INSERT [NOMBRE_TABLA] [KEY] ”[VALUE]”");
			log_error(LOGGER, "Sintaxis incorrecta, chinguengencha!");
		}
		else if (!es_numero(consulta_separada[2])|| string_to_ulint(consulta_separada[2])>65535){
			puts("ERROR: La Key debe ser un numero menor a 65.535.");
			log_error(LOGGER, "La Key es incorrecta.");
		}
		else{

			unsigned int key = (short int) string_to_ulint(consulta_separada[2]);
			unsigned long int timestamp = string_to_ulint(consulta_separada[4]);

			dictionary_put(consulta_parseada, "operacion", consulta_separada[0]);
			dictionary_put(consulta_parseada, "tabla", consulta_separada[1]);
			dictionary_put(consulta_parseada, "key", key);
			dictionary_put(consulta_parseada, "valor", consulta_separada[3]);
			dictionary_put(consulta_parseada, "timestamp", timestamp);


			/*puts(consulta_separada[0]);//insert
			puts(consulta_separada[1]);//tabla
			puts(consulta_separada[2]);//key
			puts(consulta_separada[3]);//valor
			puts(consulta_separada[4]);//timestamp*/

			log_info(LOGGER, "Consulta aceptada.");

		}
	}
	else if (strncmp(consulta_separada[0], "CREATE", 6) == 0){
		if (length != 5){
			puts("ERROR: La sintaxis correcta es > CREATE [NOMBRE_TABLA] [TIPO_CONSISTENCIA] [NUMERO_PARTICIONES] [COMPACTION_TIME]");
			log_error(LOGGER, "Sintaxis incorrecta, chinguengencha!");
		}
		else if (!es_numero(consulta_separada[3])){
			puts("ERROR: La cantidad de particiones debe ser un numero.");
			log_error(LOGGER, "La cantidad de particiones debe ser un numero.");
		}
		else if (!es_numero(consulta_separada[4])){
			puts("ERROR: El tiempo de compactacion debe ser un numero.");
			log_error(LOGGER, "El tiempo de compactacion debe ser un numero.");
		}
		else{
			unsigned int particiones = (short int) string_to_ulint(consulta_separada[3]);
			unsigned int compactacion = (short int) string_to_ulint(consulta_separada[4]);
			unsigned long int timestamp = string_to_ulint(consulta_separada[5]);

			dictionary_put(consulta_parseada, "operacion", consulta_separada[0]);
			dictionary_put(consulta_parseada, "tabla", consulta_separada[1]);
			dictionary_put(consulta_parseada, "consistencia", consulta_separada[2]);
			dictionary_put(consulta_parseada, "particiones", particiones);
			dictionary_put(consulta_parseada, "compactacion", compactacion);
			dictionary_put(consulta_parseada, "timestamp", timestamp);


			/*puts(consulta_separada[0]);//CREATE
			puts(consulta_separada[1]);//tabla
			puts(consulta_separada[2]);//consistencia
			puts(consulta_separada[3]);//particiones
			puts(consulta_separada[4]);//compactacion
			puts(consulta_separada[5]);//timestamp*/

			log_info(LOGGER, "Consulta aceptada.");

		}
	}
	else if (strncmp(consulta_separada[0], "DESCRIBE", 8) == 0){

		if (length != 2){
			puts("ERROR: La sintaxis correcta es > DESCRIBE [NOMBRE_TABLA]");
			log_error(LOGGER, "Sintaxis incorrecta, chinguengencha!");
		}
		else{

			unsigned long int timestamp = string_to_ulint(consulta_separada[2]);

			dictionary_put(consulta_parseada, "operacion", consulta_separada[0]);
			dictionary_put(consulta_parseada, "tabla", consulta_separada[1]);
			dictionary_put(consulta_parseada, "timestamp", timestamp);

			/*puts(consulta_separada[0]);//DESCRIBE
			puts(consulta_separada[1]);//tabla
			puts(consulta_separada[2]);//timestamp*/

			log_info(LOGGER, "Consulta aceptada.");

		}
	}
	else if (strncmp(consulta_separada[0], "DROP", 4) == 0){

		if (length != 2){
			puts("ERROR: La sintaxis correcta es > DROP [NOMBRE_TABLA]");
			log_error(LOGGER, "Sintaxis incorrecta, chinguengencha!");
		}
		else{

			unsigned long int timestamp = string_to_ulint(consulta_separada[2]);

			dictionary_put(consulta_parseada, "operacion", consulta_separada[0]);
			dictionary_put(consulta_parseada, "tabla", consulta_separada[1]);
			dictionary_put(consulta_parseada, "timestamp", timestamp);

			/*puts(consulta_separada[0]);//DROP
			puts(consulta_separada[1]);//tabla
			puts(consulta_separada[2]);//timestamp*/

			log_info(LOGGER, "Consulta aceptada.");

		}
	}
	else if (strncmp(string_from_format("%s %s", consulta_separada[0], consulta_separada[1]), "ADD MEMORY", 10) == 0){

		if (length != 5){
			puts("ERROR: La sintaxis correcta es > ADD MEMORY [NÚMERO] TO [CRITERIO]");
			log_error(LOGGER, "Sintaxis incorrecta, chinguengencha!");
		}
		else{

			unsigned long int timestamp = string_to_ulint(consulta_separada[5]);

			dictionary_put(consulta_parseada, "operacion", string_from_format("%s %s", consulta_separada[0], consulta_separada[1]));
			dictionary_put(consulta_parseada, "memoria", consulta_separada[2]);
			dictionary_put(consulta_parseada, "criterio", consulta_separada[4]);
			dictionary_put(consulta_parseada, "timestamp", timestamp);

			/*
			puts(consulta_separada[0]);//ADD
			puts(consulta_separada[1]);//MEMORY
			puts(consulta_separada[2]);//memoria
			puts(consulta_separada[3]);//TO
			puts(consulta_separada[4]);//criterio
			puts(consulta_separada[5]);//timestamp*/

			log_info(LOGGER, "Consulta aceptada.");

		}
	}
	else if (strncmp(consulta_separada[0], "RUN", 3) == 0 ){

		if (length != 2){
			puts("ERROR: La sintaxis correcta es > RUN [ARCHIVO]");
			log_error(LOGGER, "Sintaxis incorrecta, chinguengencha!");
		}
		else{

			unsigned long int timestamp = string_to_ulint(consulta_separada[2]);

			dictionary_put(consulta_parseada, "operacion", consulta_separada[0]);
			dictionary_put(consulta_parseada, "archivo", consulta_separada[1]);
			dictionary_put(consulta_parseada, "timestamp", timestamp);

			/*puts(consulta_separada[0]);//RUN
			puts(consulta_separada[1]);//path archivo
			puts(consulta_separada[2]);//timestamp*/

			log_info(LOGGER, "Consulta aceptada.");

		}
	}
	else if (strncmp(consulta_separada[0], "METRICS", 7) == 0){

			if (length != 1){
				puts("ERROR: La sintaxis correcta es > METRICS");
				log_error(LOGGER, "Sintaxis incorrecta, chinguengencha!");
			}
			else{

				unsigned long int timestamp = string_to_ulint(consulta_separada[1]);

				dictionary_put(consulta_parseada, "operacion", consulta_separada[0]);
				dictionary_put(consulta_parseada, "timestamp", timestamp);

				/*puts(consulta_separada[0]);//METRICS
				puts(consulta_separada[1]);//timestamp*/

				log_info(LOGGER, "Consulta aceptada.");

			}
		}
	else if (strncmp(consulta_separada[0], "JOURNAL", 7) == 0){

			if (length != 1){
				puts("ERROR: La sintaxis correcta es > JOURNAL");
				log_error(LOGGER, "Sintaxis incorrecta, chinguengencha!");
			}
			else{

				unsigned long int timestamp = string_to_ulint(consulta_separada[1]);

				dictionary_put(consulta_parseada, "operacion", consulta_separada[0]);
				dictionary_put(consulta_parseada, "timestamp", timestamp);

				/*puts(consulta_separada[0]);//JOURNAL
				puts(consulta_separada[1]);//timestamp*/

				log_info(LOGGER, "Consulta aceptada.");

			}
		}
	else {
		puts("ERROR: Las operaciones disponibles son:");
		puts("Desde Kernel      >> [SELECT, INSERT, CREATE, DESCRIBE, DROP, ADD MEMORY, RUN, METRICS]");
		puts("Desde Pool Memory >> [SELECT, INSERT, CREATE, DESCRIBE, DROP, JOURNAL]");
		puts("Desde File System >> [SELECT, INSERT, CREATE, DESCRIBE, DROP]");
		log_error(LOGGER, "Operacion no reconocida.");
	}

	return consulta_parseada;
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

void print_dictionary(t_dictionary* dictionary){

	if(es_select(dictionary)){
		printf("%s %s %hu %lu \n",
				dictionary_get(dictionary, "operacion"),
				dictionary_get(dictionary, "tabla"),
			    dictionary_get(dictionary, "key"),
				dictionary_get(dictionary, "timestamp"));
	}
	else if (es_insert(dictionary)){
		printf("%s %s %hu %s %lu \n",
				dictionary_get(dictionary, "operacion"),
				dictionary_get(dictionary, "tabla"),
			    dictionary_get(dictionary, "key"),
			    dictionary_get(dictionary, "valor"),
				dictionary_get(dictionary, "timestamp"));

	}
	else if (es_create(dictionary)){
		printf("%s %s %s %hu %hu %lu \n",
				dictionary_get(dictionary, "operacion"),
				dictionary_get(dictionary, "tabla"),
			    dictionary_get(dictionary, "consistencia"),
			    dictionary_get(dictionary, "particiones"),
			    dictionary_get(dictionary, "compactacion"),
				dictionary_get(dictionary, "timestamp"));
	}

	else if (es_describe(dictionary)){
		printf("%s %s %lu \n",
				dictionary_get(dictionary, "operacion"),
				dictionary_get(dictionary, "tabla"),
				dictionary_get(dictionary, "timestamp"));
	}
	else if (es_drop(dictionary)){
		printf("%s %s %lu \n",
				dictionary_get(dictionary, "operacion"),
				dictionary_get(dictionary, "tabla"),
				dictionary_get(dictionary, "timestamp"));
	}
	else if (es_addmemory(dictionary)){
		printf("%s %s %s %lu \n",
				dictionary_get(dictionary, "operacion"),
				dictionary_get(dictionary, "memoria"),
				dictionary_get(dictionary, "criterio"),
				dictionary_get(dictionary, "timestamp"));
	}
	else if (es_run(dictionary)){
		printf("%s %s %lu \n",
				dictionary_get(dictionary, "operacion"),
				dictionary_get(dictionary, "archivo"),
				dictionary_get(dictionary, "timestamp"));
	}
	else if (es_metrics(dictionary)){
		printf("%s %lu \n",
				dictionary_get(dictionary, "operacion"),
				dictionary_get(dictionary, "timestamp"));
	}
	else if (es_journal(dictionary)){
		printf("%s %lu \n",
				dictionary_get(dictionary, "operacion"),
				dictionary_get(dictionary, "timestamp"));
	}

}

int es_select(t_dictionary* dictionary){
	return (strncmp(dictionary_get(dictionary, "operacion"), "SELECT", 6)) == 0;
}

bool es_insert(t_dictionary* dictionary){
	return (strncmp(dictionary_get(dictionary, "operacion"), "INSERT", 6)) == 0;
}

bool es_create(t_dictionary* dictionary){
	return (strncmp(dictionary_get(dictionary, "operacion"), "CREATE", 6)) == 0;
}

bool es_describe(t_dictionary* dictionary){
	return (strncmp(dictionary_get(dictionary, "operacion"), "DESCRIBE", 8)) == 0;
}

bool es_drop(t_dictionary* dictionary){
	return (strncmp(dictionary_get(dictionary, "operacion"), "DROP", 4)) == 0;
}

bool es_addmemory(t_dictionary* dictionary){
	return (strncmp(dictionary_get(dictionary, "operacion"), "ADD MEMORY", 10)) == 0;
}

bool es_run(t_dictionary* dictionary){
	return (strncmp(dictionary_get(dictionary, "operacion"), "RUN", 3)) == 0;
}

bool es_metrics(t_dictionary* dictionary){
	return (strncmp(dictionary_get(dictionary, "operacion"), "METRICS", 7)) == 0;
}

bool es_journal(t_dictionary* dictionary){
	return (strncmp(dictionary_get(dictionary, "operacion"), "JOURNAL", 7)) == 0;
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
