#ifndef UTILGUENGUENCHA_COMUNICACION_H_
#define UTILGUENGUENCHA_COMUNICACION_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <error.h>
#include "utils.h"
#include <semaphore.h>

#define BACKLOG 10     // Cuántas conexiones pendientes se mantienen en cola

/**
* @NAME: retornarControl
* @DESC: Es la funcion que llaman cuando reciben una instruccion por servidor_comunicacion()
* @ARGS: Instruccion *instruccion -> instruccion a realizar
* 		 int socket_cliente -> fd del cliente que realizo la instruccion
* @RET: void
*/
void retornarControl(Instruccion *instruccion, int socket_cliente);
/**
* @NAME: servidor_comunicacion
* @DESC: Funcion que levanta un servicio de servidor de comunicacion para recibir instrucciones
* @ARGS: Comunicacion *comunicacion
* 		 -> char* puerto_servidor -> puerto donde quiero escuchar nuevas instrucciones
*		 -> Procesos proceso (KERNEL, FILESYSTEM, POOLMEMORY) -> Proceso que ejecuta el servidor
* @RET: void
*/
void servidor_comunicacion(Comunicacion *comunicacion);
/**
* @NAME: enviar_instruccion
* @DESC: Esta funcion recibe una instruccion y se la envia a la IP:PUERTO designados segun tipo_comu
* @ARGS: char *ip -> ip a la que le enviamos la instruccion
* 		 char *puerto -> puerto al que le enviamos la instruccion
* 		 Procesos proceso_del_que_envio -> El source desde donde envio la instruccion
* 		 Tipo_Comunicacion tipo_comu -> Es el tipo de comunicacion que realizo
* @RET:  Instruccion *respuesta -> respuesta que obtenemos del que nos envia
*		 Solo puede ser RETORNO->(Seteado con el que corresponda)
*		 					(VALOR, DATOS_DESCRIBE, TAMANIO_VALOR_MAXIMO, SUCCESS)
*		 				ERROR->(Seteado con el que corresponda)
*/
Instruccion *enviar_instruccion(char* ip, char* puerto, Instruccion *instruccion, Procesos proceso_del_que_envio, Tipo_Comunicacion tipo_comu);
/**
* @NAME: responder
* @DESC: Responder solo con RETORNO o ERROR al fd que nos consulto algo
* @ARGS: int fd_a_responder -> fd al que tenemos que responderle
* 		 Instruccion * instruccion -> (RETORNO o ERROR), es la respuesta
* @RET:  Instruccion * instruccion -> Respuesta que recibimos ante el envio
* 		 Solo puede ser RETORNO->SUCCESS o ERROR->(Error seteado segun corresp.)
*/
Instruccion *responder(int fd_a_responder, Instruccion *instruccion);
/**
* @NAME: respuesta_error
* @DESC: Devuelve una estructura Instruccion ERROR con el error seteado
* @ARGS: Error_set error -> errro a setear en la estructura a devolver
* @RET:  Instruccion * respuesta_error -> La respuesta con el error
*/
Instruccion *respuesta_error(Error_set error);
/**
* @NAME: respuesta_success
* @DESC: Devuelve una Instruccion RETORNO con SUCCESS seteado
* @ARGS: void
* @RET:  Instruccion *respuesta_success -> La respuesta success
*/
Instruccion *respuesta_success(void);
/**
* @NAME: chequear_conexion_a()
* @DESC: Chequea la conexion a la ip y puerto dados
* @ARGS: char* ip -> ip destino
*		 char* puerto -> puerto destino
* @RET:  bool
*/
bool chequear_conexion_a(char* ip, char* puerto);
#endif /* UTILGUENGUENCHA_COMUNICACION_H_ */
