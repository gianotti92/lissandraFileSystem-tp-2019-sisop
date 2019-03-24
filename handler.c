//
// Test de prueba de Sockets by Matias Busco.
//

#include "dependences.h"

// Este proc deberá levantar de la cola los que tenga y meterlos en el thread q tenga
void* handler_proc(){
    while(1){
        if(queue_size(proc_queue) != 0 ){
            t_client_proccess *client_process = queue_pop(proc_queue);
            struct sockaddr_in client_dir = client_process->client_dir;
            int client_fd = client_process->client_fd;
            printf("Recibi un request de  %s", inet_ntoa(client_dir.sin_addr));
            if (send(client_fd, "Hello, world!\n", 14, 0) == -1)
                perror("send");
                close(client_fd);
                //freeaddrinfo(client_dir);
                queue_destroy(proc_queue);
                exit(1);
        }
        printf("Chau! Me dormi 5 segundos\n");
        sleep(5);
    }
}

