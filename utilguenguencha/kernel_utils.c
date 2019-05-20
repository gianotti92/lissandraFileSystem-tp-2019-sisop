#include "kernel_utils.h"

char* leer_linea(char* path, int linea){
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
					if(c == '\n' && strlen(buffer) == 0){
						return NULL;
					}else if(strlen(buffer) == desplazamiento){
						return buffer;
					}else{
						char *retorno = malloc(desplazamiento);
						memcpy(retorno, buffer, desplazamiento);
						free(buffer);
						fclose(fileptr);
						return retorno;
					}
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
					return NULL;
		}else{
			if(c == '\n' && strlen(buffer) == 0){
				return NULL;
			}else if(strlen(buffer) == desplazamiento){
				return buffer;
			}else{
				char *retorno = malloc(desplazamiento);
				memcpy(retorno, buffer, desplazamiento);
				free(buffer);
				fclose(fileptr);
				return retorno;
			}
		}

	}else{
		return NULL;
	}
}
