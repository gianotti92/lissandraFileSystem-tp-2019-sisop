#Declaro variables
CC := gcc

OBJS := kernel.o servidor.o config_kernel.o

#Generador principal
all: kernel


kernel: $(OBJS)
	$(CC) -o kernel $(OBJS) -pthread -lcommons -lm 

kernel.o: kernel.c servidor.h config_kernel.h
	$(CC) -c kernel.c

servidor.o: servidor.c servidor.h config_kernel.h
	$(CC) -c servidor.c

config_kernel.o: config_kernel.c config_kernel.h
	$(CC) -c  config_kernel.c
	
#Borrar todo
clean:
	rm kernel $(OBJS)