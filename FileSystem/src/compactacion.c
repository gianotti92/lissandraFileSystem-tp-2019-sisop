#include "lissandra.h"

int compac_search_tmp_files(t_list* tmpc_files, char* tableName){
	char path[200];
	sprintf(path,"%s/Tables/%s",global_conf.punto_montaje,tableName);

	struct dirent *dir;
	DIR *d = opendir(path);
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			if (dir->d_type == DT_REG){
				//Verifico si empieza con el prefijo y termina con la extension correspondiente
				if(strstr(dir->d_name,"dump_")==dir->d_name && strstr(dir->d_name,"tmp") == dir->d_name+(strlen(dir->d_name)-strlen("tmp"))) {
					char * newfilename = malloc(250); 
					sprintf(newfilename,"%s/%sc",path,dir->d_name);
					char oldfilename[250];
					sprintf(oldfilename,"%s/%s",path,dir->d_name);
					if (rename(oldfilename,newfilename)!=0){
						log_error(LOGGER,"No se pudo renombrar el archivo %s a %s, %s",dir->d_name,newfilename,strerror(errno));
						return 1;
					}
					list_add(tmpc_files,newfilename);
				}
			}
		}
		closedir(d);
	} else{
		log_error(LOGGER,"No se pudo abrir el directorio %s, %s",path,strerror(errno));
		return 1;
	}
	return 0;
}
int compac_get_tmp_registers(t_list*registrosTemporales,t_list*tmpc_files){
	int ret=0;
	void getRegisters(char* file){
		ret = fs_read(file,registrosTemporales);
	}
	list_iterate(tmpc_files,(void*)getRegisters);
	return ret;
}
int compac_get_partition_registers(t_list*registrosParticiones,char*tableName,int numero_particiones){
	char basePath[200];
	sprintf(basePath,"%s/Tables/%s",global_conf.punto_montaje,tableName);
	for(int i=0;i<numero_particiones;i++){
		char filename[220];
		sprintf(filename,"%s/%d.bin",basePath,i);
		t_list* particion=list_create();
		if(fs_read(filename,particion)!=0){
			return 1;
		}
		list_add_in_index(registrosParticiones,i,(void*)particion);
	}
	return 0;
}
void compac_match_registers(t_list*registrosParticiones,t_list*registrosTemporales){
	int cantParticiones = list_size(registrosParticiones);
	void iterateRegistersTmp(struct tableRegister* tempReg){
		int partNum = tempReg->key%cantParticiones;
		t_list* partRegisters = (t_list*)list_get(registrosParticiones,partNum);
		if(partRegisters==NULL){
			return;
		}

		// Defino la funcion para buscar la key temporal dentro de los registros de la particion
		bool sameKey(struct tableRegister* partReg){
			return partReg->key==tempReg->key;
		}

		struct tableRegister* found = list_find(partRegisters,(void*)sameKey);
		if(found == NULL){ // Agrego el nuevo registro a la particion
			struct tableRegister* newReg = malloc(sizeof(struct tableRegister));
			*newReg=createTableRegister(tempReg->key,tempReg->value,tempReg->timestamp);
			list_add(partRegisters,(void*)newReg);
		} else { // me fijo cual es mas reciente (timestamp) y me quedo con ese
			if(found->timestamp<tempReg->timestamp) {
				struct tableRegister* removed = list_remove_by_condition(partRegisters,(void*)sameKey);
				free(removed);
				struct tableRegister* newReg = malloc(sizeof(struct tableRegister));
				*newReg=createTableRegister(tempReg->key,tempReg->value,tempReg->timestamp);
				list_add(partRegisters,(void*)newReg);
			}
		}
	}
	list_iterate(registrosTemporales,(void*)iterateRegistersTmp);
}
int compac_delete_tmpc_files(t_list*tmpc_files){
	int retValue=0;
	void deleteFile(char * filename){
		if(fs_delete(filename)!=0){
			retValue=1;
		}
	}
	list_iterate(tmpc_files,(void*)deleteFile);
	return retValue;
}
int compac_save_partition_registers(t_list*registrosParticiones,char* tablename){
	// Borro los .bin anteriores
	for(int i=0;i<list_size(registrosParticiones);i++){
		char filename[200];
		sprintf(filename,"%s/Tables/%s/%d.bin",global_conf.punto_montaje,tablename,i);
		if(fs_delete(filename)!=0){
			return 1;
		}
	}
	// Escribo los datos en los nuevos .bin
	for(int i=0;i<list_size(registrosParticiones);i++){
		char filename[200];
		sprintf(filename,"%s/Tables/%s/%d.bin",global_conf.punto_montaje,tablename,i);
		t_list* particion = list_get(registrosParticiones,i);
		if(fs_write(filename,particion)!=0){
			return 1;
		}
	}
	return 0;
}
void compac_clean_partition_registers(t_list*registrosParticiones){
	void limpiar(t_list * particion){
		void cleanValue(struct tableRegister* reg){
			free(reg->value);
		}
		list_iterate(particion,(void*)cleanValue);
		list_destroy(particion);
	}
	list_iterate(registrosParticiones,(void*)limpiar);
}
void compac_clean_temp_registers(t_list*registrosTemporales){
	void cleanValue(struct tableRegister* reg){
		free(reg->value);
	}
	list_iterate(registrosTemporales,(void*)cleanValue);
}