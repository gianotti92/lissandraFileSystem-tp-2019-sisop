#include "servidor.h"

t_queue * listaConexiones;
int socketKernell, socketPoolMem;

void levantar_servidor_kernel() {
	struct sockaddr_in kernelAddres;
	struct sockaddr_in consolClientAddres;
	int socketCliente;
	int activado = 1;
	fd_set rfds; //conjunto de descriptores a vigilar para hacer el select()

	listaConexiones = queue_create();
	socketKernell = socket(AF_INET, SOCK_STREAM, 0);
	if (socketKernell <= -1) {
		perror("No se pudo crear el socket servidor");
		exit_gracefully(EXIT_SUCCESS);
	}

	kernelAddres.sin_family = AF_INET;
	kernelAddres.sin_addr.s_addr = inet_addr(IP);
	kernelAddres.sin_port = htons(KERNELL_PORT);
	memset(&(kernelAddres.sin_zero), '\0', 8);

	setsockopt(socketKernell, SOL_SOCKET, SO_REUSEADDR, &activado,
			sizeof(activado));

	if (bind(socketKernell, (struct sockaddr *) &kernelAddres,
			sizeof(struct sockaddr)) == -1) {
		perror("Fallo el bind");
		exit_gracefully(EXIT_FAILURE);
	}

	if (listen(socketKernell, 100) == -1) {
		perror("Fallo en el listen");
		exit_gracefully(EXIT_FAILURE);
	}
	puts("Estoy escuchando...");
	/*--------------------------*/

	pthread_t punteroHilo;
	pthread_create(&punteroHilo, NULL, (void*) atender_cliente, NULL);

	while (1) {
		/* select() se carga el valor de rfds */
		FD_ZERO(&rfds);
		FD_SET(socketKernell, &rfds);

		if (select(socketKernell + 1, &rfds, NULL, NULL, NULL)) {
			unsigned int tamanoDireccion = sizeof(struct sockaddr_in);
			socketCliente = accept(socketKernell,
					(struct sockaddr *) &consolClientAddres, &tamanoDireccion);
			if (socketCliente != -1) {
				printf("\nSe ha conectado %s por su puerto %d\n",
						inet_ntoa(consolClientAddres.sin_addr),
						consolClientAddres.sin_port);
				queue_push(listaConexiones, (void *) socketCliente);

				printf("el numero de elementos encolados es %d\n",
						queue_size(listaConexiones));
			}
		}
	}

	exit_gracefully(EXIT_SUCCESS);
}

void atender_cliente(void* args) {
	while (1) {
		if (queue_size(listaConexiones) > 0) {
			char * buffer = malloc(100);

			int socketCliente = queue_peek(listaConexiones);
			queue_pop(listaConexiones);
			int bytesRecibidos = recv(socketCliente, buffer, 99, 0);
			if (bytesRecibidos <= 0) {
				perror("error al recibir datos");
				exit_gracefully(EXIT_FAILURE);
			}
			buffer[bytesRecibidos] = '\0';

			/*Aca vendrian a estar las query que leemos por consola (definir protocolo de comunicacion)*/
			printf("lo que recibi del netcat es: %s", buffer);

			/*aca voy a levanatar el socket y enviar al proceso de memoria despues lo extraigo en una funcion aparte*/
			struct sockaddr_in direccionServidor;
			direccionServidor.sin_family = AF_INET;
			direccionServidor.sin_addr.s_addr = inet_addr(IP);
			direccionServidor.sin_port = htons(PUERTO_POOL_MEM);

			socketPoolMem = socket(AF_INET, SOCK_STREAM, 0);
			if (connect(socketPoolMem, (void *) &direccionServidor,
					sizeof(direccionServidor)) != 0) {
				perror("Error al conectar con el cliente");
				exit_gracefully(EXIT_FAILURE);
			}

			if (send(socketPoolMem, buffer, 99, 0) <= 0) {
				perror("Error al enviar querys de la consola");
				exit_gracefully(EXIT_FAILURE);
			}
			/*fin*/
			free(buffer);
		}
	}
}

