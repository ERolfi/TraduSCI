#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "endian.h"
#include "res.h"
#include "opcodes.h"



static const oem2ansi[256] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 164, 16, 17, 18, 19, 182,
	167, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
	39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56,
	57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
	75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92,
	93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108,
	109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122,
	123, 124, 125, 126, 127, 199, 252, 233, 226, 228, 224, 229, 231, 234, 235,
	232, 239, 238, 236, 196, 197, 201, 230, 198, 244, 246, 242, 251, 249, 255,
	214, 220, 248, 163, 216, 215, 131, 225, 237, 243, 250, 241, 209, 170, 186,
	191, 174, 172, 189, 188, 161, 171, 187, 166, 166, 166, 166, 166, 193, 194,
	192, 169, 166, 166, 43, 43, 162, 165, 43, 43, 45, 45, 43, 45, 43, 227, 195,
	43, 43, 45, 45, 166, 45, 43, 164, 240, 208, 202, 203, 200, 105, 205, 206,
	207, 43, 43, 166, 95, 166, 204, 175, 211, 223, 212, 210, 245, 213, 181, 254,
	222, 218, 219, 217, 253, 221, 175, 180, 173, 177, 61, 190, 182, 167, 247, 184,
	176, 168, 183, 185, 179, 178, 166, 160
};


SCIRes  scClassIndex = {
        NULL,
        resTypeVocab,
        996
};

SCIRes  scSelectorVocab = {
        NULL,
        resTypeVocab,
        997
};

SCIRes  scOpcodeVocab = {
        NULL,
        resTypeVocab,
        998
};

SCIRes  scKernelsVocab = { //only for SCI11 games
        NULL,
        resTypeVocab,
        999
};

SCIRes  dasmScript = {
        NULL,
        resTypeScript
};

SCIRes  dasmHeap = {
        NULL,
        resTypeHeap
};

typedef struct {
        char   *name;
        int     props;
        uint16 *propDict;
        uint16 *propDefault;
} tClass;

typedef struct {
        char   *name;
        int     offset;
} tLabel;

char    labelTemp [256];

int     allProps = 1;

FILE   *outHeap, *outScript;
int     codeStart = 65535;
int     dasmNum;

int     scLabels = 0;
tLabel  scLabelData [2048];
int     scClasses = 0;
tClass  scClassData [256];
int     scSelectors;
uint8  *scSelectorName [0x2000];
int     scOpcodes;
uint8  *scOpcodeName [0x100];

char   *scGlobalName [2048];
int     scKernels;			//only for SCI11 games
char   *scKernelName [256];

void    makeDefaultGlobals () {
        FILE   *ini;
        SCIRes  heap0;
        int     numGlobals, g;

        ini = fopen ("globals.ini", "rt");
        if (ini) {
          fclose (ini);
          return;
        }

        heap0.rType = resTypeHeap;
        heap0.rNum = 0;
        if (!LoadResource (&heap0)) {
           printf ("Could not load heap 0.\n");
           exit (1);
        }
        numGlobals = get_lsb16 (heap0.buffer+2);

        ini = fopen ("globals.ini", "w");
        for (g = 0; g < numGlobals; g++) {
          fprintf (ini, "%03d = global%03d\n", g, g);
        }
        fclose (ini);
}

void    initGlobals () {
        FILE   *fGlobals;
        char    oneLine [256], *s, *t;
        int     globalNum, line, nameLen; 

        makeDefaultGlobals ();
        line = 1;
        for (globalNum = 0; globalNum < 2048; globalNum++) {
          scGlobalName [globalNum] = NULL;
        }
        fGlobals = fopen ("globals.ini", "rt");
        if (!fGlobals) {
          printf ("globals.ini not found.\n");
          exit (1);
        }
        while (fgets (oneLine, 256, fGlobals)) {
          // remove spaces
          s = oneLine;
          t = oneLine;
          while (*s != '\0') {
            if (*s == ';') break;
            if ((*s == ' ') || (*s == 13) || (*s == 10)) {
              s++;
              continue;
            }
            *t++ = *s++;
          }
          *t = '\0';
          globalNum = strtol (oneLine, &s, 10);
          if (globalNum >= 2048) {
            printf ("globals.ini line %d: global number %d too great\n", line, globalNum);
            exit (1);
          }
          if (*s != '=') {
            if (!globalNum) break;
            printf ("globals.ini line %d: = required\n", line);
            exit (1);
          }
          *s++;
          nameLen = strlen (s);
          scGlobalName [globalNum] = malloc (nameLen +1);
          strcpy (scGlobalName [globalNum], s);
          line++;
        }
        fclose (fGlobals);
}

int     initKernelsFromVocab () {	//only for SCI11 games
        int     x, len;
        uint8  *i, *s, *t;
        FILE   *fKernels;
        char    oneLine [256];

        if (!LoadResource (&scKernelsVocab)) return (0);
        i = scKernelsVocab.buffer + 2;
        scKernels = get_lsb16 (i-2);
        for (x = 0; x < scKernels; x++) {
          t = scKernelsVocab.buffer + get_lsb16 (i);
          if (t [1] == '\0') {
            len = get_lsb16 (t);
            s = t + 2;
            memmove (t, s, len);
            t [len] = '\0';
          }
          scKernelName [x] = t;
          i += 2;
        }

        fKernels = fopen ("kernels.ini", "w");
        for (x=0; x < scKernels; x++)
          fprintf (fKernels, "%03d = %s\n", x, scKernelName [x]);
        fclose (fKernels);
        return (1);
}

void    initKernels () {
        FILE   *fKernels;
        char    oneLine [256], *s, *t;
        int     KernelNum, line; 

        line = 1;
        for (KernelNum = 0; KernelNum < 256; KernelNum++) {
          scKernelName [KernelNum] = NULL;
        }
        fKernels = fopen ("kernels.ini", "rt");
        if (!fKernels) {
#ifdef _SCI11
          if (initKernelsFromVocab ()) return;
#endif
          printf ("kernels.ini not found.\n");
          exit (1);
        }
        while (fgets (oneLine, 256, fKernels)) {
          // remove spaces
          s = oneLine;
          t = oneLine;
          while (*s != '\0') {
            if (*s == ';') break;
            if ((*s == ' ') || (*s == 13) || (*s == 10)) {
              s++;
              continue;
            }
            *t++ = *s++;
          }
          *t = '\0';
          KernelNum = strtol (oneLine, &s, 10);
          if (KernelNum >= 256) {
            printf ("kernels.ini line %d: kernel number %d too great\n", line, KernelNum);
            exit (1);
          }
          if (*s != '=') {
            if (!KernelNum) break;
            printf ("kernels.ini line %d: = required\n", line);
            exit (1);
          }
          *s++;
          scKernelName [KernelNum] = malloc (strlen (s) +1);
          strcpy (scKernelName [KernelNum], s);
          line++;
        }
        fclose (fKernels);
}
int     addCodeLabel (int offset, char *name) {
        int     label, namelen;

        if (!offset) return (0);

        for (label = 0; label < scLabels; label++)
          if (scLabelData [label].offset == offset) return (0);
        
        namelen = strlen (name);
        scLabelData [scLabels].name = malloc (namelen+1);
        strcpy (scLabelData [scLabels].name, name);

        scLabelData [scLabels].offset = offset;
        scLabels++;

        if (offset < codeStart) codeStart = offset;
        return (1);
}

char   *isLabelHere (int offset) {
        int     label;

        for (label = 0; label < scLabels; label++)
          if (scLabelData [label].offset == offset) return (scLabelData [label].name);
        return (NULL);
}

void    initClasses () {
        int     class, scriptNum, classFound, propDictSize, classNameLen;
        int     p;
        uint8  *heap, *s, *scClassName;
        uint16 *t, *d;
        SCIRes  scScript, scHeap;

        if (!LoadResource (&scClassIndex)) {
           printf ("Could not load class index (vocab 996).\n");
           exit (1);
        }
        scClasses = scClassIndex.realSize >> 2;
        for (class = 0; class < scClasses; class++) {
           scClassData [class].name = NULL;
           scClassData [class].props = 0;
           scClassData [class].propDict = NULL;
           scriptNum = get_lsb16 ((scClassIndex.buffer) + (class<<2) +2);
           scScript.rType = resTypeScript;
           scScript.rNum  = scriptNum;
           scHeap.rType   = resTypeHeap;
           scHeap.rNum    = scriptNum;
           if (!LoadResource (&scScript)) {
//             printf ("Warning: Could not load script %d for class %d.\n", scriptNum, class);
             continue;
           }
           if (!LoadResource (&scHeap)) {
             printf ("Warning: Could not load heap %d for class %d.\n", scriptNum, class);
             continue;
           }
           classFound = 0;
           heap = scHeap.buffer;
           heap += 2;           // skip pointer to relocation table
           heap += (get_lsb16 (heap) << 1) +2;  // skip local variables
           while (get_lsb16 (heap) == 0x1234) {
              propDictSize = (get_lsb16 (heap+6) - get_lsb16 (heap+4));
              if (propDictSize) {
                 // if this is a class rather than an object
                 if (get_lsb16 (heap+10) == class) {
                    // if this is our class
                    // copy property dictionary
                    s = get_lsb16 (heap+4) + scScript.buffer;
                    t = malloc (propDictSize);
                    d = malloc (propDictSize);
                    if ((!t) || (!d)) {
                       printf ("Not enough memory for props in class %d, script %d.\n", class, scriptNum);
                       exit (1);
                    }
                    scClassData [class].props = propDictSize >> 1;
                    scClassData [class].propDict = t;
                    scClassData [class].propDefault = d;
                    for (p = 0; p < scClassData [class].props; p++) {
                      *t++ = get_lsb16 (s);
                      *d++ = get_lsb16 (heap + (p<<1));
                      s += 2;
                    }
                    
					if (scClassData [class].props > 8)
					{
						// copy class name
					
						scClassName = get_lsb16 (heap+16) + scHeap.buffer;
						classNameLen = strlen (scClassName);
						scClassData [class].name = malloc (classNameLen+1);
						strcpy (scClassData [class].name, scClassName);
						classFound = 1;
					}
					
                 }
              }
              // skip object
              heap += (get_lsb16 (heap+2) << 1);
           }
           if (!classFound)
              printf ("Warning: Could not find class %d in script %d\n", class, scriptNum);
           free (scHeap.buffer);
           free (scScript.buffer);
        }
}

void    initSelectors () {
        int     x, len;
        uint8  *i, *s, *t;
        FILE   *f;
        FILE   *selnames;

        if (!LoadResource (&scSelectorVocab)) {
           f = fopen ("997.voc", "rb");
           if (!f) {
             printf ("Could not load selector strings (vocab 997).\n");
             exit (1);
           }
           scSelectorVocab.buffer = malloc (65535);
           fseek (f, 2, SEEK_SET);
           fread (scSelectorVocab.buffer, 1, 65535, f);
           fclose (f);
        }
        selnames = fopen ("sels.ini", "wb");
        i = scSelectorVocab.buffer + 2;
        scSelectors = get_lsb16 (i-2) +1;
        for (x = 0; x < scSelectors; x++) {
          t = scSelectorVocab.buffer + get_lsb16 (i);
          if (t [1] == '\0') {
            len = get_lsb16 (t);
            s = t + 2;
            memmove (t, s, len);
            t [len] = '\0';
          }
          scSelectorName [x] = t;
          fprintf (selnames, "%03d=%s\n", x, scSelectorName [x]);
          i += 2;
        }
        fclose (selnames);
}

void    initOpcodes () {
        int     x, len;
        uint8  *i, *s, *t;
        FILE   *f;

        if (!LoadResource (&scOpcodeVocab)) {
           f = fopen ("998.voc", "rb");
           if (!f) {
             printf ("Could not load opcode names (vocab 998).\n");
             exit (1);
           }
           scOpcodeVocab.buffer = malloc (65535);
           fseek (f, 2, SEEK_SET);
           fread (scOpcodeVocab.buffer, 1, 65535, f);
           fclose (f);
        }
        i = scOpcodeVocab.buffer + 2;
        scOpcodes = get_lsb16 (i-2) << 1;
        for (x = 0; x < scOpcodes; x+=2) {
          t = scOpcodeVocab.buffer + get_lsb16 (i);
          if (t [1] == '\0') {
            len = get_lsb16 (t) - 2;
            s = t + 4;
            memmove (t, s, len);
            t [len] = '\0';
          }
          scOpcodeName [x+0] = t;
          scOpcodeName [x+1] = t;
          i += 2;
        }
}

void    init () {
        if (!LoadResourceMap()) {
           printf ("Could not load resource.map.\n");
           exit (1);
        }
        DetectSCIFormats ();
        initGlobals ();
        initKernels ();
        initSelectors ();
        initOpcodes ();
        initClasses ();
}

int     scrIsReloc (int scriptofs) {
        uint8  *reloc;
        int     relocs;

        reloc  = dasmScript.buffer;
        reloc += get_lsb16 (reloc);
        relocs = get_lsb16 (reloc); reloc +=2;
        while (relocs) {
          if (get_lsb16 (reloc) == scriptofs) return (1);
          relocs--;
          reloc +=2;
        }
        return (0);
}

int     hepIsReloc (int heapofs) {
        uint8  *reloc;
        int     relocs;

        reloc  = dasmHeap.buffer;
        reloc += get_lsb16 (reloc);
        relocs = get_lsb16 (reloc); reloc +=2;
        while (relocs) {
          if (get_lsb16 (reloc) == heapofs) return (1);
          relocs--;
          reloc +=2;
        }
        return (0);
}

void    disasmHeap () {
        uint8  *heap, *script, *exports, *expObj, *propData, *propDict;
        uint8  *methData;
        int     numExports, export, heappos, class, super, props, prop, isClass;
        int     numLocals, localvar, l, propValue, numMethods, propSel;
        char   *objectName;

        heap = dasmHeap.buffer;
        script = dasmScript.buffer;

        // do exports
        exports = script +8;
        numExports = get_lsb16 (script+6);
        if (numExports) fprintf (outHeap, "exports {\n");
        for (l = 0; l < numExports; l++) {
           export = get_lsb16 (exports);
           if (!export) {
              fprintf (outHeap,"   empty\n");
           }
           else
           if (scrIsReloc (exports-script)) {
              expObj = heap + export;
              if (get_lsb16 (expObj+4) == get_lsb16 (expObj+6))
                fprintf (outHeap, "   object %s\n", heap + get_lsb16 (expObj+16));
              else
                fprintf (outHeap, "   class  %s\n", heap + get_lsb16 (expObj+16));
           } else {
              fprintf (outHeap,   "   proc   proc%03d_%04x\n", dasmNum, export);
              sprintf (labelTemp, "proc%03d_%04x", dasmNum, export);
              addCodeLabel (export, labelTemp);
           }
           exports += 2;
        }
        if (numExports) fprintf (outHeap, "}\n");

        // do locals
        numLocals = get_lsb16 (heap+2);
        if (numLocals) fprintf (outHeap, "\nlocals {\n");
        for (l = 0; l < numLocals; l++) {
           localvar = get_lsb16 (heap + 4 + (l*2));
           fprintf (outHeap, "   local%03d\x09%d\n", l, localvar);
        }
        if (numLocals) fprintf (outHeap, "}\n");

        // do objects
        heappos = (numLocals<<1)+4;
        while (get_lsb16 (heap+heappos) == 0x1234) {
           if (get_lsb16 (heap+heappos+4) == get_lsb16 (heap+heappos+6)) {
              fprintf (outHeap, "\ninstance ");
              class = get_lsb16 (heap+heappos+12);
              super = get_lsb16 (heap+heappos+12);
              isClass = 0;
           } else {
              fprintf (outHeap, "\nclass ");
              class = get_lsb16 (heap+heappos+10);
              super = get_lsb16 (heap+heappos+12);
              if (super == 0xffff) super = 0;
              isClass = 1;
           }
		   if (get_lsb16 (heap+heappos+2) <9)
		   {
			   heappos += get_lsb16 (heap+heappos+2) << 1;
			   fprintf (outHeap, "{}\n");
			   continue;
		   }

		   objectName=heap + get_lsb16 (heap+heappos+16);
		   fprintf (outHeap, "%s of %s {\n", objectName, scClassData [super].name);

           // do properties
           props = scClassData [class].props;
           for (prop = 9; prop < props; prop++) {
              propData = heap + heappos + (prop << 1);
              propValue = get_lsb16 (propData);
              propSel = scClassData [class].propDict [prop];
              if ((allProps) || (isClass) || (propValue != scClassData [class].propDefault [prop])) {
                fprintf (outHeap, "   prop   %-16s ", scSelectorName [propSel]);
                if (hepIsReloc (propData-heap)) {
                   if (get_lsb16 (heap+propValue) == 0x1234) {
                      expObj = heap + get_lsb16 (heap+propValue+16);
                      fprintf (outHeap, "object %s\n", heap + get_lsb16 (expObj + 16));
                   } else
                      fprintf (outHeap, "string %s\n", heap + propValue);
                } else
                    fprintf (outHeap, "%d\n", propValue);
              }
           }
           // do methods
           methData = script + get_lsb16 (heap+heappos+6);
           numMethods = get_lsb16 (methData); methData +=2;
           while (numMethods--) {
             fprintf (outHeap, "   method %s\n", scSelectorName [get_lsb16 (methData)]);
             sprintf (labelTemp, "%s.%s", objectName, scSelectorName [get_lsb16 (methData)]);
             addCodeLabel (get_lsb16 (methData+2), labelTemp);
             methData += 4;
           }
           // skip object
           heappos += get_lsb16 (heap+heappos+2) << 1;
           fprintf (outHeap, "}\n");
        }
};

int     scanCode () {
        uint8  *script, opcode;
        int     spos, send, snext, jumppos, len;
        int16   argument1;

        script = dasmScript.buffer;
        spos = codeStart;
        snext= spos;
//        printf ("scanCode starts at %x\n", spos);
        send = get_lsb16 (script);      // start of relocation table
        while (snext < send) {
          spos = snext;
          opcode = script [spos];
          snext = spos + opcodeLength [opcode];

          if (opcode == 0x7d) {
             len = strlen (script+spos+1);
             snext = spos + len + 1 +1; //+opcode + 00 character
             continue;
          }
          if (opcode == 0x7e) {
             snext = spos + 3;
             continue;
          }
          if ((opcode & 1) || (opcodeLength [opcode] < 3)) {
             argument1 = script [spos+1];
             if (argument1 >= 0x80) argument1 = - (0x100 - argument1);
          } else
             argument1 = get_lsb16 (&script [spos+1]);
          if (opcodeLength [opcode] == 0) {
             printf ("ScanCode: Invalid opcode %02x at script offset %04x\n", opcode, spos);
             exit (1);
          }

          if (memchr (opcodeRel8 , opcode, sizeof (opcodeRel8))) {
             jumppos = snext + argument1;
             sprintf (labelTemp, "@L%04x", jumppos);
             addCodeLabel (jumppos, labelTemp);
          } else
          if (memchr (opcodeRel16, opcode, sizeof (opcodeRel16))) {
             jumppos = snext + argument1;
             sprintf (labelTemp, "@L%04x", jumppos);
             addCodeLabel (jumppos, labelTemp);
          }
        }
		return 0;
}

int     getLabelClass (char *label) {
        char    objName [256], *dot;
        uint8  *heap;
        uint32  heappos, class;

        dot = strchr (label, '.');
        if (!dot) return (0);
        strncpy (objName, label, dot-label);
        objName [dot-label] = '\0';
        heap = dasmHeap.buffer;
        heap += 2;           // skip pointer to relocation table
        heap += (get_lsb16 (heap) << 1) +2;  // skip local variables
        while (get_lsb16 (heap) == 0x1234) {
          if (!strcmp (objName, dasmHeap.buffer + (get_lsb16 (heap+16)))) {
             if (get_lsb16 (heap+4) == get_lsb16 (heap+6))
               class = get_lsb16 (heap+12);
             else
               class = get_lsb16 (heap+10);
             return (class);
          }
          // skip object
          heap += (get_lsb16 (heap+2) << 1);
        }
        return (-1);
}

void    disassemble () {
        uint8  *script, opcode, *label, *heap, *objStr;
        int     spos, send, snext, jumppos, args, len;
        int16   argument1, argument2, argument3;
        uint16  uarg1;
        int     currentClass, propSel, c;

        heap = dasmHeap.buffer;
        script = dasmScript.buffer;
        spos = codeStart;
        snext= spos;
        currentClass = -1;
        send = get_lsb16 (script);      // start of relocation table
//        printf ("disassemble starts at %x\n", spos);
        while (snext < send) {
          spos = snext;
//          printf ("%x\n", spos);
          opcode = script [spos];
          snext = spos + opcodeLength [opcode];

          label = isLabelHere (spos);
          if (label) {
            if (label [0] != '@') {
              fprintf (outScript, "\n");
              currentClass = getLabelClass (label);
//              fprintf (outScript, "current class: %s (%d)\n", currentClass, scClassData [currentClass].name);
              if (currentClass == -1) {
                fprintf (outScript, "strange: class = -1, label was %s\n", label);
                currentClass = -1;
              }
            }
            fprintf (outScript, "%s:\n", label);
          }

          if (opcode == 0x7d) {
             len = strlen (script+spos+1);
             snext = spos + len + 1 +1; //+opcode + 00 character
             fprintf (outScript, "; now in source file: %s\n", script+spos+1);
             continue;
          } else
          if (opcode == 0x7e) {
             snext = spos + 3;
             fprintf (outScript, "; source file line %d\n", get_lsb16 (script+spos+1));
             continue;
          } else
          if (opcode == 0x46) {
             argument1 = get_lsb16 (&script [spos+1]);
             uarg1 = argument1 & 0xffff;
             argument2 = get_lsb16 (&script [spos+3]);
             argument3 = script [spos+5];
          } else
          if (opcode == 0x47) {
             argument1 = script [spos+1];
             uarg1 = argument1;
             argument2 = script [spos+2];
#ifdef _SCI11
             argument3 = script [spos+3];
#else
			 argument3 = get_lsb16 (&script [spos+3]);
#endif
          } else
          if ((opcode & 1) || (opcodeLength [opcode] < 3)) {
             argument1 = script [spos+1];
             uarg1 = argument1 & 0xff;
             if (argument1 >= 0x80) argument1 = - (0x100 - argument1);
#ifdef _SCI11
             argument2 = script [spos+2];
             argument3 = script [spos+3];
#else
			 argument2 = get_lsb16 (&script [spos+2]);
             argument3 = get_lsb16 (&script [spos+4]);
#endif
          } else {
             argument1 = get_lsb16 (&script [spos+1]);
             uarg1 = argument1 & 0xffff;
#ifdef _SCI11
             argument2 = script [spos+3];
             argument3 = script [spos+4];
#else
			 argument2 = get_lsb16 (&script [spos+3]);
             argument3 = get_lsb16 (&script [spos+5]);
#endif
          }
          args = opcodeArgs [opcode];
          if (opcodeLength [opcode] == 0) {
             printf ("Disasm: Invalid opcode %02x at script offset %04x\n", opcode, spos);
             exit (1);
          }
          fprintf (outScript, "        %-08s", scOpcodeName [opcode]);
          if (memchr (opcodeRel8 , opcode, sizeof (opcodeRel8))) {
             jumppos = snext + argument1;
             label = isLabelHere (jumppos);
             if (label) fprintf (outScript, "%s", label); else fprintf (outScript, "%04x", jumppos);
             if (args > 1) fprintf (outScript, ", %d", argument2);            
          } else
          if (memchr (opcodeRel16, opcode, sizeof (opcodeRel16))) {
             jumppos = snext + argument1;
             label = isLabelHere (jumppos);
             if (label) fprintf (outScript, "%s", label); else fprintf (outScript, "%04x", jumppos);
             if (args > 1) fprintf (outScript, ", %d", argument2);
          } else
          if (((opcode&0xfe) == 0x38) && (argument1 > 0)) {
             // pushi
		     fprintf (outScript, "%d\x09; $%03x", argument1, argument1);
             if (argument1 < scSelectors) 
				if (strcmp(scSelectorName [argument1], "BAD SELECTOR"))
					fprintf (outScript, "  %s", scSelectorName [argument1]); 
          } else
          if ((opcode&0xfe) == 0x50) {
             // class
             fprintf (outScript, "%s", scClassData [uarg1].name);
          } else
          if (((opcode&0xfe) == 0x72) || (opcode&0xfe == 0x74)) {
             // lofsa/lofss
             if (get_lsb16 (heap+argument1) == 0x1234) {
                fprintf (outScript, "object ");
                objStr = heap + get_lsb16 (heap+argument1+16); 
                fprintf (outScript, "%s", objStr);
             } else {
                fprintf (outScript, "string ");
                objStr = heap + argument1;
                fprintf (outScript, "\"");
                for (c=0; c < strlen (objStr); c++) {
                  if (objStr [c] == 10)
                    fprintf (outScript, "\\n");
                  else
                    fprintf (outScript, "%c", oem2ansi[objStr [c]]);
                }
                fprintf (outScript, "\"");
             }
          } else
          if ((opcode >= 0x62) && (opcode <= 0x71) && (currentClass != -1)){
             // property opcode
             propSel = scClassData [currentClass].propDict [uarg1 >> 1];
             fprintf (outScript, "%s    ; %d, class was %s", scSelectorName [propSel], uarg1, scClassData [currentClass].name);
          } else
          if ((opcode >= 0x80) && ((opcode & 0x6) == 0)) {
             // global variable
             fprintf (outScript, "%s", scGlobalName [uarg1]);
          } else
          if ((opcode >= 0x80) && ((opcode & 0x6) == 2)) {
             // local variable
             fprintf (outScript, "local%03d", uarg1);
          } else
          if ((opcode&0xfe) == 0x46) {
             // calle
             fprintf (outScript, "%u, %d, %d", uarg1, argument2, argument3);
          } else
          if ((opcode&0xfe) == 0x42) {
             // callk
             argument1 &= 0xff;
             fprintf (outScript, "%s, %d", scKernelName [uarg1], argument2);
          } else
          if ((opcode&0xfe) == 0x56) {
             // super
             fprintf (outScript, "%s, %d", scClassData [uarg1].name, argument2);
          } else 
          if ((opcode&0xfe) == 0x3e) {
             // link
             fprintf (outScript, "%u", uarg1);
          } else {
             // generic command
             if (args > 0)
               if (opcode >= 0x80)
                fprintf (outScript, "%d", uarg1);
               else
                fprintf (outScript, "%d", argument1);
             if (args > 1) fprintf (outScript, ", %d", argument2);
             if (args > 2) fprintf (outScript, ", %d", argument3);
          }
          fprintf (outScript, "\n");
        }
}

int     main (int argc, char **argv) {
        int     x, oldCodeStart;
        char    nameHeap [32];
        char    nameScript [32];
        char    npatchHeap [32];
        char    npatchScript [32];
        FILE   *patchHeap;
        FILE   *patchScript;

        if (argc <= 1) {
           printf ("No script num specified.\n");
           exit (1);
        }
        init ();

        dasmNum = strtol (argv [1], NULL, 0);
        dasmScript.rNum = dasmNum;
        dasmHeap.rNum = dasmNum;

        sprintf (npatchHeap,   "%d.hep", dasmNum);
        sprintf (npatchScript, "%d.scr", dasmNum);

        patchScript = fopen (npatchScript, "rb");
        if (patchScript) {
           dasmScript.buffer = malloc (65536);
           fread (dasmScript.buffer, 1, 2, patchScript);
           fread (dasmScript.buffer, 1, 65536, patchScript);
           fclose (patchScript);
           printf ("Used patch script file %s!\n", npatchScript);
        } else if (!LoadResource (&dasmScript)) {
           printf ("Could not load script %d.\n", dasmNum);
           exit (1);
        }

        patchHeap = fopen (npatchHeap, "rb");
        if (patchHeap) {
           dasmHeap.buffer = malloc (65536);
           fread (dasmHeap.buffer, 1, 2, patchHeap);
           fread (dasmHeap.buffer, 1, 65536, patchHeap);
           fclose (patchHeap);
           printf ("Used patch heap file %s!\n", npatchHeap);
        } else if (!LoadResource (&dasmHeap)) {
           printf ("Could not load heap %d.\n", dasmNum);
           exit (1);
        }

        sprintf (nameHeap,   "%d.sch", dasmNum);
        sprintf (nameScript, "%d.scs", dasmNum);
        outHeap   = fopen (nameHeap  , "w");
        outScript = fopen (nameScript, "w");

        disasmHeap ();
        oldCodeStart = 65534;
        while (oldCodeStart != codeStart) {
          oldCodeStart = codeStart;
          scanCode ();
        }
        disassemble ();
        fclose (outScript);
        fclose (outHeap);
}
