
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

t_log * LOGGER;

void configure_logger();

void exit_gracefully(int);


#endif /* UTILGUENGUENCHA_UTILS_H_ */
