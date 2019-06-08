
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

t_log * LOGGER;
t_log * LOGGER_METRICS;

void configure_logger();
void exit_gracefully(int);
char *consistencia2string(Consistencias consistencia);
int string2consistencia(char* consistencia);


#endif /* UTILGUENGUENCHA_UTILS_H_ */
