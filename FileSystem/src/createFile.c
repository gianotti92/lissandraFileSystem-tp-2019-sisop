#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <utilguenguencha/utils.h>
#include "filesSystem.h"
#include <commons/string.h>
#include <utilguenguencha/comunicacion.h>
int main() {

	Create * createTable = malloc(sizeof(Create));

	MockTabla(&createTable);

	// creo la carpeta de tablas
	crear_carpeta_tables();

	crear_tabla(&createTable);

	free(createTable);
	createTable = NULL;
	// create a FILE typed pointer
//	FILE *file_pointer;

// open the file "name_of_file.txt" for writing
//	file_pointer = fopen("tables\/name_of_file.txt", "w");

// Write to the file
//	fprintf(file_pointer, "This will write to a file.");

// Close the file
//	fclose(file_pointer);
	return 0;
}

void crear_carpeta_tables() {
	mkdir("tables", 0777);
}

void crear_mem_table() {
	mkdir("tables", 0777);
}

void crear_tabla(Create * createTable) {
	char * pathTable = string_new();
	string_append(&pathTable, "tables/");
	string_append(&pathTable, createTable->nombre_tabla);

	//creo la carpeta de la tabla
	mkdir(pathTable, 0777);

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

//	int filedescriptor = open("tables/name.bin", O_RDWR | O_APPEND | O_CREAT,0777);

}

void MockTabla(Create * createTable) {

	createTable->nombre_tabla = "name";
	createTable->compactation_time = 002;
	createTable->particiones = 4;
	createTable->timestamp = 8;
	createTable->consistencia = SC;

}

