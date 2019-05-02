#include "comunicacion.h"

void servidor_comunicacion(void (*funcion_retorno) (Instruccion*, int), char* puerto_servidor, Procesos proceso){
	fd_set fd_set_master, fd_set_temporal;
	int aux1, bytes_recibidos, fd_max, server_socket;
	server_socket = iniciar_servidor(puerto_servidor);
	FD_ZERO(&fd_set_master);
	FD_ZERO(&fd_set_temporal);
	FD_SET(server_socket, &fd_set_master);
	fd_max = server_socket;
	for(;;){
		fd_set_temporal = fd_set_master;
		if (select(fd_max+1, &fd_set_temporal, NULL, NULL, NULL) == -1) {
			exit_gracefully(EXIT_FAILURE);
		}
		int fin = fd_max;
		for(aux1 = 0; aux1 <= fin; aux1++){
			if(FD_ISSET(aux1, &fd_set_temporal)){
				if(aux1 == server_socket){
					struct sockaddr_in client_address;
					size_t tamanio_client_address = sizeof(client_address);
					int socket_cliente = accept(server_socket,
												(struct sockaddr *) &client_address,
												&tamanio_client_address);
					if(socket_cliente < 0){
						exit_gracefully(EXIT_FAILURE);
					}
					FD_SET(socket_cliente, &fd_set_master);
					if(socket_cliente > fd_max){
						fd_max = socket_cliente;
					}
				}
				else{
					Procesos source;
					if((bytes_recibidos = recv(aux1, &source, sizeof(Procesos), MSG_WAITALL)) <= 0){
						liberar_conexion(aux1);
						FD_CLR(aux1, &fd_set_master);
					}
					else{
						Instruction_set inst_op;
						Instruccion *instruccion;
						switch(source){
							case KERNEL:
								if((bytes_recibidos = recv(aux1, &inst_op, sizeof(Instruction_set), MSG_WAITALL)) <= 0){
									liberar_conexion(aux1);
									FD_CLR(aux1, &fd_set_master);
								}
								if(recibir_buffer(aux1, inst_op, instruccion)){
									FD_CLR(aux1, &fd_set_master);
									funcion_retorno(instruccion, aux1);
								}
								else{
									FD_CLR(aux1, &fd_set_master);
									liberar_conexion(aux1);
								}
								break;
							case POOLMEMORY:
								if((bytes_recibidos = recv(aux1, &inst_op, sizeof(Instruction_set), MSG_WAITALL)) <= 0){
									liberar_conexion(aux1);
									FD_CLR(aux1, &fd_set_master);
								}
								if(recibir_buffer(aux1, inst_op, instruccion)){
									FD_CLR(aux1, &fd_set_master);
									funcion_retorno(instruccion, aux1);
								}
								else{
									FD_CLR(aux1, &fd_set_master);
									liberar_conexion(aux1);
								}
								break;
							case FILESYSTEM:
								if((bytes_recibidos = recv(aux1, &inst_op, sizeof(Instruction_set), MSG_WAITALL)) <= 0){
									liberar_conexion(aux1);
									FD_CLR(aux1, &fd_set_master);
								}
								if(recibir_buffer(aux1, inst_op, instruccion)){
									FD_CLR(aux1, &fd_set_master);
									funcion_retorno(instruccion, aux1);
								}
								else{
									FD_CLR(aux1, &fd_set_master);
									liberar_conexion(aux1);
								}
								break;
							default:
								log_info(LOGGER, "Recibo una conexion que no logra hacer handshake");
						}
					}
				}
			}
		}
	}
}

int iniciar_servidor(char* puerto_servidor){
    int socket_servidor;
	struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int activado = 1;

    getaddrinfo("127.0.0.1", puerto_servidor, &hints, &servinfo);

    for (p=servinfo; p != NULL; p = p->ai_next){
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;
        if (setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado)) < 0){
            close(socket_servidor);
        	continue;
            }
        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_servidor);
            continue;
        }
        break;
    }

    if (listen(socket_servidor, BACKLOG) < 0) {
    		exit_gracefully(EXIT_FAILURE);
    	}

    freeaddrinfo(servinfo);

    return socket_servidor;
}

bool recibir_buffer(int aux1, Instruction_set inst_op, Instruccion *instruccion){
	size_t buffer_size;
	void* stream;
	int bytes_recibidos;
	switch(inst_op){
		case SELECT:
			if((bytes_recibidos = recv(aux1, &buffer_size, sizeof(size_t), MSG_WAITALL)) <= 0){
				return false;
			}
			if((bytes_recibidos = recv(aux1, &stream, buffer_size, MSG_WAITALL)) <= 0){
				return false;
			}
			Select *select = malloc(sizeof(Select));
			select = desempaquetar_select(stream);
			instruccion = malloc(sizeof(Instruccion));
			instruccion->instruccion = SELECT;
			instruccion->instruccion_a_realizar = select;
			return true;
		case INSERT:
			if((bytes_recibidos = recv(aux1, &buffer_size, sizeof(size_t), MSG_WAITALL)) <= 0){
				return false;
			}
			if((bytes_recibidos = recv(aux1, &stream, buffer_size, MSG_WAITALL)) <= 0){
				return false;
			}
			Insert *insert = malloc(sizeof(Insert));
			insert = desempaquetar_insert(stream);
			instruccion = malloc(sizeof(Instruccion));
			instruccion->instruccion = INSERT;
			instruccion->instruccion_a_realizar = insert;
			return true;
		case CREATE:
			if((bytes_recibidos = recv(aux1, &buffer_size, sizeof(size_t), MSG_WAITALL)) <= 0){
				return false;
			}
			if((bytes_recibidos = recv(aux1, &stream, buffer_size, MSG_WAITALL)) <= 0){
				return false;
			}
			Create *create = malloc(sizeof(Create));
			select = desempaquetar_create(stream);
			instruccion = malloc(sizeof(Instruccion));
			instruccion->instruccion = CREATE;
			instruccion->instruccion_a_realizar = create;
			return true;
		case DESCRIBE:
			if((bytes_recibidos = recv(aux1, &buffer_size, sizeof(size_t), MSG_WAITALL)) <= 0){
				return false;
			}
			if((bytes_recibidos = recv(aux1, &stream, buffer_size, MSG_WAITALL)) <= 0){
				return false;
			}
			Describe *describe = malloc(sizeof(Describe));
			select = desempaquetar_describe(stream);
			instruccion = malloc(sizeof(Instruccion));
			instruccion->instruccion = DESCRIBE;
			instruccion->instruccion_a_realizar = describe;
			return true;
		case DROP:
			if((bytes_recibidos = recv(aux1, &buffer_size, sizeof(size_t), MSG_WAITALL)) <= 0){
				return false;
			}
			if((bytes_recibidos = recv(aux1, &stream, buffer_size, MSG_WAITALL)) <= 0){
				return false;
			}
			Drop *drop = malloc(sizeof(Drop));
			select = desempaquetar_drop(stream);
			instruccion = malloc(sizeof(Instruccion));
			instruccion->instruccion = DROP;
			instruccion->instruccion_a_realizar = drop;
			return true;
		case JOURNAL:
			if((bytes_recibidos = recv(aux1, &buffer_size, sizeof(size_t), MSG_WAITALL)) <= 0){
				return false;
			}
			if((bytes_recibidos = recv(aux1, &stream, buffer_size, MSG_WAITALL)) <= 0){
				return false;
			}
			Journal *journal = malloc(sizeof(Journal));
			journal = desempaquetar_journal(stream);
			instruccion = malloc(sizeof(Instruccion));
			instruccion->instruccion = JOURNAL;
			instruccion->instruccion_a_realizar = journal;
			return true;
		default:
			return false;
	}
}

bool enviar_instruccion(char* ip, char* puerto, Instruccion *instruccion, Procesos proceso_del_que_envio){
	int server_fd = crear_conexion(ip, puerto);
	if(server_fd == -1){
		log_error(LOGGER, "No se puede establecer comunicacion con destino");
		return false;
	}
	Instruction_set tipo_de_instruccion = instruccion->instruccion;
	if(tipo_de_instruccion == ADD || tipo_de_instruccion == RUN || tipo_de_instruccion == METRICS || tipo_de_instruccion == ERROR){
		log_error(LOGGER, "Se intento enviar una instruccion que no corresponde");
		return false;
	}
	else{
		t_paquete * paquete = crear_paquete(proceso_del_que_envio, instruccion);
		if(!enviar_paquete(paquete, server_fd)){
			return false;
		}
		eliminar_paquete(paquete);
		liberar_conexion(server_fd);
		return true;
	}
}

int crear_conexion(char *ip, char* puerto){
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1){
		return -1;
	}
	freeaddrinfo(server_info);

	return socket_cliente;
}

bool enviar_paquete(t_paquete* paquete, int socket_cliente){
	int bytes = paquete->buffer->size + 3*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);
	if((send(socket_cliente, a_enviar, bytes, 0)) < 0){
		return false;
	}
	free(a_enviar);
	return true;
}

void liberar_conexion(int socket_cliente){
	close(socket_cliente);
}

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;
	memcpy(magic + desplazamiento, &(paquete->source), sizeof(Procesos));
	desplazamiento+= sizeof(Procesos);
	memcpy(magic + desplazamiento, &(paquete->header), sizeof(Instruction_set));
	desplazamiento+= sizeof(Instruction_set);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(size_t));
	desplazamiento+= sizeof(size_t);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;
	return magic;
}

void crear_buffer(t_paquete* paquete){
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio){
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);
	paquete->buffer->size += tamanio + sizeof(int);
}

void eliminar_paquete(t_paquete* paquete){
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

t_paquete* crear_paquete(Procesos proceso_del_que_envio, Instruccion* instruccion){
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->source = proceso_del_que_envio;
	paquete->header = instruccion->instruccion;
	crear_buffer(paquete);
	switch(instruccion->instruccion){
		case SELECT:
			empaquetar_select(paquete, (Select*) instruccion->instruccion_a_realizar);
			break;
		case INSERT:
			empaquetar_insert(paquete, (Insert*) instruccion->instruccion_a_realizar);
			break;
		case CREATE:
			empaquetar_create(paquete, (Create*) instruccion->instruccion_a_realizar);
			break;
		case DESCRIBE:
			empaquetar_describe(paquete, (Describe*) instruccion->instruccion_a_realizar);
			break;
		case DROP:
			empaquetar_drop(paquete, (Drop*) instruccion->instruccion_a_realizar);
			break;
		case JOURNAL:
			empaquetar_journal(paquete, (Journal*) instruccion->instruccion_a_realizar);
			break;
		default:
			free(paquete);
			return (t_paquete*) NULL;
			break;
	}
	return paquete;
}

void empaquetar_select(t_paquete *paquete, Select *select){
	agregar_a_paquete(paquete, &select->key, sizeof(select->key));
	agregar_a_paquete(paquete, select->nombre_tabla, (strlen(select->nombre_tabla)+1));
	agregar_a_paquete(paquete, &select->timestamp, sizeof(select->timestamp));
}

void empaquetar_insert(t_paquete *paquete, Insert *insert){
	agregar_a_paquete(paquete, &insert->key, sizeof(insert->key));
	agregar_a_paquete(paquete, insert->nombre_tabla, (strlen(insert->nombre_tabla)+1));
	agregar_a_paquete(paquete, &insert->timestamp, sizeof(insert->timestamp));
	agregar_a_paquete(paquete, insert->value, (strlen(insert->value)+1));
}

void empaquetar_create(t_paquete * paquete, Create *create){
	agregar_a_paquete(paquete, &create->compactation_time, sizeof(create->compactation_time));
	agregar_a_paquete(paquete, &create->consistencia, sizeof(create->consistencia));
	agregar_a_paquete(paquete, create->nombre_tabla, (strlen(create->nombre_tabla)+1));
	agregar_a_paquete(paquete, &create->particiones, sizeof(create->particiones));
	agregar_a_paquete(paquete, &create->timestamp, sizeof(create->timestamp));
}

void empaquetar_describe(t_paquete * paquete, Describe *describe){
	agregar_a_paquete(paquete, describe->nombre_tabla, (strlen(describe->nombre_tabla)+1));
	agregar_a_paquete(paquete, &describe->timestamp, sizeof(describe->timestamp));
}

void empaquetar_drop(t_paquete * paquete, Drop * drop){
	agregar_a_paquete(paquete, drop->nombre_tabla, (strlen(drop->nombre_tabla)+1));
	agregar_a_paquete(paquete, &drop->timestamp, sizeof(drop->timestamp));
}

void empaquetar_journal(t_paquete * paquete, Journal * journal){
	agregar_a_paquete(paquete, &journal->timestamp, sizeof(journal->timestamp));
}

Select *desempaquetar_select(void* stream){
	int desplazamiento = 0;
	Select *select = malloc(sizeof(Select));
	int tamanio;
	memcpy(&tamanio, stream+desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(&select->key, stream+desplazamiento, tamanio);
	desplazamiento+=tamanio;
	memcpy(&tamanio, stream+desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(&select->nombre_tabla, stream+desplazamiento, tamanio);
	desplazamiento+=tamanio;
	memcpy(&tamanio, stream+desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(&select->timestamp, stream+desplazamiento, tamanio);
	return select;
}

Insert *desempaquetar_insert(void* stream){
	int desplazamiento = 0;
	Insert *insert= malloc(sizeof(Insert));
	int tamanio;
	return insert;
}
Create *desempaquetar_create(void* stream){
	int desplazamiento = 0;
	Create *create= malloc(sizeof(Create));
	int tamanio;
	return create;
}
Describe *desempaquetar_describe(void* stream){
	int desplazamiento = 0;
	Describe *describe= malloc(sizeof(Describe));
	int tamanio;
	return describe;
}
Drop *desempaquetar_drop(void* stream){
	int desplazamiento = 0;
	Drop *drop= malloc(sizeof(Drop));
	int tamanio;
	return drop;
}
Journal *desempaquetar_journal(void* stream){
	int desplazamiento = 0;
	Journal *journal= malloc(sizeof(Journal));
	int tamanio;
	return journal;
}

