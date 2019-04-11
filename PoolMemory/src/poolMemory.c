#include "config_poolMemory.h"
#include "cliente.h"

int main(void) {
	get_parametros_config();
	configure_logger();
	puts("Hello PoolMemory!!"); /* prints  */

	struct sockaddr_in serverPoolMemoryAddres, kernellAddres;
	int socketPoolMemory;

	serverPoolMemoryAddres.sin_family = AF_INET;
	serverPoolMemoryAddres.sin_addr.s_addr = inet_addr(IP);
	serverPoolMemoryAddres.sin_port = htons(POL_MEM_PORT);
	memset(&(serverPoolMemoryAddres.sin_zero), '\0', 8);

	socketPoolMemory = socket(AF_INET, SOCK_STREAM, 0);
	if (socketPoolMemory <= -1) {
		perror("Error al crear descriptor de fichero");
	}

	int activado = 1;
	setsockopt(socketPoolMemory, SOL_SOCKET, SO_REUSEADDR, &activado,
			sizeof(activado));

	if (bind(socketPoolMemory, (struct sockaddr *) &serverPoolMemoryAddres,
			sizeof(struct sockaddr)) == -1) {
		perror("Fallo el bind");
		exit_gracefully(EXIT_FAILURE);
	}

	puts("Estoy escuchando...");
	listen(socketPoolMemory, 100);

	/*-------------------------------------------*/

	kernellAddres.sin_family = AF_INET;
	kernellAddres.sin_addr.s_addr = inet_addr(IP);
	kernellAddres.sin_port = htons(KERNEL_PORT);
	unsigned int tamanoDireccion = sizeof(struct sockaddr_in);
	int socketKernell = accept(socketPoolMemory,
			(struct sockaddr *) &kernellAddres, &tamanoDireccion);
	if (socketKernell == -1) {
		perror("Error en el accept");
	}

	/*-------------------------------------------*/

	char * saludoDesdeKernell = malloc(100);

	int bytesRecibidos = recv(socketKernell, saludoDesdeKernell, 99, 0);
	if (bytesRecibidos <= 0) {
		perror("Error al recibir informacion.");
		exit_gracefully(EXIT_FAILURE);
	}

	saludoDesdeKernell[bytesRecibidos] = '\0';

	printf("lo que recibi del netcat es: %s", saludoDesdeKernell);

	free(saludoDesdeKernell);

	exit_gracefully(EXIT_SUCCESS);
}
