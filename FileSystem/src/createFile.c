#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

int main() {

     crear_carpeta_tables();
     crear_tabla();
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

void crear_carpeta_tables()
   {
     mkdir("tables", 0777);
   }

int crear_tabla()
   {

	int filedescriptor = open("tables/name.txt", O_RDWR | O_APPEND | O_CREAT, 0777);

	if (filedescriptor < 0)
	{
	    perror("Error creating my_log file\n");
	    exit(-1);
	}
}

