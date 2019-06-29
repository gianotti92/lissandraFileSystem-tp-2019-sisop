#ifndef UTILGUENGUENCHA_UTILS_H_
#define UTILGUENGUENCHA_UTILS_H_

#include <commons/collections/dictionary.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <commons/log.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "tipos_guenguencha.h"

#define EVENT_SIZE (sizeof (struct inotify_event))
#define EVENT_BUF_LEN (1024*(EVENT_SIZE + 16))
t_log * LOG_INFO;
t_log * LOG_ERROR;
t_log * LOG_DEBUG;
t_log * LOG_OUTPUT;
t_log * LOG_ERROR_SV;
t_log * LOG_OUTPUT_SV;
t_dictionary *fd_disponibles;
void configure_logger(void);
void exit_gracefully(int error);
char *consistencia2string(Consistencias consistencia);
int string2consistencia(char* consistencia);
int monitorNode(char * node,int mode,int(*callback)(void));
void eliminar_y_cerrar_fd_abiertos(int * fd);
void print_guenguencha(void);
char LOCAL_IP[16];
char *get_local_ip(void);
Memoria *duplicar_memoria(Memoria *memoria);
Retorno_Describe *duplicar_describe(Retorno_Describe *describe);
t_list * list_duplicate_all(t_list *lista, void*(*duplicador)(void*), pthread_mutex_t mutex);
/**
* @NAME: elimina_memoria()
* @DESC: para eliminar las memorias de una lista de gossip
*/
void eliminar_memoria(Memoria * memoria);
/**
* @NAME: eliminar_describe()
* @DESC: para eliminar describe de una lista de describes
*/
void eliminar_describe(Retorno_Describe *ret_desc);
/**
* @NAME: mostrar_memoria()
* @DESC: Mostrar las memorias
*/
void mostrar_memoria(Memoria * memoria);
#endif /* UTILGUENGUENCHA_UTILS_H_ */
