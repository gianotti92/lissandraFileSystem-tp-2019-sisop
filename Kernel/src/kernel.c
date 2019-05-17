#include "kernel.h"
#include <sys/types.h>

//implementar cola new
//mati me va a abhorrer un socket nuevo para que kernel pregunte por memorias, en memoria principal(armar un map con lo que me vaya a devolver esto) -> lista de memorias existentes
//memoria principal me va a devolver todas las memorias(las guardo en el map)
//cuando me viene add tal memoria tal criterio yo tengo que meter en el value del map de consistencias esa memoria
//cuando me viene un create tomo la consistencia, con esa consistencia busco en el map de m que me devolvio memoria principal,

/*
 *
 *
Discriminar
JOURNAL
ADD
RUN
METRICS
 *
 *
 *
 *
 * */

pthread_mutex_t mutex;
sem_t semaforoSePuedePlanificar;
sem_t semaforoInicial;
sem_t semaforoNewToReady;

int main(void) {
	pthread_mutex_init(&mutex, NULL);
	sem_init(&semaforoInicial, 0, 1);
	sem_init(&semaforoSePuedePlanificar,0,0);
	sem_init(&semaforoNewToReady, 0 ,0);

	configure_logger();
	configuracion_inicial();
	iniciarEstados();
	iniciarEstructurasAsociadas();

	pthread_t consolaKernel, preguntarPorMemorias;
	pthread_create(&consolaKernel, NULL, (void*) leer_por_consola,
			retorno_consola);

	int cantMultiprocesamiento = 0;
	while(cantMultiprocesamiento <=  HILOS_KERNEL){
		pthread_t multiProcesamientoKernell;
		pthread_create(&multiProcesamientoKernell, NULL, (void*) planificar,
						NULL);
	}
	pthread_create(&preguntarPorMemorias, NULL, (void*) preguntarPorMemorias, NULL);

	pthread_join(consolaKernel, NULL);
}

void configuracion_inicial(void) {
	t_config* CONFIG;
	CONFIG = config_create("config.cfg");
	if (!CONFIG) {
		log_error(LOGGER, "No encuentro el archivo config");
		exit_gracefully(EXIT_FAILURE);
	}

	PUERTO_DE_ESCUCHA = config_get_string_value(CONFIG,"PUERTO_DE_ESCUCHA");
	IP_MEMORIA_PPAL = config_get_string_value(CONFIG,"IP_MEMORIA_PPAL");
	PUERTO_MEMORIA_PPAL = config_get_string_value(CONFIG,"PUERTO_MEMORIA_PPAL");

	QUANTUM = config_get_int_value(CONFIG, "QUANTUM");
	TAMANO_MAXIMO_LECTURA_ARCHIVO = config_get_int_value(CONFIG,
			"TAMANO_MAXIMO_LECTURA_ARCHIVO");
	HILOS_KERNEL = config_get_int_value(CONFIG,
			"TAMANO_MAXIMO_LECTURA_ARCHIVO");
}

void retorno_consola(char* leido) {
	sem_wait(&semaforoInicial);

	log_info(LOGGER, "Kernel. Se retorno a consola");
	log_info(LOGGER, leido);

	Instruccion * instruccion = malloc(sizeof(Instruccion));
	Proceso * proceso = malloc(sizeof(Proceso));
	Memoria * memoriaAsociada = malloc(sizeof(Memoria));

	instruccion = parser_lql(leido, KERNEL);

	proceso->quantumProcesado = 0;
	proceso->instruccionActual = instruccion;

	memoriaAsociada = seleccionarMemoriaPorConsistencia(SC);

	switch(instruccion->instruccion){
		case ADD:;
			Add * add = malloc(sizeof(Add));
			add = (Add*)instruccion->instruccion_a_realizar;
			memoriaAsociada = seleccionarMemoriaPorConsistencia(add->consistencia);
			break;
		case JOURNAL:
			break;
		case RUN:
			break;
		case METRICS:
			break;
		default:
			break;

	}

	if(strcmp(instruccion->, "ADD")){

	}else if(leidoSplit[0], "JOURNAL"){
		//TODO: logica de journal
	}else if(leidoSplit[0], "RUN"){
		Instruccion * instruccion = dameSiguiente(leidoSplit[1], proceso->quantumProcesado);
		proceso->instruccionActual = instruccion;

	}else if(leidoSplit[0], "METRICS"){
		//TODO: logica de metrics
	}else{
		//TODO: logica de mandar instruccion
	}




}

void iniciarEstados() {
	log_info(LOGGER, "Kernel:Se inician estados");
	estadoReady = list_create();
	estadoNew = list_create();
	estadoExit = list_create();
	estadoExec = list_create();
	list_clean(estadoReady);
	list_clean(estadoNew);
	list_clean(estadoExit);
	list_clean(estadoExec);
}

void iniciarEstructurasAsociadas(){
	memoriasAsociadas = dictionary_create();
	tablasPorConsistencia = dictionary_create();
}

Proceso * crear_proceso() {
	Proceso * proceso = malloc(sizeof(Proceso));
	proceso->instrucciones = list_create();

	return proceso;
}

Proceso * asignar_instrucciones(char ** leidoSplit) {

	time_t echo_time;
	echo_time = time(NULL);

	if (echo_time == ((time_t)-1)){
		puts ("ERROR: Fallo al obtener la hora.");
		log_error(LOGGER, "Kernel: Fallo al obtener la hora.");
	exit_gracefully(0);
	}
	Proceso * proceso = crear_proceso();
	if (es_run(leidoSplit)) {
		FILE * f = fopen(leidoSplit[1], "r");
		char buffer[TAMANO_MAXIMO_LECTURA_ARCHIVO];
		if (f == NULL) {
			perror("No se puede abrir archivo!");
			log_error(LOGGER, "Kernel. error al leer el archivo");
			exit_gracefully(1);
		}
		while (fgets(buffer, sizeof(buffer), f) != NULL) {
			char * line = string_new();
			line = strtok(buffer, "\n");
			line = string_from_format("%s %ju", line, (uintmax_t)echo_time);
			pthread_mutex_lock(&mutex);
			list_add(proceso->instrucciones, &line);
			pthread_mutex_unlock(&mutex);
		}
		fclose(f);

	} else {
		pthread_mutex_lock(&mutex);
		list_add(proceso->instrucciones, &leidoSplit);
		pthread_mutex_unlock(&mutex);
	}

	int cantidad = cantidad_elementos(leidoSplit);
	int i = 0;
	while (i < cantidad) {
		free(leidoSplit[i]);
		i++;
	}
	return proceso;
}

void planificar() {
	while(1){
		sem_wait(&semaforoSePuedePlanificar);
		pasarPrimerProceso(estadoReady, estadoExec);
		Proceso * p;
		pthread_mutex_lock(&mutex);
		p = list_get(estadoExec, 0);
		pthread_mutex_unlock(&mutex);

		p->quantumProcesado = 0;
		while (p->quantumProcesado <= QUANTUM) {
			char * instructionString = list_get(p->instrucciones, 0);
			Instruccion * i = parser_lql(instructionString, KERNEL);
			int fileDescriptor = enviarX(i, "127.0.0.1", 60000);

			p->file_descriptor = fileDescriptor;
			p->quantumProcesado +=1;

			if(p->quantumProcesado == QUANTUM && list_size(p->instrucciones) != 0){
				pthread_mutex_lock(&mutex);
				cambiarEstado(p, estadoReady);
				pthread_mutex_unlock(&mutex);
			}
			if(list_size(p->instrucciones) == 0){
				pthread_mutex_lock(&mutex);
				cambiarEstado(p, estadoExit);
				pthread_mutex_unlock(&mutex);
			}

			free(i);
			free(instructionString);


		}
		sem_post(&semaforoInicial);
		free(p);
	}
}

void liberarProceso(Proceso * proceso) {
	free(proceso);
}

void encolar(t_list * cola, Proceso * proceso) {
	pthread_mutex_lock(&mutex);
	list_add(cola, proceso);
	pthread_mutex_unlock(&mutex);
}

void encolarNew(t_list * cola, Memoria * memoriaAsociada){
	pthread_mutex_lock(&mutex);
	list_add(cola, memoriaAsociada);
	pthread_mutex_unlock(&mutex);
	free(memoriaAsociada);
}

void pasarPrimerProceso(t_list *from, t_list *to) {
	if (list_size(from) > 0) {
		Proceso * p = (Proceso *) list_get(from, 0);
		list_add(to, p);
	}
}

void cambiarEstado(Proceso* p, t_list * estado){
	list_add(estado, p);
}

void ponerProcesosEneady(){
	while(1){
		sem_wait(&semaforoNewToReady);
		char ** charSplit = list_get(estadoNew, 0);
		Proceso * proceso = asignar_instrucciones(charSplit);

		encolar(estadoReady, proceso);

		free(charSplit);
		liberarProceso(proceso);
		sem_post(&semaforoSePuedePlanificar);
	}

}

void preguntarPorMemorias(){
	while(1){
		//preguntar como hicieron funcion
		/*mock*/
		t_list * memorias = list_create();
		Memoria * mem_ejemplo_1 = malloc(sizeof(Memoria));
		Memoria * mem_ejemplo_2 = malloc(sizeof(Memoria));
		Memoria * mem_ejemplo_3 = malloc(sizeof(Memoria));
		list_add(memorias, mem_ejemplo_1);
		list_add(memorias, mem_ejemplo_2);
		list_add(memorias, mem_ejemplo_3);
		//Aca no se si va la direccion de la lista o la lista (memorias)
		dictionary_put(memoriasAsociadas, CONSISTENCIAS_STRING[SC], memorias);
		/*mock*/
	}
}

Memoria* seleccionarMemoriaPorConsistencia(Consistencias consistencia){
	t_list * memorias = dictionary_get(memoriasAsociadas,CONSISTENCIAS_STRING[consistencia]);
}

void llenarTablasPorConsistencia(char * key, char * value){
	dictionary_put(tablasPorConsistencia, key, value);
	free(key);
	free(value);
}

Proceso * crearProceso(char ** leidoSplit){
	Proceso * proceso = malloc(sizeof(Proceso));
	proceso->instruccionActual
	return proceso;

}

// mock
int enviarX(Instruccion * i, char*ip, int puerto){
	return 4;
}
