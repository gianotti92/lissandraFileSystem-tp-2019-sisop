#include "kernel.h"

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
			default:
				agregarSiNoExiste(lista_disp, memoria);
				break;
		}
		pthread_mutex_unlock(&mutex);
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
				pthread_mutex_t mutexito;
				pthread_mutex_init(&mutexito, NULL);
				t_list *nuevas_disp = list_duplicate_all(gossip->lista_memorias, (void*)duplicar_memoria, mutexito);
				pthread_mutex_destroy(&mutexito);
				lista_disp = nuevas_disp;
				pthread_mutex_unlock(&mutex_disp);
				list_destroy_and_destroy_elements(lista_memorias_disponibles_vieja, (void*)eliminar_memoria);
			}
		}
		free_retorno(resp);
	}
}

Memoria *get_memoria(int idMemoria, Consistencias consistencia){
	pthread_mutex_t mutex = dame_mutex_de_consistencia(consistencia);
	pthread_mutex_lock(&mutex);
	t_list * lista = dame_lista_de_consistencia(consistencia);
	pthread_mutex_unlock(&mutex);
	int aux = 0;
	Memoria *mem;
	while((mem = list_get(lista, aux)) != NULL){
		if(mem->idMemoria == idMemoria){
			Memoria* retorno = duplicar_memoria(mem);
			list_destroy_and_destroy_elements(lista, (void*)eliminar_memoria);
			return retorno;
		}
		aux++;
	}
	pthread_mutex_unlock(&mutex);
	list_destroy_and_destroy_elements(lista, (void*)eliminar_memoria);
	return mem;
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
		if(!existe_memoria_en(m, list)){
			Memoria * mem = duplicar_memoria(m);
			list_add(list, mem);
       }
}

t_list *dame_lista_de_consistencia(Consistencias consistencia){
	t_list *lista;
	switch(consistencia){
		case SC:
			lista = list_duplicate_all(lista_sc, (void*)duplicar_memoria, mutex_sc);
			break;
		case EC:
			lista = list_duplicate_all(lista_ec, (void*)duplicar_memoria, mutex_ec);
			break;
		case SHC:
			lista = list_duplicate_all(lista_shc, (void*)duplicar_memoria, mutex_shc);
			break;
		default:
			lista = list_duplicate_all(lista_disp, (void*)duplicar_memoria, mutex_disp);
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

char* leer_linea(char* path, int linea) {
	char *line_buf = NULL;
	size_t line_buf_size = 0;
	ssize_t line_size;
	FILE * fp = fopen(path, "r");

	if(!fp){
		perror("Error al leer archivo");
		return NULL;
	}
	int i;
	for(i = 0; i <= linea; i++){
		line_size = getline(&line_buf, &line_buf_size, fp);
	}

	if(line_size >= 0){
		line_buf[line_size - 1] = '\0';
	}else{
		line_buf = NULL;
	}
	fclose(fp);
	return line_buf;
}
