#include "kernel.h"

// Hay que definirla ya que no tiene definición en kernel y sino rompería, de todos modos no se usa
void retornarControl(Instruccion *instruccion, int socket_cliente) {
}
;
void *TH_describe(void *p);

int main(void) {
	print_guenguencha();
	pthread_mutex_init(&mutexRecursosCompartidos, NULL);
	pthread_mutex_init(&lista_de_tablas_mx, NULL);
	sem_init(&semaforoSePuedePlanificar, 0, 0);
	sem_init(&semaforoNewToReady, 0, 0);

	configure_logger();
	configuracion_inicial();
	iniciarEstados();
	iniciarEstructurasAsociadas();
	
	fd_disponibles = dictionary_create();
	
	pthread_t consolaKernel, memoriasDisponibles, pasarNewToReady,
			calcularMetrics, T_confMonitor, T_describe;

	pthread_create(&memoriasDisponibles, NULL, (void*) lanzar_gossiping, NULL);
	pthread_detach(memoriasDisponibles);

	pthread_create(&T_describe, NULL, TH_describe, NULL);
	pthread_detach(T_describe);

	pthread_create(&T_confMonitor, NULL, TH_confMonitor, NULL);
	pthread_detach(T_confMonitor);

	pthread_create(&consolaKernel, NULL, (void*) leer_por_consola,
			retorno_consola);

	pthread_create(&pasarNewToReady, NULL, (void*) newToReady, NULL);
	pthread_detach(pasarNewToReady);

	pthread_create(&calcularMetrics, NULL, (void*) calculoMetrics, NULL);
	pthread_detach(calcularMetrics);

	int cantMultiprocesamiento = 0;
	while (cantMultiprocesamiento < HILOS_KERNEL) {
		pthread_t multiProcesamientoKernell;
		pthread_create(&multiProcesamientoKernell, NULL, (void*) ejecutar,
		NULL);
		pthread_detach(multiProcesamientoKernell);
		cantMultiprocesamiento++;
	}
	pthread_join(consolaKernel, NULL);
}

void retorno_consola(char* leido) {
	if(strcmp(leido, "memorias") == 0){
		pthread_mutex_lock(&mutex_disp);
		list_iterate(lista_disp, (void*)mostrar_memoria);
		pthread_mutex_unlock(&mutex_disp);
		free(leido);
		return;
	}
	Proceso * proceso = malloc(sizeof(Proceso));
	Instruccion * instruccion = parser_lql(leido, KERNEL);
	if (instruccion->instruccion != ERROR) {
		proceso->instruccion = instruccion;
		proceso->instruccionAProcesar = NULL;
		proceso->numeroInstruccion = 0;
		proceso->quantumProcesado = 0;
		encolar(estadoNew, proceso);
		sem_post(&semaforoNewToReady);
	}
}

void newToReady() {
	while (true) {
		sem_wait(&semaforoNewToReady);
		Proceso * proceso = desencolar(estadoNew);
		encolar(estadoReady, proceso);
		sem_post(&semaforoSePuedePlanificar);
	}
}

void ejecutar() {
	while (1) {
		sem_wait(&semaforoSePuedePlanificar);
		Proceso * proceso = desencolar(estadoReady);
		time_t fin;
		int diff;
		switch (((Instruccion*) proceso->instruccion)->instruccion) {
		case RUN:
			;
			proceso = logicaRun(proceso);
			proceso->esProcesoRun = true;
			proceso->quantumProcesado = 0;
			break;

		case METRICS:
			;
			logicaMetrics(proceso->instruccion);
			proceso->esProcesoRun = false;
			break;

		case SELECT:
			;
			logicaSelect(proceso->instruccion);

			fin = get_timestamp();
			diff = difftime(fin, ((Select*) proceso->instruccion)->timestamp);
			proceso->segundosQueTardo = diff;
			proceso->esProcesoRun = false;
			break;

		case INSERT:
			;
			logicaInsert(proceso->instruccion);

			fin = get_timestamp();
			diff = difftime(fin, ((Insert*) proceso->instruccion)->timestamp);
			proceso->segundosQueTardo = diff;
			proceso->esProcesoRun = false;
			break;

		case CREATE:
			;
			logicaCreate(proceso->instruccion);
			proceso->esProcesoRun = false;
			break;

		case DROP:
			;
			logicaDrop(proceso->instruccion);
			proceso->esProcesoRun = false;
			break;

		case ADD:
			;
			logicaAdd(proceso->instruccion);
			proceso->esProcesoRun = false;
			break;

		case DESCRIBE:
			logicaDescribe(proceso->instruccion);
			proceso->esProcesoRun = false;
			break;

		case JOURNAL:
			;
			logicaJournal(proceso->instruccion);
			proceso->esProcesoRun = false;
			break;

		default:
			;
			break;

			//free(proceso->instruccionAProcesar); // REVISAR: Porque sino nunca la liberamos
			if (!proceso->esProcesoRun
					|| proceso->instruccionAProcesar->instruccion == ERROR) {
				encolar(estadoExit, proceso);
			} else {
				sem_post(&semaforoSePuedePlanificar);
			}
		}
	}
}

/*
 Manejo Monitoreo
 */
void *TH_confMonitor(void * p) {

	int confMonitor_cb(void) {
		t_config* conf = config_create("config.cfg");
		if (conf == NULL) {
			log_error(LOG_ERROR,
					"Archivo de configuracion: config.cfg no encontrado");
			return 1;
		}
		actualizar_configuracion(conf);
		config_destroy(conf);
		return 0;
	}

	int retMon = monitorNode("config.cfg", IN_MODIFY, &confMonitor_cb);
	if (retMon != 0) {
		return (void*) 1;
	}
	return (void*) 0;
}

/*
 Manejo Describe
 */
void *TH_describe(void *p) {
	lista_de_tablas = list_create();
	while (true) {
		realizarDescribeGeneral();
		usleep(TIEMPO_DESCRIBE * 1000);
	}
	list_destroy(lista_de_tablas);
	pthread_mutex_destroy(&lista_de_tablas_mx);
}
