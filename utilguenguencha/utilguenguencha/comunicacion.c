#include "comunicacion.h"

size_t sizeof_instruccion(Instruccion* instruccion){
	Instruction_set tipo = instruccion->instruccion;
	switch(tipo){
		case SELECT:
			return sizeof_select(instruccion->instruccion_a_realizar);
		case INSERT:
			return sizeof_insert(instruccion->instruccion_a_realizar);
		case CREATE:
			return sizeof_create(instruccion->instruccion_a_realizar);
		case DESCRIBE:
			return sizeof_describe(instruccion->instruccion_a_realizar);
		case DROP:
			return sizeof_drop(instruccion->instruccion_a_realizar);
		case ADD:
			return sizeof_add(instruccion->instruccion_a_realizar);
		case RUN:
			return sizeof_run(instruccion->instruccion_a_realizar);
		case JOURNAL:
			return sizeof_journal(instruccion->instruccion_a_realizar);
		case METRICS:
			return sizeof_metrics(instruccion->instruccion_a_realizar);
		default:
			return ERROR;
	}
}

size_t sizeof_select(Select *select){
	return sizeof(select->key)+
		   (strlen(select->nombre_tabla)+1)*sizeof(char)+
		   sizeof(select->timestamp);
}

size_t sizeof_insert(Insert *insert){
	return sizeof(insert->key)+
		   (strlen(insert->nombre_tabla)+1)*sizeof(char)+
		   sizeof(insert->timestamp)+
		   (strlen(insert->value)+1)*sizeof(char);
}

size_t sizeof_create(Create *create){
	return sizeof(create->compactation_time)+
		   sizeof(create->consistencia)+
		   (strlen(create->nombre_tabla)+1)*sizeof(char)+
		   sizeof(create->particiones)+
		   sizeof(create->timestamp);
}

size_t sizeof_describe(Describe *describe){
	return (strlen(describe->nombre_tabla)+1)*sizeof(char)+
			sizeof(describe->timestamp);
}

size_t sizeof_drop(Drop *drop){
	return (strlen(drop->nombre_tabla)+1)*sizeof(char)+
			sizeof(drop->timestamp);
}

size_t sizeof_add(Add *add){
	return sizeof(add->consistencia)+
		   sizeof(add->memoria)+
		   sizeof(add->timestamp);
}

size_t sizeof_run(Run *run){
	return (strlen(run->path)+1)*sizeof(char)+
		    sizeof(run->timestamp);
}

size_t sizeof_journal(Journal *journal){
	return sizeof(journal->timestamp);
}

size_t sizeof_metrics(Metrics *metrics){
	return sizeof(metrics->timestamp);
}




