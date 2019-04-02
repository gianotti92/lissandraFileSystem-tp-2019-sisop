#Declaro variables
CC := gcc

OBJS := kernel.o config_kernel.o

#Generador principal
all: kernel


kernel: $(OBJS)
	$(CC) -o kernel $(OBJS) -lcommons -lm 

kernel.o: kernel.c config_kernel.h
	$(CC) -c kernel.c
	
config_kernel.o: config_kernel.c config_kernel.h
	$(CC) -c  config_kernel.c
	
#Borrar todo
clean:
	rm kernel $(OBJS)