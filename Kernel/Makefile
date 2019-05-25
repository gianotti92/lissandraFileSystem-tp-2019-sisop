# -*- MakeFile-*-

FLAGS := -Wall -g

LIBS := -pthread -lcommons -lreadline

all: default

default: dir kernel

dir:
	mkdir -p Release

kernel: src/*.* ../utilguenguencha/*.* */*.cfg
	gcc src/*.c ../utilguenguencha/*.c $(FLAGS) $(LIBS) -o Release/$@

clean:
	rm -rf Release Debug */logger.log
