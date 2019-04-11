/*
 ============================================================================
 Name        : kernel.c
 Author      : 
 Version     :
 Copyright   : 
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "config_kernel.h"
#include "servidor.h"

int main(void) {
	get_parametros_config();
	configure_logger();
	puts("Hello Kernel!!"); /* prints  */
	levantar_servidor_kernel();
	return EXIT_SUCCESS;
}
