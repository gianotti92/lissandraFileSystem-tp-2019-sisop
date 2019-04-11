#Declaro variables
CC := gcc #Compilador

CFLAGS := -Wall -Werror -g -std=c99 #Flags para Valgrind

FILES := $(TARGET) $(CONFIG) $(SERVIDOR)#Nombre de archivos a usar

TARGET := kernel #Principal

CONFIG := config_kernel #Secundario

SERVIDOR := servidor #Secundario

INCLUDES := -lcommons -lpthread #Librerias a linkear

#####################
#Generador principal#
#####################

#Borro los archivos que antes estaban
all: clean $(FILES)

#Build programa
$(TARGET): $(TARGET).o $(CONFIG).o $(SERVIDOR).o <#Tenemos que poner aca todos los que buildean el programa
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).o $(CONFIG).o $(SERVIDOR).o $(INCLUDES)

#######################################
#Archivos .c que conforman el programa#
#######################################
#Hay que hacer un build por cada uno de los archivos y sus dependencias

#Build archivo fileSystem.c	
$(TARGET).o: $(TARGET).c $(CONFIG).h 
	$(CC) $(CFLAGS) $(TARGET).c -o $(TARGET) $(INCLUDES)

#Build archivo config_fileSystem.c
$(CONFIG).o: $(CONFIG).c $(CONFIG).h
	$(CC) $(CFLAGS) $(CONFIG).c -o $(CONFIG) $(INCLUDES)

#Build archivo servidor.c
$(SERVIDOR).o: $(SERVIDOR).c $(SERVIDOR).h
	$(CC) $(CFLAGS) $(SERVIDOR).c -o $(SERVIDOR) $(INCLUDES)

#Borrar todo
clean:
	rm -f $(FILES))