#include "comunicacion.h"

void servidor_comunicacion(t_comunicacion *comunicacion){
	fd_set fd_set_master, fd_set_temporal;
	int aux1, bytes_recibidos, fd_max, server_socket;
	server_socket = iniciar_servidor(comunicacion->puerto_servidor);
	FD_ZERO(&fd_set_master);
	FD_ZERO(&fd_set_temporal);
	FD_SET(server_socket, &fd_set_master);
	fd_max = server_socket; //4
	for (;;) {
		fd_set_temporal = fd_set_master;
		if (select(fd_max + 1, &fd_set_temporal, NULL, NULL, NULL) == -1) {
			exit_gracefully(EXIT_FAILURE);
		}
		int fin = fd_max;
		for (aux1 = 0; aux1 <= fin; aux1++) {
			if (FD_ISSET(aux1, &fd_set_temporal)) {
				if (aux1 == server_socket) {
					struct sockaddr_in client_address;
					size_t tamanio_client_address = sizeof(client_address);
					int socket_cliente = accept(server_socket,
							(struct sockaddr *) &client_address,
							&tamanio_client_address);
					if (socket_cliente < 0) {
						exit_gracefully(EXIT_FAILURE);
					}
					FD_SET(socket_cliente, &fd_set_master);
					if (socket_cliente > fd_max) {
						fd_max = socket_cliente;
					}
				} else {
					Procesos source;
					if ((bytes_recibidos = recv(aux1, &source, sizeof(Procesos),
					MSG_WAITALL)) <= 0) {
						liberar_conexion(aux1);
						FD_CLR(aux1, &fd_set_master);
					} else {
						Instruction_set inst_op;
						Instruccion *instruccion = malloc(sizeof(Instruccion));
						switch (source) {
						case KERNEL:
							if ((bytes_recibidos = recv(aux1, &inst_op,
									sizeof(Instruction_set), MSG_WAITALL))
									<= 0) {
								liberar_conexion(aux1);
								FD_CLR(aux1, &fd_set_master);
							}
							if (recibir_buffer(aux1, inst_op, instruccion)) {
								FD_CLR(aux1, &fd_set_master);
								retornarControl(instruccion, aux1);
							} else {
								FD_CLR(aux1, &fd_set_master);
								liberar_conexion(aux1);
							}
							break;
						case POOLMEMORY:
							if ((bytes_recibidos = recv(aux1, &inst_op,
									sizeof(Instruction_set), MSG_WAITALL))
									<= 0) {
								liberar_conexion(aux1);
								FD_CLR(aux1, &fd_set_master);
							}
							if (recibir_buffer(aux1, inst_op, instruccion)) {
								FD_CLR(aux1, &fd_set_master);
								retornarControl(instruccion, aux1);
							} else {
								FD_CLR(aux1, &fd_set_master);
								liberar_conexion(aux1);
							}
							break;
						case FILESYSTEM:
							if ((bytes_recibidos = recv(aux1, &inst_op,
									sizeof(Instruction_set), MSG_WAITALL))
									<= 0) {
								liberar_conexion(aux1);
								FD_CLR(aux1, &fd_set_master);
							}
							if (recibir_buffer(aux1, inst_op, instruccion)) {
								FD_CLR(aux1, &fd_set_master);
								retornarControl(instruccion, aux1);
							} else {
								FD_CLR(aux1, &fd_set_master);
								liberar_conexion(aux1);
							}
							break;
						default:
							log_info(LOGGER,
									"Recibo una conexion que no logra hacer handshake");
							liberar_conexion(aux1);
							FD_CLR(aux1, &fd_set_master);
						}
					}
				}
			}
		}
	}
}

int iniciar_servidor(char* puerto_servidor) {
	int socket_servidor;
	struct addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int activado = 1;

	getaddrinfo("127.0.0.1", puerto_servidor, &hints, &servinfo);

	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((socket_servidor = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1)
			continue;
		if (setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &activado,
				sizeof(activado)) < 0) {
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

bool recibir_buffer(int aux1, Instruction_set inst_op, Instruccion *instruccion) {
	size_t buffer_size;
	void* stream;
	int bytes_recibidos;
	switch (inst_op) {
	case SELECT:
		if ((bytes_recibidos = recv(aux1, &buffer_size, sizeof(size_t),
		MSG_WAITALL)) <= 0) {
			return false;
		}
		stream = malloc(buffer_size);
		if ((bytes_recibidos = recv(aux1, stream, buffer_size, MSG_WAITALL))
				<= 0) {
			return false;
		}
		Select *select;
		select = desempaquetar_select(stream);
		instruccion->instruccion = SELECT;
		instruccion->instruccion_a_realizar = select;
		return true;
	case INSERT:
		if ((bytes_recibidos = recv(aux1, &buffer_size, sizeof(size_t),
		MSG_WAITALL)) <= 0) {
			return false;
		}
		stream = malloc(buffer_size);
		if ((bytes_recibidos = recv(aux1, stream, buffer_size, MSG_WAITALL))
				<= 0) {
			return false;
		}
		Insert *insert;
		insert = desempaquetar_insert(stream);
		instruccion->instruccion = INSERT;
		instruccion->instruccion_a_realizar = insert;
		return true;
	case CREATE:
		if ((bytes_recibidos = recv(aux1, &buffer_size, sizeof(size_t),
		MSG_WAITALL)) <= 0) {
			return false;
		}
		stream = malloc(buffer_size);
		if ((bytes_recibidos = recv(aux1, stream, buffer_size, MSG_WAITALL))
				<= 0) {
			return false;
		}
		Create *create;
		create = desempaquetar_create(stream);
		instruccion->instruccion = CREATE;
		instruccion->instruccion_a_realizar = create;
		return true;
	case DESCRIBE:
		if ((bytes_recibidos = recv(aux1, &buffer_size, sizeof(size_t),
		MSG_WAITALL)) <= 0) {
			return false;
		}
		stream = malloc(buffer_size);
		if ((bytes_recibidos = recv(aux1, stream, buffer_size, MSG_WAITALL))
				<= 0) {
			return false;
		}
		Describe *describe;
		describe = desempaquetar_describe(stream);
		instruccion->instruccion = DESCRIBE;
		instruccion->instruccion_a_realizar = describe;
		return true;
	case DROP:
		if ((bytes_recibidos = recv(aux1, &buffer_size, sizeof(size_t),
		MSG_WAITALL)) <= 0) {
			return false;
		}
		stream = malloc(buffer_size);
		if ((bytes_recibidos = recv(aux1, stream, buffer_size, MSG_WAITALL))
				<= 0) {
			return false;
		}
		Drop *drop;
		drop = desempaquetar_drop(stream);
		instruccion->instruccion = DROP;
		instruccion->instruccion_a_realizar = drop;
		return true;
	case JOURNAL:
		if ((bytes_recibidos = recv(aux1, &buffer_size, sizeof(size_t),
		MSG_WAITALL)) <= 0) {
			return false;
		}
		stream = malloc(buffer_size);
		if ((bytes_recibidos = recv(aux1, stream, buffer_size, MSG_WAITALL))
				<= 0) {
			return false;
		}
		Journal *journal;
		journal = desempaquetar_journal(stream);
		instruccion->instruccion = JOURNAL;
		instruccion->instruccion_a_realizar = journal;
		return true;
	case GOSSIP:
		if ((bytes_recibidos = recv(aux1, &buffer_size, sizeof(size_t),
		MSG_WAITALL)) <= 0) {
			return false;
		}
		stream = malloc(buffer_size);
		if ((bytes_recibidos = recv(aux1, stream, buffer_size, MSG_WAITALL))
				<= 0) {
			return false;
		}
		Gossip *gossip;
		gossip = desempaquetar_gossip(stream);
		instruccion->instruccion = GOSSIP;
		instruccion->instruccion_a_realizar = gossip;
		return true;
	default:
		return false;
	}
}

int enviar_instruccion(char* ip, char* puerto, Instruccion *instruccion,
		Procesos proceso_del_que_envio) {
	int server_fd = crear_conexion(ip, puerto);
	if (server_fd == -1) {
		log_error(LOGGER, "No se puede establecer comunicacion con destino");
		return false;
	}
	Instruction_set tipo_de_instruccion = instruccion->instruccion;
	if (tipo_de_instruccion == ADD || tipo_de_instruccion == RUN
			|| tipo_de_instruccion == METRICS || tipo_de_instruccion == ERROR) {
		log_error(LOGGER,
				"Se intento enviar una instruccion que no corresponde");
		return false;
	} else {
		t_paquete * paquete = crear_paquete(proceso_del_que_envio, instruccion);
		if (!enviar_paquete(paquete, server_fd)) {
			liberar_conexion(server_fd);
			return -1;
		}
		eliminar_paquete(paquete);
		return server_fd;
	}
}

int crear_conexion(char *ip, char* puerto) {
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family,
			server_info->ai_socktype, server_info->ai_protocol);

	if (connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen)
			== -1) {
		return -1;
	}
	freeaddrinfo(server_info);

	return socket_cliente;
}

bool enviar_paquete(t_paquete* paquete, int socket_cliente) {
	int bytes = paquete->buffer->size + 3 * sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);
	if ((send(socket_cliente, a_enviar, bytes, 0)) < 0) {
		return false;
	}
	free(a_enviar);
	return true;
}

void liberar_conexion(int socket_cliente) {
	close(socket_cliente);
}

void* serializar_paquete(t_paquete* paquete, int bytes) {
	void * magic = malloc(bytes);
	int desplazamiento = 0;
	memcpy(magic + desplazamiento, &(paquete->source), sizeof(Procesos));
	desplazamiento += sizeof(Procesos);
	memcpy(magic + desplazamiento, &(paquete->header), sizeof(Instruction_set));
	desplazamiento += sizeof(Instruction_set);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(size_t));
	desplazamiento += sizeof(size_t);
	memcpy(magic + desplazamiento, paquete->buffer->stream,
			paquete->buffer->size);
	desplazamiento += paquete->buffer->size;
	return magic;
}

void crear_buffer(t_paquete* paquete) {
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

void eliminar_paquete(t_paquete* paquete) {
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

t_paquete* crear_paquete(Procesos proceso_del_que_envio,
		Instruccion* instruccion) {
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->source = proceso_del_que_envio;
	paquete->header = instruccion->instruccion;
	crear_buffer(paquete);
	switch (instruccion->instruccion) {
	case SELECT:
		empaquetar_select(paquete,
				(Select*) instruccion->instruccion_a_realizar);
		break;
	case INSERT:
		empaquetar_insert(paquete,
				(Insert*) instruccion->instruccion_a_realizar);
		break;
	case CREATE:
		empaquetar_create(paquete,
				(Create*) instruccion->instruccion_a_realizar);
		break;
	case DESCRIBE:
		empaquetar_describe(paquete,
				(Describe*) instruccion->instruccion_a_realizar);
		break;
	case DROP:
		empaquetar_drop(paquete, (Drop*) instruccion->instruccion_a_realizar);
		break;
	case JOURNAL:
		empaquetar_journal(paquete,
				(Journal*) instruccion->instruccion_a_realizar);
		break;
	case GOSSIP:
		empaquetar_gossip(paquete,
				(Gossip*) instruccion->instruccion_a_realizar);
		break;
	default:
		free(paquete->buffer);
		free(paquete);
		return (t_paquete*) NULL;
		break;
	}
	return paquete;
}

void empaquetar_select(t_paquete *paquete, Select *select) {
	size_t tamanio_nombre_tabla = (strlen(select->nombre_tabla) + 1);
	paquete->buffer->stream = malloc(
			sizeof(select->key) + sizeof(size_t) + tamanio_nombre_tabla
					+ sizeof(select->timestamp));
	memcpy(paquete->buffer->stream, &select->key, sizeof(select->key));
	paquete->buffer->size += sizeof(select->key);
	memcpy(paquete->buffer->stream + paquete->buffer->size,
			&tamanio_nombre_tabla, sizeof(tamanio_nombre_tabla));
	paquete->buffer->size += sizeof(tamanio_nombre_tabla);
	memcpy(paquete->buffer->stream + paquete->buffer->size,
			select->nombre_tabla, tamanio_nombre_tabla);
	paquete->buffer->size += tamanio_nombre_tabla;
	memcpy(paquete->buffer->stream + paquete->buffer->size, &select->timestamp,
			sizeof(select->timestamp));
	paquete->buffer->size += sizeof(select->timestamp);
}

void empaquetar_insert(t_paquete *paquete, Insert *insert) {
	size_t tamanio_nombre_tabla = (strlen(insert->nombre_tabla) + 1);
	size_t tamanio_value = (strlen(insert->value) + 1);
	paquete->buffer->stream = malloc(
			sizeof(insert->key) + sizeof(size_t) + tamanio_nombre_tabla
					+ sizeof(insert->timestamp) + sizeof(size_t) + tamanio_value);
	memcpy(paquete->buffer->stream, &insert->key, sizeof(insert->key));
	paquete->buffer->size += sizeof(insert->key);
	memcpy(paquete->buffer->stream + paquete->buffer->size,
			&tamanio_nombre_tabla, sizeof(tamanio_nombre_tabla));
	paquete->buffer->size += sizeof(tamanio_nombre_tabla);
	memcpy(paquete->buffer->stream + paquete->buffer->size,
			insert->nombre_tabla, tamanio_nombre_tabla);
	paquete->buffer->size += tamanio_nombre_tabla;
	memcpy(paquete->buffer->stream + paquete->buffer->size, &insert->timestamp,
			sizeof(insert->timestamp));
	paquete->buffer->size += sizeof(insert->timestamp);
	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio_value,
			sizeof(tamanio_value));
	paquete->buffer->size += sizeof(tamanio_value);
	memcpy(paquete->buffer->stream + paquete->buffer->size, insert->value,
			tamanio_value);
	paquete->buffer->size += tamanio_value;
}

void empaquetar_create(t_paquete * paquete, Create *create) {
	size_t tamanio_nombre_tabla = (strlen(create->nombre_tabla) + 1);
	paquete->buffer->stream = malloc(
			sizeof(create->compactation_time) + sizeof(create->consistencia)
					+ sizeof(size_t) + tamanio_nombre_tabla + sizeof(create->particiones)
					+ sizeof(create->timestamp));
	memcpy(paquete->buffer->stream, &create->compactation_time,
			sizeof(create->compactation_time));
	paquete->buffer->size += sizeof(create->compactation_time);
	memcpy(paquete->buffer->stream + paquete->buffer->size,
			&create->consistencia, sizeof(create->consistencia));
	paquete->buffer->size += sizeof(create->consistencia);
	memcpy(paquete->buffer->stream + paquete->buffer->size,
			&tamanio_nombre_tabla, sizeof(tamanio_nombre_tabla));
	paquete->buffer->size += sizeof(tamanio_nombre_tabla);
	memcpy(paquete->buffer->stream + paquete->buffer->size,
			create->nombre_tabla, tamanio_nombre_tabla);
	paquete->buffer->size += tamanio_nombre_tabla;
	memcpy(paquete->buffer->stream + paquete->buffer->size,
			&create->particiones, sizeof(create->particiones));
	paquete->buffer->size += sizeof(create->particiones);
	memcpy(paquete->buffer->stream + paquete->buffer->size, &create->timestamp,
			sizeof(create->timestamp));
	paquete->buffer->size += sizeof(create->timestamp);
}

void empaquetar_describe(t_paquete * paquete, Describe *describe) {
	size_t tamanio_nombre_tabla = (strlen(describe->nombre_tabla) + 1);
	paquete->buffer->stream = malloc( sizeof(size_t) +
			tamanio_nombre_tabla + sizeof(describe->timestamp));
	memcpy(paquete->buffer->stream, &tamanio_nombre_tabla,
			sizeof(tamanio_nombre_tabla));
	paquete->buffer->size += sizeof(tamanio_nombre_tabla);
	memcpy(paquete->buffer->stream + paquete->buffer->size,
			describe->nombre_tabla, tamanio_nombre_tabla);
	paquete->buffer->size += tamanio_nombre_tabla;
	memcpy(paquete->buffer->stream + paquete->buffer->size,
			&describe->timestamp, sizeof(describe->timestamp));
	paquete->buffer->size += sizeof(describe->timestamp);
}

void empaquetar_drop(t_paquete * paquete, Drop * drop) {
	size_t tamanio_nombre_tabla = (strlen(drop->nombre_tabla) + 1);
	paquete->buffer->stream = malloc( sizeof(size_t) +
			tamanio_nombre_tabla + sizeof(drop->timestamp));
	memcpy(paquete->buffer->stream, &tamanio_nombre_tabla,
			sizeof(tamanio_nombre_tabla));
	paquete->buffer->size += sizeof(tamanio_nombre_tabla);
	memcpy(paquete->buffer->stream + paquete->buffer->size, drop->nombre_tabla,
			tamanio_nombre_tabla);
	paquete->buffer->size += tamanio_nombre_tabla;
	memcpy(paquete->buffer->stream + paquete->buffer->size, &drop->timestamp,
			sizeof(drop->timestamp));
	paquete->buffer->size += sizeof(drop->timestamp);
}

void empaquetar_journal(t_paquete * paquete, Journal * journal) {
	paquete->buffer->stream = malloc(sizeof(journal->timestamp));
	memcpy(paquete->buffer->stream, &journal->timestamp,
			sizeof(journal->timestamp));
	paquete->buffer->size += sizeof(journal->timestamp);
}

void empaquetar_gossip(t_paquete * paquete, Gossip * gossip) {
	int cantidad_memorias = list_size(gossip->lista_memorias);
	paquete->buffer->stream = malloc(sizeof(int));
	memcpy(paquete->buffer->stream, &cantidad_memorias, sizeof(int));
	paquete->buffer->size += sizeof(int);
	while(cantidad_memorias > 0){
		Memoria *memoria = malloc(sizeof(Memoria));
		memoria = list_get(gossip->lista_memorias, cantidad_memorias - 1);
		size_t tamanio_ip = (strlen(memoria->ip) + 1 );
		size_t tamanio_puerto = (strlen(memoria->puerto) + 1 );
		size_t tamanio_id = sizeof(memoria->idMemoria);
		size_t tamanio = tamanio_ip + sizeof(size_t) + tamanio_puerto + sizeof(size_t) + tamanio_id;
		paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio);
		memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio_ip, sizeof(size_t));
		paquete->buffer->size += sizeof(size_t);
		memcpy(paquete->buffer->stream + paquete->buffer->size, memoria->ip, (strlen(memoria->ip)+1));
		paquete->buffer->size += strlen(memoria->ip) + 1;
		memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio_puerto, sizeof(size_t));
		paquete->buffer->size += sizeof(size_t);
		memcpy(paquete->buffer->stream + paquete->buffer->size, memoria->puerto, (strlen(memoria->puerto)+1));
		paquete->buffer->size += strlen(memoria->puerto) + 1;
		memcpy(paquete->buffer->stream + paquete->buffer->size, &memoria->idMemoria, sizeof(memoria->idMemoria));
		paquete->buffer->size += sizeof(memoria->idMemoria);
		cantidad_memorias--;
		free(memoria);
	}
}

Select *desempaquetar_select(void* stream) {
	int desplazamiento = 0;
	Select *select = malloc(sizeof(Select));
	int tamanio;
	memcpy(&select->key, stream, sizeof(select->key));
	desplazamiento += sizeof(select->key);
	memcpy(&tamanio, stream + desplazamiento, sizeof(size_t));
	desplazamiento += sizeof(size_t);
	select->nombre_tabla = malloc(tamanio);
	memcpy(select->nombre_tabla, stream + desplazamiento, tamanio);
	desplazamiento += tamanio;
	memcpy(&select->timestamp, stream + desplazamiento,
			sizeof(select->timestamp));
	return select;
}

Insert *desempaquetar_insert(void* stream) {
	int desplazamiento = 0;
	Insert *insert = malloc(sizeof(Insert));
	size_t tamanio;
	memcpy(&insert->key, stream, sizeof(insert->key));
	desplazamiento += sizeof(insert->key);
	memcpy(&tamanio, stream + desplazamiento, sizeof(tamanio));
	desplazamiento += sizeof(tamanio);
	insert->nombre_tabla = malloc(tamanio);
	memcpy(insert->nombre_tabla, stream + desplazamiento, tamanio);
	desplazamiento += tamanio;
	memcpy(&insert->timestamp, stream + desplazamiento,
			sizeof(insert->timestamp));
	desplazamiento += sizeof(insert->timestamp);
	memcpy(&tamanio, stream + desplazamiento, sizeof(tamanio));
	desplazamiento += sizeof(tamanio);
	insert->value = malloc(tamanio);
	memcpy(insert->value, stream + desplazamiento, tamanio);
	return insert;
}
Create *desempaquetar_create(void* stream) {
	int desplazamiento = 0;
	Create *create = malloc(sizeof(Create));
	size_t tamanio;
	memcpy(&create->compactation_time, stream,
			sizeof(create->compactation_time));
	desplazamiento += sizeof(create->compactation_time);
	memcpy(&create->consistencia, stream + desplazamiento,
			sizeof(create->consistencia));
	desplazamiento += sizeof(create->consistencia);
	memcpy(&tamanio, stream + desplazamiento, sizeof(tamanio));
	desplazamiento += sizeof(tamanio);
	create->nombre_tabla = malloc(tamanio);
	memcpy(create->nombre_tabla, stream + desplazamiento, tamanio);
	desplazamiento += tamanio;
	memcpy(&create->particiones, stream + desplazamiento,
			sizeof(create->particiones));
	desplazamiento += sizeof(create->particiones);
	memcpy(&create->timestamp, stream + desplazamiento,
			sizeof(create->timestamp));
	return create;
}
Describe *desempaquetar_describe(void* stream) {
	int desplazamiento = 0;
	Describe *describe = malloc(sizeof(Describe));
	size_t tamanio;
	memcpy(&tamanio, stream, sizeof(tamanio));
	desplazamiento += sizeof(tamanio);
	describe->nombre_tabla = malloc(tamanio);
	memcpy(describe->nombre_tabla, stream + desplazamiento, tamanio);
	desplazamiento += tamanio;
	memcpy(&describe->timestamp, stream + desplazamiento,
			sizeof(describe->timestamp));
	return describe;
}

Drop *desempaquetar_drop(void* stream) {
	int desplazamiento = 0;
	Drop *drop = malloc(sizeof(Drop));
	size_t tamanio;
	memcpy(&tamanio, stream, sizeof(tamanio));
	desplazamiento += sizeof(tamanio);
	drop->nombre_tabla = malloc(tamanio);
	memcpy(drop->nombre_tabla, stream + desplazamiento, tamanio);
	desplazamiento += tamanio;
	memcpy(&drop->timestamp, stream + desplazamiento, sizeof(drop->timestamp));
	return drop;
}
Journal *desempaquetar_journal(void* stream) {
	Journal *journal = malloc(sizeof(Journal));
	memcpy(&journal->timestamp, stream, sizeof(journal->timestamp));
	return journal;
}

Gossip *desempaquetar_gossip(void* stream){
	int desplazamiento = 0;
	Gossip *gossip = malloc(sizeof(Gossip));
	gossip->lista_memorias = list_create();
	size_t cantidad_memorias, tamanio;
	memcpy(&cantidad_memorias, stream, sizeof(cantidad_memorias));
	desplazamiento += sizeof(cantidad_memorias);
	while(cantidad_memorias != 0){
		Memoria *memoria = malloc(sizeof(Memoria));
		memcpy(&tamanio, stream + desplazamiento, sizeof(tamanio));
		desplazamiento += sizeof(tamanio);
		memoria->ip = malloc(tamanio);
		memcpy(memoria->ip, stream + desplazamiento, tamanio);
		desplazamiento += tamanio;
		memcpy(&tamanio, stream + desplazamiento, sizeof(tamanio));
		desplazamiento += sizeof(tamanio);
		memoria->puerto = malloc(tamanio);
		memcpy(memoria->puerto, stream + desplazamiento, tamanio);
		desplazamiento += tamanio;
		memcpy(&memoria->idMemoria, stream + desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);
		list_add(gossip->lista_memorias, memoria);
		free(memoria->ip);
		free(memoria->puerto);
		free(memoria);
		cantidad_memorias--;
	}
	return gossip;
}
