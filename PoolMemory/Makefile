# -*-MakeFile-*-

FLAGS := -Wall -g

LIBS := -pthread -lcommons -lreadline

utilguenguencha_path := ../utilguenguencha/

all: default

default: clean dir poolmemory

.PHONY: dir
dir:
	mkdir -p Release

poolmemory: src/*.* utilguenguencha
	gcc src/*.c $(utilguenguencha_path)Release/*.o $(FLAGS) $(LIBS) -o Release/$@

.PHONY: utilguenguencha
utilguenguencha:
	make -C $(utilguenguencha_path)

clean:
	rm -rf Release Debug */logger.log
