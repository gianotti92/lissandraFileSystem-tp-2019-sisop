#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <utilguenguencha/utils.h>
#include "filesSystem.h"
#include <commons/string.h>
#include <utilguenguencha/comunicacion.h>
#include <utilguenguencha/utils.h>
#include <unistd.h>
#include <ftw.h>


int main() {
	configuracion_inicial_file_system();
	Create * createTable = malloc(sizeof(Create));

	MockTabla(&createTable);

	crear_folder_tablas();

	crear_tabla(&createTable);
	printf("sali bien");
	return 0;
}

void crear_folder_tablas() {
	mkdir("tables", 0777);
}

void crear_tabla(Create * createTable) {
	char * pathTable = string_new();
	string_append(&pathTable, "tables/");
	string_append(&pathTable, createTable->nombre_tabla);

	//creo la carpeta de la tabla
	mkdir(pathTable, 0777);
	crearMetaData(pathTable, createTable);

	for (int i = 0; i < createTable->particiones; ++i) {
		char * numberBin = string_new();
		string_append(&numberBin, pathTable);
		string_append(&numberBin, "/");
		string_append(&numberBin, string_itoa(i));
		string_append(&numberBin, ".bin");
		int filedescriptor = open(numberBin, O_RDWR | O_APPEND | O_CREAT, 0777);
		if (filedescriptor < 0) {
			perror("Error creating my_log file\n");
			exit(-1);
		}
		free(numberBin);
	}
	free(pathTable);

}

void MockTabla(Create * createTable) {
	createTable->nombre_tabla = "name";
	createTable->compactation_time = 002;
	createTable->particiones = 4;
	createTable->timestamp = 8;
	createTable->consistencia = SC;
}

void crearMetaData(char * pathTable, Create * createTable) {
	char * metaDataPath = string_new();
	string_append(&metaDataPath, pathTable);
	string_append(&metaDataPath, "/Metadata");
	int filedescriptor = open(metaDataPath, O_RDWR | O_APPEND | O_CREAT, 0777);
	if (filedescriptor < 0) {
		perror("Error creating my_log file\n");
		exit(-1);
	}
	escribirMetadata(metaDataPath, createTable);
	free(metaDataPath);
}
void escribirMetadata(char * metaDataPath, Create * createTable) {

	FILE *metaDataFile;

	// open file for writing
	metaDataFile = fopen(metaDataPath, "wb+");
	if (metaDataFile == NULL) {
		fprintf(stderr, "\nError opend file\n");
		exit(1);
	}

	char* formato = string_new();

	string_append_with_format(&formato, "CONSISTENCY=%s\n", consistency_to_string(createTable->consistencia));
	string_append_with_format(&formato, "PARTITIONS=%d\n", createTable->particiones);
	string_append_with_format(&formato, "COMPACTATION_TIME=%d\n", createTable->compactation_time);

	fwrite(formato, strlen(formato), 1, metaDataFile);

	if (fwrite != 0)
		printf("contents to file written successfully !\n");
	else
		printf("error writing file !\n");

	fclose(metaDataFile);
	free(formato);

}
void configuracion_inicial_file_system(){

		t_config* CONFIG;
		CONFIG = config_create("Metadata/metadata.bin");
		if (!CONFIG) {
			printf("No encuentro el archivo config\n");
			exit_gracefully(EXIT_FAILURE);
		}
		BLOCK_SIZE = config_get_int_value(CONFIG,"BLOCK_SIZE");
		BLOCKS = config_get_int_value(CONFIG,"BLOCKS");
	//	MAGIC_NUMBER = string_new();
		crear_bloques(BLOCKS);
	//	string_append(&MAGIC_NUMBER, config_get_int_value(CONFIG,"MAGIC_NUMBER"));
		config_destroy(CONFIG);

}

void crear_bloques(int cantidad_bloques){
	crear_folder_bloques();
	char * pathTable = string_new();
		string_append(&pathTable, "FS_LISSANDRA/Bloques");
		for (int i = 0; i < cantidad_bloques; ++i) {
			char * numberBin = string_new();
			string_append(&numberBin, pathTable);
			string_append(&numberBin, "/");
			string_append(&numberBin, string_itoa(i));
			string_append(&numberBin, ".bin");
			int filedescriptor = open(numberBin, O_RDWR | O_APPEND | O_CREAT, 0777);
			if (filedescriptor < 0) {
				perror("Error creating my_log file\n");
				exit(-1);
			}
			free(numberBin);
		}
		free(pathTable);

}
void crear_folder_bloques() {
	//mkdir("FS_LISSANDRA");
	mkdir("FS_LISSANDRA", 0777);
	mkdir("FS_LISSANDRA/Bloques", 0777);
}
int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
	{
	    int rv = remove(fpath);

	    if (rv)
	        perror(fpath);

	    return rv;
	}


}
int rmrf(char *path)
	{
	    return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
	}
int dropTable()
	{
		char *path = string_new();
		string_append(&path, "tables/name");
rmrf(path);
	}
