#include "config_poolMemory.h"
#include "cliente.h"

int main(void) {
	get_parametros_config();
	configure_logger();
	puts("Hello PoolMemory!!"); /* prints  */

	struct sockaddr_in poolMemoryAddres, kernellAddres, fileSystemAddres;
	int socketPoolMemory;

	poolMemoryAddres.sin_family = AF_INET;
	poolMemoryAddres.sin_addr.s_addr = inet_addr(IP);
	poolMemoryAddres.sin_port = htons(POOL_MEM_PORT);
	memset(&(poolMemoryAddres.sin_zero), '\0', 8);

	socketPoolMemory = socket(AF_INET, SOCK_STREAM, 0);
	if (socketPoolMemory <= -1) {
		perror("Error al crear descriptor de fichero");
	}

	int activado = 1;
	setsockopt(socketPoolMemory, SOL_SOCKET, SO_REUSEADDR, &activado,
			sizeof(activado));

	if (bind(socketPoolMemory, (struct sockaddr *) &poolMemoryAddres,
			sizeof(struct sockaddr)) == -1) {
		perror("Fallo el bind");
		exit_gracefully(EXIT_FAILURE);
	}

	puts("Estoy escuchando...");
	listen(socketPoolMemory, 100);

	/*-------------------------------------------*/

	unsigned int tamanoDireccion = sizeof(struct sockaddr_in);

	kernellAddres.sin_family = AF_INET;
	kernellAddres.sin_addr.s_addr = inet_addr(IP);
	kernellAddres.sin_port = htons(KERNEL_PORT);
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

	/*Enviar msj a file system (ABSTRAEME PORFAVOR)*/

	fileSystemAddres.sin_family = AF_INET;
	fileSystemAddres.sin_addr.s_addr = inet_addr(IP);
	fileSystemAddres.sin_port = htons(FILE_SYSTEM_PORT);

	int socketFileSystem = socket(AF_INET, SOCK_STREAM, 0);

	if(connect(socketFileSystem, (void *) &fileSystemAddres, sizeof(fileSystemAddres)) != 0){
		perror("Error al conectar con File System");
		exit_gracefully(EXIT_FAILURE);
	}

	if(send(socketFileSystem, saludoDesdeKernell, 99, 0) <= 0){
		perror("Error al enviar querys de la consola");
		exit_gracefully(EXIT_FAILURE);
	}
	/*Fin enviar file system */


	/*Recibir mensaje de file system*/



	/*Fin Recibir mensaje de file system*/

	free(saludoDesdeKernell);

	exit_gracefully(EXIT_SUCCESS);
}
