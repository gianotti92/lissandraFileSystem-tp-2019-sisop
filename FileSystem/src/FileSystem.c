/*
 ============================================================================
 Name        : FileSystem.c
 Author      : 
 Version     :
 Copyright   : 
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "config_fileSystem.h"

int main(void) {
	get_parametros_config();
	configure_logger();
	puts("Hello FileSystem!!"); /* prints  */
	printf("%d", PUERTO_ESCUCHA_CONEXION);
	return EXIT_SUCCESS;
}
