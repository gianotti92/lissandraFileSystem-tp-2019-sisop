# -*- MakeFile -*-

fileSystem_path := FileSystem/
kernel_path := Kernel/
poolMemory_path := PoolMemory/
utilguenguencha_path := utilguenguencha/

all: default

default: utilguenguencha fs kernel mem

.PHONY: fs
fs:
	make -C $(fileSystem_path) general

.PHONY: kernel
kernel:
	make -C $(kernel_path) general

.PHONY: mem
mem:
	make -C $(poolMemory_path) general

.PHONY: utilguenguencha
utilguenguencha:
	make -C $(utilguenguencha_path)

.PHONY: clean
clean:
	rm -rf ./*/Release ./*/Debug */logger.log
