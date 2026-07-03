sciasm11.exe:    sciasm.o	endian.o res.o delzw.o delzwmsb.o delzs.o dedcl.o
	gcc -Os -D_SCI11 -o sciasm11.exe sciasm.o endian.o	res.o delzw.o delzwmsb.o delzs.o						dedcl.o

sciasm.o:      sciasm.c	opcodes.h
	gcc -c -Os -D_SCI11 sciasm.c

res.o:		res.c
	gcc -c -Os -D_SCI11 res.c

endian.o:	endian.c
	gcc -c -Os -D_SCI11 endian.c

delzw.o:	delzw.c
	gcc -c -Os -D_SCI11 delzw.c

delzwmsb.o:	delzwmsb.c
	gcc -c -Os -D_SCI11 delzwmsb.c

delzs.o:	delzs.c
	gcc -c -Os -D_SCI11 delzs.c

dedcl.o:	dedcl.c
	gcc -c -Os -D_SCI11 dedcl.c

