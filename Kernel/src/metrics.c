#include "kernel.h"

void calculoMetrics(){
	int contadorInsert = 0;
	int contadorSelect = 0;
	int contadorSelectInsert = 0;
	int tiempoTotalSelect = 0;
	int tiempoTotalInsert = 0;
	t_list* contador_por_memoria = list_create();
	AcumuladorMemoria * acumuladorMemoria = NULL;

	while(1){

		pthread_mutex_lock(&mutex_metrics);

		int elemento = (acum30sMetrics->elements_count -1);

		while (elemento >= 0){
			AcumMetrics* acumMetrics = list_remove(acum30sMetrics, elemento);

			acumuladorMemoria = (AcumuladorMemoria*) dameAcumulador(acumMetrics->id_memoria, contador_por_memoria);

			if(acumuladorMemoria == NULL){
				acumuladorMemoria = malloc(sizeof(AcumuladorMemoria));
				acumuladorMemoria->id_memoria = acumMetrics->id_memoria;
				acumuladorMemoria->cantidad_instrucciones = 0;
				acumuladorMemoria->cantidad_ins_sel = 0;

				list_add(contador_por_memoria, acumuladorMemoria);
			}


			switch(acumMetrics->instruccion){
			case SELECT:;
				contadorSelect ++;
				tiempoTotalSelect += acumMetrics->tiempo;
				contadorSelectInsert++;
				acumuladorMemoria->cantidad_ins_sel++;
				acumuladorMemoria->cantidad_instrucciones++;
				break;
			case INSERT:;
				contadorInsert++;
				tiempoTotalInsert += acumMetrics->tiempo;
				contadorSelectInsert++;
				acumuladorMemoria->cantidad_ins_sel++;
				acumuladorMemoria->cantidad_instrucciones++;
				break;
			default:;
				acumuladorMemoria->cantidad_instrucciones++;
				break;
			}

		free(acumMetrics);
		elemento--;

		}

		READS = contadorSelect;
		if(contadorSelect>0){
			READ_LAT = tiempoTotalSelect/contadorSelect;
		} else {
			READ_LAT = 0;
		}

		WRITES = contadorInsert;
		if(contadorInsert>0){
			WRITE_LAT = tiempoTotalInsert/contadorInsert;
		} else {
			WRITE_LAT = 0;
		}

		if(MEM_LOAD->elements_count>0){
			list_clean_and_destroy_elements(MEM_LOAD, (void*)free);
		}
		/*//PARA ORDENAR LA LISTA Y QUE QUEDE MAS LINDO, NO LE GUSTA EL BOOL*

		bool* orderMemory (AcumuladorMemoria* mem1, AcumuladorMemoria* mem2){
			bool resultado = (mem1->id_memoria < mem2->id_memoria);//la ordeno al de mayor a menor para imprimirla de abajo hacia arriba.
			return  &resultado;
		}

		list_sort(contador_por_memoria, (bool*)orderMemory);
		*/

		MEM_LOAD = contador_por_memoria;

		pthread_mutex_unlock(&mutex_metrics);

		loguear_metrics();

		sleep(SEGUNDOS_METRICS);
	}
}

void loguear_metrics(){

	log_info(LOGGER_METRICS, "Read Latency: %d", READ_LAT);
	log_info(LOGGER_METRICS, "Write Latency: %d", WRITE_LAT);
	log_info(LOGGER_METRICS, "Reads: %d", READS);
	log_info(LOGGER_METRICS, "Writes: %d", WRITES);

	int elemento = (MEM_LOAD->elements_count -1);

	AcumuladorMemoria* acum;
	if (elemento < 0){
		log_info(LOGGER_METRICS, "Memory: - Memory Load: No hay valores acumulados.");
	}

	while (elemento >= 0) {
		acum = list_get(MEM_LOAD, elemento);
		log_info(LOGGER_METRICS, "Memory: %d Memory Load: %d / %d", acum->id_memoria, acum->cantidad_ins_sel, acum->cantidad_instrucciones);
		elemento--;
	}
}

void print_metrics(){

	log_info(LOG_OUTPUT, "Read Latency: %d", READ_LAT);
	log_info(LOG_OUTPUT, "Write Latency: %d", WRITE_LAT);
	log_info(LOG_OUTPUT, "Reads: %d", READS);
	log_info(LOG_OUTPUT, "Writes: %d", WRITES);

	int elemento = (MEM_LOAD->elements_count -1);
	AcumuladorMemoria* acum;

	if (elemento < 0){
		log_info(LOG_OUTPUT, "Memory: - Memory Load: No hay valores acumulados.");
	}

	while (elemento >= 0) {
		acum = list_get(MEM_LOAD, elemento);
		log_info(LOG_OUTPUT, "Memory: %d Memory Load: %d / %d", acum->id_memoria, acum->cantidad_ins_sel, acum->cantidad_instrucciones);
		elemento--;
	}
}

AcumuladorMemoria* dameAcumulador(int id_memoria, t_list* lista_acumuladores){
	int elemento = lista_acumuladores->elements_count -1;
	AcumuladorMemoria* acum;

	while(elemento >=0 ){
		acum = list_get(lista_acumuladores, elemento);
		if (acum->id_memoria == id_memoria){
			return acum;
		}
		elemento--;
	}
	return NULL;
}
