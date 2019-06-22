#include "lissandra.h"

struct file_mdata{
	long size;
    t_list* blocks;
};

struct filesystem_conf{
	int BLOCK_SIZE;
	int BLOCKS;
	char*MAGIC_NUMBER;
};

struct filesystem_conf global_fs_conf;

/* Structure Initialization */
int create_directory_structure(void);
int fs_get_conf(void);

/* Write Functions */
char* fs_write_registers_to_buffer(t_list *registros,int* buffer_size);
int fs_write_get_free_blocks(t_list* blocks,int cant_blocks);
int fs_write_buffer_to_blocks(char* buffer,struct file_mdata* mdata);
int fs_write_set_mdata(char* filename,struct file_mdata* mdata);

/* Create Functions */
int fs_create_set_mdata(char* filename); 

/* Delete Functions */
int fs_delete_set_free_blocks(t_list* blocks);

/* Read Functions */
int fs_read_blocks_to_buffer(char*buffer,struct file_mdata* mdata);
void fs_read_buffer_to_registers(char* buff,long bufferSize,t_list* registros);
int fs_read_get_mdata(char* filename,struct file_mdata* mdata);

/* Bitarray */
void bitarray_init(void);
int bitarray_size(void);
void bitarray_to_file(char *bitarray);
t_bitarray* bitarray_get(void);
void bitarray_show(t_bitarray* bitarray);

/* Utils */
int existeDirectorio(char *path);
int file_exist (char *filename);
int fs_get_reg_size(void);
char* directorio_sin_ultima_barra(char*path);

/*
	Public Functions
*/
int fs_init(void){
	if(create_directory_structure()!=0){
		return 1;
	}
	if(fs_get_conf()!=0){
		return 1;
	}
	bitarray_init();
	return 0;
}
void fs_destroy(void){
	free(global_fs_conf.MAGIC_NUMBER);
}
int fs_read(char* filename, t_list* registros){
	struct file_mdata mdata;
	int retval=fs_read_get_mdata(filename,&mdata);
	if(retval != 0){
		return retval;
	}
	if(mdata.size == 0){
		return 0;
	}
	char *bufferData=malloc(mdata.size+1);
	retval=fs_read_blocks_to_buffer(bufferData,&mdata);
	if(retval != 0){
		free(bufferData);
		list_destroy_and_destroy_elements(mdata.blocks, (void*)free);
		return retval;
	}
	fs_read_buffer_to_registers(bufferData,mdata.size,registros);
	free(bufferData);
	if(mdata.blocks!=NULL){
		list_destroy_and_destroy_elements(mdata.blocks, (void*)free);
	}
	return 0;
}
int fs_write(char* filename, t_list* registros){
	int buffer_size;
	char *buffer=fs_write_registers_to_buffer(registros,&buffer_size);

	struct file_mdata mdata;
	mdata.size=buffer_size;

	int cant_blocks = mdata.size/global_fs_conf.BLOCK_SIZE;
	if(cant_blocks*global_fs_conf.BLOCK_SIZE < mdata.size)
		cant_blocks++;

	mdata.blocks = list_create();
	int retval = fs_write_get_free_blocks(mdata.blocks,cant_blocks);
	if(retval != 0){
		list_destroy_and_destroy_elements(mdata.blocks, (void*)free);
		free(buffer);
		return retval;
	}
	retval = fs_write_buffer_to_blocks(buffer,&mdata);
	if(retval != 0){
		list_destroy_and_destroy_elements(mdata.blocks, (void*)free);
		free(buffer);
		return retval;
	}
	retval = fs_write_set_mdata(filename,&mdata);
	if(retval != 0){
		list_destroy_and_destroy_elements(mdata.blocks, (void*)free);
		free(buffer);
		return retval;
	}

	list_destroy_and_destroy_elements(mdata.blocks, (void*)free);
	free(buffer);
	return 0;
}
int fs_create(char* filename){
	return fs_create_set_mdata(filename);
}
int fs_delete(char* filename){
	struct file_mdata mdata;
	int retval = fs_read_get_mdata(filename,&mdata);
	if(retval != 0){
		return retval;
	}
	if(mdata.size==0) {
		if(remove(filename)){
			log_error(LOG_ERROR,"Error al eliminar el archivo '%s', %s",filename,strerror(errno));
			return FILE_DELETE_ERROR;
		}
		return 0;
	}
	retval = fs_delete_set_free_blocks(mdata.blocks);
	if(retval != 0){
		list_destroy_and_destroy_elements(mdata.blocks, (void*)free);
		return retval;
	}
	
	if(remove(filename)){
		log_error(LOG_ERROR,"Error al eliminar el archivo '%s', %s",filename,strerror(errno));
		list_destroy_and_destroy_elements(mdata.blocks, (void*)free);
		return FILE_DELETE_ERROR;
	}

	list_destroy_and_destroy_elements(mdata.blocks, (void*)free);
	return 0;
}

/*
	Write Functions
*/
char* fs_write_registers_to_buffer(t_list *registros,int* buffer_size){
	char* buffer = malloc(fs_get_reg_size()*list_size(registros));
	*buffer_size=0;
	void addToBuffer(struct tableRegister* reg){
		char* key = malloc(digitos(reg->key)+2);
		char* value = malloc(global_conf.max_value_size+2);
		char* timestamp = malloc(digitos_long((long)reg->timestamp)+2);
		sprintf(key,"%d;",reg->key);
		sprintf(value,"%s;",reg->value);
		sprintf(timestamp,"%d\n",reg->timestamp);
		int wsize = strlen(key)+strlen(value)+strlen(timestamp)+1;
		char wstring[wsize];
		sprintf(wstring,"%s%s%s",key,value,timestamp);
		memcpy(buffer+*buffer_size,wstring,wsize-1);
		*buffer_size+=wsize-1;
		free(key);
		free(value);
		free(timestamp);
	}
	list_iterate(registros,(void*)addToBuffer);
	return buffer;
}
int fs_write_get_free_blocks(t_list* blocks,int cant_blocks){
	if(cant_blocks>global_fs_conf.BLOCKS){
		log_error(LOG_ERROR,"Error al asignar %d bloques, pasa el maximo de %d",cant_blocks,global_fs_conf.BLOCKS);
		return BLOCK_MAX_REACHED;
	}
	t_bitarray* bitarray = bitarray_get();
	if(bitarray==NULL){
		return MISSING_FILE;
	}
	int cantAsign=0;
	for(int i=0; i<global_fs_conf.BLOCKS && cantAsign<cant_blocks;i++){
		if(bitarray_test_bit(bitarray,i)==false){
			bitarray_set_bit(bitarray,i);
			int *bloque = malloc(sizeof(int));
			*bloque=i;
			list_add(blocks,(void*)bloque);
			cantAsign++;
		}
	}
	if(cant_blocks != cantAsign){
		free(bitarray->bitarray);
		bitarray_destroy(bitarray);
		log_error(LOG_ERROR,"Error al asignar %i bloques, se asignaron %d",cant_blocks,cantAsign);
		return BLOCK_ASSIGN_ERROR;
	}
	bitarray_to_file(bitarray->bitarray);
	free(bitarray->bitarray);
	bitarray_destroy(bitarray);
	return 0;
}
int fs_write_buffer_to_blocks(char* buffer,struct file_mdata* mdata) {
	int i=0,error=0;
	int cant = mdata->size/global_fs_conf.BLOCK_SIZE;

	void writeInFile(int * block){
		char*path=malloc(strlen(global_conf.directorio_bloques)+digitos(*block)+5);
		sprintf(path,"%s%d.bin",global_conf.directorio_bloques,*block);

		FILE * FileBlock = fopen(path, "wb");
		if(FileBlock==NULL){
			log_error(LOG_ERROR,"Error al abrir el archivo '%s', %s",path,strerror(errno));
			error=FILE_OPEN_ERROR;
			free(path);
			return;
		}
		free(path);

		if(i!=cant){
				char * toBlock = malloc(global_fs_conf.BLOCK_SIZE);
				memcpy(toBlock,buffer+global_fs_conf.BLOCK_SIZE*i,global_fs_conf.BLOCK_SIZE);
				fwrite(*&toBlock,global_fs_conf.BLOCK_SIZE,1,FileBlock);
				free(toBlock);
		}else{
				char * toBlock = malloc(global_fs_conf.BLOCK_SIZE);
				memcpy(toBlock,buffer+global_fs_conf.BLOCK_SIZE*i,mdata->size-global_fs_conf.BLOCK_SIZE*i);
				fwrite(*&toBlock,mdata->size-global_fs_conf.BLOCK_SIZE*i,1,FileBlock);
				free(toBlock);
		}
		fclose(FileBlock);
		i++;
	}
	list_iterate(mdata->blocks,(void*)writeInFile);
	return error;
}
int fs_write_set_mdata(char* filename,struct file_mdata* mdata){
	FILE* f=fopen(filename,"w");
	if(f==NULL){
		log_error(LOG_ERROR,"Error al crear el archivo '%s', %s",filename,strerror(errno));
		return FILE_OPEN_ERROR;
	}
	int lengthBlocks=0;
	void sumLengthBlocks(int* block){
		lengthBlocks+=digitos(*block)+1;
	}
	list_iterate(mdata->blocks,(void*)sumLengthBlocks);

	char*buff=malloc(5+digitos_long(mdata->size)+9+lengthBlocks+1);
	sprintf(buff,"SIZE=%lu\nBLOCKS=[",mdata->size);

	void addBlockToBuff(int* block){
		if(block==NULL)
			return;
		char*aux=malloc(digitos(*block)+2);
		sprintf(aux,"%d,",*block);
		strcat(buff,aux);
		free(aux);
	}

	list_iterate(mdata->blocks,(void*)addBlockToBuff);
	buff[strlen(buff)-1]=']';

	fwrite(buff,strlen(buff),1,f);
	free(buff);
	fclose(f);
	return 0;
}

/*
	Create Functions
*/
int fs_create_set_mdata(char* filename){
	FILE* f=fopen(filename,"w");
	if(f==NULL){
		log_error(LOG_ERROR,"Error al crear el archivo '%s', %s",filename,strerror(errno));
		return FILE_OPEN_ERROR;
	}
	char buff[17];
	strcpy(buff,"SIZE=0\nBLOCKS=[]");
	fwrite(buff,strlen(buff)+1,1,f);
	fclose(f);
	return 0;
}

/*
	Delete Functions
*/
int fs_delete_set_free_blocks(t_list* blocks){
	t_bitarray* bitarray = bitarray_get();
	if(bitarray==NULL){
		return MISSING_FILE;
	}
	int error=0;
	void limpiar(int *block){
		char*filename=malloc(strlen(global_conf.directorio_bloques)+digitos(*block)+5);
		sprintf(filename,"%s%d.bin",global_conf.directorio_bloques,*block);		
		if(remove(filename)) {
			log_error(LOG_ERROR,"Error al eliminar el archivo '%s', %s",filename,strerror(errno));
			error=FILE_DELETE_ERROR;
		}
		free(filename);
		bitarray_clean_bit(bitarray,*block);
	}
	list_iterate(blocks,(void*)limpiar);

	bitarray_to_file(bitarray->bitarray);
	free(bitarray->bitarray);
	bitarray_destroy(bitarray);
	return error;
}

/*
	Read Functions
*/
int fs_read_blocks_to_buffer(char*buffer,struct file_mdata* mdata){
	int i=0,error=0;

	void readFile(int*block){
		char*path=malloc(strlen(global_conf.directorio_bloques)+digitos(*block)+5);
		sprintf(path,"%s%d.bin",global_conf.directorio_bloques,*block);

		FILE * FileBlock = fopen(path, "rb");
		if(FileBlock==NULL){
			log_error(LOG_ERROR,"Error al abrir el archivo '%s', %s",path,strerror(errno));
			error=FILE_OPEN_ERROR;
			free(path);
			return;
		}
		free(path);
		if(i!=list_size(mdata->blocks)-1){
			char toBlock[global_fs_conf.BLOCK_SIZE];
			fread(toBlock,global_fs_conf.BLOCK_SIZE,1,FileBlock);
			memcpy(buffer+global_fs_conf.BLOCK_SIZE*i,toBlock,global_fs_conf.BLOCK_SIZE);
		}else{
			char toBlock[global_fs_conf.BLOCK_SIZE];
			fread(toBlock,mdata->size-global_fs_conf.BLOCK_SIZE*i,1,FileBlock);
			memcpy(buffer+global_fs_conf.BLOCK_SIZE*i,toBlock,mdata->size-global_fs_conf.BLOCK_SIZE*i);
		}
		fclose(FileBlock);
		i++;
	}
	list_iterate(mdata->blocks,(void*)readFile);
	if(mdata->size>0) {
		buffer[mdata->size-1]='\0';
	} else{
		buffer[0]='\0';
	}
	return error;
}
void fs_read_buffer_to_registers(char* buffParam,long bufferSize,t_list* registros){
	char buff[bufferSize];
	strcpy(buff,buffParam);
	char*lineSave=NULL;
	char* line = strtok_r(buff,"\n",&lineSave);
	while(line!=NULL){
		char*itemSave=NULL;
		struct tableRegister *reg = malloc(sizeof(struct tableRegister));
		char*keystr=strtok_r(line,";",&itemSave);
		sscanf(keystr,"%hu",&reg->key);
		char*value=strtok_r(NULL,";",&itemSave);
		reg->value=malloc(strlen(value)+1);
		strcpy(reg->value,value);
		char*timestampstr=strtok_r(NULL,";",&itemSave);
		sscanf(timestampstr,"%d",&reg->timestamp);
		list_add(registros,(void*)reg);
		line = strtok_r(NULL,"\n",&lineSave);
	}
}
int fs_read_get_mdata(char* filename,struct file_mdata* mdata){
	t_config* conf=config_create(filename);
	if(conf==NULL){
		log_error(LOG_ERROR,"Error al abrir el archivo '%s', %s",filename,strerror(errno));
		return FILE_OPEN_ERROR;
	}
	mdata->size = config_get_long_value(conf,"SIZE");
	if(mdata->size==0) {
		config_destroy(conf);
		return 0;
	}
	char**blocks = config_get_array_value(conf,"BLOCKS"); // Aca valgind se queja, posible memory leak
	mdata->blocks = list_create();
	char**p=blocks;
	while(p!=NULL && *p!=NULL){
		int * block=malloc(sizeof(int)); // Aca valgind se queja, posible memory leak
		sscanf(*p,"%d",block);
		list_add(mdata->blocks,(void*)block);
		free(*p);
		p++;
	}
	free(blocks);
	config_destroy(conf);
	return 0;
}
/*
	Structure Initialization
*/
int create_directory_structure() {
	char*path=directorio_sin_ultima_barra(global_conf.directorio_tablas);
	if(!existeDirectorio(path)){
		if(mkdirRecursivo(path)!=0){
			free(path);
			return 1;
		}
	}
	free(path);
	path=directorio_sin_ultima_barra(global_conf.directorio_bloques);
	if(!existeDirectorio(path)){
		if(mkdirRecursivo(path)!=0){
			free(path);
			return 1;
		}
	}
	free(path);
	path=directorio_sin_ultima_barra(global_conf.directorio_metadata);
	if(!existeDirectorio(path)){
		if(mkdirRecursivo(path)!=0){
			free(path);
			return 1;
		}
	}
	free(path);
	return 0;
}
int fs_get_conf(void){
	char*filename=malloc(strlen(global_conf.directorio_metadata)+13);
	sprintf(filename,"%sMetadata.bin",global_conf.directorio_metadata);
	t_config* conf = config_create(filename);
	if(conf==NULL){
		log_error(LOG_ERROR,"Error al abrir el archivo '%s', %s",filename,strerror(errno));
		free(filename);
		return FILE_OPEN_ERROR;
	}
	free(filename);
	global_fs_conf.BLOCK_SIZE = config_get_int_value(conf,"BLOCK_SIZE");
	global_fs_conf.BLOCKS = config_get_int_value(conf,"BLOCKS");
	global_fs_conf.MAGIC_NUMBER=malloc(strlen(config_get_string_value(conf,"MAGIC_NUMBER"))+1);
	strcpy(global_fs_conf.MAGIC_NUMBER,config_get_string_value(conf,"MAGIC_NUMBER"));
	config_destroy(conf);
	return 0;
}

/*
	Bitarray
*/
void bitarray_init(void){
	char*filename=malloc(strlen(global_conf.directorio_metadata)+11);
	sprintf(filename,"%sBitmap.bin",global_conf.directorio_metadata);
	if(!file_exist(filename)){
		char *bitarray = malloc(bitarray_size());
		memset(bitarray,0,bitarray_size());
		bitarray_to_file(bitarray);
		free(bitarray);
	}
	free(filename);
}
void bitarray_to_file(char *bitarray){
	char*filename=malloc(strlen(global_conf.directorio_metadata)+11);
	sprintf(filename,"%sBitmap.bin",global_conf.directorio_metadata);
	FILE * fd = fopen(filename, "wb");
	if(fd==NULL){
		log_error(LOG_ERROR,"Error al abrir el archivo %s, %s",filename,strerror(errno));
		free(filename);
		return;
	}
	fwrite(bitarray,bitarray_size(),1,fd);
	fclose(fd);
	free(filename);
}
int bitarray_size(void){
	int CANTIDAD = global_fs_conf.BLOCKS;
	int size = CANTIDAD/8;
	if(CANTIDAD > 8*size)
		size++;
	return size;
}
t_bitarray* bitarray_get(void){
	char*filename=malloc(strlen(global_conf.directorio_metadata)+11);
	sprintf(filename,"%sBitmap.bin",global_conf.directorio_metadata);
	FILE * fd = fopen(filename, "rb");
	if(fd==NULL){
		log_error(LOG_ERROR,"Error al abrir el archivo %s, %s",filename,strerror(errno));
		free(filename);
		return NULL;
	}
	free(filename);
	char *bitarray = malloc(bitarray_size());
	fread(bitarray,bitarray_size(),1,fd);
	fclose(fd);
	return bitarray_create_with_mode(bitarray,bitarray_size(),MSB_FIRST);
}
void bitarray_show(t_bitarray* bitarray){
	printf("Imprimo bitarray: ");
	for (int i = 0; i < global_fs_conf.BLOCKS; i++){
		printf("%d",bitarray_test_bit(bitarray,i));
	}
	printf("\n");
	return;
}

/*
	Utils
*/
int existeDirectorio(char *path){
	struct stat sb;
	if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode))
		return 1;
	return 0;
}
int file_exist (char *filename){
	struct stat buffer;
	return (stat (filename, &buffer) == 0);
}
int fs_get_reg_size(void){
	// max key: 65536 -> 5 char + max value -> conf + max timestamp -> 19 char + 2 ';' + '\n' + '\0'
	return 5+global_conf.max_value_size+19+4;
}
char* directorio_sin_ultima_barra(char*path){
	char*ret=malloc(strlen(path));
	memcpy(ret,path,strlen(path));
	ret[strlen(path)-1]='\0';
	return ret;
}