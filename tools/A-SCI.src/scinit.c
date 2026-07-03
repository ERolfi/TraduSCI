#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "endian.h"

//THESE FUNCTIONS ARE HERE ONLY FOR SCI32 GAMES

uint8  *memstr (uint8 *memoryData, uint32 memorySize, char *stringData, uint32 stringLength) {
        uint8  *c, *memoryEnd;

        memoryEnd = memoryData + memorySize;
        c = memoryData;
        while (c = (uint8 *) memchr (c, stringData [0], memoryEnd - c)) {
          if (memcmp (c, stringData, stringLength) == 0) return (c);
          c++;
        }
        return (NULL);
}

void    getKernelsFromInterp () {
        FILE   *interpFile;
        uint32  interpSize, count;
        uint8  *interpData, *kernelList;

        interpFile = fopen ("sierra.exe", "rb");
        if (!interpFile) {
           perror ("sierra.exe");
           exit (1);
        }
        fseek (interpFile, 0, SEEK_END); interpSize = ftell (interpFile); fseek (interpFile, 0, SEEK_SET);
        interpData = (uint8 *) malloc (interpSize);
        if (!interpData) {
           fclose (interpFile);
           printf ("Not enough memory for interpreter file.\n");
           exit (1);
        }
        fread (interpData, 1, interpSize, interpFile);
        fclose (interpFile);

        kernelList = memstr (interpData, interpSize, "Load\0Unload\0", 12);
        if (!kernelList) {
           free (interpData);
           printf ("Could not find Kernel list in sierra.exe.\n");
           exit (1);
        }
        count = 0;
        while (memcmp (kernelList, "..", 2) == 1) {
           printf ("%03d=%s\n", count, kernelList);
           kernelList = (uint8 *) memchr (kernelList, 0, interpSize-(kernelList-interpData)) +1;
           count++;
        }
        free (interpData);
}

int     main (int argc, char **arg) {

        getKernelsFromInterp ();
}
