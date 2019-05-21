# -*- MakeFile -*-
utils_path := utilguenguencha/utilguenguencha/
fileSystem_path := FileSystem/
kernel_path := Kernel/
poolMemory_path := PoolMemory/

all: default

default: fs kernel mem

fs:
	make -C $(fileSystem_path)

kernel:
	make -C $(kernel_path)

mem:
	make -C $(poolMemory_path)

clean:
	rm -rf ./*/Release ./*/Debug
