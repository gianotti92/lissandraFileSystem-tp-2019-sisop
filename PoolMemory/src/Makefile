#Declaro variables
CC := gcc

OBJS := poolMemory.o config_poolMemory.o

#Generador principal
all: poolMemory


poolMemory: $(OBJS)
	$(CC) -o poolMemory $(OBJS) -lcommons -lm 

poolMemory.o: poolMemory.c config_poolMemory.h
	$(CC) -c poolMemory.c
	
config_poolMemory.o: config_poolMemory.c config_poolMemory.h
	$(CC) -c  config_poolMemory.c
	
#Borrar todo
clean:
	rm poolMemory $(OBJS)