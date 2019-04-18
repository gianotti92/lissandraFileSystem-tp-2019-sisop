#ifndef KERNEL_H_
#define KERNEL_H_

#include <utilguenguencha/utilguenguencha.h>

typedef enum categoriaDeMensaje{
	ERROR,
	RUN,
	QUERY
} CategoriaDeMensaje;

t_dictionary *estadoReady;
t_dictionary *estadoNew;
t_dictionary *estadoExit;
t_dictionary *estadoExec;

void iniciarEstados();
CategoriaDeMensaje categoria(char ** mensaje);
void moverAEstado(CategoriaDeMensaje categoria, char ** mensaje);

#endif /* KERNEL_H_ */
