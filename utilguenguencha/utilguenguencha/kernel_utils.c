#include "kernel_utils.h"

int leer_linea(char* path, int linea){
	FILE *fileptr;
	if((fileptr = fopen(path, "r"))){
		char *buffer = "";
		int lineas_leidas = 0, desplazamiento = 0;
		char c;
		c = fgetc(fileptr);
		while(c != EOF){
			if(lineas_leidas == linea){
				if(c != '\n'){
					if(strlen(buffer) == 0){
						buffer = calloc(1, sizeof(char));
					}else{
						buffer = realloc(buffer, strlen(buffer) + sizeof(char));
					}
					memcpy(buffer+desplazamiento, &c, sizeof(char));
					desplazamiento++;
					c = fgetc(fileptr);
					continue;
				}else{
					char *retorno = string_new();
					string_append(&retorno, buffer);
					memcpy(retorno, buffer, desplazamiento);
					free(buffer);
					fclose(fileptr);
					return retorno;
				}
			}else{
				if(c == '\n'){
					lineas_leidas++;
				}
				c = fgetc(fileptr);
			}
		}
		if(desplazamiento==0){
			fclose(fileptr);
					return -1;
		}else{
			char *retorno = malloc(desplazamiento);
			memcpy(retorno, buffer, desplazamiento);
			free(buffer);
			fclose(fileptr);
			return retorno;
		}

	}else{
		return -1;
	}
}

