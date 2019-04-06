#include "cliente.h"

void saludo_inicial_servidor(int sockfd, char* nombre) {
	//Recibo saludo
	int numbytes = 0;
	int32_t longitud = 0;
	if ((numbytes = recv(sockfd, &longitud, sizeof(int32_t), 0)) == -1) {
		printf("No se pudo recibir la longitud del saludo del %s\n", nombre);
		//MUERO
		exit(1);
	}
	char* mensajeSaludoRecibido = malloc(sizeof(char) * longitud);
	if ((numbytes = recv(sockfd, mensajeSaludoRecibido, longitud, 0)) == -1) {
		printf("No se pudo recibir saludo del %s\n", nombre);
		//MUERO
		exit(1);
	}
	int ID_OBTENIDO = 0;
	if(strcmp(nombre,"KERNEL") == 0){
		if ((numbytes = recv(sockfd, &ID_OBTENIDO, sizeof(int32_t), 0))
				== -1) {
			printf("No se pudo recibir mi ID\n");
			//MUERO
			exit(1);
		}

	}
	printf("Saludo recibido: %s\n", mensajeSaludoRecibido);

	if(ID_OBTENIDO != 0){
		printf("ID recibido: %d\n", ID_OBTENIDO);

	}
	free(mensajeSaludoRecibido);

	//Envio saludo
	if(strcmp(nombre,"KERNEL") == 0){
		t_respuesta_para_kernel respuesta_kernel = {.id_tipo_respuesta = 5, .id_esi = ID_OBTENIDO,
				.mensaje = "" };

		strcpy(respuesta_kernel.mensaje, "Hola, soy el POOLMEMORY");
		respuesta_kernel.mensaje[strlen(respuesta_kernel.mensaje)] = '\0';

		if (send(sockfd, &respuesta_kernel,sizeof(t_respuesta_para_kernel), 0) == -1) {
			printf("No se pudo enviar saludo al KERNEL\n");
			exit(1);
		}
		printf("Saludo enviado correctamente\n");

	}else{
		char * mensajeSaludoEnviado = malloc(sizeof(char) * 100);
		strcpy(mensajeSaludoEnviado, "Hola, soy el POOLMEMORY");
		mensajeSaludoEnviado[strlen(mensajeSaludoEnviado)] = '\0';

		int32_t longitud_mensaje = strlen(mensajeSaludoEnviado) + 1;

		void* bufferEnvio = malloc(sizeof(int32_t) + sizeof(char) * longitud_mensaje);
		memcpy(bufferEnvio, &longitud_mensaje, sizeof(int32_t));
		memcpy(bufferEnvio + sizeof(int32_t), mensajeSaludoEnviado,
				longitud_mensaje);
		if (send(sockfd, bufferEnvio,sizeof(int32_t) + sizeof(char) * longitud_mensaje, 0) == -1) {
			printf("No se pudo enviar saludo\n");

			exit(1);
		}
		printf("Saludo enviado correctamente\n");

		free(bufferEnvio);
		free(mensajeSaludoEnviado);


	}


}

int conectar_servidor(int puerto , char* ip, char* nombre) {

	//Creamos un socket
	int sockfd;
	struct sockaddr_in their_addr; // información de la dirección de destino

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("ERROR socket");
		//free_parametros_config();
		exit(1);
	}

	their_addr.sin_family = AF_INET;    // Ordenación de bytes de la máquina
	their_addr.sin_port = htons(puerto);  // short, Ordenación de bytes de la red
	their_addr.sin_addr.s_addr = inet_addr(ip);  //toma la ip directo

	memset(&(their_addr.sin_zero), 0, 8); // poner a cero el resto de la estructura

	if (connect(sockfd, (struct sockaddr *) &their_addr,
			sizeof(struct sockaddr)) == -1) {
		printf("No me pude conectar al %s\n", nombre);

		//free_parametros_config();
		exit(1);
	}

	return (sockfd);
}
