#include "servidor.h"

t_queue * listaConexiones;
int socketServer, socketPoolMem;

void levantar_servidor(Instruccion *instruccion) {
	listaConexiones = queue_create(); // Lista de conexiones que va a ir teniendo el servidor

	struct sockaddr_in serverAddress;
	struct sockaddr_in clientAddress;
	socketServer = socket(AF_INET, SOCK_STREAM, 0);
	if (socketServer <= -1) {
		log_error(LOGGER, "No se pudo crear el socket servidor");
		exit_gracefully(EXIT_SUCCESS);
	}

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(IP);
	serverAddress.sin_port = htons(PUERTO_KERNELL);
	memset(&(serverAddress.sin_zero), '\0', sizeof(serverAddress.sin_zero));

	int activado = 1;
	setsockopt(socketServer, SOL_SOCKET, SO_REUSEADDR, &activado,
			sizeof(activado));

	if (bind(socketServer, (struct sockaddr *) &serverAddress,
			sizeof(struct sockaddr)) == -1) {
		log_error(LOGGER, "Fallo el bind");
		exit_gracefully(EXIT_FAILURE);
	}

	if (listen(socketServer, 100) == -1) {
		log_error(LOGGER, "Fallo en el listen");
		exit_gracefully(EXIT_FAILURE);
	}
	pthread_t hiloHandler;
	pthread_create(&hiloHandler, NULL, (void*) atender_cliente, instruccion);

	while (1) {
		unsigned int tamanoDireccion = sizeof(struct sockaddr_in);
		int socketCliente = accept(socketServer,
				(struct sockaddr *) &clientAddress, &tamanoDireccion);
		if (socketCliente != -1) {
			printf("\nSe ha conectado %s por su puerto %d\n",
					inet_ntoa(clientAddress.sin_addr), clientAddress.sin_port);
			fcntl(socketCliente, F_SETFL, O_NONBLOCK);
			queue_push(listaConexiones, (int *) socketCliente);

			printf("el numero de elementos encolados es %d\n", queue_size(listaConexiones));
		}
	}
}

void atender_cliente(Instruccion *instruccion) {
	char * buffer = malloc(100);
	while (1) {
		if (queue_size(listaConexiones) > 0) {
			int socketCliente = queue_peek(listaConexiones);
			queue_pop(listaConexiones);
			int bytesRecibidos = recv(socketCliente, buffer, 99, 0);
			if (bytesRecibidos == 0){
				char * error = string_new();
				string_append(&error, "Cliente se desconecto: ");
				string_append(&error, strerror(errno));
				close(socketCliente);
				log_error(LOGGER, error);
			}else if ((errno == EAGAIN || errno == EWOULDBLOCK) && bytesRecibidos == -1) {
				queue_push(listaConexiones, (int *)socketCliente);
			}else{
				buffer[bytesRecibidos] = '\0';
				instruccion->funcion(buffer); // Aca pincha porque no debo estar allocando bien la memoria para la instruccion
			}
		}
	}
}

/*Aca vendrian a estar las query que leemos por consola (definir protocolo de comunicacion)*/
//char * msj = string_new();
//string_append(&msj, "lo que recibi del netcat es:");
//string_append(&msj, buffer);
//log_info(LOGGER, msj);
//queue_push(listaConexiones, (int *)socketCliente);

/*aca voy a levanatar el socket y enviar al proceso de memoria despues lo extraigo en una funcion aparte*/
/*
struct sockaddr_in direccionServidor;
direccionServidor.sin_family = AF_INET;
direccionServidor.sin_addr.s_addr = inet_addr(IP);
direccionServidor.sin_port = htons(PUERTO_POOL_MEM);

socketPoolMem = socket(AF_INET, SOCK_STREAM, 0);
if (connect(socketPoolMem, (void *) &direccionServidor,
		sizeof(direccionServidor)) != 0) {

	char * error = string_new();
	string_append(&error, "Error al conectar con el cliente:");
	string_append(&error, strerror(errno));
	close(*socketCliente);
	log_error(LOGGER, error);
	exit_gracefully(EXIT_FAILURE);
}

if (send(socketPoolMem, buffer, 99, 0) <= 0) {
	char * error = string_new();
	string_append(&error, "Error al enviar querys de la consola:");
	string_append(&error, strerror(errno));
	close(*socketCliente);
	log_error(LOGGER, error);
	exit_gracefully(EXIT_FAILURE);
}
#fin
free(buffer);
*/
