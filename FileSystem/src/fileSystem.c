#include <stdio.h>
#include <stdlib.h>
#include "config_fileSystem.h"

int main(void) {
	configure_logger();
	get_parametros_config();
	printf("%d\n", PUERTO_ESCUCHA_CONEXION);
	exit_gracefully(EXIT_SUCCESS);
}
