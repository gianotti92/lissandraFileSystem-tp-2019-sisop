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

char* getTablasSafe(t_dictionary * dic, char*key){
	pthread_mutex_lock(&mutexRecursosCompartidos);
	char * nombre = (char*)dictionary_get(dic, key);
	pthread_mutex_unlock(&mutexRecursosCompartidos);
	return nombre;
}

void asignar_memoria_a_consistencia(Memoria * memoria, Consistencias consistencia){
	if(memoria != NULL){
		pthread_mutex_t mutex = dame_mutex_de_consistencia(consistencia);
		pthread_mutex_lock(&mutex);
		if(existe_memoria_en(memoria, lista_disp) < 0 ){
			switch(consistencia){
				case SC:
					agregarSiNoExiste(lista_sc, memoria);
					break;
				case EC:
					agregarSiNoExiste(lista_ec, memoria);
					break;
				case SHC:
					agregarSiNoExiste(lista_shc, memoria);
					break;
				case DISP:
					agregarSiNoExiste(lista_disp, memoria);
			}
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
	asignar_memoria_a_consistencia(memoriaPrincipal, DISP);
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
				pthread_mutex_lock(&mutex_disp);
				t_list *lista_memorias_disponibles_vieja = lista_disp;
				lista_disp = gossip->lista_memorias;
				pthread_mutex_unlock(&mutex_disp);
				list_destroy_and_destroy_elements(lista_memorias_disponibles_vieja, (void*)eliminar_memoria);
				//FIXME: Hay que purgar las listas
				//purgar_listas();
			}
		}
	}
}

bool existe_memoria_en(Memoria *mem1, t_list* lista){
		int aux = 0;
		Memoria * mem2;
		while((mem2 = list_get(lista, aux)) != NULL){
	       	if(strcmp(mem1->ip, mem2->ip) == 0 && strcmp(mem1->puerto, mem2->puerto) == 0){
               	return true;
	       	}
	       	aux ++;
		}
		return false;
}

void mostrarId(Memoria * memoria){
	printf("%d ", memoria->idMemoria);
}

void agregarSiNoExiste(t_list * list, Memoria *m){
		if(existe_memoria_en(m, list)){
			Memoria * mem = duplicar_memoria(m);
			list_add(list, mem);
       }
}

t_list *dame_lista_de_consistencia(Consistencias consistencia){
	t_list *lista;
	switch(consistencia){
		case SC:
			lista = lista_sc;
			break;
		case EC:
			lista = lista_ec;
			break;
		case SHC:
			lista = lista_shc;
			break;
		case DISP:
			lista = lista_disp;
			break;
	}
	return lista;
}

pthread_mutex_t dame_mutex_de_consistencia(Consistencias consistencia){
	pthread_mutex_t mutex;
	switch(consistencia){
		case SC:
			mutex = mutex_sc;
			break;
		case EC:
			mutex = mutex_ec;
			break;
		case SHC:
			mutex = mutex_shc;
			break;
		default:
			mutex = mutex_disp;
			break;
	}
	return mutex;
}
