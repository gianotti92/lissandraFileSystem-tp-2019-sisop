# -*- MakeFile -*-

FLAGS := -Wall -g

LIBS := -pthread -lcommons -lreadline

all: default

default: dir poolmemory

dir:
	mkdir -p Release

poolmemory:
	gcc src/*.c ../utilguenguencha/*.c $(FLAGS) $(LIBS) -o Release/$@

clean:
	rm -rf Release/*

