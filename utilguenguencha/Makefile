# -*- MakeFile -*-

FLAGS := -Wall -g

LIBS := -pthread -lcommons -lreadline

all: default

default: clean dir comunicacion kernel_utils parser utils

.PHONY: dir
dir:
	mkdir -p Release

comunicacion: src/$@.*
	gcc -c src/$@.c $(FLAGS) $(LIBS) -o Release/$@.o

kernel_utils: src/$@.*
	gcc -c src/$@.c $(FLAGS) $(LIBS) -o Release/$@.o

parser: src/$@.*
	gcc -c src/$@.c $(FLAGS) $(LIBS) -o Release/$@.o

utils: src/$@.*
	gcc -c src/$@.c $(FLAGS) $(LIBS) -o Release/$@.o

clean:
	rm -rf Release Debug */logger.log
