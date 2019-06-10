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
	echo "- Se compilo correctamente comunicacion"
kernel_utils: src/$@.*
	gcc -c src/$@.c $(FLAGS) $(LIBS) -o Release/$@.o
	echo "- Se compilo correctamente kernel_utils"
parser: src/$@.*
	gcc -c src/$@.c $(FLAGS) $(LIBS) -o Release/$@.o
	echo "- Se compilo correctamente parser"
utils: src/$@.*
	gcc -c src/$@.c $(FLAGS) $(LIBS) -o Release/$@.o
	echo "- Se compilo correctamente utils"

clean:
	rm -rf Release Debug */logger.log
	find ./ \( -name ".cproject" -or -name ".project" -or -name ".settings" \) -exec rm -rf {} \;
	echo "Se hizo clean de las Utilguenguenchas"
