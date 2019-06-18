
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
#include "tipos_guenguencha.h"
#include <sys/inotify.h>


#define EVENT_SIZE (sizeof (struct inotify_event))
#define EVENT_BUF_LEN (1024*(EVENT_SIZE + 16))
t_log * LOGGER;
t_log * LOG_ERROR;

void configure_logger();
void exit_gracefully(int);
char *consistencia2string(Consistencias consistencia);
int string2consistencia(char* consistencia);
int monitorNode(char * node,int mode,int(*callback)(void));
//Proceso *dame_siguiente(Proceso* proceso);


void print_guenguencha();


#endif /* UTILGUENGUENCHA_UTILS_H_ */
