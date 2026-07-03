scidasm.exe:	scidasm.o endian.o res.o delzw.o delzwmsb.o delzs.o dedcl.o
	gcc -Os -D_SCI32	-o scidasm.exe scidasm.o endian.o res.o	delzw.o	delzwmsb.o delzs.o	dedcl.o

scidasm.o:      scidasm.c opcodes.h
	gcc -c -Os -D_SCI32 scidasm.c

res.o:		res.c
	gcc -c -Os -D_SCI32 res.c

endian.o:	endian.c
	gcc -c -Os -D_SCI32 endian.c

delzw.o:	delzw.c
	gcc -c -Os -D_SCI32 delzw.c

delzwmsb.o:	delzwmsb.c
	gcc -c -Os -D_SCI32 delzwmsb.c

delzs.o:	delzs.c
	gcc -c -Os -D_SCI32 delzs.c

dedcl.o:	dedcl.c
	gcc -c -Os -D_SCI32 dedcl.c

