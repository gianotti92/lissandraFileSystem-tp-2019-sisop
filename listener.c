#include "dependences.h"

void listen_new_calls(int server_socket){ // Escucho mientras que no tire error
    while(listen(server_socket, BACKLOG) > 0){
        struct sockaddr_in client_dir;
        int dir_size = sizeof(struct sockaddr_in);
        client_socket = accept(server_socket, (void *) &client_dir, &dir_size);
        if(client_socket > 0){
            queue_push(proc_queue, client_create(client_socket, client_dir)); // Ingreso el cliente en la cola
        }
        else{
            log_error(logger, strerror(errno)); // Fallo el accept

        }
    log_error(logger, strerror(errno)); // Fallo el listen
    }
}

void* listener_proc(int sin_port, in_addr_t sin_addr){

    struct sockaddr_in server_dir;
    server_dir.sin_family = AF_INET;
    server_dir.sin_addr.s_addr = sin_addr;
    server_dir.sin_port = htons(sin_port);

    // Levanto el socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1){
        log_error(logger, strerror(errno)); // Fallo el socket
        //freeaddrinfo(server_dir);
        close(server_socket);
        exit(1);
    }

    // Para permitir multiples conexiones
    int activado = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado)) == -1){
        log_error(logger, strerror(errno)); // Fallo el setsockopt
        //freeaddrinfo(server_dir);
        close(server_socket);
        exit(1);
    }

    // Asocio el socket a la direccion y el puerto
    if (bind(server_socket, (void*) &server_dir, sizeof(server_dir)) != 0){
        log_error(logger, strerror(errno)); // Fallo el bind
        //freeaddrinfo(server_dir);
        close(server_socket);
        exit(1);
    }

    // Libero ya que no lo usamos
    //freeaddrinfo(server_dir); 

    listen_new_calls(server_socket);
}


