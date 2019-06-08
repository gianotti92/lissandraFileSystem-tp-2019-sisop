#include "kernel.h"

void calculoMetrics(bool * direct){
	int contadorInsert = 0;
	int contadorSelect = 0;
	int contadorSelectInsert = 0;
	int operacionesTotales = 0;
	int tiempoPromedioSelect = 0;
	int tiempoPromedioInsert = 0;
	while(1){
		if(!(*direct)){
			sleep(30);
		}
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
		if(*direct){
			return;
		}

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

	char * rChar = string_new();
	char * wChar = string_new();
	char * mlChar = string_new();
	char * rlChar = string_new();
	char * wlChar = string_new();

	sprintf(rChar, "%f", r);
	sprintf(wChar, "%f", w);
	sprintf(mlChar, "%f", ml);
	sprintf(rlChar, "%f",rl);
	sprintf(wlChar, "%f", wl);

	string_append(&reads, rChar);
	string_append(&writes, wChar);
	string_append(&memLoad, mlChar);
	string_append(&readLatency, rlChar);
	string_append(&writeLatency,wlChar);

	log_info(LOGGER, reads);
	log_info(LOGGER, writes);
	log_info(LOGGER, memLoad);
	log_info(LOGGER, writeLatency);
	log_info(LOGGER, readLatency);

	free(reads);
	free(writes);
	free(rChar);
	free(wChar);
	free(memLoad);
	free(mlChar);
	free(writeLatency);
	free(readLatency);
	free(rlChar);
	free(wlChar);
}