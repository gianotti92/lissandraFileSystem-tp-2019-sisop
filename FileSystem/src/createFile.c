#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <utilguenguencha/utils.h>
#include "filesSystem.h"
#include <commons/string.h>
#include <utilguenguencha/comunicacion.h>
#include <utilguenguencha/utils.h>
#include <string.h>
#include <stdlib.h>

// POSIX dependencies
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>


int main() {
	configuracion_inicial_file_system();
	Create * createTable = malloc(sizeof(Create));

	MockTabla(&createTable);

	crear_folder_tablas();

	crear_tabla(&createTable);
	dropTable(&createTable);
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
void crear_folder_bloques(create) {
	//mkdir("FS_LISSANDRA");
	mkdir("FS_LISSANDRA", 0777);
	mkdir("FS_LISSANDRA/Bloques", 0777);
}
int dropTable(Create * createTable)
	{
		char* path = string_new();
		string_append(&path,"tables/");
		string_append(&path,createTable->nombre_tabla);
		eliminar_tabla(path);
		return 0;
	}

void eliminar_tabla(const char* path[])
{
    size_t path_len;
    char *full_path;
    DIR *dir;
    struct stat stat_path, stat_entry;
    struct dirent *entry;

    // stat for the path
    stat(path, &stat_path);

    // if path does not exists or is not dir - exit with status -1
    if (S_ISDIR(stat_path.st_mode) == 0) {
        fprintf(stderr, "%s: %s\n", "Is not directory", path);
        exit(-1);
    }

    // if not possible to read the directory for this user
    if ((dir = opendir(path)) == NULL) {
        fprintf(stderr, "%s: %s\n", "Can`t open directory", path);
        exit(-1);
    }

    // the length of the path
    path_len = strlen(path);

    // iteration through entries in the directory
    while ((entry = readdir(dir)) != NULL) {

        // skip entries "." and ".."
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        // determinate a full path of an entry
        full_path = calloc(path_len + strlen(entry->d_name) + 1, sizeof(char));
        strcpy(full_path, path);
        strcat(full_path, "/");
        strcat(full_path, entry->d_name);

        // stat for the entry
        stat(full_path, &stat_entry);

        // recursively remove a nested directory
        if (S_ISDIR(stat_entry.st_mode) != 0) {
        	eliminar_tabla(full_path);
            continue;
        }

        // remove a file object
        if (unlink(full_path) == 0)
            printf("Removed a file: %s\n", full_path);
        else
            printf("Can`t remove a file: %s\n", full_path);
    }

    // remove the devastated directory and close the object of it
    if (rmdir(path) == 0)
        printf("Removed a directory: %s\n", path);
    else
        printf("Can`t remove a directory: %s\n", path);

    closedir(dir);
}

