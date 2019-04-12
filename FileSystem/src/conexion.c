#include "conexion.h"
#include "config_fileSystem.h"

void inicial_servidor() {
	log_info(LOGGER, "File System. Se inicia servidor");
	struct sockaddr_in fileSystemAddres, poolMemoryAddress;
	int socketFileSystem = iniciar_socket();
	cargar_valores_address(&fileSystemAddres, PUERTO_FS);
	evitar_bloqueo_puerto(socketFileSystem);
	realizar_bind(socketFileSystem, &fileSystemAddres);
	ponerse_a_escuchar(socketFileSystem, CANTIDAD_CONEXIONES);
	puts("Estoy escuchando...");
	log_info(LOGGER, "File System. Estoy escuchando");

	unsigned int tamanoDireccion = sizeof(struct sockaddr_in);
	int socketPoolMemory = accept(socketFileSystem,
			(struct sockaddr *) &poolMemoryAddress, &tamanoDireccion);
	if (socketPoolMemory != -1) {
		printf("se conecto vieja, saludos");
		log_info(LOGGER, "File System. Cliente conectado");
		recibir_informacion(socketPoolMemory);

		/*envio de mensaje a pool memory luego abstraer*/

		char * saludoDesdeFileSystem = "hola desde file system";
		int pruebaAverQUeonda = socket(AF_INET, SOCK_STREAM, 0);

		poolMemoryAddress.sin_family = AF_INET;
		poolMemoryAddress.sin_addr.s_addr = inet_addr(IP);
		poolMemoryAddress.sin_port = htons(PUERTO_POOL_MEM);
		memset(&(poolMemoryAddress.sin_zero), '\0', 8);

		if (connect(pruebaAverQUeonda, (void *) &poolMemoryAddress, sizeof(poolMemoryAddress)) != 0) {
			perror("Error al conectar con el cliente");
			exit_gracefully(EXIT_FAILURE);
		}

		if (send(pruebaAverQUeonda, saludoDesdeFileSystem, 99, 0) <= 0) {
			perror("Error al enviar querys de la consola");
			log_error(LOGGER,
					"File System. Error al enviar querys de la consola");
			exit_gracefully(EXIT_FAILURE);
		}
		log_info(LOGGER, "File System. El mensaje se envio correctamente");

		//enviar_informacion(socketPoolMemory, &poolMemoryAddress);

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

void cargar_valores_address(struct sockaddr_in *fileSystemAddres, int puerto) {
	fileSystemAddres->sin_family = AF_INET;
	fileSystemAddres->sin_addr.s_addr = inet_addr(IP);
	fileSystemAddres->sin_port = htons(puerto);
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
		log_error(LOGGER, "File System. Fallo el listen");
		exit_gracefully(EXIT_FAILURE);
	}
}
void recibir_informacion(int socket) {
	char * buffer = malloc(100);
	int bytesRecibidos = recv(socket, buffer, 99, 0);
	if (bytesRecibidos <= 0) {
		perror("error al recibir datos");
		log_info(LOGGER, "File System. Error al recibir datos");
		exit_gracefully(EXIT_FAILURE);
	}

	buffer[bytesRecibidos] = '\0';
	printf("lo que recibi del netcat es: %s", buffer);
	log_info(LOGGER, "File System. Se recibieron mensaje correctamente");
	free(buffer);
}

void enviar_informacion(int pija, struct sockaddr_in * address) {
	char * saludoDesdeFileSystem = "hola desde file system";

	cargar_valores_address(address, PUERTO_POOL_MEM);

	int p = socket(AF_INET, SOCK_STREAM, 0);

	if (connect(p, (void *) address, sizeof(*address)) != 0) {
		perror("Error al conectar con el cliente");
		exit_gracefully(EXIT_FAILURE);
	}

	if (send(p, saludoDesdeFileSystem, 99, 0) <= 0) {
		perror("Error al enviar querys de la consola");
		log_error(LOGGER, "File System. Error al enviar querys de la consola");
		exit_gracefully(EXIT_FAILURE);
	}

}

