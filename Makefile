# -*- MakeFile -*-

fileSystem_path := FileSystem/
kernel_path := Kernel/
poolMemory_path := PoolMemory/

all: default

default: fs kernel mem

.PHONY: fs
fs:
	make -C $(fileSystem_path)

.PHONY: kernel
kernel:
	make -C $(kernel_path)

.PHONY: mem
mem:
	make -C $(poolMemory_path)

.PHONY: clean
clean:
	rm -rf ./*/Release ./*/Debug
