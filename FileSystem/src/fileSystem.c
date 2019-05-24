#include "lissandra.h"

struct file_mdata{
	long size;
    t_list* blocks;
};

struct filesystem_conf{
	long BLOCK_SIZE;
	long BLOCKS;
	char MAGIC_NUMBER[4];
};

struct filesystem_conf global_fs_conf;

/* Structure Initialization */
int create_directory_structure(char* root);
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
long bitarray_size(void);
void bitarray_to_file(char *bitarray);
t_bitarray* bitarray_get(void);
void bitarray_show(t_bitarray* bitarray);

/* Utils */
int create_dir(char* path);
int existeDirectorio(char *path);
int file_exist (char *filename);
int fs_get_reg_size(void);

/*
	Public Functions
*/
int fs_init(void){
	if(create_directory_structure(global_conf.punto_montaje)!=0){
		return 1;
	}
	if(fs_get_conf()!=0){
		return 1;
	}
	bitarray_init();
	return 0;
}
int fs_read(char* filename, t_list* registros){
	struct file_mdata mdata;
	if(fs_read_get_mdata(filename,&mdata)!=0){
		return 1;
	}
	if(mdata.size == 0){
		return 0;
	}
	char *bufferData=malloc(mdata.size+1);
	if(fs_read_blocks_to_buffer(bufferData,&mdata)!=0){
		free(bufferData);
		list_destroy(mdata.blocks);
		return 1;
	}

	fs_read_buffer_to_registers(bufferData,mdata.size,registros);
	free(bufferData);
	if(mdata.blocks!=NULL){
		list_destroy(mdata.blocks);
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
	if(fs_write_get_free_blocks(mdata.blocks,cant_blocks)){
		list_destroy(mdata.blocks);
		free(buffer);
		return -1;
	}

	if(fs_write_buffer_to_blocks(buffer,&mdata)){
		list_destroy(mdata.blocks);
		free(buffer);
		return -1;
	}

	if(fs_write_set_mdata(filename,&mdata)!=0){
		list_destroy(mdata.blocks);
		free(buffer);
		return -1;
	}

	list_destroy(mdata.blocks);
	free(buffer);
	return 0;
}
int fs_create(char* filename){
	return fs_create_set_mdata(filename);
}
int fs_delete(char* filename){
	struct file_mdata mdata;
	if(fs_read_get_mdata(filename,&mdata)!=0){
		return 1;
	}
	if(mdata.size==0) {
		if(remove(filename)){
			log_error(LOGGER,"Error al eliminar el archivo '%s', %s",filename,strerror(errno));
			return 1;
		}
		return 0;
	}
	if(fs_delete_set_free_blocks(mdata.blocks)!=0){
		list_destroy(mdata.blocks);
		return 1;
	}
	
	if(remove(filename)){
		log_error(LOGGER,"Error al eliminar el archivo '%s', %s",filename,strerror(errno));
		list_destroy(mdata.blocks);
		return 1;
	}

	list_destroy(mdata.blocks);
	return 0;
}

/*
	Write Functions
*/
char* fs_write_registers_to_buffer(t_list *registros,int* buffer_size){
	char* buffer = malloc(fs_get_reg_size()*list_size(registros));
	*buffer_size=0;
	void addToBuffer(struct tableRegister* reg){
		char key[5+2], value[global_conf.max_value_size+2], timestamp[19+2];
		sprintf(key,"%hu;",reg->key);
		sprintf(value,"%s;",reg->value);
		sprintf(timestamp,"%lu\n",reg->timestamp);
		int wsize = strlen(key)+strlen(value)+strlen(timestamp)+1;
		char wstring[wsize];
		sprintf(wstring,"%s%s%s",key,value,timestamp);
		memcpy(buffer+*buffer_size,wstring,wsize-1);
		*buffer_size+=wsize-1;
	}
	list_iterate(registros,(void*)addToBuffer);
	return buffer;
}
int fs_write_get_free_blocks(t_list* blocks,int cant_blocks){
	if(cant_blocks>global_fs_conf.BLOCKS){
		log_error(LOGGER,"Error asignar %d bloques, pasa el maximo de %lu",cant_blocks,global_fs_conf.BLOCKS);
		return 1;
	}
	t_bitarray* bitarray = bitarray_get();
	if(bitarray==NULL){
		log_error(LOGGER,"Error al abrir el archivo %s/Metadata/Bitmap.bin",global_conf.punto_montaje);
		return 1;
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
		bitarray_destroy(bitarray);
		log_error(LOGGER,"Error al reservar %i bloques, se intento asignar %d",cant_blocks,cantAsign);
		return 1;
	}
	bitarray_to_file(bitarray->bitarray);
	bitarray_destroy(bitarray);
	return 0;
}
int fs_write_buffer_to_blocks(char* buffer,struct file_mdata* mdata) {
	int i=0,error=0;
	int cant = mdata->size/global_fs_conf.BLOCK_SIZE;

	void writeInFile(int * block){
		char path[100];
		sprintf(path,"%s/Bloques/%i.bin",global_conf.punto_montaje,*block);

		FILE * FileBlock = fopen(path, "wb");

		if(FileBlock==NULL){
			log_error(LOGGER,"Error al abrir el archivo '%s', %s",path,strerror(errno));
			error=-1;
			return;
		}

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
		log_error(LOGGER,"No se pudo crear el archivo '%s', %s",filename,strerror(errno));
		return 1;
	}
	char buff[200];
	sprintf(buff,"SIZE=%lu\nBLOCKS=[",mdata->size);

	void addBlockToBuff(int* block){
		if(block==NULL)
			return;
		char aux[11];
		sprintf(aux,"%d,",*block);
		strcat(buff,aux);
	}

	list_iterate(mdata->blocks,(void*)addBlockToBuff);
	buff[strlen(buff)-1]=']';

	fwrite(buff,strlen(buff),1,f);
	fclose(f);
	return 0;
}

/*
	Create Functions
*/
int fs_create_set_mdata(char* filename){
	FILE* f=fopen(filename,"w");
	if(f==NULL){
		log_error(LOGGER,"No se pudo crear el archivo '%s', %s",filename,strerror(errno));
		return 1;
	}
	char buff[20];
	strcpy(buff,"SIZE=0\nBLOCKS=[]");
	fwrite(buff,strlen(buff),1,f);
	fclose(f);
	return 0;
}

/*
	Delete Functions
*/
int fs_delete_set_free_blocks(t_list* blocks){
	t_bitarray* bitarray = bitarray_get();
	if(bitarray==NULL){
		log_error(LOGGER,"Error al abrir el archivo %s/Metadata/Bitmap.bin",global_conf.punto_montaje);
		return 1;
	}
	int error=0;
	void limpiar(int *block){
		char filename[100];
		sprintf(filename,"%s/Bloques/%i.bin",global_conf.punto_montaje,*block);		
		if(remove(filename)) {
			log_error(LOGGER,"Error al eliminar el archivo '%s', %s",filename,strerror(errno));
			error=1;
		}
		bitarray_clean_bit(bitarray,*block);
	}
	list_iterate(blocks,(void*)limpiar);

	bitarray_to_file(bitarray->bitarray);
	bitarray_destroy(bitarray);
	return error;
}

/*
	Read Functions
*/
int fs_read_blocks_to_buffer(char*buffer,struct file_mdata* mdata){
	int i=0,error=0;

	void readFile(int*block){
		char path[100];
		sprintf(path,"%s/Bloques/%i.bin",global_conf.punto_montaje,*block);

		FILE * FileBlock = fopen(path, "rb");
		if(FileBlock==NULL){
			log_error(LOGGER,"Error al abrir el archivo '%s', %s",path,strerror(errno));
			error=1;
			return;
		}
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
		sscanf(timestampstr,"%lu",&reg->timestamp);
		list_add(registros,(void*)reg);
		line = strtok_r(NULL,"\n",&lineSave);
	}
}
int fs_read_get_mdata(char* filename,struct file_mdata* mdata){
	t_config* conf=config_create(filename);
	if(conf==NULL){
		log_error(LOGGER,"No existe el archivo '%s'",filename);
		return 1;
	}
	mdata->size = config_get_long_value(conf,"SIZE");
	if(mdata->size==0) {
		config_destroy(conf);
		return 0;
	}
	char**blocks = config_get_array_value(conf,"BLOCKS");
	mdata->blocks = list_create();
	char**p=blocks;
	while(p!=NULL && *p!=NULL){
		int * block=malloc(sizeof(int));
		sscanf(*p,"%d",block);
		list_add(mdata->blocks,(void*)block);
		p++;
	}
	config_destroy(conf);
	return 0;
}
/*
	Structure Initialization
*/
int create_directory_structure(char* root) {
	if(!existeDirectorio(root)){
		if(mkdirRecursivo(root)!=0){
			log_error(LOGGER,"No se pudo crear el directorio '%s', %s",root,strerror(errno));
			return 1;
		}
	}
	char path[100];
	sprintf(path,"%s/Metadata",root);
	if(!existeDirectorio(path)){
		if(create_dir(path)!=0){
			log_error(LOGGER,"No se pudo crear el directorio '%s', %s",path,strerror(errno));
			return 1;
		}
	}
	sprintf(path,"%s/Tables",root);
	if(!existeDirectorio(path)){
		if(create_dir(path)!=0){
			log_error(LOGGER,"No se pudo crear el directorio '%s', %s",path,strerror(errno));
			return 1;
		}
	}
	sprintf(path,"%s/Metadata",root);
	if(!existeDirectorio(path)){
		if(create_dir(path)!=0){
			log_error(LOGGER,"No se pudo crear el directorio '%s', %s",path,strerror(errno));
			return 1;
		}
	}
	sprintf(path,"%s/Bloques",root);
	if(!existeDirectorio(path)){
		if(create_dir(path)!=0){
			log_error(LOGGER,"No se pudo crear el directorio '%s', %s",path,strerror(errno));
			return 1;
		}
	}
	return 0;
}
int fs_get_conf(void){
	char filename[200];
	sprintf(filename,"%s/Metadata/Metadata.bin",global_conf.punto_montaje);
	t_config* conf = config_create(filename);
	if(conf==NULL){
		log_error(LOGGER,"Archivo de configuracion: %s no encontrado",filename);
		return 1;
	}
	global_fs_conf.BLOCK_SIZE = config_get_long_value(conf,"BLOCK_SIZE");
	global_fs_conf.BLOCKS = config_get_long_value(conf,"BLOCKS");
	strcpy(global_fs_conf.MAGIC_NUMBER,config_get_string_value(conf,"MAGIC_NUMBER"));
	config_destroy(conf);
	return 0;
}

/*
	Bitarray
*/
void bitarray_init(void){
	char filename[100];
	sprintf(filename,"%s/Metadata/Bitmap.bin",global_conf.punto_montaje);
	if(!file_exist(filename)){
		char *bitarray = malloc(bitarray_size());
		memset(bitarray,0,bitarray_size());
		bitarray_to_file(bitarray);
		free(bitarray);
	}
}
void bitarray_to_file(char *bitarray){
	char filename[100];
	sprintf(filename,"%s/Metadata/Bitmap.bin",global_conf.punto_montaje);
	FILE * fd = fopen(filename, "wb");
	if(fd==NULL){
		log_error(LOGGER,"No se pudo abrir el archivo %s, %s",filename,strerror(errno));
		return;
	}
	fwrite(bitarray,bitarray_size(),1,fd);
	fclose(fd);
}
long bitarray_size(void){
	int CANTIDAD = global_fs_conf.BLOCKS;
	int size = CANTIDAD/8;
	if(CANTIDAD > 8*size)
		size++;
	return size;
}
t_bitarray* bitarray_get(void){
	char filename[100];
	sprintf(filename,"%s/Metadata/Bitmap.bin",global_conf.punto_montaje);
	FILE * fd = fopen(filename, "rb");
	if(fd==NULL){
		log_error(LOGGER,"No se pudo abrir el archivo %s, %s",filename,strerror(errno));
		return NULL;
	}
	char *bitarray = malloc(bitarray_size());
	fread(bitarray,bitarray_size(),1,fd);
	fclose(fd);
	return bitarray_create_with_mode(bitarray,bitarray_size(),MSB_FIRST);
}
void bitarray_show(t_bitarray* bitarray){
	printf("Imprimo bitarray: ");
	for (long i = 0; i < global_fs_conf.BLOCKS; i++){
		printf("%d",bitarray_test_bit(bitarray,i));
	}
	printf("\n");
	return;
}

/*
	Utils
*/
int create_dir(char* path) {
	return mkdir(path,S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
}
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