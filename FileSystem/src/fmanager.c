/*
 * fileManager.c
 *
 *  Created on: 13 abr. 2019
 *      Author: utnso
 */
#include "lissandra.h"

struct PartitionData{
	int size;
	t_list* blocks;
};

char * makeMetadataTableString(struct TableMetadata tableMeta);
char * makeMetadataString(struct FileSystemMetadata systemMeta);
struct PartitionData getPartitionData(char* nameTable, int partition);
int setPartitionData(struct PartitionData partitionD);

int crearDirectorio(char* source,char* name){
	/*if(!existeDirectorio(source)){
		if(mkdirRecursivo(source)!=0){
			return 1;
		}
	}*/
	char PATH[200];
	sprintf(PATH,"%s/%s",source,name);

	if(!existeDirectorio(PATH)){
		if(mkdir(PATH,S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)==0){ //Directorio creado con 755
			return 0;
		} else {
			log_error(LOGGER,"Error al crear el directorio '%s', %s",PATH,strerror(errno));
			return 1;
		}
	}
	log_error(LOGGER,"Tabla '%s' existente",name);
    return 1;
}

int mkdirRecursivo(char*pathSrc) { //Creados con 755
	//Copia del pathsrc
	char* path = malloc(strlen(pathSrc)+1);
	strcpy(path,pathSrc);
	//Acumulador de directorios creados
	char* create = malloc(strlen(pathSrc)+1);
	strcpy(create,"/");
	//Spliteo el path
	char* find = strtok(path,"/");
	while(find != NULL) {
		sprintf(create,"%s%s",create,find);
		if(!existeDirectorio(create)) {
			if(mkdir(create,S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)!=0){
				log_error(LOGGER,"Error al crear el directorio '%s', %s",create,strerror(errno));
				free(create);
				free(path);
				return 1;
			}
		}
		sprintf(create,"%s/",create);
		find = strtok(NULL,"/");
	}
	free(create);
	free(path);
	return 0;
}


char * borrarDirectorio(char*source, char*name){
	char PATH[200];
	printf(PATH,"%s/%s",source,name);

	char *result = (char *)malloc(strlen(PATH)+1);
	strcpy(result,PATH);

	if(existeDirectorio(PATH)){
		if(rmdir(PATH)==0){
			return result;
		}
	}

    return NULL;
}

char * makeMetadataTableString(struct TableMetadata tableMeta){
	char DATA[100];
	strcpy(DATA, "CONSISTENCY=");
	char* consistencia=consistencia2string(tableMeta.consistencia);
	strcat(DATA,consistencia);
	free(consistencia);
	strcat(DATA, "\nPARTITIONS=");
	char n1[10];
	snprintf(n1, sizeof(n1), "%d",tableMeta.numero_particiones);
	strcat(DATA, n1);
	strcat(DATA, "\nCOMPACTION_TIME=");
	char n2[10];
	snprintf(n2, sizeof(n2), "%d",tableMeta.compaction_time);
	strcat(DATA, n2);

	char *result = malloc(strlen(DATA)+1);
	strcpy(result,DATA);

	return result;
}


/*
 * BLOCK_SIZE=64
 * BLOCKS=5192
 * MAGIC_NUMBER=LISSANDRA
*/
char * makeMetadataString(struct FileSystemMetadata systemMeta){
	char DATA[100];

	strcpy( DATA, "BLOCK_SIZE=");
	char n3[10];
	snprintf(n3, sizeof(n3), "%d",systemMeta.block_size);
	strcat( DATA, n3 );

	strcat( DATA, "\nBLOCKS=");
	char n1[10];
	snprintf(n1, sizeof(n1), "%d",systemMeta.blocks);
	strcat( DATA, n1);

	strcat( DATA, "\nMAGIC_NUMBER=");
	char n2[10];
	snprintf(n2, sizeof(n2), "%d",systemMeta.magic_number);
	strcat( DATA, n2);

	char *result = (char *)malloc(strlen(DATA)+1);
	strcpy(result,DATA);

	return result;
}

int crearMetadataTableFile(char*directorio,struct TableMetadata tableMeta){
	char*PATH = malloc(strlen(directorio)+13+1);
	sprintf(PATH,"%s/Metadata.txt",directorio);
	FILE *pf = txt_open_for_append(PATH);
	free(PATH);
	if (pf == NULL){
		log_error(LOGGER,"Error al abrir el archivo '%s', %s",PATH,strerror(errno));
		return 1;
	}
	char * data = makeMetadataTableString(tableMeta);
	txt_write_in_file(pf, data);
	txt_close_file(pf);
	free(data);
	return 0;
}


int crearBinarios(char*directorio, char*tabla, int numero_particiones){
	char FILE_NAME[10];
	char PATH[50];
	int n=0;
	strcat(directorio,"/");

	while(n<numero_particiones){
		strcpy(PATH, directorio );
		snprintf(FILE_NAME, sizeof(FILE_NAME), "%d",n);
		strcat(FILE_NAME, ".bin");
		strcat(PATH, FILE_NAME);
		/*FILE*file = fopen (PATH, "wb");
		if (file==NULL) {
		   return 1;
		}
		fclose(file);*/
		if(fs_create(PATH)!=0){
			log_error(LOGGER,"Error al crear el archivo '%s', %s",PATH,strerror(errno));
			return 1;
		}
		n++;
	}
	return 0;
}

/*
 *  Block_size: Indica el tamaño en bytes de cada bloque
	Blocks: Indica la cantidad de bloques del File System
	Magic_Number: Un string fijo con el valor "LFS"
 *
 */
int crearMetadataFile(char*directorio,struct FileSystemMetadata systemMeta){
	char PATH[50];
	sprintf(PATH,"%s/Metadata.txt",directorio);

	FILE *pf = txt_open_for_append(PATH);
	if (pf == NULL)
		return 1;

	char * data = makeMetadataString(systemMeta);

	txt_write_in_file(pf, data);
	txt_close_file(pf);
	free(data);
	return 0;
}

struct TableMetadata setTableMetadata(Consistencias consistencia, int numero_particiones,double compaction_time){
	struct TableMetadata mData;
	mData.consistencia = consistencia;
	mData.numero_particiones = numero_particiones;
	mData.compaction_time = compaction_time;
	return mData;
}
struct FileSystemMetadata setFileSystemMetadata(int block_size,int blocks,int magic_number){
	struct  FileSystemMetadata fData;
	fData.block_size = block_size;
	fData.blocks = blocks;
	fData.magic_number = magic_number ;
	return fData;
}





