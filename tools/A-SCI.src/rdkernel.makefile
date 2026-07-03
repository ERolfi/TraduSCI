rdkernel.exe:    scinit.o	endian.o
	gcc -Os -D_SCI32	-o rdkernel.exe scinit.o endian.o

scinit.o:      scinit.c	common.h
	gcc -c -Os -D_SCI32 scinit.c

endian.o:	endian.c
	gcc -c -Os -D_SCI32 endian.c

