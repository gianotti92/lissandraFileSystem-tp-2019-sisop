/*
 * kernel.h
 *
 *  Created on: 16 abr. 2019
 *      Author: luqui
 */

#ifndef KERNEL_H_
#define KERNEL_H_

#include <commons/collections/dictionary.h>
#include <commons/string.h>
#include <string.h>

typedef enum categoriaDeMensaje{
	ERROR,
	RUN,
	QUERY
} CategoriaDeMensaje;


void iniciarEstados();
CategoriaDeMensaje categoria(char * mensaje);
void moverAEstado(CategoriaDeMensaje categoria, char * mensaje);

#endif /* KERNEL_H_ */
