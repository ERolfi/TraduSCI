#include "delzs.h"

unsigned char *s,*d;
unsigned long srcpos;

unsigned long getrevbits(int numbits)
{
//up to 16 bits at once
	unsigned long result=0;
	if(numbits>0)
	{
		int bytpos=srcpos/8;
		int bitpos=srcpos%8;
		result=(s[bytpos]<<16)|(s[bytpos+1]<<8)|s[bytpos+2];
		result=(result>>(24-numbits-bitpos))&((1L<<numbits)-1);
		srcpos+=numbits;
	}
	return result;
}

int getlen(void)
{
	int bits;
	int length=2;
	do
	{
		bits=getrevbits(2);
		length+=bits;
	}
	while((bits==3)&&(length<8));

	if(length==8)
		do
		{
			bits=getrevbits(4);
			length+=bits;
		}
		while(bits==15);

	return length;
}

/*
	Unpack p to u, return actual unpacked data size.
*/
unsigned long lzs_Decompress (void *p,void *u, unsigned long int outsize)
{
        int len, ofs, tag;
        unsigned char *dic, *dstart;

        s = (unsigned char *) p;
        d = (unsigned char *) u;
        dstart = d;

	srcpos=0;

	do
	{
                tag=getrevbits(1);
		if(tag==0)					// Uncompressed byte
			*d++=(unsigned char)getrevbits(8);
		else						// Chain
		{
			tag=getrevbits(1);
			if(tag==1)				// 7-bit offset or END
			{
				ofs=getrevbits(7);
				if(ofs==0)			// END of stream
					break;
			}
			else					// 11-bit offset
				ofs=getrevbits(11);
                        len=getlen();
                        dic=d-ofs;
                        if(dic< dstart)
			{
				break;
			}
			while(len--)
				*d++=*dic++;
		}
	}
	while(1);						//Until breaked

	return (long)d-(long)u;
}
