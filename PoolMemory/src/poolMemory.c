/*
 ============================================================================
 Name        : PoolMemory.c
 Author      : 
 Version     :
 Copyright   : 
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "config_poolMemory.h"
#include "cliente.h"

int main(void) {
	get_parametros_config();
	configure_logger();
	puts("Hello PoolMemory!!"); /* prints  */

	//Conecta como cliente al KERNEL
	int fd_KERNEL = conectar_servidor(PUERTO_CONFIG_KERNEL,IP_CONFIG_KERNEL, "KERNEL");
	saludo_inicial_servidor(fd_KERNEL, "KERNEL");

	//Conecta como cliente al FileSysyem
	//int fd_POOLMEMORY = conectar_servidor(PUERTO_CONFIG_POOLMEMORY,IP_CONFIG_POOLMEMORY, "POOLMEMORY");
	//saludo_inicial_servidor(fd_POOLMEMORY, "POOLMEMORY");


	return EXIT_SUCCESS;
}
