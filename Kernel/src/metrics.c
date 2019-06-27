#include "kernel.h"

void calculoMetrics(){
	int contadorInsert = 0;
	int contadorSelect = 0;
	int contadorSelectInsert = 0;
	int operacionesTotales = 0;
	int tiempoPromedioSelect = 0;
	int tiempoPromedioInsert = 0;
	while(1){
		Proceso * proceso = desencolar(estadoExit);
		while(proceso != NULL){
			operacionesTotales++;
			switch(proceso->instruccion->instruccion){
				case INSERT:;
					tiempoPromedioInsert += proceso->segundosQueTardo;
					contadorInsert++;
					contadorSelectInsert++;
					free(proceso);
					break;
				case SELECT:;
					tiempoPromedioSelect += proceso->segundosQueTardo;
					contadorSelect++;
					contadorSelectInsert++;
					free(proceso);
					break;

				default:
					free(proceso);
					break;
			}
			proceso = desencolar(estadoExit);
		}
		if(contadorInsert == 0){
			contadorInsert = 1;
		}

		if(contadorSelect == 0){
			contadorSelect = 1;
		}
		tiempoPromedioInsert = tiempoPromedioInsert / contadorInsert;
		tiempoPromedioSelect = tiempoPromedioSelect / contadorSelect;

		graficar(contadorInsert, contadorSelect, contadorSelectInsert, operacionesTotales,
				tiempoPromedioInsert, tiempoPromedioSelect);
		contadorInsert = 0;
		contadorSelect = 0;
		contadorSelectInsert = 0;

		sleep(SEGUNDOS_METRICS);
	}
}

void graficar(int contadorInsert, int contadorSelect, int contadorSelectInsert,
		int operacionesTotales, int tiempoPromedioInsert, int tiempoPromedioSelect){
	char * reads = string_new();
	char * writes = string_new();
	char * memLoad = string_new();
	char * readLatency = string_new();
	char * writeLatency = string_new();
	//FIXME: fijarse porque al principio me devuelve un cantidad read 0.003333 ya que deberia estar vacia lista exit
	string_append(&reads, "Cantidad de reads: ");
	string_append(&writes, "Cantidad de writres: ");
	string_append(&memLoad, "Memory load: ");
	string_append(&readLatency, "Read latency: ");
	string_append(&writeLatency, "Write Latency: ");

	double r = (double)contadorSelect / 30;
	double w = (double)contadorInsert / 30;
	double ml = (double)contadorSelectInsert / operacionesTotales;
	double rl = (double)tiempoPromedioSelect / contadorSelect;
	double wl = (double)tiempoPromedioInsert / contadorSelectInsert;

	char * rChar = malloc(sizeof(char) * 21);
	char * wChar = malloc(sizeof(char) * 21);
	char * mlChar = malloc(sizeof(char) * 21);
	char * rlChar = malloc(sizeof(char) * 21);
	char * wlChar = malloc(sizeof(char) * 21);

	sprintf(rChar, "%lf", r);
	sprintf(wChar, "%lf", w);
	sprintf(mlChar, "%lf", ml);
	sprintf(rlChar, "%lf",rl);
	sprintf(wlChar, "%lf", wl);

	string_append(&reads, rChar);
	string_append(&writes, wChar);
	string_append(&memLoad, mlChar);
	string_append(&readLatency, rlChar);
	string_append(&writeLatency,wlChar);

	log_info(LOGGER_METRICS, reads);
	log_info(LOGGER_METRICS, writes);
	log_info(LOGGER_METRICS, memLoad);
	log_info(LOGGER_METRICS, writeLatency);
	log_info(LOGGER_METRICS, readLatency);

	pthread_mutex_lock(&mutexRecursosCompartidos);
	dictionary_put(metrics, READS, reads);
	dictionary_put(metrics, WRITES, writes);
	dictionary_put(metrics, MEM_LOAD, memLoad);
	dictionary_put(metrics, WRITE_LAT, writeLatency);
	dictionary_put(metrics, READ_LAT, readLatency);
	pthread_mutex_unlock(&mutexRecursosCompartidos);

	free(rChar);
	free(wChar);
	free(mlChar);
	free(rlChar);
	free(wlChar);
}
