#include "kernel_utils.h"

char* leer_linea(char* path, int linea) {
	char *line_buf = NULL;
	size_t line_buf_size = 0;
	ssize_t line_size;
	FILE * fp = fopen(path, "r");

	if(!fp){
		perror("Error al leer archivo");
		return NULL;
	}
	int i;
	for(i = 0; i <= linea; i++){
		line_size = getline(&line_buf, &line_buf_size, fp);
	}

	if(line_size >= 0){
		line_buf[line_size - 1] = '\0';
	}else{
		line_buf = NULL;
	}
	fclose(fp);
	return line_buf;
}
