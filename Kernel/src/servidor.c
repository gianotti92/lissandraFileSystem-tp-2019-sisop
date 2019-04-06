#include "servidor.h"

void sigchld_handler(int s) {
	while (wait(NULL) > 0);
}

int recibir_saludo(int fdCliente) {
	int resultado = 0;
	int numbytes = 0;
	int32_t longitud = 0;
	if ((numbytes = recv(fdCliente, &longitud, sizeof(int32_t), 0)) == -1) {
		//MUERO
		exit(1);
	}
	char* mensajeSaludoRecibido = malloc(sizeof(char) * longitud);
	if ((numbytes = recv(fdCliente, mensajeSaludoRecibido, longitud, 0)) == -1) {
		printf("No se pudo recibir mensaje saludo\n");
		//MUERO
		exit(1);
	}
	printf("Saludo recibido: %s\n", mensajeSaludoRecibido);

	if (strstr(mensajeSaludoRecibido, "POOL") != NULL) {
		resultado = 1;
	}

	free(mensajeSaludoRecibido);
	return resultado;
}

void enviar_saludo(int fdCliente) {

	char * mensajeSaludoEnviado = malloc(sizeof(char) * 100);
	strcpy(mensajeSaludoEnviado, "Hola, soy el KERNEL");
	mensajeSaludoEnviado[strlen(mensajeSaludoEnviado)] = '\0';

	int32_t longitud_mensaje = strlen(mensajeSaludoEnviado) + 1;

	void* bufferEnvio = malloc(sizeof(int32_t)+ sizeof(char)*longitud_mensaje);
	memcpy(bufferEnvio, &longitud_mensaje,sizeof(int32_t));
	memcpy(bufferEnvio + sizeof(int32_t),mensajeSaludoEnviado,longitud_mensaje);

	if (send(fdCliente, bufferEnvio,sizeof(int32_t)+ sizeof(char)*longitud_mensaje, 0) == -1) {
		perror("recv");
		printf("No se pudo enviar saludo\n");
		exit(1);
	}
	printf("Saludo enviado correctamente\n");

	free(bufferEnvio);
	free(mensajeSaludoEnviado);
}

void levantar_servidor_kernel() {

	int sockfd; // Escuchar sobre: sock_fd, nuevas conexiones sobre: idSocketCliente
	struct sockaddr_in my_addr;    // información sobre mi dirección
	struct sockaddr_in their_addr; // información sobre la dirección del idSocketCliente
	int sin_size;
	int yes = 1;
	struct sigaction sa;

	//1° CREAMOS EL SOCKET
	//sockfd: numero o descriptor que identifica al socket que creo
	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		printf("Error al abrir el socket de escucha\n");
		//free_parametros_config();
		exit(1);
	}
	printf("Se creo el socket correctamente\n");

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		printf("Address already in use\n");
		//free_parametros_config();
		exit(1);
	}

	my_addr.sin_family = PF_INET;         // Ordenación de bytes de la máquina
	my_addr.sin_port = htons(PUERTO_ESCUCHA_CONEXION);    // short, Ordenación de bytes de la red
	my_addr.sin_addr.s_addr = inet_addr(IP_CONFIG_MIO); //INADDR_ANY (aleatoria) o 127.0.0.1 (local)
	memset(&(my_addr.sin_zero), '\0', 8); // Poner a cero el resto de la estructura

	//2° Relacionamos los datos de my_addr <=> socket
	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))== -1) {
		printf("Fallo el bind\n");
		//free_parametros_config();
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
			//free_parametros_config();
			printf("Fallo el listen\n");
			exit(1);
		}

	printf("Socket escuchando!!!\n");


	//-------
		sa.sa_handler = sigchld_handler; // Eliminar procesos muertos
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = SA_RESTART;
		if (sigaction(SIGCHLD, &sa, NULL) == -1) {
			perror("sigaction");
			exit(1);
		}
		//--------
		sin_size = sizeof(struct sockaddr_in);

	//4° acepta y atiende
	while (1) {
		int *idSocketCliente = (int *) malloc(sizeof(int32_t));
		idSocketCliente[0] = -1; //TODO: que pasa con esta variable con varios hilos ?
		if ((idSocketCliente[0] = accept(sockfd,(struct sockaddr *) &their_addr, &sin_size)) == -1) {
			perror("Error al usar accept");
		}

		//CREAMOS UN HILO PARA ATENDER A CUALQUIER CLIENTE
		pthread_t punteroHilo;
		pthread_create(&punteroHilo, NULL, (void*) atender_cliente,
				idSocketCliente);

	}
	free_parametros_config();
	log_destroy(LOGGER);
	close(sockfd);


}

void atender_cliente(void* idSocketCliente) {

	//Saludo a todos los q se conectaron
	int fdCliente = ((int *) idSocketCliente)[0];

	enviar_saludo(fdCliente);
	int tipo_cliente = recibir_saludo(fdCliente);

	//FIN
	close(fdCliente);
	free((int *) idSocketCliente);

}
