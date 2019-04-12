#include <stdio.h>
#include <stdlib.h>
#include "config_fileSystem.h"
#include "conexion.h"

int main(void) {
	configure_logger();
	get_parametros_config();
	printf("%d\n", PUERTO_ESCUCHA_CONEXION);
	inicial_servidor();
	exit_gracefully(EXIT_SUCCESS);
}
