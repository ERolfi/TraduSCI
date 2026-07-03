#include "common.h"
#include "delzw.h"

typedef struct {
  uint16        PrefixCode;
  uint8         Literal;
} DictEntry;

uint16 GetBitsLSB (uint8 **source, uint16 *bitpos, uint16 bits) {
        uint32 x32;
        uint8  *s;
        uint16 x;

        s = *source;
        x32 = (s[0] + (s[1]<<8) + (s[2]<<16) + (s[3]<<24));
        x = ((x32 >> *bitpos) & ((1 << bits)-1) );
        *bitpos += bits;
        while (*bitpos >= 8) {
          s++;
          *bitpos -= 8;
        }
        *source = s;
        return (x);
}

int lzw_Decompress (uint8 *source, uint8 *target, uint32 outsize) {
        uint8  *s, *t;
        uint16  BitPosition;
        uint32  BytesWritten;
        uint16  CodeLength, NextCode, MaxCode, ReadCode, ThisCode, PrefixCode;
        uint8   LastCharacter;
        uint8   stringBuf [256];
        uint16  stringCount;
        DictEntry  Dictionary [4096];

        s = source; t = target; BitPosition = 0; BytesWritten = 0;
        CodeLength = 9; NextCode = 0x0102; MaxCode = 0x1ff;
        stringCount = 0;

        while (BytesWritten <= outsize) {
           ReadCode = GetBitsLSB (&s, &BitPosition, CodeLength);
           ThisCode = ReadCode;
           if (ReadCode == 0x101) return (BytesWritten);
           if (ReadCode == 0x100) {
              CodeLength = 9;
              NextCode = 0x0102;
              MaxCode = 0x1ff;
              PrefixCode = GetBitsLSB (&s, &BitPosition, CodeLength);
              LastCharacter = PrefixCode;
              *t++ = PrefixCode; BytesWritten++;
              continue;
           }
           while (1==1) {       // examine code
              if (ThisCode > NextCode)
                 return (err_Decompress_InvalidCode);
              if (ThisCode == NextCode) {       // repeat last character
                 // put last character into string buffer
                 stringBuf [stringCount] = LastCharacter;
                 stringCount++;
                 ThisCode = PrefixCode;
                 continue;      // examine previous code
              }
              if (ThisCode >= 0x100) {          // compressed code
                 // put that code's literal into string buffer
                 stringBuf [stringCount] = Dictionary [ThisCode].Literal;
                 stringCount++;
                 ThisCode = Dictionary [ThisCode].PrefixCode;
                 continue;
              }
              // literal character: put into string buffer
              stringBuf [stringCount] = ThisCode;
              stringCount++;
              LastCharacter = ThisCode;
              // output string buffer
              while (stringCount > 0) {
                stringCount--;
                *t++ = stringBuf [stringCount]; BytesWritten++;
              }
              // update dictionary
              if (NextCode <= MaxCode) {        // prevent dictionary overflow
                 Dictionary [NextCode].Literal    = LastCharacter;
                 Dictionary [NextCode].PrefixCode = PrefixCode;
                 NextCode++;
                 // check if more bits needed
                 if ((NextCode > MaxCode) && CodeLength < 12) {
                    CodeLength++;
                    MaxCode = ((1 << CodeLength) -1);
                 }
              }
              PrefixCode = ReadCode;
              break;
           }
        }
        // if here, outsize is not big enough
        return (err_Decompress_BufferOverflow);
}
