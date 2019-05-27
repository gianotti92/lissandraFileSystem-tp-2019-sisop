#include "lissandra.h"

int _insert(Insert* insert);
int _select(Select* select);
int _create(Create* create);
int _describe(Describe* describe);
int _drop(Drop* drop);

int _insert(Insert* insert){
	if(!tableExists(insert->nombre_tabla)) {
		log_error(LOGGER,"INSERT: no existe la tabla '%s'",insert->nombre_tabla);
		return 1;
	}
	if(strlen(insert->value)>global_conf.max_value_size) {
		log_error(LOGGER,"INSERT: value demasiado largo: '%d', maximo: '%d'",strlen(insert->value),global_conf.max_value_size);
		return 1;
	}
	struct tableRegister reg = createTableRegister(insert->key,insert->value,insert->timestamp);
	/* Espero posible Dump*/
	pthread_mutex_lock(&memtableMutex);
	/* Inserto valor en la memtable */
	insertInMemtable(createMemtableItem(insert->nombre_tabla,reg));
	/* desbloqueo mutex*/
	pthread_mutex_unlock(&memtableMutex);
	return 0;
}
int _select(Select* select){
	if(!tableExists(select->nombre_tabla)) {
		log_error(LOGGER,"SELECT: no existe la tabla '%s'",select->nombre_tabla);
		return 1;
	}
	struct tableMetadataItem* found = get_table_metadata(select->nombre_tabla);
	if(found == NULL) {
		log_error(LOGGER,"SELECT: No se encontro la metadata de la tabla '%s'",select->nombre_tabla);
		return 1;
	}

	t_list* listaRegistros = list_create();

	// Reviso Memtable
	findInMemtable(select->nombre_tabla,select->key,listaRegistros);

	int particion = select->key%found->metadata.numero_particiones;
	char*particionFile = malloc(strlen(global_conf.directorio_tablas)+strlen(select->nombre_tabla)+digitos(particion)+6);
	sprintf(particionFile,"%s%s/%d.bin",global_conf.directorio_tablas,select->nombre_tabla,particion);

	// Espero posible compactacion
	pthread_rwlock_rdlock(&found->lock);

	// Reviso particion
	if(fs_read(particionFile,listaRegistros)!=0){
		clean_registers_list(listaRegistros);
		list_destroy(listaRegistros);
		pthread_rwlock_unlock(&found->lock);
		free(particionFile);
		return 1;
	}
	free(particionFile);

	// Reviso archivos temporales
	if(read_temp_files(select->nombre_tabla,listaRegistros)!=0){
		clean_registers_list(listaRegistros);
		list_destroy(listaRegistros);
		pthread_rwlock_unlock(&found->lock);
		return 1;
	}

	//Levanto el bloqueo
	pthread_rwlock_unlock(&found->lock);
	
	// Busco el mayor
	struct tableRegister max;
	max.value=NULL;
	max.timestamp=0l;
	void lastest(struct tableRegister * reg){
		if(reg->key == select->key) {
			if(max.timestamp < reg->timestamp){ /// SERIO PROBLEMA CON EL MATCHING DE TIMESTAMPS
				if(max.value!=NULL){
					free(max.value);
				}
				max = createTableRegister(reg->key,reg->value,reg->timestamp);
			}
		}
	}
	list_iterate(listaRegistros,(void*)lastest);


	if(max.value==NULL){
		log_error(LOGGER,"SELECT: No existe la key %d en la tabla '%s'",select->key,select->nombre_tabla);
		clean_registers_list(listaRegistros);
		list_destroy(listaRegistros);
		free(max.value);
		return 1;
	}

	clean_registers_list(listaRegistros);
	list_destroy(listaRegistros);
	printf("Key: %d, Value: %s, Timestamp: %lu\n",max.key,max.value,max.timestamp);
	free(max.value);

	return 0;
}
int _create(Create* create){
	char*directorio=getTablePath(create->nombre_tabla);
	if(crearDirectorio(global_conf.directorio_tablas,create->nombre_tabla)!=0){
		free(directorio);
		return 1;
	}
	if(crearMetadataTableFile(directorio,setTableMetadata(create->consistencia,create->particiones,create->compactation_time)) != 0) {
		free(directorio);
		return 1;
	}

	if(crearBinarios(directorio,create->particiones)!=0) {
		free(directorio);
		return 1;
	}
	free(directorio);
	/* Agrego la nueva tabla a la metadata global */
	pthread_mutex_lock(&tableMetadataMutex);
	insertInTableMetadata(create->nombre_tabla,setTableMetadata(create->consistencia,create->particiones,create->compactation_time));
	pthread_mutex_unlock(&tableMetadataMutex);
	return 0;
}
int _describe(Describe * describe){
	if(!tableExists(describe->nombre_tabla)) {
		log_error(LOGGER,"DESCRIBE: no existe la tabla '%s'",describe->nombre_tabla);
		return 1;
	}
	struct tableMetadataItem* found = get_table_metadata(describe->nombre_tabla);
	if(found==NULL){
		log_error(LOGGER,"DESCRIBE: No se encontro la metadata de la tabla %s",describe->nombre_tabla);
		return 1;
	}
	char*consistencia=consistencia2string(found->metadata.consistencia);
	printf("Consistency: %s, Partitions: %d, Compaction time: %d\n",consistencia,found->metadata.numero_particiones,found->metadata.compaction_time);
	free(consistencia);
	return 0;
}
int _drop(Drop* drop){
	if(!tableExists(drop->nombre_tabla)) {
		log_error(LOGGER,"DROP: no existe la tabla '%s'",drop->nombre_tabla);
		return 1;
	}
	pthread_mutex_lock(&tableMetadataMutex);
	deleteInTableMetadata(drop->nombre_tabla);
	pthread_mutex_unlock(&tableMetadataMutex);
	if(deleteTable(drop->nombre_tabla)!=0){
		return 1;
	}
	return 0;
}
int controller(Instruccion* instruccion){
	int ret=0;
	switch(instruccion->instruccion){
		case SELECT:
			ret=_select((Select*)instruccion->instruccion_a_realizar);
		break;
		case INSERT:
			ret=_insert((Insert*)instruccion->instruccion_a_realizar);
		break;
		case CREATE:
			ret=_create((Create*)instruccion->instruccion_a_realizar);
		break;
		case DESCRIBE:
			ret=_describe((Describe*)instruccion->instruccion_a_realizar);
		break;
		case DROP:
			ret=_drop((Drop*)instruccion->instruccion_a_realizar);
		break;
		default:
			// veo
		break;			
	}
	return ret;
}
