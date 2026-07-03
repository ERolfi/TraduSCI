#include "common.h"
#include "endian.h"
#include "dedcl.h"
#include "dedcltr.h"

//Report about unpacking problems
//#define SELFDEBUG

#define getbits(bits)     _getbits(pksrcbuf,bits,&pksrcidx)
#define readbits(bits)    _readbits(pksrcbuf,bits,&pksrcidx)

void *pksrcbuf,*pkdstbuf;
unsigned long pksrcidx,pkdstidx;

unsigned long _getbits(void *buf,int numbits,unsigned long *idx)
{
        unsigned long bytpos;
        unsigned long bitpos;
        uint32 srcdata;

        bytpos = *idx / 8;
        bitpos = *idx % 8;

        srcdata = get_lsb32 ((unsigned long)buf + bytpos);

        srcdata >>=bitpos;
	if(numbits!=32)
                srcdata&=(1<<numbits)-1;
        *idx = *idx + numbits;
        return srcdata;
}

unsigned long _readbits(void *buf,int numbits, unsigned long *idx)
{
        unsigned long tmpidx;

        tmpidx = *idx;
        return _getbits(buf,numbits, &tmpidx);
}

unsigned long getnode(NODE *tree,int maxbits)
{
    int found, bitlen;
    unsigned long result;
    unsigned long allbits, bits;

    allbits = readbits (maxbits); // maximum possible bits in token
    found = 0;
    while(tree->len!=-1)
    {
        bitlen=tree->len;
        bits= allbits & ((1<<bitlen)-1);
        while(tree->len==bitlen)
        {
             if(tree->path==bits)
             {
                 result=tree->value;
                 found = 1;
                 getbits(bitlen);     //discard found bits
                 break;
             }
             tree++;
        }
        if(found)
            break;
    }
    return (result);
}

/*
	Unpack p to u, return actual unpacked data size.
*/
unsigned long dcl_Decompress(void *p,void *u, unsigned long int outsize)
{
        int flexliteral, windowbits;
        unsigned long lit, len;
        long ofs;
        char *dic, *s, *d, *dend;

        s = (char *) p;
        d = (char *) u;
        dend = d + outsize;

#ifdef SELFDEBUG
    if(s[0]>1)
        printf("PKWDCLUnp: Strange flexflag\n");
    if((s[1]<4)||(s[1]>6))
        printf("PKWDCLUnp: Strange dictionary size\n");
#endif

        flexliteral=*s++;     //ASCII-compression, accroding to DCL documentation
        windowbits=*s++;

        pksrcbuf = (void*) s;
        pkdstbuf = (void*) d;
        pksrcidx = pkdstidx = 0;

	do
	{
		if(getbits(1)==0)
		{												//literal token
			if(flexliteral==0)
				*d++=(char)getbits(8);
			else
			{
                                lit = getnode(literals,13);
				if(lit==-1)
				{ 
#ifdef SELFDEBUG
					printf("PKWDCLUnp: Unknown literal\n");
#endif
					break;
				}
				else
					*d++=(char)lit;
			}
		}
		else
		{
                        len=getnode(lengths,7);
			if(len==-1)
			{
#ifdef SELFDEBUG
				printf("PKWDCLUnp: Unknown lenght code\n");
#endif
				break;
			}
			else
				len=(len&0xFFF)+getbits((len>>12)&0xF); //12bit base+4bit extra
			if(len==519)
				break;									//End of stream
                        ofs=getnode(distances,8);
			if(ofs==-1)
			{
#ifdef SELFDEBUG
				printf("PKWDCLUnp: Unknown distance code\n");
#endif
				break;
			}
			if(len==2)
				ofs=(ofs<<2)|getbits(2);
			else
				ofs=(ofs<<windowbits)|getbits(windowbits);
			// Done with fetching dist/len, now copy data from dictionary
                        dic=d-ofs-1;
#ifdef SELFDEBUG
			if(dic<u)
			{
				printf("PKWDCLUnp: Dictionary underflow\n");
				break;
			}
#endif
                        if (d >= dend) {
                           break;
                        }
			while(len--)
				*d++=*dic++;
		}
	}
	while(1);
	return (long)(d)-(long)(u);
}
