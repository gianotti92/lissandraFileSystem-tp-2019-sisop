
#include <stdio.h>
#include <stdlib.h>
#include "config_fileSystem.h"

int main(void) {
	configure_logger();
	funcion_para_testear_valgrind();
	get_parametros_config();
	printf("%d\n", PUERTO_ESCUCHA_CONEXION);
	exit_gracefully(EXIT_SUCCESS);
}


// Para correr valgrind van a FileSystem/Debug luego de compilar
// ejecutan valgrind --leak-check=full -v ./FileSystem
// Deberian tener lo siguiente:
// total heap usage: X allocs, X frees, Y bytes allocated <- Notese que tiene que haber un free por cada alloc
// y en el ERROR SUMMARY deberiamos tener 0

void funcion_para_testear_valgrind(void){
	int i;
	const int variable = 3;
	//int *cantidad = malloc(variable); // Descomentar para que falle
	int *cantidad = malloc(variable * sizeof(*cantidad)); // Comentar para que falle
	for(i=1; i<variable; i++){
		cantidad[i] = i * i;
		printf("%d: %d\n",i,cantidad[i]);
	}
	free(cantidad); // Comentar para que falle
}
