#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "endian.h"
#include "delzw.h"
#include "delzwmsb.h"
#include "dedcl.h"
#include "delzs.h"

#include "res.h"

int16   resErrCode;
uint8   FormatNames;
uint8   TypeNums;
uint8   TypeFormat;
uint8   ResHeadLength;
uint8   MapEntryFormat;
uint8   VolumeFormat;
uint8  *resourceMap;
uint32  resourceMapSize = 0;
uint8  *messageMap;
uint32  messageMapSize = 0;


int     DecodeView256_10 (uint8 *source, uint8 *target, uint32 outsize) {
        uint8   *s, *t, *sCelSizes, *sCountBytes, *sDataBytes;
        uint8   *tCelPointers, *tFramePointers, *sFrameSizes;
        uint16  numCels, totalFrames, framesInCel, paletteStart, cel, frame;
        uint16  rasterBytesDone, rasterBytesToGo, count, x;
        uint32  celMask, uniqueCels;

        /* structure of input file
        $00:    pointer to frame sizes (number of raster bytes per frame
                in target structure)
        $02:    total number of cells (including flipped ones)
        $03:    number of unique cells
        $04:    cel flip mask
        $08:    pointer to palette in target
        $0C:    n bytes (n=uniqueCels): number of frames in cel
        ...     frame headers, 7 bytes each (8th byte will always be 0)
        $fs-2   zero word (?)
        $fs     pointed to by $00, frame sizes
        ...     Count bytes, bitfield: aabbbbbb
                aa=11 (c0): skip bbb pixels (no data bytes)
                aa=10 (80): repeat following pixel bbb times (1 data byte)
                aa=00 (00): copy bbb pixels (bbb data bytes)
        ...     Data bytes
        */
        // get total number of cells (including flipped ones)
        numCels = source[2];
        // get number of unique cells
        uniqueCels = source[3];
        // get mask of flipped cells. bit =1: cell same as previos but flipped
        celMask = get_lsb32 (source+0x04);
        // pointer to palette in target
        paletteStart = get_lsb16 (source+0x08);
        // number of frames across all cells
        totalFrames = get_lsb16 (source+0x0a);

        sFrameSizes = source + get_lsb16 (source+0) +2;
        sCountBytes = sFrameSizes + (totalFrames << 1);

        // calculate number of count bytes to reach the data bytes
        sDataBytes = sCountBytes;
        rasterBytesToGo = 0;
        for (frame = 0; frame < totalFrames; frame++) {
          rasterBytesToGo += get_lsb16 (&sFrameSizes [frame << 1]);
        }
        while (rasterBytesToGo > 0) {
          x     = (*sDataBytes & 0xc0);
          count = (*sDataBytes & 0x3f);
          sDataBytes++; rasterBytesToGo--;
          if      (x == 0xc0);
          else if (x == 0x80) rasterBytesToGo--;
          else rasterBytesToGo -= count;
        }

        // create view header
        t = target;
        *t++ = numCels; // ofs $00: number of cels
        *t++ = 0x80;    // ofs $01: type ($80: 256 colors)
        put_lsb32 (t, celMask);      t+=4;      // ofs $02: celMask
        put_lsb16 (t, paletteStart); t+=2;      // ofs $06: palette offset
        tCelPointers = t;
        sCelSizes = source + 0x0c;
        t+= (numCels << 1);
        s = sCelSizes + uniqueCels;

        for (cel = 0; cel < numCels; cel++) {
          if (celMask & 1) {
            // same as previous cell
            put_lsb16 (tCelPointers, get_lsb16 (tCelPointers-2));
            tCelPointers+=2;
            celMask = celMask >> 1;
            continue;
          }
          celMask = celMask >> 1;
          framesInCel = *sCelSizes++;
          // fill cel pointer list
          put_lsb16 (tCelPointers, t-target); tCelPointers+=2;
          // start cel header
          put_lsb32 (t, framesInCel); t += 4;
          tFramePointers = t;
          t += (framesInCel << 1);
          for (frame = 0; frame < framesInCel; frame++) {
            put_lsb16 ((uint8 *) tFramePointers, t-target); tFramePointers+=2;
            // copy frame header
            memcpy (t, s, 7); t+=7; s+=7;
            *t++ = 0;           // dummy byte
            // copy raster data for frame
            rasterBytesToGo = get_lsb16 ((uint8 *) sFrameSizes); sFrameSizes+=2;
            rasterBytesDone = 0;
            while (rasterBytesDone < rasterBytesToGo) {
              x     = (*sCountBytes & 0xc0);
              count = (*sCountBytes & 0x3f);
              rasterBytesDone++;
              *t++ = *sCountBytes++;

              if (x == 0xc0) {
                // pixels to skip
              } else if (x == 0x80) {
                // RLE encoding
                *t++ = *sDataBytes++;
                rasterBytesDone++;
              } else {
                // pixels to copy
                memcpy (t, sDataBytes, count); t += count; sDataBytes += count;
                rasterBytesDone += count;
              }
            }
          }
        }
        // copy palette
        *t++ = 'P';
        *t++ = 'A';
        *t++ = 'L';
        for (x=0; x<256; x++) {
          *t++ = x;
        }
        *t++ = 0;       // ???
        *t++ = 0;       // ???
        *t++ = 0;       // ???
        *t++ = 0;       // ???
        memcpy (t, s, 4*256); t+=4*256;

        return (0);
}

int     DecodePic256_10 (uint8 *source, uint8 *target, uint32 outsize) {
        uint16  x, sizeControlMap, sizeRaster1, sizeRaster2, sizeRasterTotal;
        uint16  count, pStartPic, bytesDone;
        uint8   *s, *t, *sRaster1, *sRaster2, *sControlMap, *tStartPic;

        sizeRasterTotal = get_lsb16 (source+0x0);
        pStartPic       = get_lsb16 (source+0x2);
        sizeRaster1     = get_lsb16 (source+0x4);
        sizeRaster2     = sizeRasterTotal - sizeRaster1;
        sizeControlMap  = outsize - (sizeRasterTotal + pStartPic + 0x0f);

        t = target; tStartPic = pStartPic +t;

        // palette
        *t++ = 0xfe;
        *t++ = 2;
        for (x=0; x<256; x++) *t++ = x;
        *t++ = 0; *t++ = 0; *t++ = 0; *t++ = 0;
        s = source + 0x0d;
        memcpy (t, s, 256*4+0x16); t+=256*4+0x16; s+=256*4+0x16;
        sControlMap = s;
        sRaster1 = sControlMap + sizeControlMap;
        sRaster2 = sRaster1 + sizeRaster1;

        // pic
        *t++ = 0xfe;
        *t++ = 0x1;
        *t++ = 0; *t++ = 0; *t++ = 0;
        put_lsb16 (t, sizeRasterTotal +8); t+=2;
        memcpy (t, source+6, 7); t+=7; *t++ = 0;

        // raster data
        bytesDone = 0;
        while (bytesDone < sizeRasterTotal) {
          x    = (*sRaster2 & 0xc0);
          count= (*sRaster2 & 0x3f);
          *t++ = *sRaster2++; bytesDone++;
          if      (x == 0xc0) {
             // skip
          }
          else if (x == 0x80) {
             // RLE
             *t++ = *sRaster1++; bytesDone++;
          }
          else {
             // direct
             memcpy (t, sRaster1, count);
             t += count; sRaster1 += count; bytesDone += count;
          }
        }

        // control map
        memcpy (t, sControlMap, sizeControlMap); t+=sizeControlMap;
        return (0);
}

int     LoadResourceMap () {
        FILE    *resMap, *msgMap;

        resMap = fopen ("resmap.000", "rb");
        if (!resMap) resMap = fopen ("resource.map", "rb");
        if (!resMap) {
           resErrCode = resErrMapNotFound;
           return (0);
        } 
        fseek (resMap, 0, SEEK_END);
        resourceMapSize = ftell (resMap);
        resourceMap = (uint8 *) malloc (resourceMapSize);
        if (!resourceMap) {
           resErrCode = resErrNoMapMem;
           fclose (resMap);
           return (0);
        }
        fseek (resMap, 0, SEEK_SET);
        fread (resourceMap, 1, resourceMapSize, resMap);
        fclose (resMap);

        // load message map, if present
        msgMap = fopen ("message.map", "rb");
        if (!msgMap) return (1);
        fseek (msgMap, 0, SEEK_END);
        messageMapSize = ftell (msgMap);
        messageMap = (uint8 *) malloc (messageMapSize);
        if (!messageMap) {
           fclose (msgMap);
           return (1);
        }
        fseek (msgMap, 0, SEEK_SET);
        fread (messageMap, 1, messageMapSize, msgMap);
        fclose (msgMap);

        return (1);
}

void    FreeResourceMap () {
        if (messageMapSize) free (messageMap);
        free (resourceMap);
}

int     HighestVolumeFile () {
        char    VolumeName [32];
        int     currentVolume;
        FILE    *temp;

        currentVolume = 1;
        while (1==1) {
          sprintf (VolumeName, "resource.%03d", currentVolume);
          temp = fopen (VolumeName, "rb");
          if (!temp) break;
          fclose (temp);
          currentVolume++;
        }
        return (currentVolume -1);
}

int     HighestMapVolume26p6v () {
        uint8   *mapEntry;
        uint32  volume;
        int     highestVolume;

        highestVolume = 0;
        // only works for resHead8, which is ok, since they are the only
        // ones using the 26p6v entry format
        mapEntry = resourceMap;
        while (get_lsb32 (mapEntry) != 0xffffffff) {
           volume = get_lsb32 (mapEntry+2) >> 26;
           if (volume > highestVolume) highestVolume = volume;
           mapEntry += 6;
        }
        return (highestVolume);
}

int     IsSCI32 () {
        FILE    *temp;
        uint8   resHeader [16];

        temp = fopen ("resource.000", "rb");
        if (!temp) return (0);
        fread (resHeader, 1, 16, temp);
        fclose (temp);
        if (get_lsb16(resHeader+5) +4 < get_lsb16(resHeader+3))
           /* in 16 bit, [5] is real size, [3] is compressed size
            therefore [5] is always greater than [3]
            if not, [5] is the high word of the compressed size, therefore
            it's SCI32
            +4 because in SCI1.0 uncompressed files, compressed size
            is 4 bytes too large */
           return (1);
        else
           return (0);
}

int     IsSCI11 () {
        FILE    *temp;
        uint8   resHeader [16];
        uint8   *mapEntry;
        uint8   wantedType;
        uint16  wantedNum;
        uint32  wantedPos;

        // Just try if loading a resource using the 24p0v method works...
        mapEntry = resourceMap;
        mapEntry += get_lsb16 (resourceMap+1); // point to first type
        wantedType = resourceMap [0];
        // find first entry which has not a zero offset
        while (get_lsb32 (mapEntry+2) & 0xffffff == 0) mapEntry += 5;
        wantedNum = get_lsb16 (mapEntry);
        wantedPos = (get_lsb32 (mapEntry +2) & 0xffffff) << 1;
        temp = fopen ("resource.000", "rb");
        if (!temp) return (0);
        fseek (temp, wantedPos, SEEK_SET);
        fread (resHeader, 1, 16, temp);
        fclose (temp);
        // check if map entry and resource header match
        if (wantedType != resHeader[0]) return (0);
        if (wantedNum != get_lsb16 (resHeader+1)) return (0);
        // match, therefore it's SCI1.1!
        return (1);
}

void    DetectSCIFormats () {
        FILE    *temp;
        // check if SCI 2.1/3.0 with new map name
        temp = fopen ("resmap.000", "rb");
        if (temp) {
           fclose (temp);
           FormatNames = resNamesNew;
           TypeFormat = resId3;
           TypeNums = resTypes00;
           ResHeadLength = resHead13;
           MapEntryFormat = map32p0v;
           VolumeFormat = volNamesNew;
           return;
        }
        VolumeFormat = volNamesOld;
        // check if resource map ends with FF FF FF FF
        if (!memcmp (resourceMap+resourceMapSize-4, "\xff\xff\xff\xff", 4)) {
           // SCI0 or early SCI1
           TypeFormat = resId2bytes;
           TypeNums = resTypes80;
           ResHeadLength = resHead8;
           if (HighestVolumeFile () == HighestMapVolume26p6v ()) {
              MapEntryFormat = map26p6v;
              FormatNames = resNamesOld;
           } else {
              MapEntryFormat = map28p4v;
              FormatNames = resNamesNew;
           }
           return;
        }
        // resource map does not end with FF FF FF FF
        // => type-separated resource map
        TypeFormat = resId3;
        FormatNames = resNamesNew;
        if (resourceMap [0] < 0x80) {
           // SCI 2.1
           TypeNums = resTypes00;
           ResHeadLength = resHead13;
           MapEntryFormat = map32p0v;
           return;
        }
        TypeNums = resTypes80;
        // check if SCI32 (SCI 2.0 in this case)
        if (IsSCI32 ()) {
           ResHeadLength = resHead13;
           MapEntryFormat = map32p0v;
           return;
        }
        // not SCI32
        ResHeadLength = resHead9;
        // check if SCI 1.1 or 1.0
        if (IsSCI11 ())
           MapEntryFormat = map24p0v;
        else
           MapEntryFormat = map28p4v;
        return;
}

int     DetectGFXType () {
        uint16  resNum, x;
        SCIRes  view;

        if ((MapEntryFormat == map24p0v) || (MapEntryFormat == map32p0v))
           return (gfxVGA);
           
        // load any view resource
        for (resNum = 0; resNum < 65535; resNum++) {
           view.rType = resTypeView;
           view.rNum  = resNum;
           if (LoadResource (&view)) break;
        }
        if (resNum == 65535) {
           resErrCode = resErrNoViews;
           return (0);
        }
        if (view.buffer [1] == 0x80) {
           free (view.buffer);
           return (gfxVGA);
        }
        // get transparency information for first frame
        x = get_lsb16 (view.buffer + 8);        // point to cel header
        x = get_lsb16 (view.buffer + x + 4);    // point to frame header
        x = view.buffer [x + 6];                // transparency pixel
        free (view.buffer);
        if (x > 15)
           return (gfxAmiga);
        else
           return (gfxEGA);
}

int     FindInResourceMap (SCIRes *res, int whichMap) {
        uint8   *mapstart, *map, *mapend, *typeend;
        uint16  resId;

        res->id = (res->rType << 11) + res->rNum;

        if (whichMap == mapMessage) {
           if (messageMapSize) {
             mapstart = messageMap;
             mapend = mapstart + messageMapSize;
           } else {
             return (0);
           }
        } else {
           mapstart = resourceMap;
           mapend = mapstart + resourceMapSize;
        }

        map = mapstart;

        if (TypeFormat == resId2bytes) {
          while (get_lsb32 (map) != 0xffffffff) {
            if (get_lsb16 (map) == res->id) {
              if (MapEntryFormat == map26p6v) {
                res->volNum = get_lsb32 (map+2) >> 26;
                res->volOfs = get_lsb32 (map+2) & ((1 << 26)-1);
              } else {
                // 28p4v
                res->volNum = get_lsb32 (map+2) >> 28;
                res->volOfs = get_lsb32 (map+2) & ((1 << 28)-1);
              }
              return (1);
            }
            map += 6;
          }
          // not found
          return (0);
        } else {
          // type-separated resource map.
          // find type in index
          while ((*map != (res->rType | TypeNums) ) && (*map != 0xff))
              map+=3;
          if (*map == 0xff) return (0);
          typeend = mapstart + get_lsb16 (map+4);
          map = mapstart + get_lsb16 (map+1);

          // find resnum in typelist
          while (map < typeend) {
            if (get_lsb16 (map) != res->rNum) {
               if (MapEntryFormat == map24p0v)
                  map+=5;
               else
                  map+=6;
               continue;
            }
            // this is the requestedResourceNum;
            switch (MapEntryFormat) {
              case map24p0v: {
                   res->volNum = whichMap;
                   res->volOfs = (get_lsb32 (map+2) & 0x00ffffff) << 1;
                   break;
                   }
              case map28p4v: {
                   res->volNum = get_lsb32 (map+2) >> 28;
                   res->volOfs = get_lsb32 (map+2) & 0x0fffffff;
                   break;
                   }
              case map26p6v: {
                   res->volNum = get_lsb32 (map+2) >> 26;
                   res->volOfs = get_lsb32 (map+2) & 0x3fffffff;
                   break;
                   }
              case map32p0v: {
                   res->volNum = whichMap;
                   res->volOfs = get_lsb32 (map+2);
                   break;
                   }
            }
            return (1);
          }
          // not found
          return (0);
        }
}

FILE   *ReadResHeader (SCIRes *res) {
        FILE   *volumeFile;
        char    volumeName [32];
        uint8   resHeader [32];

        if (res->volNum == mapMessage)
          sprintf (volumeName, "resource.msg");
        else if (VolumeFormat == volNamesNew)
			sprintf (volumeName, "ressci.%03d", res->volNum);
		else
			sprintf (volumeName, "resource.%03d", res->volNum);

        volumeFile = fopen (volumeName, "rb");
        if (!volumeFile) {
          resErrCode = resErrVolumeNotFound;
          return (0);
        }
        fseek (volumeFile, res->volOfs, SEEK_SET);
        fread (resHeader, 1, ResHeadLength, volumeFile);
        if (ResHeadLength == resHead8) {
           if (get_lsb16 (resHeader+0) != res->id) {
              resErrCode = resErrResNotInVolume;
              fclose (volumeFile);
              return (0);
           }
           res->compressedSize = get_lsb16 (resHeader+2);
           res->realSize       = get_lsb16 (resHeader+4);
           res->packType       = get_lsb16 (resHeader+6);
        } else {
           if ((resHeader[0] != (res->rType | TypeNums)) ||
               (get_lsb16 (resHeader+1) != res->rNum)) {
               resErrCode = resErrResNotInVolume;
               fclose (volumeFile);
               return (0);
           }
           if (ResHeadLength == resHead9) {
             res->compressedSize = get_lsb16 (resHeader+3);
             res->realSize       = get_lsb16 (resHeader+5);
             res->packType       = get_lsb16 (resHeader+7);
           } else {
             // resHead13 => SCI32
             res->compressedSize = get_lsb16 (resHeader+3);
             res->realSize       = get_lsb16 (resHeader+7);
             if (res->compressedSize < res->realSize)
                res->packType = 0x20;
             else
                res->packType = 0x00;
           }
        }
        return (volumeFile);
}

int     LoadResource (SCIRes *res) {
        uint8  *compressed, *work;
        int     x;
        FILE   *volumeFile;

        if ((TypeFormat == resId2bytes) && (res->rNum > 2047)) {
          resErrCode = resErrInvalidResNum;
          return (0);
        }
        if (!FindInResourceMap (res, mapMessage))
          if (!FindInResourceMap (res, 0)) {
            resErrCode = resErrResNotInMap;
            return (0);
          }

        volumeFile = ReadResHeader (res);
        if (!volumeFile) return (0);

        if ((res->packType == 0x13) || (res->packType == 0x14))
           res->realSize += 0x18;

        res->buffer = (uint8 *) malloc (res->realSize);
        if (!res->buffer) {
           resErrCode = resErrNoResMem;
           return (0);
        } 

        if (res->packType == 0) {
           fread (res->buffer, 1, res->realSize, volumeFile);
           fclose (volumeFile);
           return (1);
        }

        compressed = (uint8 *) malloc (res->compressedSize);
        if (!compressed) {
           resErrCode = resErrNoDecompMem;
           free (res->buffer);
           return (0);
        }

        if ((res->packType == 3) || (res->packType == 4)) {
           work = (uint8 *) malloc (res->realSize);
           if (!work) {
              resErrCode = resErrNoDecodeMem;
              free (compressed);
              free (res->buffer);
              return (0);
           }
        }
        fread (compressed, 1, res->compressedSize, volumeFile);
        fclose (volumeFile);

        switch (res->packType) {
                case 0x01: lzw_Decompress (compressed, res->buffer, res->realSize); break;
                case 0x02: lzwmsb_Decompress (compressed, res->buffer, res->realSize); break;
                case 0x03: lzwmsb_Decompress (compressed, work, res->realSize);
                           DecodeView256_10 (work, res->buffer, res->realSize);
                           free (work);
                           break;
                case 0x04: lzwmsb_Decompress (compressed, work, res->realSize);
                           DecodePic256_10 (work, res->buffer, res->realSize);
                           free (work);
                           break;
                case 0x12: dcl_Decompress (compressed, res->buffer, res->realSize); break;
                case 0x13: dcl_Decompress (compressed, res->buffer+0x18, res->realSize);
                           memset (res->buffer, 0, 0x18);
                           break;
                case 0x14: dcl_Decompress (compressed, res->buffer+0x18, res->realSize);
                           memset (res->buffer, 0, 0x18);
                           break;
                case 0x20: lzs_Decompress (compressed, res->buffer, res->realSize); break;
                default:   free (res->buffer);
                           free (compressed);
                           resErrCode = resErrInvalidPacktype;
                           return (0);
        }
        free (compressed);
        return (1);
};
