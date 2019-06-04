#include "lissandra.h"

struct tableRegister createTableRegister(uint16_t key,char* value,long timestamp) {
	struct tableRegister reg;
	reg.key=key;
	reg.value=malloc(global_conf.max_value_size);
	strcpy(reg.value,value);
	reg.timestamp = (timestamp==0)?getTimestamp():timestamp;
	return reg;
}

/*
	Global Conf
*/
void global_conf_load(t_config* conf){
	// Directorios
	char * p_m = config_get_string_value(conf,"PUNTO_MONTAJE");
	global_conf.directorio_tablas = malloc(strlen(p_m)+8+1);
	sprintf(global_conf.directorio_tablas,"%s/Tables/",p_m);
	global_conf.directorio_bloques = malloc(strlen(p_m)+9+1);
	sprintf(global_conf.directorio_bloques,"%s/Bloques/",p_m);
	global_conf.directorio_metadata = malloc(strlen(p_m)+10+1);
	sprintf(global_conf.directorio_metadata,"%s/Metadata/",p_m);
	// Puerto escucha
	char * p_e = config_get_string_value(conf,"PUERTO_ESCUCHA");
	global_conf.puerto=malloc(strlen(p_e)+1);
	strcpy(global_conf.puerto,p_e);
	// Retardo
	global_conf.retardo = config_get_int_value(conf,"RETARDO");
	// Value
	global_conf.max_value_size = config_get_int_value(conf,"MAX_VALUE_SIZE");
	// Dump
	global_conf.tiempo_dump = config_get_int_value(conf,"TIEMPO_DUMP");
}
void global_conf_update(t_config* conf){
	global_conf.retardo = config_get_int_value(conf,"RETARDO");
	global_conf.tiempo_dump = config_get_int_value(conf,"TIEMPO_DUMP");
}
void global_conf_destroy(void){
	free(global_conf.directorio_tablas);
	free(global_conf.directorio_bloques);
	free(global_conf.directorio_metadata);
	free(global_conf.puerto);
}

long getTimestamp(void) {
	struct timeval t;
	gettimeofday(&t,NULL);
	return t.tv_sec*1000+t.tv_usec/1000;
}
int tableExists(char * table){
	char* path = getTablePath(table);
	int retval = existeDirectorio(path);
	free(path);
	return retval;
}
int getNumLastFile(char* prefix,char* extension,char* path) {
	int retnum = 0;
	struct dirent *dir;
	DIR *d = opendir(path);
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			if (dir->d_type == DT_REG){
				//Verifico si empieza con el prefijo y termina con la extension correspondiente
				if(strstr(dir->d_name,prefix)==dir->d_name && strstr(dir->d_name,extension) == dir->d_name+(strlen(dir->d_name)-strlen(extension))) {				
					int currnum;
					//Seteo como fin de string justo cuando comienza la extension
					*(dir->d_name+(strlen(dir->d_name)-strlen(extension)-1))='\0';
					char*strnum=malloc(strlen(dir->d_name+strlen(prefix))+1);
					//Copio el valor numerico del archivo
					strcpy(strnum,dir->d_name+strlen(prefix));
					sscanf(strnum,"%d",&currnum);
					free(strnum);
					retnum = (currnum>retnum)?currnum:retnum;
				}
			}
		}
		closedir(d);
	}
	return retnum;
}

int monitorNode(char * node,int mode,int(*callback)(void)){
	char buffer[EVENT_BUF_LEN];
	int infd = inotify_init();
	if (infd < 0) {
		log_error(LOGGER,"Problemas al crear el inotify para %s, %s",node,strerror(errno));
		return 1;
	}
	int wfd = inotify_add_watch(infd,node,mode);
	while(1){
		int length = read(infd,buffer,EVENT_BUF_LEN);
		if (length < 0) {
			log_error(LOGGER,"Problemas al leer el inotify de %s, %s",node,strerror(errno));
			inotify_rm_watch(infd,wfd);
			close(infd);
			return 1;
		}
		if((*callback)()!=0){
			inotify_rm_watch(infd,wfd);
			close(infd);
			return 1;
		}
		inotify_rm_watch(infd,wfd);
		wfd = inotify_add_watch(infd,node,mode);
	}
	inotify_rm_watch(infd,wfd);
	close(infd);
	return 0;
}
struct tableMetadataItem* get_table_metadata(char* tabla){
	bool criterioTablename(struct tableMetadataItem* t){
		return !strcmp(tabla,t->tableName);
	}
	pthread_mutex_lock(&tableMetadataMutex);
	struct tableMetadataItem* found = list_find(global_table_metadata,(void*)criterioTablename);
	pthread_mutex_unlock(&tableMetadataMutex);
	return found;
}
int deleteTable(char* tabla){
	char*path = getTablePath(tabla);
	struct dirent *dir;
	DIR *d = opendir(path);
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			if (dir->d_type == DT_REG){
				char*filename=malloc(strlen(path)+strlen(dir->d_name)+2);
				sprintf(filename,"%s/%s",path,dir->d_name);
				if(strcmp(dir->d_name,"Metadata.txt")==0){
					if(remove(filename)){
						log_error(LOGGER,"Error al eliminar el archivo '%s', %s",filename,strerror(errno));
						closedir(d);
						free(path);
						free(filename);
						return 1;
					}
				} else {
					if(fs_delete(filename)!=0){
						closedir(d);
						free(path);
						free(filename);
						return 1;
					}
				}
				free(filename);
			}
		}
		closedir(d);
	} else {
		log_error(LOGGER,"Error al abrir el directorio %s, %s",path, strerror(errno));
		free(path);
		return 1;
	}
	if(remove(path)){
		log_error(LOGGER,"Error al eliminar el directorio '%s', %s",path,strerror(errno));
		free(path);
		return 1;
	}
	free(path);
	return 0;
}
void clean_registers_list(t_list*registers){
	void cleanValue(struct tableRegister* reg){
		free(reg->value);
	}
	list_iterate(registers,(void*)cleanValue);
}
int read_temp_files(char* tabla,t_list* listaRegistros){
	char*path = getTablePath(tabla);
	struct dirent *dir;
	DIR *d = opendir(path);
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			if (dir->d_type == DT_REG){
				// Verifico que termine con .tmp o con .tmpc
				if(strstr(dir->d_name,".tmp") == dir->d_name+(strlen(dir->d_name)-strlen(".tmp")) || strstr(dir->d_name,".tmpc") == dir->d_name+(strlen(dir->d_name)-strlen(".tmpc"))){
					char*filename=malloc(strlen(path)+strlen(dir->d_name)+2);
					sprintf(filename,"%s/%s",path,dir->d_name);
					if(fs_read(filename,listaRegistros)!=0){
						closedir(d);
						free(path);
						free(filename);
						return 1;
					}
					free(filename);
				}
			}
		}
		closedir(d);
	} else {
		log_error(LOGGER,"Error al abrir el directorio %s, %s",path,strerror(errno));
		free(path);
		return 1;
	}
	free(path);
	return 0;
}
char *consistencia2string(Consistencias consistencia){
	char* str=malloc(4);
	switch(consistencia){
		case EC:
			strcpy(str,"EC");
		break;
		case SC:
			strcpy(str,"SC");
		break;
		case SHC:
			strcpy(str,"SHC");
		break;
	}
	return str;
}
int string2consistencia(char* consistencia){
	if(strcmp(consistencia,"EC")==0){
		return EC;
	}
	if(strcmp(consistencia,"SC")==0){
		return SC;
	}
	return SHC;
}
char* getTablePath(char*tabla){
	char*path = malloc(strlen(global_conf.directorio_tablas)+strlen(tabla)+1);
	sprintf(path,"%s%s",global_conf.directorio_tablas,tabla);
	return path;
}
int digitos(int num){
	int l=!num;
	while(num){l++; num/=10;}
	return l;
}
int digitos_long(long num){
	long l=!num;
	while(num){l++; num/=10;}
	return (int)l;
}