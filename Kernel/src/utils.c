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

void putTablaSafe(t_dictionary * dic, char* key, char * value){
	pthread_mutex_lock(&mutexRecursosCompartidos);
	dictionary_put(dic, key, value);
	pthread_mutex_unlock(&mutexRecursosCompartidos);
}

void sacarMemoriaDeTodasLasListas(Memoria *memoria){
	Consistencias consistencia;
	for(consistencia = EC; consistencia < (DISP+1); consistencia++){
		t_list * lista_consistencia = list_get(memorias, consistencia);
		int  aux = existe_memoria_en(memoria, lista_consistencia);
		if(aux > 0){
			list_remove_and_destroy_element(lista_consistencia, aux, (void*)eliminar_memoria);
		}
	}
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

char* getTablasSafe(t_dictionary * dic, char*key){
	pthread_mutex_lock(&mutexRecursosCompartidos);
	char * nombre = (char*)dictionary_get(dic, key);
	pthread_mutex_unlock(&mutexRecursosCompartidos);
	return nombre;
}

void asignarConsistenciaAMemoria(Memoria * memoria, Consistencias consistencia){
	if(memoria != NULL){
		if(existe_memoria_en(memoria, list_get(memorias, consistencia)) < 0 ){
			list_add(list_get(memorias, consistencia), memoria);
			pthread_mutex_unlock(&mutexRecursosCompartidos);
			pthread_mutex_lock(&mutexRecursosCompartidos);
		}
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
				t_list * lista_memorias_retorno_gossip =  gossip->lista_memorias;
				if(lista_memorias_retorno_gossip->elements_count == 0) {
					break;
				}
				int aux = 0;
				while(aux < lista_memorias_retorno_gossip->elements_count){
					Memoria *mem = getMemoria(lista_memorias_retorno_gossip, aux);
					if(getMemoria(list_get(memorias, DISP), mem->idMemoria) == NULL){
						pthread_mutex_lock(&mutexRecursosCompartidos);
						list_add(list_get(memorias, DISP), mem);
						pthread_mutex_unlock(&mutexRecursosCompartidos);
					}
					aux++;
				}
				aux = 0;
				while(aux < ((t_list*)list_get(memorias, DISP))->elements_count){
					Memoria *mem = getMemoria(list_get(memorias, DISP), aux);
					if(existe_memoria_en(mem, lista_memorias_retorno_gossip) < 0){
						list_remove_and_destroy_element(list_get(memorias, DISP), aux, (void*)eliminar_memoria);
					}
					aux++;
				}
				list_destroy_and_destroy_elements(lista_memorias_retorno_gossip, (void*)eliminar_memoria);
			}
		}
		free(resp->instruccion_a_realizar);
		free(resp);
	}
}

void eliminar_memoria(Memoria * memoria){
	free(memoria->ip);
	free(memoria->puerto);
	free(memoria);
}

int existe_memoria_en(Memoria *mem1, t_list* lista){
	int aux = 0;
	while(aux < lista->elements_count){
		Memoria * mem2 = list_get(lista, aux);
		if(strcmp(mem1->ip, mem2->ip) == 0 &&
		   strcmp(mem1->puerto, mem2->puerto) == 0 &&
		   mem1->idMemoria == mem2->idMemoria){
			return aux;
		}
		aux ++;
	}
	return -1;
}

void mostrarId(Memoria * memoria){
	printf("%d ", memoria->idMemoria);
}
