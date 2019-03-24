//
// Test de prueba de Sockets by Matias Busco.
//

#include "listener.c"
#include "handler.c"


void configure_logger(){
    logger = log_create("socket_server.log", "SOCKET_SERVER", true, LOG_LEVEL_INFO);
}

int main(){
    configure_logger();
    pthread_t listener_thread, handler_thread;
    pthread_create(&listener_thread, NULL, listener_proc(8080, INADDR_ANY), NULL);
    pthread_create(&handler_thread, NULL, handler_proc(), NULL);
    return 0;

}