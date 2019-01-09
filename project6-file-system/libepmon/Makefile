CC = mipsel-linux-gcc
AR = mipsel-linux-ar

all: build copy

build: libepmon.a 

libepmon.a: libepmon.o
	${AR} crv $@ printstr.o printch.o sdwrite.o sdread.o memcopy.o Write_Single_Block.o isQEMU.o serial_port_read.o

libepmon.o: printstr.c printch.c sdwrite.c sdread.c memcopy.c Write_Single_Block.c isQEMU.c serial_port_read.o
	${CC} -c printstr.c printch.c sdwrite.c sdread.c memcopy.c Write_Single_Block.c isQEMU.c serial_port_read.c

copy:
	cp libepmon.a /mnt/shared/project_6_1128/finished_code/libs/ 

clean:
	rm -rf *.a *.o
