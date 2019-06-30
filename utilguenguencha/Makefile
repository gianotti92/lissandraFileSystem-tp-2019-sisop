# -*- MakeFile -*-

FLAGS := -Wall -g

LIBS := -pthread -lcommons -lreadline

all: default

default: clean dir comunicacion kernel_utils parser utils

.PHONY: dir
.SILENT: dir
dir:
	mkdir -p Release

.SILENT: comunicacion
comunicacion: src/$@.*
	gcc -c src/$@.c $(FLAGS) $(LIBS) -o Release/$@.o
	echo "- Se compilo correctamente comunicacion"
.SILENT: kernel_utils
kernel_utils: src/$@.*
	gcc -c src/$@.c $(FLAGS) $(LIBS) -o Release/$@.o
	echo "- Se compilo correctamente kernel_utils"
.SILENT: parser
parser: src/$@.*
	gcc -c src/$@.c $(FLAGS) $(LIBS) -o Release/$@.o
	echo "- Se compilo correctamente parser"
.SILENT: utils
utils: src/$@.*
	gcc -c src/$@.c $(FLAGS) $(LIBS) -o Release/$@.o
	echo "- Se compilo correctamente utils"

.SILENT: clean
clean:
	rm -rf Release Debug */logger.log
	echo "Se hizo clean de las Utilguenguenchas"
