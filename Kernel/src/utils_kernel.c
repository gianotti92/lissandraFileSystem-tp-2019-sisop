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
		if(mem->idMemoria == idMemoria){
			return mem;
		}
		aux++;
	}
	return NULL;
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
	Memoria * memoriaPrincipal = malloc(sizeof(Memoria));
	memoriaPrincipal->idMemoria = 1;
	memoriaPrincipal->puerto = malloc(strlen(PUERTO_MEMORIA_PPAL) + 1);
	strcpy(memoriaPrincipal->puerto, PUERTO_MEMORIA_PPAL);
	memoriaPrincipal->ip = malloc(strlen(IP_MEMORIA_PPAL) + 1);
	strcpy(memoriaPrincipal->ip, IP_MEMORIA_PPAL);
	agregarSiNoExiste(list_get(memorias, DISP), memoriaPrincipal);
	eliminar_memoria(memoriaPrincipal);
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
		if(resp->instruccion == RETORNO){
			Retorno_Generico *ret = resp->instruccion_a_realizar;
			if (ret->tipo_retorno == RETORNO_GOSSIP){
				Gossip * gossip = ret->retorno;
				t_list * lista_memorias_retorno_gossip =  gossip->lista_memorias;
				if(lista_memorias_retorno_gossip->elements_count == 0) {
					break;
				}
				int aux = 0;
				while(aux < lista_memorias_retorno_gossip->elements_count){
					Memoria *mem = list_get(lista_memorias_retorno_gossip, aux);
					agregarSiNoExiste(list_get(memorias, DISP), mem);
					aux++;
				}
				aux = 0;
				t_list * lista_disponibles = list_create();
				t_list * lista_a_borrar = list_create();
				while(aux < ((t_list*)list_get(memorias, DISP))->elements_count){
					Memoria *mem = list_get(list_get(memorias, DISP), aux);
					if(existe_memoria_en(mem, lista_memorias_retorno_gossip) != -1){
						list_add(lista_disponibles, mem);
					}else{
						list_add(lista_a_borrar, mem);
					}
					aux++;
				}
				aux = 0;
				while(aux < lista_a_borrar->elements_count){
					Memoria *m = list_get(lista_a_borrar, aux);
					sacarMemoriaDeTodasLasListas(m);
				}
				pthread_mutex_lock(&mutexRecursosCompartidos);
				list_destroy(list_get(memorias, DISP));
				list_add_in_index(memorias, DISP, lista_disponibles);
				pthread_mutex_unlock(&mutexRecursosCompartidos);
			}
		}
	}
}

int existe_memoria_en(Memoria *mem1, t_list* lista){
	int aux = 0;
	while(aux < lista->elements_count){
		Memoria * mem2 = list_get(lista, aux);
		if(strcmp(mem1->ip, mem2->ip) == 0 &&
		   strcmp(mem1->puerto, mem2->puerto) == 0){
			return aux;
		}
		aux ++;
	}
	return -1;
}

void mostrarId(Memoria * memoria){
	printf("%d ", memoria->idMemoria);
}

void agregarSiNoExiste(t_list * list, Memoria *m){
	if(existe_memoria_en(m, list) < 0){
		Memoria * mem = malloc(sizeof(Memoria));
		mem->idMemoria = m->idMemoria;
		mem->ip = malloc(strlen(m->ip) +1);
		mem->puerto = malloc(strlen(m->puerto) + 1);
		strcpy(mem->ip, m->ip);
		strcpy(mem->puerto, m->puerto);
		pthread_mutex_lock(&mutexRecursosCompartidos);
		list_add(list, mem);
		pthread_mutex_unlock(&mutexRecursosCompartidos);
	}
}
