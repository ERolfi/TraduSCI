#include "common.h"
#include "endian.h"

typedef struct {
  uint8        *buffer;
  uint8         rType;
  uint16        rNum;
  uint16        id;
  uint8         volNum;
  uint32        volOfs;
  uint32        compressedSize;
  uint32        realSize;
  uint16        packType;
} SCIRes;

extern uint8   FormatNames;
extern uint8   TypeNums;
extern int16   resErrCode;

#define mapMessage 0x99
#define resErrInvalidResNum     0x1000  // resNum > 2047 on SCI0/early SCI1
#define resErrResNotInMap       0x1001  // resource not found in map
#define resErrNoResMem          0x1002  // not enough memory for resource
#define resErrNoDecompMem       0x1003  // not enough memory for decompress
#define resErrNoDecodeMem       0x1004  // not enough memory for GFX decode
#define resErrInvalidPacktype   0x1005  // unknown compression type
#define resErrVolumeNotFound    0x1006  // could not find volume file
#define resErrResNotInVolume    0x1007  // resource not where mapfile stated
#define resErrMapNotFound       0x1008  // could not find resource.map
#define resErrNoMapMem          0x1009  // not enough memory for resource map
#define resErrNoViews           0x100a  // no VIEWs found to determine colors

#define gfxEGA   16
#define gfxVGA   256
#define gfxAmiga 32

#define resTypeView     0x00
#define resTypePic      0x01
#define resTypeScript   0x02
#define resTypeText     0x03
#define resTypeSound    0x04
#define resTypeMemory   0x05
#define resTypeVocab    0x06
#define resTypeFont     0x07
#define resTypeCursor   0x08
#define resTypePatch    0x09
#define resTypeBitmap   0x0a
#define resTypePalette  0x0b
#define resTypeCDAudio  0x0c
#define resTypeAudio    0x0d
#define resTypeSync     0x0e
#define resTypeMessage  0x0f
#define resTypeMap      0x10
#define resTypeHeap     0x11

#define resNamesOld 0       // i.e. script.100
#define resNamesNew 1       // i.e. 100.scr
#define resNamesv3  2       // i.e. 100.csc
#define resId2bytes 0       // type -0x80 SHL 11 + num
#define resId3      1       // byte type, word num
#define resTypes80  0x80    // 8xh
#define resTypes00  0x00    // 0xh
#define resHead8    8       // 2 bytes id, 16 bit sizes
#define resHead9    9       // 3 bytes id, 16 bit sizes
#define resHead13   13      // 3 bytes id, 32 bit sizes
#define map26p6v    0       // 26 bit ofs, 6 bit volnum
#define map28p4v    1       // 28 bit ofs, 4 bit volnum
#define map24p0v    2       // 24 bit ofs/2, no volnum
#define map32p0v    3       // 32 bit ofs, no volnum
#define volNamesOld 0       // RESOURCE.MAP
#define volNamesNew 1       // RESSCI.000

int     LoadResourceMap ();
void    FreeResourceMap ();
void    DetectSCIFormats ();
int     DetectGFXType ();
int     LoadResource (SCIRes *res);
