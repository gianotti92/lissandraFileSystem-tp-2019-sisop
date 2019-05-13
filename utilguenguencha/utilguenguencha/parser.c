
#include "parser.h"

Instruccion* parser_lql(char* consulta, Procesos procesoOrigen){

	Instruccion* consultaParseada;

	/*
	time_t echo_time;
	echo_time = time(NULL);

	if (echo_time == ((time_t)-1)){
		puts ("ERROR: Fallo al obtener la hora.");
		log_error(LOGGER, "Parser: Fallo al obtener la hora.");

		return instruccion_error();
	}*/

	uintmax_t echo_time = get_timestamp();

	consulta = string_from_format("%s %ju", consulta, (uintmax_t)echo_time);
	log_info(LOGGER, "Parser: Consulta - %s", consulta);

	char** consulta_separada = string_split(consulta, " ");

	int length = cantidad_elementos(consulta_separada) - 1;

	if (es_select(consulta_separada)){

		if (length != 3){
			puts("ERROR: La sintaxis correcta es > SELECT [NOMBRE_TABLA] [KEY]");
			log_error(LOGGER, "Parser: Sintaxis incorrecta, chinguengencha!");

			return instruccion_error();
		}
		else if(!es_numero(consulta_separada[2])|| string_to_ulint(consulta_separada[2])>65535){
			puts("ERROR: La Key debe ser un numero menor a 65.535.");
			log_error(LOGGER, "Parser: La Key es incorrecta.");

			return instruccion_error();
		}
		else{

			// es SELECT

			Select * nuevoSelect = malloc(sizeof(Select));

			string_to_upper(consulta_separada[1]);
			nuevoSelect->nombre_tabla = consulta_separada[1]; 		  				// cargo tabla
			uint16_t key = (int) string_to_ulint(consulta_separada[2]);
			nuevoSelect->key = key;	 												// cargo key
			uint32_t timestamp = (uint32_t) string_to_ulint(consulta_separada[3]);
			nuevoSelect->timestamp = timestamp;										// cargo timestamp

			//Select * p_select = malloc(sizeof(nuevoSelect));
			//p_select = &nuevoSelect;

			//consultaParseada->instruccion = SELECT;
			//consultaParseada->instruccion_a_realizar = p_select;
			consultaParseada = crear_instruccion(SELECT, nuevoSelect, sizeof(nuevoSelect));

		}
	}
	else if (es_insert(consulta_separada)){


		if (length < 4){

			puts("ERROR: La sintaxis correcta es > INSERT [NOMBRE_TABLA] [KEY] ”[VALUE]” ?[TIMESTAMP]");
			log_error(LOGGER, "Parser: Sintaxis incorrecta, chinguengencha!");

			return instruccion_error();
		}
		if (length >= 5){

			int largo_primer_palabra = string_length(consulta_separada[3]);

		if(length >= 5 && string_starts_with(consulta_separada[3], "\"") && (!string_ends_with(consulta_separada[3], "\"") || largo_primer_palabra == 1)){

				int i = 0;

				while((!string_ends_with(consulta_separada[3], "\"") || i == 0) && (i+4 < length)){
					consulta_separada[3] = string_from_format("%s %s", consulta_separada[3], consulta_separada[4]);
					i++;
					consulta_separada[4] = consulta_separada[4+i];
				}
				consulta_separada[5] = consulta_separada[5+i];
				consulta_separada[6] = consulta_separada[6+i];

				length = length - i;
			}


			if (length > 5){

				puts("ERROR: La sintaxis correcta es > INSERT [NOMBRE_TABLA] [KEY] ”[VALUE]” ?[TIMESTAMP]");
				log_error(LOGGER, "Parser: Sintaxis incorrecta, chinguengencha!");

				return instruccion_error();
			}
		}

		if ((length == 5) && !es_numero(consulta_separada[4])){
			puts("ERROR: La sintaxis correcta es > INSERT [NOMBRE_TABLA] [KEY] ”[VALUE]” ?[TIMESTAMP]");
			log_error(LOGGER, "Parser: Sintaxis incorrecta, chinguengencha!");

			return instruccion_error();
		}
		else if (!es_numero(consulta_separada[2])|| string_to_ulint(consulta_separada[2])>65535){
			puts("ERROR: La Key debe ser un numero menor a 65.535.");
			log_error(LOGGER, "Parser: La Key es incorrecta.");

			return instruccion_error();
		}
		else{

			 // es INSERT
			Insert* nuevoInsert = malloc(sizeof(Insert));

			string_to_upper(consulta_separada[1]);
			nuevoInsert->nombre_tabla = consulta_separada[1];	 						 // cargo tabla
			uint16_t key = (int) string_to_ulint(consulta_separada[2]);
			nuevoInsert->key = key;					 									 // cargo key

			nuevoInsert->value = consulta_separada[3];									 // cargo value

			if (length == 4) {															 // no vino con timestamp en la consulta
				uint32_t timestamp = (uint32_t) string_to_ulint(consulta_separada[4]);
				nuevoInsert->timestamp_insert = timestamp;								 // cargo timestamp_insert
				nuevoInsert->timestamp = timestamp;										 // cargo timestamp
			}
			else {																		 // vino con timestamp en la consulta
				uint32_t timestamp_insert = (uint32_t) string_to_ulint(consulta_separada[4]);
				nuevoInsert->timestamp_insert = timestamp_insert;				 		 // cargo timestamp_insert
				uint32_t timestamp = (uint32_t) string_to_ulint(consulta_separada[5]);
				nuevoInsert->timestamp = timestamp;										 // cargo timestamp
			}


			consultaParseada = crear_instruccion(INSERT, nuevoInsert, sizeof(nuevoInsert));

		}
	}
	else if (es_create(consulta_separada)){
		if (length != 5){
			puts("ERROR: La sintaxis correcta es > CREATE [NOMBRE_TABLA] [TIPO_CONSISTENCIA] [NUMERO_PARTICIONES] [COMPACTION_TIME]");
			log_error(LOGGER, "Parser: Sintaxis incorrecta, chinguengencha!");

			return instruccion_error();
		}
		else if (!es_numero(consulta_separada[3])){
			puts("ERROR: La cantidad de particiones debe ser un numero.");
			log_error(LOGGER, "Parser: La cantidad de particiones debe ser un numero.");

			return instruccion_error();
		}
		else if (!es_numero(consulta_separada[4])){
			puts("ERROR: El tiempo de compactacion debe ser un numero.");
			log_error(LOGGER, "Parser: El tiempo de compactacion debe ser un numero.");

			return instruccion_error();
		}
		else{

			// es CREATE
			Create * nuevoCreate = malloc(sizeof(Create));

			string_to_upper(consulta_separada[1]);
			nuevoCreate->nombre_tabla = consulta_separada[1]; 					// cargo tabla

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

				return instruccion_error();
			};

			nuevoCreate->consistencia = consistencia;									 // cargo consistencia
			uint8_t particiones = (int) string_to_ulint(consulta_separada[3]);
			nuevoCreate->particiones = particiones;										 // cargo particiones
			uint32_t compactation_time = (uint32_t) string_to_ulint(consulta_separada[4]);
			nuevoCreate->compactation_time = compactation_time;							 // cargo compactation_time
			uint32_t timestamp = (uint32_t) string_to_ulint(consulta_separada[5]);
			nuevoCreate->timestamp = timestamp;											 // cargo timestamp


			consultaParseada = crear_instruccion(CREATE, nuevoCreate, sizeof(nuevoCreate));

		}
	}
	else if (es_describe(consulta_separada)){

		if (length != 2){
			puts("ERROR: La sintaxis correcta es > DESCRIBE [NOMBRE_TABLA]");
			log_error(LOGGER, "Parser: Sintaxis incorrecta, chinguengencha!");

			return instruccion_error();
		}
		else{

			// es DESCRIBE
			Describe * nuevoDescribe = malloc(sizeof(Describe));;

			string_to_upper(consulta_separada[1]);
			nuevoDescribe->nombre_tabla = consulta_separada[1]; 							// cargo tabla
			uint32_t timestamp = (uint32_t) string_to_ulint(consulta_separada[2]);
			nuevoDescribe->timestamp = timestamp;				 				 		// cargo timestamp

			consultaParseada = crear_instruccion(DESCRIBE, nuevoDescribe, sizeof(nuevoDescribe));


		}
	}
	else if (es_drop(consulta_separada)){

		if (length != 2){
			puts("ERROR: La sintaxis correcta es > DROP [NOMBRE_TABLA]");
			log_error(LOGGER, "Parser: Sintaxis incorrecta, chinguengencha!");

			return instruccion_error();
		}
		else{

			// es DROP
			Drop* nuevoDrop = malloc(sizeof(Drop));;

			string_to_upper(consulta_separada[1]);
			nuevoDrop->nombre_tabla = consulta_separada[1]; 									// cargo tabla
			uint32_t timestamp = (uint32_t) string_to_ulint(consulta_separada[2]);
			nuevoDrop->timestamp = timestamp;												// cargo timestamp

			consultaParseada = crear_instruccion(DROP, nuevoDrop, sizeof(nuevoDrop));


		}
	}
	else if (es_add(consulta_separada) & (procesoOrigen == KERNEL)){

		if (length != 5){
			puts("ERROR: La sintaxis correcta es > ADD MEMORY [NÚMERO] TO [CRITERIO]");
			log_error(LOGGER, "Parser: Sintaxis incorrecta, chinguengencha!");

			return instruccion_error();
		}
		else if (!es_numero(consulta_separada[2])){
			puts("ERROR: La memoria debe ser un numero.");
			log_error(LOGGER, "Parser: La memoria debe ser un numero.");

			return instruccion_error();
		}
		else if (!(string_equals_ignore_case(string_from_format("%s %s", consulta_separada[0], consulta_separada[1]), "ADD MEMORY") & (string_equals_ignore_case(consulta_separada[3], "TO")))){
			puts("ERROR: La sintaxis correcta es > ADD MEMORY [NÚMERO] TO [CRITERIO]");
			log_error(LOGGER, "Parser: Sintaxis incorrecta, chinguengencha!");

			return instruccion_error();
		}
		else{

			// es ADD MEMORY
			Add * nuevoAddMemory = malloc(sizeof(Add));

			nuevoAddMemory->memoria = (int) string_to_ulint(consulta_separada[2]);			// cargo memoria

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

				return instruccion_error();
			}

			nuevoAddMemory->consistencia = consistencia;									// cargo consistencia
			uint32_t timestamp = (uint32_t) string_to_ulint(consulta_separada[5]);
			nuevoAddMemory->timestamp = timestamp;					 		 			// cargo timestamp


			consultaParseada = crear_instruccion(ADD, nuevoAddMemory, sizeof(nuevoAddMemory));

		}
	}
	else if (es_run(consulta_separada) & (procesoOrigen == KERNEL)){

		if (length != 2){
			puts("ERROR: La sintaxis correcta es > RUN [ARCHIVO]");
			log_error(LOGGER, "Parser: Sintaxis incorrecta, chinguengencha!");

			return instruccion_error();
		}
		else{

			// es RUN
			Run * nuevoRun = malloc(sizeof(Run));;

			nuevoRun->path = consulta_separada[1]; 								// cargo path
			uint32_t timestamp = (uint32_t) string_to_ulint(consulta_separada[2]);
			nuevoRun->timestamp = timestamp;				 				 				// cargo timestamp

			consultaParseada = crear_instruccion(RUN, nuevoRun, sizeof(nuevoRun));


		}
	}
	else if (es_metrics(consulta_separada) & (procesoOrigen == KERNEL)){

			if (length != 1){
				puts("ERROR: La sintaxis correcta es > METRICS");
				log_error(LOGGER, "Parser: Sintaxis incorrecta, chinguengencha!");

				return instruccion_error();
			}
			else{

				// es METRICS
				Metrics* nuevoMetrics = malloc(sizeof(Metrics));

				uint32_t timestamp = (uint32_t) string_to_ulint(consulta_separada[1]);
				nuevoMetrics->timestamp = timestamp;						 				 	// cargo timestamp

				consultaParseada = crear_instruccion(METRICS, nuevoMetrics, sizeof(nuevoMetrics));

			}
		}
	else if (es_journal(consulta_separada) & (procesoOrigen == POOLMEMORY || procesoOrigen == KERNEL)){

			if (length != 1){
				puts("ERROR: La sintaxis correcta es > JOURNAL");
				log_error(LOGGER, "Parser: Sintaxis incorrecta, chinguengencha!");

				return instruccion_error();
			}
			else{

				// es JOURNAL
				Journal* nuevoJournal = malloc(sizeof(Journal));

				uint32_t timestamp = (uint32_t) string_to_ulint(consulta_separada[1]);
				nuevoJournal->timestamp = timestamp;						 				 	// cargo timestamp

				consultaParseada = crear_instruccion(JOURNAL, nuevoJournal, sizeof(nuevoJournal));

			}
		}
	else {
		puts("ERROR: Las operaciones disponibles son:");

		switch(procesoOrigen){
			case KERNEL: {puts("[SELECT, INSERT, CREATE, DESCRIBE, DROP, ADD MEMORY, RUN, METRICS, JOURNAL]"); break;}
			case POOLMEMORY: {puts("[SELECT, INSERT, CREATE, DESCRIBE, DROP, JOURNAL]"); break;}
			case FILESYSTEM: {puts("[SELECT, INSERT, CREATE, DESCRIBE, DROP]"); break;}
		}

		log_error(LOGGER, "Parser: Operacion no reconocida.");

		return instruccion_error();
	}

	log_info(LOGGER, "Parser: Consulta aceptada.");

	//print_instruccion_parseada(consultaParseada);

	return consultaParseada;
}


Instruccion* crear_instruccion(Instruction_set operacion, void* operacion_a_realizar, int tamanio){

	void * p_instruccion_a_realizar = malloc(sizeof(tamanio));
	p_instruccion_a_realizar = operacion_a_realizar;

	Instruccion * p_instruccion = malloc(sizeof(Instruccion));
	p_instruccion->instruccion = operacion;
	p_instruccion->instruccion_a_realizar = operacion_a_realizar;

	return p_instruccion;

}

Instruccion* instruccion_error(){
	return crear_instruccion(ERROR, NULL, 1);
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

void print_instruccion_parseada(Instruccion * instruccion_parseada){

	switch(instruccion_parseada->instruccion){
			case SELECT: {Select * select = instruccion_parseada->instruccion_a_realizar;
						 printf("Tabla: %s Key: %i TS: %lu \n",select->nombre_tabla, select->key, select->timestamp);
						 break;}
			case INSERT: {Insert * insert = instruccion_parseada->instruccion_a_realizar;
						 printf("Tabla: %s Key: %i Valor: %s TSins: %lu TS: %lu \n",insert->nombre_tabla,insert->key, insert->value, insert->timestamp_insert, insert->timestamp);
						 break;}
			case CREATE: {Create * create = instruccion_parseada->instruccion_a_realizar;
						 printf("Tabla: %s Particiones: %i Compactacion: %lu Consistencia: %i TS: %lu \n",create->nombre_tabla,create->particiones, create->compactation_time, create->consistencia, create->timestamp);
						 break;}
			case DESCRIBE: {Describe * describe = instruccion_parseada->instruccion_a_realizar;
							printf("Tabla: %s TS: %lu\n",describe->nombre_tabla, describe->timestamp);
							break;}
			case ADD: {Add * add = instruccion_parseada->instruccion_a_realizar;
					   printf("Memoria: %i Consistencia: %i TS: %lu\n",add->memoria, add->consistencia, add->timestamp);
					   break;}
			case RUN: {Run * run = instruccion_parseada->instruccion_a_realizar;
					   printf("Path: %s TS: %lu\n",run->path, run->timestamp);
					   break;}
			case DROP: {Drop * drop = instruccion_parseada->instruccion_a_realizar;
						printf("Tabla: %s TS: %lu\n",drop->nombre_tabla, drop->timestamp);
						break;}
			case JOURNAL: {Journal * journal = instruccion_parseada->instruccion_a_realizar;
						   printf("TS: %lu \n",journal->timestamp);
						   break;}
			case METRICS: {Metrics * metrics = instruccion_parseada->instruccion_a_realizar;
						   printf("TS: %lu \n",metrics->timestamp);
						   break;}
			case ERROR: printf("ERROR DE CONSULTA \n");
		}

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

bool es_add(char** consulta_parseada){
	return (string_equals_ignore_case(consulta_parseada[0], "ADD"));
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

	while (!string_equals_ignore_case(leido, "EXIT")){
		f(leido);
		free(leido);
		leido = readline("\n>>");
		add_history(leido);
	}

	free(leido);
	log_info(LOGGER, "Salida del sistema por consola");
	exit_gracefully(EXIT_SUCCESS);
}


void free_consulta(Instruccion* consulta){
	free(consulta->instruccion_a_realizar);
	free(consulta);
}



uintmax_t get_timestamp(){

	time_t echo_time;
	echo_time = time(NULL);

	if (echo_time == ((time_t)-1)){
		puts ("ERROR: Fallo al obtener la hora.");
		log_error(LOGGER, "Parser: Fallo al obtener la hora.");

		return instruccion_error();
	}

	return echo_time;

}

