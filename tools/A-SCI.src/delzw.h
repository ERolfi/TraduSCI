#define err_None 0
#define err_Decompress_InvalidCode -0x2000
#define err_Decompress_BufferOverflow -0x2001

int lzw_Decompress (uint8 *source, uint8 *target, uint32 outsize);
