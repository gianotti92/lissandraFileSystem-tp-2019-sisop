#include "kernel.h"

void calculoMetrics(){
	int contadorInsert = 0;
	int contadorSelect = 0;
	int contadorSelectInsert = 0;
	int operacionesTotales = 0;
	int tiempoPromedioSelect = 0;
	int tiempoPromedioInsert = 0;
	while(true){
		pthread_mutex_lock(&mutex_metrics);

		int elemento = (acum30sMetrics->elements_count -1);
		while (elemento >= 0){
			AcumMetrics acumMetrics = list_remove(acum30sMetrics, elemento);
			switch(acumMetrics->instruccion){
			case SELECT:;
				contadorSelect ++;
				tiempoPromedioSelect += acumMetrics->tiempo;
				contadorSelectInsert++;
				break;
			case INSERT:;
				contadorInsert++;
				tiempoPromedioInsert += acumMetrics->tiempo;
				contadorSelectInsert++;
				break;
			default:;
				break;
			}


			//acum por memoria


		}

		READS = contadorSelect;
		if(contadorSelect>0){
			READ_LAT = tiempoPromedioSelect/contadorSelect;
		} else {
			READ_LAT = 0;
		}

		WRITES = contadorInsert;
		if(contadorInsert>0){
			WRITE_LAT = tiempoPromedioInsert/contadorInsert;
		} else {
			WRITE_LAT = 0;
		}

		pthread_mutex_unlock(&mutex_metrics);

		graficar(contadorInsert, contadorSelect, contadorSelectInsert, operacionesTotales,
				tiempoPromedioInsert, tiempoPromedioSelect);

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

	string_append(&readLatency, "Read latency: ");
	string_append(&writeLatency, "Write Latency: ");
	string_append(&reads, "Cantidad de reads: ");
	string_append(&writes, "Cantidad de writres: ");
	string_append(&memLoad, "Memory load: ");

	double r = (double)contadorSelect / 30;
	double w = (double)contadorInsert / 30;

	double ml;

	if (operacionesTotales>0) {
		ml = (double)contadorSelectInsert / operacionesTotales;
	} else {
		ml = (double) 0;
	}

	double rl;
	if (contadorSelectInsert>0) {
		rl = (double)tiempoPromedioSelect / contadorSelectInsert;
	} else {
		rl = (double) 0;
	}

	double wl;
	if (contadorSelectInsert>0) {
		wl = (double)tiempoPromedioInsert / contadorSelectInsert;
	} else {
		wl = (double) 0;
	}

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

	log_info(LOGGER_METRICS, readLatency);
	log_info(LOGGER_METRICS, writeLatency);
	log_info(LOGGER_METRICS, reads);
	log_info(LOGGER_METRICS, writes);
	log_info(LOGGER_METRICS, memLoad);

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

	free(reads);
	free(writes);
	free(memLoad);
	free(writeLatency);
	free(readLatency);
}
