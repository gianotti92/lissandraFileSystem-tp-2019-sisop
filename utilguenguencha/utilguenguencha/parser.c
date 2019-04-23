
#include "parser.h"

void print_consulta(Instruccion consulta){

	printf("%i %i", consulta.instruccion, consulta.instruccion_a_realizar);
}

Instruccion parser_lql(char* consulta, Procesos procesoOrigen){

	Instruccion consultaParseada;

	Instruccion consultaParseadaError;
	consultaParseadaError.instruccion = ERROR;
	consultaParseadaError.instruccion_a_realizar = NULL;

	time_t echo_time;
	echo_time = time(NULL);

	if (echo_time == ((time_t)-1)){
		puts ("ERROR: Fallo al obtener la hora.");
		log_error(LOGGER, "Parser: Fallo al obtener la hora.");

		return consultaParseadaError;
	}

	consulta = string_from_format("%s %ju", consulta, (uintmax_t)echo_time);
	log_info(LOGGER, "Parser: Consulta - %s", consulta);

	char** consulta_separada = string_split(consulta, " ");

	int length = cantidad_elementos(consulta_separada) - 1;

	if (es_select(consulta_separada)){

		if (length != 3){
			puts("ERROR: La sintaxis correcta es > SELECT [NOMBRE_TABLA] [KEY]");
			log_error(LOGGER, "Parser: Sintaxis incorrecta, chinguengencha!");
			return consultaParseadaError;
		}
		else if(!es_numero(consulta_separada[2])|| string_to_ulint(consulta_separada[2])>65535){
			puts("ERROR: La Key debe ser un numero menor a 65.535.");
			log_error(LOGGER, "Parser: La Key es incorrecta.");
			return consultaParseadaError;
		}
		else{

			// es SELECT
			string_to_upper(consulta_separada[1]);
			nuevoSelect.nombre_tabla = consulta_separada[1]; 		  				// cargo tabla
			uint16_t key = (int) string_to_ulint(consulta_separada[2]);
			nuevoSelect.key = key;	 												// cargo key
			uint32_t timestamp = (uint32_t) string_to_ulint(consulta_separada[3]);
			nuevoSelect.timestamp = timestamp;										// cargo timestamp

			consultaParseada.instruccion = SELECT;
			consultaParseada.instruccion_a_realizar = &nuevoSelect;

		}
	}
	else if (es_insert(consulta_separada)){

		if (length < 4 ){
			puts("ERROR: La sintaxis correcta es > INSERT [NOMBRE_TABLA] [KEY] ”[VALUE]” ?[TIMESTAMP]");
			log_error(LOGGER, "Parser: Sintaxis incorrecta, chinguengencha!");
			return consultaParseadaError;
		}
		else if ( ((length != 4) & (length != 5)) | ((length == 5) & (!es_numero(consulta_separada[4]))) ){
			puts("ERROR: La sintaxis correcta es > INSERT [NOMBRE_TABLA] [KEY] ”[VALUE]” ?[TIMESTAMP]");
			log_error(LOGGER, "Parser: Sintaxis incorrecta, chinguengencha!");
			return consultaParseadaError;
		}
		else if (!es_numero(consulta_separada[2])|| string_to_ulint(consulta_separada[2])>65535){
			puts("ERROR: La Key debe ser un numero menor a 65.535.");
			log_error(LOGGER, "Parser: La Key es incorrecta.");
			return consultaParseadaError;
		}
		else{

			 // es INSERT
			string_to_upper(consulta_separada[1]);
			nuevoInsert.nombre_tabla = consulta_separada[1];	 						 // cargo tabla
			uint16_t key = (int) string_to_ulint(consulta_separada[2]);
			nuevoInsert.key = key;					 									 // cargo key

			nuevoInsert.value = consulta_separada[3];									 // cargo value

			if (length == 4) {															 // no vino con timestamp en la consulta
				uint32_t timestamp = (uint32_t) string_to_ulint(consulta_separada[4]);
				nuevoInsert.timestamp_insert = timestamp;								 // cargo timestamp_insert
				nuevoInsert.timestamp = timestamp;										 // cargo timestamp
			}
			else {																		 // vino con timestamp en la consulta
				uint32_t timestamp_insert = (uint32_t) string_to_ulint(consulta_separada[4]);
				nuevoInsert.timestamp_insert = timestamp_insert;				 		 // cargo timestamp_insert
				uint32_t timestamp = (uint32_t) string_to_ulint(consulta_separada[5]);
				nuevoInsert.timestamp = timestamp;										 // cargo timestamp
			}

			consultaParseada.instruccion = INSERT;
			consultaParseada.instruccion_a_realizar = &nuevoInsert;

		}
	}
	else if (es_create(consulta_separada)){
		if (length != 5){
			puts("ERROR: La sintaxis correcta es > CREATE [NOMBRE_TABLA] [TIPO_CONSISTENCIA] [NUMERO_PARTICIONES] [COMPACTION_TIME]");
			log_error(LOGGER, "Parser: Sintaxis incorrecta, chinguengencha!");
			return consultaParseadaError;
		}
		else if (!es_numero(consulta_separada[3])){
			puts("ERROR: La cantidad de particiones debe ser un numero.");
			log_error(LOGGER, "Parser: La cantidad de particiones debe ser un numero.");
			return consultaParseadaError;
		}
		else if (!es_numero(consulta_separada[4])){
			puts("ERROR: El tiempo de compactacion debe ser un numero.");
			log_error(LOGGER, "Parser: El tiempo de compactacion debe ser un numero.");
			return consultaParseadaError;
		}
		else{

			// es CREATE
			string_to_upper(consulta_separada[1]);
			nuevoCreate.nombre_tabla = consulta_separada[1]; 					// cargo tabla

			Consistencias consistencia;

			if (string_equals_ignore_case(consulta_separada[2],"SC")){
				consistencia = SC;
			}
			else if (string_equals_ignore_case(consulta_separada[2],"SHC")){
				consistencia = SHC;

			}else if (string_equals_ignore_case(consulta_separada[2],"EC")){
				consistencia = EC;

			}
			else {
				puts("ERROR: Los criterios de consistencia aceptados son [SC, SHC, EC]");
				log_error(LOGGER, "Parser: Criterio de consistencia no aceptado.");
				return consultaParseadaError;
			};

			nuevoCreate.consistencia = consistencia;									 // cargo consistencia
			uint8_t particiones = (int) string_to_ulint(consulta_separada[3]);
			nuevoCreate.particiones = particiones;										 // cargo particiones
			uint32_t compactation_time = (uint32_t) string_to_ulint(consulta_separada[4]);
			nuevoCreate.compactation_time = compactation_time;							 // cargo compactation_time
			uint32_t timestamp = (uint32_t) string_to_ulint(consulta_separada[5]);
			nuevoCreate.timestamp = timestamp;											 // cargo timestamp

			consultaParseada.instruccion = CREATE;
			consultaParseada.instruccion_a_realizar = &nuevoCreate;

		}
	}
	else if (es_describe(consulta_separada)){

		if (length != 2){
			puts("ERROR: La sintaxis correcta es > DESCRIBE [NOMBRE_TABLA]");
			log_error(LOGGER, "Parser: Sintaxis incorrecta, chinguengencha!");
			return consultaParseadaError;
		}
		else{

			// es DESCRIBE
			string_to_upper(consulta_separada[1]);
			nuevoDescribe.nombre_tabla = consulta_separada[1]; 							// cargo tabla
			uint32_t timestamp = (uint32_t) string_to_ulint(consulta_separada[2]);
			nuevoDescribe.timestamp = timestamp;				 				 		// cargo timestamp

			consultaParseada.instruccion = DESCRIBE;
			consultaParseada.instruccion_a_realizar = &nuevoDescribe;


		}
	}
	else if (es_drop(consulta_separada)){

		if (length != 2){
			puts("ERROR: La sintaxis correcta es > DROP [NOMBRE_TABLA]");
			log_error(LOGGER, "Parser: Sintaxis incorrecta, chinguengencha!");
			return consultaParseadaError;
		}
		else{

			// es DROP
			string_to_upper(consulta_separada[1]);
			nuevoDrop.nombre_tabla = consulta_separada[1]; 									// cargo tabla
			uint32_t timestamp = (uint32_t) string_to_ulint(consulta_separada[2]);
			nuevoDrop.timestamp = timestamp;												// cargo timestamp

			consultaParseada.instruccion = DROP;
			consultaParseada.instruccion_a_realizar = &nuevoDrop;


		}
	}
	else if (string_equals_ignore_case(string_from_format("%s %s", consulta_separada[0], consulta_separada[1]), "ADD MEMORY") & (string_equals_ignore_case(consulta_separada[3], "TO")) & (procesoOrigen == KERNEL)){

		if (length != 5){
			puts("ERROR: La sintaxis correcta es > ADD MEMORY [NÚMERO] TO [CRITERIO]");
			log_error(LOGGER, "Parser: Sintaxis incorrecta, chinguengencha!");
			return consultaParseadaError;
		}
		else if (!es_numero(consulta_separada[2])){
			puts("ERROR: La memoria debe ser un numero.");
			log_error(LOGGER, "Parser: La memoria debe ser un numero.");
			return consultaParseadaError;
		}
		else{

			// es ADD MEMORY

			nuevoAddMemory.memoria = (int) string_to_ulint(consulta_separada[2]);			// cargo memoria

			Consistencias consistencia;

			if (string_equals_ignore_case(consulta_separada[4],"SC")){
				consistencia = SC;
			}
			else if (string_equals_ignore_case(consulta_separada[4],"SHC")){
				consistencia = SHC;
			}
			else if (string_equals_ignore_case(consulta_separada[4],"EC")){
				consistencia = EC;
			}
			else {
				puts("ERROR: Los criterios de consistencia aceptados son [SC, SHC, EC]");
				log_error(LOGGER, "Parser: Criterio de consistencia no aceptado.");
				return consultaParseadaError;
			};

			nuevoAddMemory.consistencia = consistencia;									// cargo consistencia
			uint32_t timestamp = (uint32_t) string_to_ulint(consulta_separada[5]);
			nuevoAddMemory.timestamp = timestamp;					 		 			// cargo timestamp

			consultaParseada.instruccion = ADD;
			consultaParseada.instruccion_a_realizar = &nuevoAddMemory;


		}
	}
	else if (es_run(consulta_separada) & (procesoOrigen == KERNEL)){

		if (length != 2){
			puts("ERROR: La sintaxis correcta es > RUN [ARCHIVO]");
			log_error(LOGGER, "Parser: Sintaxis incorrecta, chinguengencha!");
			return consultaParseadaError;
		}
		else{

			// es RUN
			nuevoRun.path = consulta_separada[1]; 								// cargo path
			uint32_t timestamp = (uint32_t) string_to_ulint(consulta_separada[2]);
			nuevoRun.timestamp = timestamp;				 				 				// cargo timestamp

			consultaParseada.instruccion = RUN;
			consultaParseada.instruccion_a_realizar = &nuevoRun;


		}
	}
	else if (es_metrics(consulta_separada) & (procesoOrigen == KERNEL)){

			if (length != 1){
				puts("ERROR: La sintaxis correcta es > METRICS");
				log_error(LOGGER, "Parser: Sintaxis incorrecta, chinguengencha!");
				return consultaParseadaError;
			}
			else{

				// es METRICS
				uint32_t timestamp = (uint32_t) string_to_ulint(consulta_separada[1]);
				nuevoMetrics.timestamp = timestamp;						 				 	// cargo timestamp

				consultaParseada.instruccion = METRICS;
				consultaParseada.instruccion_a_realizar = &nuevoMetrics;

			}
		}
	else if (es_journal(consulta_separada) & (procesoOrigen == POOLMEMORY)){

			if (length != 1){
				puts("ERROR: La sintaxis correcta es > JOURNAL");
				log_error(LOGGER, "Parser: Sintaxis incorrecta, chinguengencha!");
				return consultaParseadaError;
			}
			else{

				// es JOURNAL
				uint32_t timestamp = (uint32_t) string_to_ulint(consulta_separada[1]);
				nuevoJournal.timestamp = timestamp;						 				 	// cargo timestamp

				consultaParseada.instruccion = JOURNAL;
				consultaParseada.instruccion_a_realizar = &nuevoJournal;

			}
		}
	else {
		puts("ERROR: Las operaciones disponibles son:");
		if ((procesoOrigen == KERNEL)) puts("[SELECT, INSERT, CREATE, DESCRIBE, DROP, ADD MEMORY, RUN, METRICS]");
		if ((procesoOrigen == POOLMEMORY)) puts("[SELECT, INSERT, CREATE, DESCRIBE, DROP, JOURNAL]");
		if ((procesoOrigen == FILESYSTEM)) puts("[SELECT, INSERT, CREATE, DESCRIBE, DROP]");
		log_error(LOGGER, "Parser: Operacion no reconocida.");
		return consultaParseadaError;
	}

	log_info(LOGGER, "Parser: Consulta aceptada.");
	return consultaParseada;
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

void* leer_por_consola(void (*f) (char*)){
	char* leido;

	leido = readline(">>");
	add_history(leido);

	while (strncmp(leido, "EXIT", 4) != 0){
		f(leido);
		free(leido);
		leido = readline("\n>>");
		add_history(leido);
	}

	free(leido);
	log_info(LOGGER, "Salida del sistema por consola");
	exit_gracefully(EXIT_SUCCESS);
}
