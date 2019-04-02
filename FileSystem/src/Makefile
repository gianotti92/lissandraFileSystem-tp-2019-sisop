#Declaro variables
CC := gcc

OBJS := fileSystem.o config_fileSystem.o

#Generador principal
all: fileSystem


fileSystem: $(OBJS)
	$(CC) -o fileSystem $(OBJS) -lcommons -lm 

fileSystem.o: fileSystem.c config_fileSystem.h
	$(CC) -c fileSystem.c
	
config_fileSystem.o: config_fileSystem.c config_fileSystem.h
	$(CC) -c  config_fileSystem.c
	
#Borrar todo
clean:
	rm fileSystem $(OBJS)