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

Memoria *getMemoria(t_list *lista_memorias, int idMemoria){
	int aux = 0;
	Memoria *mem = NULL;
	while(aux < lista_memorias->elements_count){
		mem = list_get(lista_memorias, aux);
		if(mem->idMemoria == idMemoria)break;
		mem = NULL;
		aux++;
	}
	return mem;
}

t_list * getMemoriasAsociadasSafe(Consistencia consistencia){
	pthread_mutex_lock(&mutexRecursosCompartidos);
	t_list *listaMemorias = list_get(memoriasAsociadas, consistencia);
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
	if(memoria != NULL){
		t_list* lista_de_una_consistencia = list_get(memoriasAsociadas, consistencia);
		pthread_mutex_lock(&mutexRecursosCompartidos);
		list_add(lista_de_una_consistencia, memoria);
		pthread_mutex_unlock(&mutexRecursosCompartidos);
	}
}


void lanzar_gossiping(){
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
					pthread_mutex_lock(&mutexRecursosCompartidos);
					if(getMemoria(memoriasDisponibles, mem->idMemoria) == NULL){
						list_add(memoriasDisponibles, mem);
						pthread_mutex_unlock(&mutexRecursosCompartidos);
					}else{
						free(mem->ip);
						free(mem->puerto);
						free(mem);
					}
				}
				list_destroy(listaMemDisp);
			}
		}
		free(resp->instruccion_a_realizar);
		free(resp);
	}

}

void mostrarId(Memoria * memoria){
	printf("%d ", memoria->idMemoria);
}
