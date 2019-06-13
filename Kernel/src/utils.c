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
	char * k = string_new();
	string_append(&k, key);
	pthread_mutex_lock(&mutexRecursosCompartidos);
	dictionary_put(dic, k, value);
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

void preguntarPorMemoriasDisponibles(){
	while(true){
		sleep(PREGUNTAR_POR_MEMORIAS);
		Instruccion *inst = malloc(sizeof(Instruccion));
		inst->instruccion = GOSSIP;
		Gossip *gossip = malloc(sizeof(Gossip));
		gossip->lista_memorias = list_create();
		inst->instruccion_a_realizar = gossip;
		Instruccion * resp = enviar_instruccion(IP_MEMORIA_PPAL,
												PUERTO_MEMORIA_PPAL,
												inst,
												KERNEL,
												T_GOSSIPING);
		list_destroy(gossip->lista_memorias);
		free(gossip);
		free(inst);
		if(resp->instruccion == RETORNO){
			Retorno_Generico *ret = resp->instruccion_a_realizar;
			free(resp);
			if (ret->tipo_retorno == RETORNO_GOSSIP){
				Gossip * gossip = ret->retorno;
				free(ret);
				t_list * listaMemDisp =  gossip->lista_memorias;
				while(list_size(listaMemDisp) > 0){
					Memoria * mem = list_remove(listaMemDisp, 0);
					char * key = string_new();
					sprintf(key, "%d", mem->idMemoria);
					pthread_mutex_lock(&mutexRecursosCompartidos);
					if(!dictionary_has_key(memoriasDisponibles, key)){
						dictionary_put(memoriasDisponibles, key, mem);
						pthread_mutex_unlock(&mutexRecursosCompartidos);
					}else{
						free(mem->ip);
						free(mem->puerto);
						free(mem);
					}
				}
			}		
		}
		// Aca caigo si fue un error
		free(resp->instruccion_a_realizar);
		free(resp);
	}

}
