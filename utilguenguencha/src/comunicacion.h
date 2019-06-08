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
#include <errno.h>
#include <error.h>
#include "utils.h"

#define BACKLOG 10     // Cuántas conexiones pendientes se mantienen en cola
extern int errno;


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
*		 -> Tipo_Comunicacion tipo_comunicacion (T_GOSSIPING, T_INSTRUCCION, T_VALUE)
* @RET: void
*/
void servidor_comunicacion(Comunicacion *comunicacion);
/**
* @NAME: iniciar_servidor
* @DESC: Es la funcion que inicia los parametros para recibir en localhost:puerto
* @ARGS: char* puerto -> puerto donde va a escuchar
* @RET: int fd_servidor -> fd donde correspondiente al servidor
*/
int iniciar_servidor(char* puerto);
/**
* @NAME: crear_conexion
* @DESC: Funcion que crea una conexion con la IP y Puerto y devuelve fd asociado
* @ARGS: char* ip -> ip asociada al destino que quiero conectarme
* 		 char* puerto -> puerto asociado al destino donde quiero conectarme
* @RET: int fd_destino -> fd del destino
*/
int crear_conexion(char* ip, char* puerto);
/**
* @NAME: crear_paquete
* @DESC: Funcion que crea un paquete con los datos suministrados
* @ARGS: Tipo_Comunicacion tipo_comu -> tipo de comunicacion que se esta realizando
* 		 Procesos proceso_del_que_envio -> source del request
* 		 Instruccion *instruccion -> instruccion a empaquetar
* @RET: t_paquete *paquete -> paquete a enviar
*/
t_paquete* crear_paquete(Tipo_Comunicacion tipo_comu, Procesos proceso_del_que_envio, Instruccion* instruccion);
/**
* @NAME: crear_paquete_retorno
* @DESC: Crea un paquete de retorno para enviar
* @ARGS: Instruccion* instruccion -> el retorno o error a enviar
* @RET:  t_paquete *paquete -> paquete armado
*/
t_paquete_retorno *crear_paquete_retorno(Instruccion *instruccion);
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
* @NAME: enviar_paquete
* @DESC: Funcion que serializa el paquete y lo envia al fd indicado
* @ARGS: t_paquete *paquete -> paquete a enviar
* 		 int socket_cliente -> fd del destino al que le envio le paquete
* @RET:  bool Enviado / No Enviado
*/
bool enviar_paquete(t_paquete* paquete, int socket_cliente);
/**
* @NAME: liberar_conexion
* @DESC: Cierra el fd enviado por parametro
* @ARGS: int socket_cliente -> fd a cerrar
* @RET:  void
*/
void liberar_conexion(int socket_cliente);
/**
* @NAME: eliminar_paquete
* @DESC: Vacia las estructuras de un paquete y libera memoria
* @ARGS: t_paquete* paquete -> paquete a liberar
* @RET:  void
*/
void eliminar_paquete(t_paquete* paquete);
/**
* @NAME: eliminar_paquete_retorno()
* @DESC: Borra las estrucutas del paquete
* @ARGS: t_paquete *paquete -> paquete a borrar
* @RET:  void
*/
void eliminar_paquete_retorno(t_paquete_retorno* paquete);
/**
* @NAME: empaquetar_select
* @DESC: Empaqueta la estructura de un select en el paquete enviado
* @ARGS: t_paquete *paquete -> paquete donde empaquetar
* 		 Select *select -> Estructura a empaquetar
* @RET:  void
*/
void empaquetar_select(t_paquete *paquete, Select *select);
/**
* @NAME: empaquetar_insert
* @DESC: Empaqueta la estructura de un insert en el paquete enviado
* @ARGS: t_paquete *paquete -> paquete donde empaquetar
* 		 Insert *insert -> Estructura a empaquetar
* @RET:  void
*/
void empaquetar_insert(t_paquete *paquete, Insert *insert);
/**
* @NAME: empaquetar_create
* @DESC: Empaqueta la estructura de un create en el paquete enviado
* @ARGS: t_paquete *paquete -> paquete donde empaquetar
* 		 Create *create -> Estructura a empaquetar
* @RET:  void
*/
void empaquetar_create(t_paquete * paquete, Create *create);
/**
* @NAME: empaquetar_describe
* @DESC: Empaqueta la estructura de un describe en el paquete enviado
* @ARGS: t_paquete *paquete -> paquete donde empaquetar
* 		 Describe *describe -> Estructura a empaquetar
* @RET:  void
*/
void empaquetar_describe(t_paquete * paquete, Describe *describe);
/**
* @NAME: empaquetar_drop
* @DESC: Empaqueta la estructura de un drop en el paquete enviado
* @ARGS: t_paquete *paquete -> paquete donde empaquetar
* 		 Drop *drop -> Estructura a empaquetar
* @RET:  void
*/
void empaquetar_drop(t_paquete * paquete, Drop * drop);
/**
* @NAME: empaquetar_journal
* @DESC: Empaqueta la estructura de un journal en el paquete enviado
* @ARGS: t_paquete *paquete -> paquete donde empaquetar
* 		 Journal *journal -> Estructura a empaquetar
* @RET:  void
*/
void empaquetar_journal(t_paquete * paquete, Journal * journal);
/**
* @NAME: empaquetar_gossip
* @DESC: Empaqueta la estructura de un gossip en el paquete enviado
* @ARGS: t_paquete *paquete -> paquete donde empaquetar
* 		 Gossip *gossip -> Estructura a empaquetar
* @RET:  void
*/
void empaquetar_gossip(t_paquete * paquete, Gossip * gossip);
/**
* @NAME: serializar_paquete
* @DESC: Devuelve un puntero a un stream en el que se contiene todo
* @ARGS: t_paquete *paquete -> paquete a serializar
* 		 int bytes -> cantidad de bytes que tendra el tream de output
* @RET:  void *
*/
void* serializar_paquete(t_paquete* paquete, int bytes);
/**
* @NAME: recibir_buffer
* @DESC: Abstraccion realizada para recibir lo correspondiente a un servidor
* @ARGS: int aux1 -> fd donde estoy recibiendo
* 		 Instruction_set inst_op -> Instruccion que estoy por leer
* 		 Instruccion *instruccion -> instruccion donde dejo lo recibido
* 		 Tipo_Comunicacion tipo_comu -> tipo de comunicacion que tealizo
* @RET:  bool PUDE RECIBIR / NO PUDE RECIBIR
*/
bool recibir_buffer(int aux1, Instruction_set inst_op, Instruccion *instruccion, Tipo_Comunicacion tipo_comu);
/**
* @NAME: desempaquetar_select
* @DESC: Desempaqueta lo recibido en stream y lo mete en la estructura
* @ARGS: void *stream -> stream donde estan los datos a leer
* @RET:  Select *select -> estructura que levanto del desempaquetado
*/
Select *desempaquetar_select(void* stream);
/**
* @NAME: desempaquetar_insert
* @DESC: Desempaqueta lo recibido en stream y lo mete en la estructura
* @ARGS: void *stream -> stream donde estan los datos a leer
* @RET:  Insert *insert -> estructura que levanto del desempaquetado
*/
Insert *desempaquetar_insert(void* stream);
/**
* @NAME: desempaquetar_create
* @DESC: Desempaqueta lo recibido en stream y lo mete en la estructura
* @ARGS: void *stream -> stream donde estan los datos a leer
* @RET:  Create *create -> estructura que levanto del desempaquetado
*/
Create *desempaquetar_create(void* stream);
/**
* @NAME: desempaquetar_describe
* @DESC: Desempaqueta lo recibido en stream y lo mete en la estructura
* @ARGS: void *stream -> stream donde estan los datos a leer
* @RET:  Describe *describe -> estructura que levanto del desempaquetado
*/
Describe *desempaquetar_describe(void* stream);
/**
* @NAME: desempaquetar_drop
* @DESC: Desempaqueta lo recibido en stream y lo mete en la estructura
* @ARGS: void *stream -> stream donde estan los datos a leer
* @RET:  Drop *drop -> estructura que levanto del desempaquetado
*/
Drop *desempaquetar_drop(void* stream);
/**
* @NAME: desempaquetar_journal
* @DESC: Desempaqueta lo recibido en stream y lo mete en la estructura
* @ARGS: void *stream -> stream donde estan los datos a leer
* @RET:  Journal *journal -> estructura que levanto del desempaquetado
*/
Journal *desempaquetar_journal(void* stream);
/**
* @NAME: desempaquetar_gossip
* @DESC: Desempaqueta lo recibido en stream y lo mete en la estructura
* @ARGS: void *stream -> stream donde estan los datos a leer
* @RET:  Gossip *gossip -> estructura que levanto del desempaquetado
*/
Gossip *desempaquetar_gossip(void* stream);
/**
* @NAME: validar_sender
* @DESC: Valida el sender segun el tipo de comunicacion, el sender y el receiver
* @ARGS: Procesos sender -> quien envio la instruccion
* 		 Procesos receiver -> quien esta recibiendo la instruccion
* 		 Tipo_Comunicacion -> el tipo de comunicacion que se tiene
* @RET:  bool Valido Recibir mensaje / No valido Recibir Mensaje
*/
bool validar_sender(Procesos sender, Procesos receiver, Tipo_Comunicacion comunicacion);
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
* @NAME: recibir_respuesta
* @DESC: Se bloquea (Temporalmente) esperando respuesta de a quien le envie
* @ARGS: int fd_a_escuchar -> a donde temos que escuchar
* @RET:	 Instruccion *respuesta -> respuesta que obtenemos del que nos envia
*		 Solo puede ser RETORNO->(Seteado con el que corresponda)
*		 					(VALOR, DATOS_DESCRIBE, TAMANIO_VALOR_MAXIMO, SUCCESS)
*		 				ERROR->(Seteado con el que corresponda)
*/
Instruccion *recibir_respuesta(int fd_a_escuchar);
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
* @NAME: recibir_retorno
* @DESC: Recibe Retorno y devuelve el la instruccion
* @ARGS: int fd_a_escuchar -> es el fd donde escucho el retorno
* @RET:  Instruccion *respuesta -> La respuesta del retorno
*/
Instruccion *recibir_retorno(int fd_a_escuchar);
/**
* @NAME: armar_retorno_value
* @DESC: Devuelve la instruccion correspondiente a Retorno con value
* @ARGS: char *value -> el valor a retornar
* 		 t_timestamp timestamp -> el timestamp correspondiente
* @RET:  Instruccion *respuesta -> La respuesta del retorno con el value
*/
Instruccion *armar_retorno_value(void *chunk);
/**
* @NAME: recibir_error
* @DESC: Recibe el error correspondiente en el fd que le enviamos por parametro
* @ARGS: int fd_a_escuchar -> donde recibimos el error
* @RET:  Instruccion *respuesta -> La respuesta con el error asiciado
*/
Instruccion *recibir_error(int fd_a_escuchar);
/**
* @NAME: armar_retorno_gossip()
* @DESC: 
* @ARGS: 
* @RET:  
*/
Instruccion *armar_retorno_gossip(void *chunk);
/**
* @NAME: serializar_paquete_retorno
* @DESC: Devuelve un puntero a un stream en el que se contiene todo
* @ARGS: t_paquete_retorno *paquete -> paquete a serializar
* 		 int bytes -> cantidad de bytes que tendra el tream de output
* @RET:  void*
*/
void *serializar_paquete_retorno(t_paquete_retorno *paquete, int bytes);
/**
* @NAME: enviar_paquete_retorno
* @DESC: Funcion que serializa el paquete y lo envia al fd indicado
* @ARGS: t_paquete_retorno *paquete -> paquete a enviar
* 		 int socket_cliente -> fd del destino al que le envio le paquete
* @RET:  bool Enviado / No Enviado
*/
bool enviar_paquete_retorno(t_paquete_retorno* paquete, int socket_cliente);
/**
* @NAME: armar_retorno_max_value()
* @DESC: Devuelve una instruccion con el MAX_VALUE seteado
* @ARGS: size_t max_value -> corresponde al valor maximo de dato en tabla
* @RET:  Instruccion* instruccion -> instruccion con el max value seteado
*/
Instruccion *armar_retorno_max_value(void *chunk);
/**
* @NAME: armar_retorno_describe()
* @DESC: Recibe una lista de describes y devuelve una instruccion de retorno
*		 con la lista
* @ARGS: t_list *lista_describes -> lista a meter en la instruccion
* @RET:  Instruccion* instruccion -> instruccion con la lista dentro
*/
Instruccion *armar_retorno_describe(void *chunk);
/**
* @NAME: empaquetar_retorno_valor()
* @DESC: Mete en el paquete los datos necesarios
* @ARGS: t_paquete *paquete -> paquete donde meter todo
*		 Retorno_Value *ret_val -> lo que necesito meter en el paquete
* @RET:  void
*/
void empaquetar_retorno_valor(t_paquete_retorno *paquete, Retorno_Value *ret_val);
/**
* @NAME: empaquetar_retorno_describe()
* @DESC: Mete en el paquete los datos necesarios
* @ARGS: t_paquete *paquete -> paquete donde meter todo
*		 t_list* list_of_describes -> lo que necesito meter en el paquete
* @RET:  void
*/
void empaquetar_retorno_describe(t_paquete_retorno *paquete, Describes *describes);
/**
* @NAME: empaquetar_retorno_max_val()
* @DESC: Mete en el paquete los datos necesarios
* @ARGS: t_paquete *paquete -> paquete donde meter todo
*		 Retorno_Max_Value *max_val -> lo que necesito meter en el paquete
* @RET:  void
*/
void empaquetar_retorno_max_val(t_paquete_retorno *paquete, Retorno_Max_Value *max_val);
/**
* @NAME: empaquetar_retorno_error()
* @DESC: Mete en el paquete los datos necesarios
* @ARGS: t_paquete *paquete -> paquete donde meter todo
*		 Error *error -> lo que necesito meter en el paquete
* @RET:  void
*/
void empaquetar_retorno_error(t_paquete_retorno *paquete, Error *error);
/**
* @NAME: empaquetar_retorno_gossip()
* @DESC: Mete en el paquete los datos necesarios
* @ARGS: t_paquete *paquete -> paquete donde meter todo
*		 Gossip *ret_gos -> lo que necesito meter en el paquete
* @RET:  void
*/
void empaquetar_retorno_gossip(t_paquete_retorno *paquete, Gossip *ret_gos);
/**
* @NAME: empaquetar_retorno_success()
* @DESC: Empaqueta un success
* @ARGS: t_paquete *paquete -> paquete donde meter todo
* @RET:  void
*/
void empaquetar_retorno_success(t_paquete_retorno *paquete);

#endif /* UTILGUENGUENCHA_COMUNICACION_H_ */
