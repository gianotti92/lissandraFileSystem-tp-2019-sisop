TARGET = lissandra # Definimos la tarea que va a ejecutar make por defecto Y el nombre del binario final
LIBS = -lreadline -lcommons -pthread # Incluimos las bibliotecas que necesitamos, las commons y readline
CC = gcc # Vamos a usar gcc como compilador, porque nos encanta!
CFLAGS = -g -Wall # Compilamos con los flags -g (con simbolos de debug) y -Wall (que nos muestre todos los warnings)

default: $(TARGET) # la regla por defecto (tp0)
all: default # Si ejecutan all, que ejecute la default

$(TARGET):
	$(CC) src/*.c ../utilguenguencha/utilguenguencha/*.c $(CFLAGS) $(LIBS) -o Release/$@

clean: # Finalmente creamos una tarea para eliminar todos los .o y el binario
		-rm -f Release/$(TARGET)
