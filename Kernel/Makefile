# -*-MakeFile-*-

FLAGS := -Wall -g

LIBS := -pthread -lcommons -lreadline

utilguenguencha_path := ../utilguenguencha/

all: default

default: clean dir kernel

general: dir kernel_general

.PHONY: dir
dir:
	mkdir -p Release

kernel: src/*.* utilguenguencha
	gcc src/*.c $(utilguenguencha_path)Release/*.o $(FLAGS) $(LIBS) -o Release/$@

kernel_general: src/*.*
	gcc src/*.c $(utilguenguencha_path)Release/*.o $(FLAGS) $(LIBS) -o Release/$@

.PHONY: utilguenguencha
utilguenguencha:
	make -C $(utilguenguencha_path)

clean:
	rm -rf Release Debug */logger.log
