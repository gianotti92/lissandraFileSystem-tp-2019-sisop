#include <stdio.h>
#include <stdlib.h>
#include "config_kernel.h"
#include "servidor.h"

int main(void) {
	get_parametros_config();
	configure_logger();
	levantar_servidor_kernel();
	exit_gracefully(EXIT_SUCCESS);
}
