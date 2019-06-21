#include "kernel.h"

// Hay que definirla ya que no tiene definición en kernel y sino rompería, de todos modos no se usa
void retornarControl(Instruccion *instruccion, int socket_cliente){};
void *TH_describe(void *p);

int main(void) {
	print_guenguencha();
	pthread_mutex_init(&mutexRecursosCompartidos, NULL);
	pthread_mutex_init(&lista_de_tablas_mx,NULL);
	sem_init(&semaforoSePuedePlanificar,0,0);
	sem_init(&semaforoNewToReady, 0, 0);

	configure_logger();
	configuracion_inicial();
	iniciarEstados();
	iniciarEstructurasAsociadas();

	pthread_t consolaKernel, memoriasDisponibles, pasarNewToReady, calcularMetrics, T_confMonitor, T_describe;

	pthread_create(&T_describe,NULL,TH_describe,NULL);
	pthread_detach(T_describe);

	pthread_create(&T_confMonitor,NULL,TH_confMonitor,NULL);
	pthread_detach(T_confMonitor);

	pthread_create(&memoriasDisponibles, NULL, (void*) lanzar_gossiping, NULL);
	pthread_detach(memoriasDisponibles);

	pthread_create(&consolaKernel, NULL, (void*) leer_por_consola,
			retorno_consola);

	pthread_create(&pasarNewToReady, NULL, (void*) newToReady, NULL);
	pthread_detach(pasarNewToReady);

	pthread_create(&calcularMetrics, NULL, (void*) calculoMetrics, NULL);
	pthread_detach(calcularMetrics);


	int cantMultiprocesamiento = 0;
	while(cantMultiprocesamiento <  HILOS_KERNEL){
		pthread_t multiProcesamientoKernell;
		pthread_create(&multiProcesamientoKernell, NULL, (void*) ejecutar,
						NULL);
		pthread_detach(multiProcesamientoKernell);
		cantMultiprocesamiento++;
	}
	pthread_join(consolaKernel, NULL);
}

void retorno_consola(char* leido) {

	Proceso * proceso = malloc(sizeof(Proceso));
	Instruccion * instruccion = parser_lql(leido, KERNEL);
	if(instruccion->instruccion != ERROR){
		proceso->instruccion = instruccion;
		proceso->numeroInstruccion = 0;
		proceso->quantumProcesado = 0;
		encolar(estadoNew, proceso);
		sem_post(&semaforoNewToReady);
	}
}

void newToReady(){
	while(true){
		sem_wait(&semaforoNewToReady);
		Proceso * proceso = desencolar(estadoNew);
		encolar(estadoReady, proceso);
		sem_post(&semaforoSePuedePlanificar);
	}
}

void ejecutar() {
	while(1){
		sem_wait(&semaforoSePuedePlanificar);
		Proceso * proceso = desencolar(estadoReady);
		time_t fin;
		int diff;
		switch(proceso->instruccion->instruccion){
			case RUN:{
				Run * run = (Run *) proceso->instruccion->instruccion_a_realizar;
				proceso = logicaRun(run, proceso);
				proceso->esProcesoRun=true;
				proceso->quantumProcesado = 0;
				break;
			}
			case METRICS:{

				pthread_mutex_lock(&mutexRecursosCompartidos);
				char * campo1 = dictionary_get(metrics, key1);
				char * campo2 = dictionary_get(metrics, key2);
				char * campo3 = dictionary_get(metrics, key3);
				char * campo4 = dictionary_get(metrics, key4);
				char * campo5 = dictionary_get(metrics, key5);
				pthread_mutex_unlock(&mutexRecursosCompartidos);

				printf("%s\n", campo1);
				printf("%s\n", campo2);
				printf("%s\n", campo3);
				printf("%s\n", campo4);
				printf("%s\n", campo5);

				proceso->esProcesoRun=false;
				break;
			}
			case SELECT:{
				Select * select = (Select *) proceso->instruccion->instruccion_a_realizar;
				logicaSelect(select);

				fin = get_timestamp();
				diff = difftime(fin ,select->timestamp);
				proceso->segundosQueTardo = diff;
				proceso->esProcesoRun=false;
				break;
			}
			case INSERT:{
				Insert * insert = (Insert *) proceso->instruccion->instruccion_a_realizar;
				logicaInsert(insert);

				fin = get_timestamp();
				diff = difftime(fin, insert->timestamp);
				proceso->segundosQueTardo = diff;
				proceso->esProcesoRun=false;
				break;
			}
			case CREATE:{
				Create * create = (Create *) proceso->instruccion->instruccion_a_realizar;
				logicaCreate(create);
				proceso->esProcesoRun=false;
				break;
			}
			case DROP:{
				Drop * drop = (Drop *) proceso->instruccion->instruccion_a_realizar;
				logicaDrop(drop);
				proceso->esProcesoRun=false;
				break;
			}
			case ADD:{
				Add * add = (Add *) proceso->instruccion->instruccion_a_realizar;
				logicaAdd(add);
				proceso->esProcesoRun=false;
				break;
			}
			case DESCRIBE:{
				Describe * describe = (Describe*) proceso->instruccion->instruccion_a_realizar;
				logicaDescribe(describe);
				proceso->esProcesoRun=false;
				break;
			}
			case JOURNAL:{
				Journal * journal = (Journal*) proceso->instruccion->instruccion_a_realizar;
				logicaJournal(journal);
				proceso->esProcesoRun=false;
				break;
			}
			default:
				break;
		}
		free(proceso->instruccionAProcesar); // REVISAR: Porque sino nunca la liberamos
		if(!proceso->esProcesoRun || proceso->instruccionAProcesar->instruccion == ERROR){
			encolar(estadoExit, proceso);
		}else {
			sem_post(&semaforoSePuedePlanificar);
		}
	}
}



/*
	Manejo Monitoreo
*/
void *TH_confMonitor(void * p){

	int confMonitor_cb(void){
		t_config* conf = config_create("config.cfg");
		if(conf == NULL) {
			log_error(LOG_ERROR,"Archivo de configuracion: config.cfg no encontrado");
			return 1;
		}
		actualizar_configuracion(conf);
		config_destroy(conf);
		return 0;
	}

	int retMon = monitorNode("config.cfg",IN_MODIFY,&confMonitor_cb);
	if(retMon!=0){
		return (void*)1;
	}
	return (void*)0;
}

/*
	Manejo Describe
*/
void *TH_describe(void *p){
	lista_de_tablas = list_create();
	while(true){
		realizarDescribeGeneral();
		usleep(TIEMPO_DESCRIBE*1000);
	}
	list_destroy(lista_de_tablas);
	pthread_mutex_destroy(&lista_de_tablas_mx);
}
