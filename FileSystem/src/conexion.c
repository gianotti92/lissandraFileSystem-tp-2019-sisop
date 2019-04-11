#include "conexion.h"
#include "config_fileSystem.h"

void inicial_servidor() {
	log_info(LOGGER, "File System. Se inicia servidor");
	struct sockaddr_in fileSystemAddres, poolMemoryAddress;
	int socketFileSystem = iniciar_socket();
	cargar_valores_address(&fileSystemAddres);
	evitar_bloqueo_puerto(socketFileSystem);
	realizar_bind(socketFileSystem, &fileSystemAddres);
	ponerse_a_escuchar(socketFileSystem, CANTIDAD_CONEXIONES);
	puts("Estoy escuchando...");
	log_info(LOGGER, "File System. Estoy escuchando");

	unsigned int tamanoDireccion = sizeof(struct sockaddr_in);
	int socketPoolMemory = accept(socketFileSystem, (struct sockaddr *) &poolMemoryAddress, &tamanoDireccion);
	if (socketPoolMemory != -1) {
		printf("se conecto vieja, saludos");
		log_info(LOGGER, "File System. Cliente conectado");
	}

}

int iniciar_socket() {
	int socketFileSystem = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFileSystem <= -1) {
		perror("No se pudo crear el socket servidor");
		log_error(LOGGER, "No se pudo crear el socket servidor");
		exit_gracefully(EXIT_SUCCESS);
	}
	return socketFileSystem;
}

void cargar_valores_address(struct sockaddr_in *fileSystemAddres) {
	fileSystemAddres->sin_family = AF_INET;
	fileSystemAddres->sin_addr.s_addr = inet_addr(IP);
	fileSystemAddres->sin_port = htons(PUERTO_FS);
	memset(&(fileSystemAddres->sin_zero), '\0', 8);
}

void evitar_bloqueo_puerto(int socket) {
	int activado = 1;
	setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));
}

void realizar_bind(int socket, struct sockaddr_in * address) {
	if (bind(socket, (struct sockaddr *) address, sizeof(struct sockaddr))
			== -1) {
		perror("Fallo el bind");
		log_error(LOGGER, "Fallo en bind");
		exit_gracefully(EXIT_FAILURE);
	}
}

void ponerse_a_escuchar(int socket, int cantidadConexiones) {
	if (listen(socket, cantidadConexiones) == -1) {
		perror("Fallo en el listen");
		log_error(LOGGER, "fallo el listen");
		exit_gracefully(EXIT_FAILURE);
	}
}
