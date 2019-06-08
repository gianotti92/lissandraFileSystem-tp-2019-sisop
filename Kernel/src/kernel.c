#include "kernel.h"

// Hay que definirla ya que no tiene definición en kernel y sino rompería, de todos modos no se usa
void retornarControl(Instruccion *instruccion, int socket_cliente){};

int main(void) {

	pthread_mutex_init(&mutexRecursosCompartidos, NULL);
	sem_init(&semaforoSePuedePlanificar,0,0);
	sem_init(&semaforoNewToReady, 0, 0);

	configure_logger();
	configuracion_inicial();
	iniciarEstados();
	iniciarEstructurasAsociadas();

	pthread_t consolaKernel, memoriasDisponibles, pasarNewToReady;
	//, calcularMetrics;

	pthread_create(&memoriasDisponibles, NULL, (void*) preguntarPorMemoriasDisponibles, NULL);
	pthread_detach(memoriasDisponibles);

	pthread_create(&consolaKernel, NULL, (void*) leer_por_consola,
			retorno_consola);

	pthread_create(&pasarNewToReady, NULL, (void*) newToReady, NULL);
	pthread_detach(pasarNewToReady);
/*
	bool dormir = false;

	pthread_create(&calcularMetrics, NULL, (void*) calculoMetrics, (void*)&dormir);
	pthread_detach(calcularMetrics);
*/

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
		proceso->file_descriptor = -1;
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
				bool dormir = true;
				calculoMetrics((void*)&dormir);
				proceso->esProcesoRun=false;
				break;
			}
			case SELECT:{
				Select * select = (Select *) proceso->instruccion->instruccion_a_realizar;
				proceso->file_descriptor = logicaSelect(select);
				fin = get_timestamp();
				diff = difftime(fin ,select->timestamp);
				proceso->segundosQueTardo = diff;
				proceso->esProcesoRun=false;
				break;
			}
			case INSERT:{
				Insert * insert = (Insert *) proceso->instruccion->instruccion_a_realizar;
				proceso->file_descriptor = logicaInsert(insert);
				fin = get_timestamp();
				diff = difftime(fin, insert->timestamp);
				proceso->segundosQueTardo = diff;
				proceso->esProcesoRun=false;
				break;
			}
			case CREATE:{
				Create * create = (Create *) proceso->instruccion->instruccion_a_realizar;
				proceso->file_descriptor = logicaCreate(create);
				proceso->esProcesoRun=false;
				break;
			}
			case DROP:{
				Drop * drop = (Drop *) proceso->instruccion->instruccion_a_realizar;
				proceso->file_descriptor = logicaDrop(drop);
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
		if(!proceso->esProcesoRun || proceso->instruccionAProcesar->instruccion == ERROR){
			encolar(estadoExit, proceso);
		}else {
			sem_post(&semaforoSePuedePlanificar);
		}
	}
}


/* MOCK */
int enviarInstruccionLuqui(char* ip, char* puerto, Instruccion *instruccion,
		Procesos proceso_del_que_envio){
	if(ip == NULL){
		return -1;
	}
	if(puerto == NULL){
		return -2;
	}
	if(instruccion == NULL){
		return -3;
	}
	return 6;
}
/* MOCK */
