#Declaro variables
CC := gcc

OBJS := kernel.o config_kernel.o parser.o

#Generador principal
all: kernel


kernel: $(OBJS)
	$(CC) -o kernel $(OBJS) -lcommons -lreadline -lm 

kernel.o: kernel.c config_kernel.h
	$(CC) -c kernel.c
	
config_kernel.o: config_kernel.c config_kernel.h
	$(CC) -c  config_kernel.c 
	
parser.o: parser.c parser.h
	$(CC) -c parser.c
	
#Borrar todo
clean:
	rm kernel $(OBJS)