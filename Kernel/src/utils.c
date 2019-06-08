#include "kernel.h"

bool esFinQuantum(Proceso * p, char * instruccionALeer){
	return instruccionALeer != NULL && p->quantumProcesado == 2;
}

bool esFinLectura(Proceso * p, char * instruccionALeer){
	return instruccionALeer == NULL;
}

void encolar(t_list * cola, Proceso * proceso) {
	pthread_mutex_lock(&mutexRecursosCompartidos);
	list_add(cola, proceso);
	pthread_mutex_unlock(&mutexRecursosCompartidos);
}

Proceso * desencolar(t_list * cola){
	pthread_mutex_lock(&mutexRecursosCompartidos);
	Proceso * p = list_remove(cola, 0);
	pthread_mutex_unlock(&mutexRecursosCompartidos);
	return p;
}

Memoria * desencolarMemoria(t_list * lista, int posicion){
	pthread_mutex_lock(&mutexRecursosCompartidos);
	Memoria * m = list_get(lista, posicion);
	pthread_mutex_unlock(&mutexRecursosCompartidos);
	return m;
}

void putTablaSafe(t_dictionary * dic, char* key, char * value){
	pthread_mutex_lock(&mutexRecursosCompartidos);
	dictionary_put(dic, key, value);
	pthread_mutex_unlock(&mutexRecursosCompartidos);
}

void putMemoryListSafe(t_dictionary * dic, char* key, t_list * value){
	pthread_mutex_lock(&mutexRecursosCompartidos);
	dictionary_put(dic, key, value);
	pthread_mutex_unlock(&mutexRecursosCompartidos);
}

void putMemorySafe(t_dictionary * dic, char* key, Memoria * value){
	pthread_mutex_lock(&mutexRecursosCompartidos);
	dictionary_put(dic, key, value);
	pthread_mutex_unlock(&mutexRecursosCompartidos);
}

Memoria * getMemoriaSafe(t_dictionary * dic, char*key){
	pthread_mutex_lock(&mutexRecursosCompartidos);
	Memoria * m = (Memoria*)dictionary_get(dic, key);
	pthread_mutex_unlock(&mutexRecursosCompartidos);
	return m;
}

t_list * getMemoriasAsociadasSafe(t_dictionary * dic, char*key){
	pthread_mutex_lock(&mutexRecursosCompartidos);
	t_list * listaMemorias = dictionary_get(dic, key);
	pthread_mutex_unlock(&mutexRecursosCompartidos);
	return listaMemorias;
}


char* getTablasSafe(t_dictionary * dic, char*key){
	pthread_mutex_lock(&mutexRecursosCompartidos);
	char * nombre = (char*)dictionary_get(dic, key);
	pthread_mutex_unlock(&mutexRecursosCompartidos);
	return nombre;
}

void asignarConsistenciaAMemoria(Memoria * memoria, Consistencias consistencia){
	Memoria * m = malloc(sizeof(Memoria));
	m = memoria;

	if(m != NULL){
		char* consistencia_str=consistencia2string(consistencia);
		if( consistencia == SC){
			list_clean(memoriasSc);
			list_add(memoriasSc,m);
			putMemoryListSafe(memoriasAsociadas, consistencia_str, memoriasSc);
		}else if(consistencia == EC){
			list_add(memoriasEv, m);
			putMemoryListSafe(memoriasAsociadas, consistencia_str, memoriasEv);
		}else{
			list_add(memoriasHc, m);
			putMemoryListSafe(memoriasAsociadas, consistencia_str, memoriasHc);
		}
		free(consistencia_str);
	}else{
		log_error(LOGGER, "Error al asignar una memoria");
	}

}

//void preguntarPorMemoriasDisponibles(){
//	while(true){
//		sleep(PREGUNTAR_POR_MEMORIAS);
//		Instruccion *instruccion = malloc(sizeof(Instruccion));
//		instruccion->instruccion = GOSSIP;
//
//		Instruccion * instruccionRespuesta = enviar_instruccion(IP_MEMORIA_PPAL, PUERTO_MEMORIA_PPAL, instruccion, KERNEL, T_GOSSIPING);
//
//		switch(instruccionRespuesta->instruccion){
//			case ERROR:;
//				log_error(LOGGER, "Kernel. Error al recibir la respuesta");
//				break;
//
//			case GOSSIP:;
//				Gossip * gossip = (Gossip *) instruccionRespuesta->instruccion_a_realizar;
//				t_list * listaMemDisp =  gossip->lista_memorias;
//
//				while(list_size(listaMemDisp) > 0){
//					Memoria * mem = list_remove(listaMemDisp, 0);
//
//					char * key = string_new();
//
//					sprintf(key, "%d", mem->idMemoria);
//
//					pthread_mutex_lock(&mutexRecursosCompartidos);
//					dictionary_put(memoriasDisponibles, key, mem);
//					pthread_mutex_unlock(&mutexRecursosCompartidos);
//				}
//				break;
//
//			default:;
//				log_error(LOGGER, "KERNEL. Error no debería pasar por aca.");
//				break;
//		}
//	}
//}

void preguntarPorMemoriasDisponibles(){
	Memoria * m = malloc(sizeof(Memoria));

	/* funcion de conexiones que me devuelve memoria disponible */
	m->idMemoria = 1;
	char * ip = string_new();
	char * puerto = string_new();
	string_append(&ip, IP_MEMORIA_PPAL);
	string_append(&puerto, PUERTO_MEMORIA_PPAL);
	m->puerto = puerto;
	m->ip = ip;
	/* funcion de conexiones que me devuelve memoria disponible */

	char * key = string_new();
	sprintf(key, "%d", m->idMemoria);
	putMemorySafe(memoriasDisponibles, key , m);
}

