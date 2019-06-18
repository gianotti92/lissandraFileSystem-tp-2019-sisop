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
#include "tipos_guenguencha.h"

#define EVENT_SIZE (sizeof (struct inotify_event))
#define EVENT_BUF_LEN (1024*(EVENT_SIZE + 16))
t_log * LOGGER;
t_log * LOG_ERROR;
t_log * LOGGER_METRICS;

void configure_logger(void);
void exit_gracefully(int error);
char *consistencia2string(Consistencias consistencia);
int string2consistencia(char* consistencia);
int monitorNode(char * node,int mode,int(*callback)(void));
void print_guenguencha(void);
char LOCAL_IP[16];
char *get_local_ip(void);

#endif /* UTILGUENGUENCHA_UTILS_H_ */
