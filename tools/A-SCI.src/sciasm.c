#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "endian.h"
#include "res.h"
#include "opcodes.h"


static const ansi2oem[256] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
	64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
	80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
	96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
	112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126,
	127, 95, 95, 39, 159, 34, 46, 197, 206, 94, 37, 83, 60, 79, 95, 90, 95, 95,
	39, 39, 34, 34, 7, 45, 45, 126, 84, 115, 62, 111, 95, 122, 89, 255, 173,
	189, 156, 207, 190, 221, 245, 249, 184, 166, 174, 170, 240, 169, 238, 248,
	241, 253, 252, 239, 230, 244, 250, 247, 251, 167, 175, 172, 171, 243, 168,
	183, 181, 182, 199, 142, 143, 146, 128, 212, 144, 210, 211, 222, 214, 215,
	216, 209, 165, 227, 224, 226, 229, 153, 158, 157, 235, 233, 234, 154, 237,
	232, 225, 133, 160, 131, 198, 132, 134, 145, 135, 138, 130, 136, 137, 141,
	161, 140, 139, 208, 164, 149, 162, 147, 228, 148, 246, 155, 151, 163, 150,
	129, 236, 231, 152 
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

typedef struct {
        char   *name;
        int     props;
        uint16 *propDict;
        uint16 *propDefault;
} tClass;

typedef struct {
        char   *name;           // the symbol being referred
        uint8  *refsource;      // where referenced from, always 16-bit additive
        int     type;           // target symbol in script or heap
} tRef;

typedef struct {
        char   *name;           // the symbol name
        int     refpos;         // where in the script/heap it is located, 16-bit
        int     type;           // target symbol in script or heap
} tSymbol;

int     asmNum;
int     scSelectors;
uint8  *scSelectorName [0x2000];
int     scOpcodes;
uint8  *scOpcodeName [0x100];
int     scClasses = 0;
tClass  scClassData [256];
int     scReferences = 0;
tRef    scReferenceData [32000];
int     scSymbols = 0;
tSymbol scSymbolData [32000];

char   *scGlobalName [2048];
char   *scLocalName [2048];
char   *scKernelName [256];

char    nameSCH [32];
char    nameSCS [32];
char    nameHEP [32];
char    nameSCR [32];
FILE   *fSCH, *fSCS;
int     lineHeap, lineScript;

uint8  *outStringData, *outHeapData, *outScriptData;
int     outStringPos = 0;
int     outHeapPos = 2;
int     outScriptPos = 6;

uint8  *HeapRelocData, *ScriptRelocData; 
int     HeapRelocs = 0;
int     ScriptRelocs = 0;

int     exportsDone = 0;
int     localsDone = 0;

#define SCRIPT 0
#define HEAP 1


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
          i += 2;
        }
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

int     largc;
char   *largv [32];
char    ldata [2560];

void    skipPastSpaces (char **s) {
        char    *c;
        c = *s;
        while (*c == ' ') c++;
        *s = c;
}

void    skipPastWord (char **s) {
        char    *c;
        c = *s;
        while ((*c != ' ') && (*c != '\0')) {
          c++;
        } 
        *s = c;
}

int     parseLine (FILE *textFile, int *linecounter) {
        char    *s, *t, c;
        int     lc;

        lc = *linecounter;
        largc = 0;
        while (largc == 0) {
          if (!fgets (ldata, 2560, textFile)) {
            *linecounter = lc;
            return (0);
          }
          lc++;
          // replace tabs with spaces
          while (s = strchr (ldata, 9)) *s = ' ';

          // end line at semicolon/13/10
          s = strchr (ldata, ';'); if (s) *s = '\0';
          s = strchr (ldata, 13 ); if (s) *s = '\0';
          s = strchr (ldata, 10 ); if (s) *s = '\0';

          s = ldata; t = ldata;
          skipPastSpaces (&s);
          while (*s != '\0') {
            if (*s == '"') {
              s++;
              largv [largc++] = s;
              while ((*s != '"') && (*s != '\0')) s++;
              if (*s == '\0') break;
              *s = '\0';
              skipPastSpaces (&s);
              continue;
            }
            largv [largc++] = s;
            skipPastWord (&s);
            if (*s == '\0') break;
            *s++ = '\0';
            skipPastSpaces (&s);
          }
        }
       *linecounter = lc;
}

void    addReference (char *name, uint8 *src, int type) {
        int     namelen, i;

        if (scReferences >= 32000) {
          printf ("Too many references!\n");
          exit (1);
        }
        scReferenceData [scReferences].refsource = src;
        scReferenceData [scReferences].type = type;
        namelen = strlen (name);
        scReferenceData [scReferences].name = malloc (namelen+1);
		strcpy (scReferenceData [scReferences].name, name);
        scReferences++;
}

void    addSymbol (char *name, int refpos, int type) {
        int     namelen;

        if (scSymbols >= 32000) {
          printf ("Too many symbols!\n");
          exit (1);
        }
        scSymbolData [scSymbols].refpos = refpos;
        scSymbolData [scSymbols].type = type;
        namelen = strlen (name);
        scSymbolData [scSymbols].name = malloc (namelen+1);
        strcpy (scSymbolData [scSymbols].name, name);
        scSymbols++;
}

void    addString (char *text) {
        int     len, i;

        len = strlen (text);
        strcpy (outStringData + outStringPos, text);
        for (i = 0; i < len; i++) {
          if ((text [0] == '\\') && (text [1] == 'n')) {
            text += 2;
            outStringData [outStringPos++] = 10;
          } else {
            outStringData [outStringPos++] = ansi2oem[(unsigned char)*text++];
          }
        }
        outStringData [outStringPos++] = 0;
}

void    addHeapReloc (int target) {
        put_lsb16 (HeapRelocData + (HeapRelocs<<1), target);
        HeapRelocs++;
}

void    addScriptReloc (int target) {
        put_lsb16 (ScriptRelocData + (ScriptRelocs<<1), target);
        ScriptRelocs++;
}

void    doExports () {
        int     numExports;
        int     pExports;

        if (exportsDone) {
          printf ("%s: line %d: Cannot have more than one export block.\n", nameSCH, lineHeap);
          exit (1);
        }
        if (strcmp (largv [1], "{")) {
          printf ("%s: line %d: Missing '{' parentheses in exports definition.\n", nameSCH, lineHeap);
          exit (1);
        }
        numExports = 0;
        pExports = outScriptPos +2;
        while (parseLine (fSCH, &lineHeap)) {
               if (!strcmp (largv [0], "empty")) {
                 numExports++;
                 put_lsb16 (outScriptData+ pExports, 0);
                 pExports += 2;
               }
          else if (!strcmp (largv [0], "object")) {
                 numExports++;
                 put_lsb16 (outScriptData + pExports, 0);
                 addReference (largv [1], outScriptData + pExports, HEAP);
                 addScriptReloc (pExports);
                 pExports +=2;
          }
          else if (!strcmp (largv [0], "class")) {
                 numExports++;
                 put_lsb16 (outScriptData + pExports, 0);
                 addReference (largv [1], outScriptData + pExports, HEAP);
                 addScriptReloc (pExports);
                 pExports +=2;
          }
          else if (!strcmp (largv [0], "proc")) {
                 numExports++;
                 put_lsb16 (outScriptData + pExports, 0);
                 addReference (largv [1], outScriptData + pExports, SCRIPT);
                 pExports +=2;
          }
          else if (!strcmp (largv [0], "}")) break;
          else {
               printf ("%s line %d: invalid exports keyword: %s\n", nameSCH, lineHeap, largv [0]);
               exit (1);
          }
        }
        put_lsb16 (outScriptData + outScriptPos, numExports);
        outScriptPos = pExports;
        exportsDone = 1;
}

void    doLocals () {
        int     numLocals, namelen, value, pLocals;

        if (localsDone) {
          printf ("%s: line %d: Cannot have more than one local block.\n", nameSCH, lineHeap);
          exit (1);
        }
        if (strcmp (largv [1], "{")) {
          printf ("%s: line %d: Missing '{' parentheses in locals definition.\n", nameSCH, lineHeap);
          exit (1);
        }
        numLocals = 0;
        pLocals = outHeapPos +2;

        while (parseLine (fSCH, &lineHeap)) {
          if (!strcmp (largv [0], "}")) break;
          namelen = strlen (largv [0]);
          scLocalName [numLocals] = malloc (namelen+1);
          strcpy (scLocalName [numLocals], largv [0]);
          if (!strcmp (largv [1], "string")) {
             value = outStringPos;
             addString (largv [2]);
             addReference ("$$$STRINGS", outHeapData + pLocals, HEAP);
             addHeapReloc (pLocals);
          } else {
            value = strtol (largv [1], NULL, 0);
          }
          put_lsb16 (outHeapData + pLocals, value);
          pLocals += 2;
          numLocals++;
        }
        put_lsb16 (outHeapData + outHeapPos, numLocals);
        outHeapPos = pLocals;
        localsDone = 1;
}

int     getClass (char *name) {
        int     x;

        for (x=0; x<scClasses; x++)
          if (!strcmp (scClassData [x].name, name)) return (x);
        return (-1);
}

int     getPropNum (int class, char *propName) {
        int     x, propSel;
        uint16  *propSels;

        propSels = scClassData [class].propDict;
        for (x=0; x<scClassData [class].props; x++) {
          propSel = propSels [x];
          if (!strcmp (scSelectorName [propSel], propName)) return (x);
        }
        return (0);
}

int     getSelectorNum (char *selName) {
        int     x;

        for (x=0; x<scSelectors; x++)
          if (!strcmp (scSelectorName [x], selName)) return (x);
        return (0);
}

void    doInstance () {
        int     class, pObjName, numProps, numMethods, pNumMethods, pMethods;
        int     pProps, x, value, propNum, pString;
        char    methodName [64], *objectName;
        uint16 *propDefault;

        if (!exportsDone) {
          exportsDone = 1;
          put_lsb16 (outScriptData+6, 0);
          outScriptPos = 8;
        }
        if (!localsDone) {
          localsDone = 1;
          put_lsb16 (outHeapData+2, 0);
          outHeapPos = 4;
        }
        if (strcmp (largv [2], "of")) {
          printf ("%s: line %d: Missing 'of' keyword in instance definition.\n", nameSCH, lineHeap);
          exit (1);
        }
        if (strcmp (largv [4], "{")) {
          printf ("%s: line %d: Missing '{' parentheses in instance definition.\n", nameSCH, lineHeap);
          exit (1);
        }
        class = getClass (largv [3]);
        if (class == -1) {
          printf ("%s: line %d: Unknown class: %s\n", nameSCH, lineHeap, largv [3]);
        }
        // add object name to strings
        pObjName = outStringPos;
        addString (largv [1]);
        objectName = outStringData + pObjName;

        // put default property values on heap
        numMethods = 0;
        pNumMethods = outScriptPos;
        pMethods = pNumMethods +2;
        pProps = outHeapPos;
        numProps = scClassData [class].props;
        propDefault = scClassData [class].propDefault;
        for (x = 0; x < numProps; x++) {
          value = propDefault [x];
          put_lsb16 (outHeapData + pProps + (x<<1), value);
        }
        put_lsb16 (outHeapData + pProps + 0x00, 0x1234);
        put_lsb16 (outHeapData + pProps + 0x02, numProps);
        put_lsb16 (outHeapData + pProps + 0x04, pNumMethods);
        put_lsb16 (outHeapData + pProps + 0x06, pNumMethods);
        put_lsb16 (outHeapData + pProps + 0x08, 0);
        put_lsb16 (outHeapData + pProps + 0x0a, 0xffff);
        put_lsb16 (outHeapData + pProps + 0x0c, class);
        put_lsb16 (outHeapData + pProps + 0x0e, 0);
        put_lsb16 (outHeapData + pProps + 0x10, pObjName);
        addReference ("$$$STRINGS", outHeapData + pProps+0x10, HEAP);
        addHeapReloc (pProps + 0x10);

        while (parseLine (fSCH, &lineHeap)) {
               if (!strcmp (largv [0], "}")) break;
          else if (!strcmp (largv [0], "prop")) {
                  propNum = getPropNum (class, largv [1]);
                  if (!propNum) {
                    printf ("%s line %d: '%s' is no property for '%s'\n", nameSCH, lineHeap, largv [1], scClassData [class].name);
                    exit (1);
                  }
                  if (!strcmp (largv [2], "object")) {
                    value = 0;
                    addReference (largv [2], outHeapData + pProps + (propNum<<1), HEAP);
                  } else
                  if (!strcmp (largv [2], "string")) {
                    value = outStringPos;
                    addString (largv [3]);
                    addReference ("$$$STRINGS", outHeapData + pProps + (propNum<<1), HEAP);
                    addHeapReloc (pProps + (propNum<<1));
                  } else
                    value = strtol (largv [2], NULL, 0);
                  put_lsb16 (outHeapData + pProps + (propNum<<1), value);
               }
          else if (!strcmp (largv [0], "method")) {
                  value = getSelectorNum (largv [1]);
                  if (!value) {
                     printf ("%s line %d: '%s' is not a valid method selector.\n", nameSCH, lineHeap, largv [1]);
                  }
                  sprintf (methodName, "%s.%s", objectName, largv [1]);
                  put_lsb16 (outScriptData + pMethods + 0, value);
                  put_lsb16 (outScriptData + pMethods + 2, 0);
                  addReference (methodName, outScriptData + pMethods + 2, SCRIPT);
                  pMethods +=4;
                  numMethods++;
               }
          else {
               printf ("%s line %d: invalid instance keyword: %s\n", nameSCH, lineHeap, largv [0]);
               exit (1);
          }
        }
        addSymbol (objectName, outHeapPos, HEAP);
        outHeapPos += (numProps << 1);
        put_lsb16 (outScriptData + pNumMethods, numMethods);
        outScriptPos = pMethods;
}

void    doClass () {
        int     class, pObjName, numProps, numMethods, pNumMethods, pMethods;
        int     pProps, x, value, propNum, pString, pPropDict, super;
        char    methodName [64], *objectName;
        uint16 *propDefault, *propDict;

        if (!exportsDone) {
          exportsDone = 1;
          put_lsb16 (outScriptData+6, 0);
          outScriptPos = 8;
        }
        if (!localsDone) {
          localsDone = 1;
          put_lsb16 (outHeapData+2, 0);
          outHeapPos = 4;
        }
        if (strcmp (largv [2], "of")) {
          printf ("%s: line %d: Missing 'of' keyword in class definition.\n", nameSCH, lineHeap);
          exit (1);
        }
        if (strcmp (largv [4], "{")) {
          printf ("%s: line %d: Missing '{' parentheses in class definition.\n", nameSCH, lineHeap);
          exit (1);
        }
        class = getClass (largv [1]);
        if (class == -1) {
          printf ("%s: line %d: Unknown class: %s\n", nameSCH, lineHeap, largv [3]);
        }
        super = getClass (largv [3]);
        if (super == -1) {
          printf ("%s: line %d: Unknown class: %s\n", nameSCH, lineHeap, largv [3]);
        }
        // add object name to strings
        pObjName = outStringPos;
        addString (largv [1]);
        objectName = outStringData + pObjName;

        // put property Dictionary on Heap
        pPropDict = outScriptPos;
        numProps = scClassData [class].props;
        propDict = scClassData [class].propDict;
        for (x = 0; x < numProps; x++) {
          value = propDict [x];
          put_lsb16 (outScriptData + pPropDict + (x<<1), value);
        }
        outScriptPos += (numProps << 1);

        // put default property values on heap
        numMethods = 0;
        pNumMethods = outScriptPos;
        pMethods = pNumMethods +2;
        pProps = outHeapPos;
        numProps = scClassData [class].props;
        propDefault = scClassData [class].propDefault;
        for (x = 0; x < numProps; x++) {
          value = propDefault [x];
          put_lsb16 (outHeapData + pProps + (x<<1), value);
        }
        if ((class == 0) && (super == 0)) super = -1;
        put_lsb16 (outHeapData + pProps + 0x00, 0x1234);
        put_lsb16 (outHeapData + pProps + 0x02, numProps);
        put_lsb16 (outHeapData + pProps + 0x04, pPropDict);
        put_lsb16 (outHeapData + pProps + 0x06, pNumMethods);
        put_lsb16 (outHeapData + pProps + 0x08, 0);
        put_lsb16 (outHeapData + pProps + 0x0a, class);
        put_lsb16 (outHeapData + pProps + 0x0c, super);
        put_lsb16 (outHeapData + pProps + 0x0e, 0x8000);
        put_lsb16 (outHeapData + pProps + 0x10, pObjName);
        addReference ("$$$STRINGS", outHeapData + pProps+0x10, HEAP);
        addHeapReloc (pProps + 0x10);

        while (parseLine (fSCH, &lineHeap)) {
               if (!strcmp (largv [0], "}")) break;
          else if (!strcmp (largv [0], "prop")) {
                  propNum = getPropNum (class, largv [1]);
                  if (!propNum) {
                    printf ("%s line %d: '%s' is no property for '%s'\n", nameSCH, lineHeap, largv [1], scClassData [class].name);
                    exit (1);
                  }
                  if (!strcmp (largv [2], "object")) {
                    value = 0;
                    addReference (largv [2], outHeapData + pProps + (propNum<<1), HEAP);
                  } else
                  if (!strcmp (largv [2], "string")) {
                    value = outStringPos;
                    addString (largv [3]);
                    addReference ("$$$STRINGS", outHeapData + pProps + (propNum<<1), HEAP);
                    addHeapReloc (pProps + (propNum<<1));
                  } else
                    value = strtol (largv [2], NULL, 0);
                  put_lsb16 (outHeapData + pProps + (propNum<<1), value);
               }
          else if (!strcmp (largv [0], "method")) {
                  value = getSelectorNum (largv [1]);
                  if (!value) {
                     printf ("%s line %d: '%s' is not a valid method selector.\n", nameSCH, lineHeap, largv [1]);
                  }
                  sprintf (methodName, "%s.%s", objectName, largv [1]);
                  put_lsb16 (outScriptData + pMethods + 0, value);
                  put_lsb16 (outScriptData + pMethods + 2, 0);
                  addReference (methodName, outScriptData + pMethods + 2, SCRIPT);
                  pMethods +=4;
                  numMethods++;
               }
          else {
               printf ("%s line %d: invalid instance keyword: %s\n", nameSCH, lineHeap, largv [0]);
               exit (1);
          }
        }
        addSymbol (objectName, outHeapPos, HEAP);
        outHeapPos += (numProps << 1);
        put_lsb16 (outScriptData + pNumMethods, numMethods);
        outScriptPos = pMethods;
}

void    assembleHeap () {
        int     x;

        lineHeap = 0;
        while (parseLine (fSCH, &lineHeap)) {
               if (!strcmp (largv [0], "exports"))  doExports ();
          else if (!strcmp (largv [0], "locals"))   doLocals ();
          else if (!strcmp (largv [0], "instance")) doInstance ();
          else if (!strcmp (largv [0], "class"))    doClass ();
          else {
               printf ("%s line %d: invalid heap keyword: %s\n", nameSCH, lineHeap, largv [0]);
               exit (1);
               break;
          }
        }

        // end object list
        put_lsb16 (outHeapData + outHeapPos, 0);
        outHeapPos += 2;
}

int     getOpcodeNum (char *opName) {
        int     x;

        for (x=0; x<scOpcodes; x++)
          if (!strcmp (scOpcodeName [x], opName)) return (x);
        return (-1);
}

int     getKernelNum (char *kernelName) {
        int     x;

        for (x=0; x<256; x++)
          if (!strcmp (scKernelName [x], kernelName)) return (x);
        return (-1);
}

int     getGlobalNum (char *globalName) {
        int     x;

        for (x=0; x<2048; x++)
          if (!strcmp (scGlobalName [x], globalName)) return (x);
        return (-1);
}

int     getLocalNum (char *localName) {
        int     x;

        for (x=0; x<2048; x++)
          if (!strcmp (scLocalName [x], localName)) return (x);
        return (-1);
}

void    removeComma (char *str) {
        int     len;

        if (!str) return;
        len = strlen (str);
        if (str [len-1] == ',') str [len-1] = '\0';
}

int     getLabelClass (char *label) {
        char    objName [64], *s;
        int     heappos, namepos, class;

        s = strchr (label, '.');
        if (!s) return (-1);
        strcpy (objName, label);
        objName [s-label] = '\0';       // stop at dot
        heappos = (get_lsb16 (outHeapData +2) << 1) +4;
        //printf ("getLabelClass label %s, objName %s\n", label, objName);
        while (get_lsb16 (outHeapData +heappos) == 0x1234) {
          namepos = get_lsb16 (outHeapData +heappos + 0x10);
          if (!strcmp (outStringData + namepos, objName)) {
             if ((get_lsb16 (outHeapData +heappos +4) == get_lsb16 (outHeapData +heappos +6)))
               class = get_lsb16 (outHeapData +heappos+0x0c);
             else
               class = get_lsb16 (outHeapData +heappos+0x0a);
             return (class);
          } 
          heappos += get_lsb16 (outHeapData +heappos+2) << 1;
        }
        return (-1);
}

void    assembleScript () {
        char   *opcodeName;
        int     pCode, pNext, opcode;
        int     arg1, arg2, arg3, arg4, arg, args, class, c;

        pNext = outScriptPos;
        while (parseLine (fSCS, &lineScript)) {
          pCode = pNext;
          opcodeName = largv [0];
          if (opcodeName [strlen (opcodeName) -1] == ':') {
            // label
            opcodeName [strlen (opcodeName) -1] = '\0'; // remove ':'
            addSymbol (opcodeName, pCode, SCRIPT);
            c = getLabelClass (opcodeName);
            if (c != -1) class = c;
            continue;
          }
          removeComma (largv [1]);
          removeComma (largv [2]);
          removeComma (largv [3]);
          removeComma (largv [4]);
          opcode = getOpcodeNum (largv [0]);
          if (opcode == -1) {
               printf ("%s line %d: invalid opcode: %s\n", nameSCS, lineScript, largv [0]);
               exit (1);
          }
          pNext = pCode + opcodeLength [opcode];
		  if (largv [1])
			arg1 = strtol (largv [1], NULL, 0);
          if (largv [2])
			arg2 = strtol (largv [2], NULL, 0);
          if (largv [3])
			arg3 = strtol (largv [3], NULL, 0);
          if (largv [4])
			arg4 = strtol (largv [4], NULL, 0);
          if ((opcode >= 0x2e) && (opcode <= 0x33)) {
            // relative jump
            outScriptData [pCode+0] = opcode;
            put_lsb16 (outScriptData + pCode + 1, -pNext);
            addReference (largv [1], outScriptData + pCode + 1, SCRIPT);
          } else
          if (opcode == 0x46) {
            // calle
            outScriptData [pCode+0] = opcode;
            put_lsb16 (outScriptData + pCode + 1, arg1);
            put_lsb16 (outScriptData + pCode + 3, arg2);
#ifdef _SCI11
            outScriptData [pCode +5] = arg3;
#else
			put_lsb16 (outScriptData + pCode + 5, arg3);
#endif
          } else
          if (opcode == 0x40) {
            // call
            outScriptData [pCode+0] = opcode;
            put_lsb16 (outScriptData + pCode + 1, -pNext);
            put_lsb16 (outScriptData + pCode + 3, arg2);
            addReference (largv [1], outScriptData + pCode + 1, SCRIPT);
          } else
          if ((opcode >= 0x62) && (opcode <= 0x71)) {
            // property manipulation
            arg1 = getPropNum (class, largv [1]);
            if (!arg1) {
              printf ("%s line %d: '%s' is no property for '%s' (%d)\n", nameSCS, lineScript, largv [1], scClassData [class].name, class);
              exit (1);
            }
            arg1 = arg1 << 1;
            if (abs (arg1) < 256) {
              opcode++;
              pNext = pCode + opcodeLength [opcode];
              outScriptData [pCode+0] = opcode;
              outScriptData [pCode+1] = arg1;
            }
            else {
              outScriptData [pCode+0] = opcode;
              put_lsb16 (outScriptData + pCode+1, arg1);
            }
          } else
          if (opcode == 0x42) {
            // kernel call
            arg1 = getKernelNum (largv [1]);
            if (arg1 == -1) {
              printf ("%s line %d: Invalid kernel call: %s\n", nameSCS, lineScript, largv [1]);
              exit (1);
            }
            opcode++; pNext = pCode + opcodeLength [opcode];
            outScriptData [pCode+0] = opcode;
            outScriptData [pCode+1] = arg1;
            put_lsb16 (outScriptData + pCode + 2, arg2);
          } else
          if (opcode == 0x50) {
            // class
            arg1 = getClass (largv [1]);
            if (arg1 == -1) {
              printf ("%s line %d: Invalid class: %s\n", nameSCS, lineScript, largv [1]);
              exit (1);
            }
            if (abs (arg1) < 256) {
              opcode++;
              pNext = pCode + opcodeLength [opcode];
              outScriptData [pCode+0] = opcode;
              outScriptData [pCode+1] = arg1;
            }
            else {
              outScriptData [pCode+0] = opcode;
              put_lsb16 (outScriptData + pCode+1, arg1);
            }
          } else
          if (opcode == 0x58) {
            // &rest; must be 0x59
            opcode++;
            pNext = pCode + opcodeLength [opcode];
            outScriptData [pCode+0] = opcode;
            outScriptData [pCode + 1] = arg1;
          } else
          if (opcode == 0x5a) {
            // lea
            outScriptData [pCode+0] = opcode;
            put_lsb16 (outScriptData+ pCode + 1, arg1);
            put_lsb16 (outScriptData+ pCode + 3, arg2);
          } else
          if ((opcode == 0x72) || (opcode == 0x74)) {
            // lofsa, lofss
            outScriptData [pCode+0] = opcode;
            if (!strcmp (largv [1], "string")) {
              arg1 = outStringPos;
              addString (largv [2]);
              addReference ("$$$STRINGS", outScriptData + pCode + 1, HEAP);
            } else
            if (!strcmp (largv [1], "object")) {
              arg1 = 0;
              addReference (largv [2], outScriptData + pCode + 1, HEAP);
            } else {
              printf ("%s line %d: Invalid lofs keyword: %s\n", nameSCS, lineScript, largv [1]);
              exit (1);             
            }
            addScriptReloc (pCode + 1);
            put_lsb16 (outScriptData + pCode+1, arg1);
          } else
          if ((opcode == 0x38) || (opcode == 0x34)){
            // pushi, ldi
            if (abs (arg1) < 128) {
              opcode++;
              pNext = pCode + opcodeLength [opcode];
              outScriptData [pCode+0] = opcode;
              outScriptData [pCode + 1] = arg1;
            }
            else {
              outScriptData [pCode+0] = opcode;
              put_lsb16 (outScriptData + pCode+1, arg1);
            }
          } else
          if (opcode == 0x54) {
            // self
            outScriptData [pCode+0] = opcode;
            outScriptData [pCode+1] = arg1;
          } else
          if (opcode == 0x56) {
            // super
            arg1 = getClass (largv [1]);
            if (arg1 == -1) {
              printf ("%s line %d: Invalid class: %s\n", nameSCS, lineScript, largv [1]);
              exit (1);
            }
            if (abs (arg1) < 256) {
              opcode++;
              pNext = pCode + opcodeLength [opcode];
              outScriptData [pCode+0] = opcode;
              outScriptData [pCode+1] = arg1;
              put_lsb16 (outScriptData + pCode+2, arg2);
            }
            else {
              outScriptData [pCode+0] = opcode;
              put_lsb16 (outScriptData + pCode+1, arg1);
              put_lsb16 (outScriptData + pCode+3, arg2);
            }
          } else
          if ((opcode >= 0x80) && ((opcode & 6) == 0)) {
            // global var
            arg1 = getGlobalNum (largv [1]);
            if (arg1 == -1) {
              printf ("%s line %d: Invalid global var: %s\n", nameSCS, lineScript, largv [1]);
              exit (1);
            }
            if (abs (arg1) < 256) {
              opcode++;
              pNext = pCode + opcodeLength [opcode];
              outScriptData [pCode+0] = opcode;
              outScriptData [pCode+1] = arg1;
            }
            else {
              outScriptData [pCode+0] = opcode;
              put_lsb16 (outScriptData + pCode+1, arg1);
            }
          } else
          if ((opcode >= 0x80) && ((opcode & 6) == 2)) {
            // local var
            arg1 = getLocalNum (largv [1]);
            if (arg1 == -1) {
              printf ("%s line %d: Invalid local var: %s\n", nameSCS, lineScript, largv [1]);
              exit (1);
            }
            if (abs (arg1) < 256) {
              opcode++;
              pNext = pCode + opcodeLength [opcode];
              outScriptData [pCode+0] = opcode;
              outScriptData [pCode+1] = arg1;
            }
            else {
              outScriptData [pCode+0] = opcode;
              put_lsb16 (outScriptData + pCode+1, arg1);
            }
          } else
          if (opcode >= 0x80) {
            if (abs (arg1) < 256) {
              opcode++;
              pNext = pCode + opcodeLength [opcode];
              outScriptData [pCode+0] = opcode;
              outScriptData [pCode+1] = arg1;
            }
            else {
              outScriptData [pCode+0] = opcode;
              put_lsb16 (outScriptData + pCode+1, arg1);
            }
          } else  {
              outScriptData [pCode+0] = opcode;
              args = opcodeArgs [opcode];
              if (args < 1) continue;
              outScriptData [pCode+0] = opcode;
              put_lsb16 (outScriptData + pCode+1, arg1);
#ifdef _SCI11
              if (args >= 2) outScriptData [pCode +3] = arg2;
              if (args >= 3) outScriptData [pCode +4] = arg3;
              if (args >= 4) outScriptData [pCode +5] = arg4;
#else
			  if (args >= 2) put_lsb16 (outScriptData + pCode +3, arg2);
              if (args >= 3) put_lsb16 (outScriptData + pCode +5, arg3);
              if (args >= 4) put_lsb16 (outScriptData + pCode +7, arg4);
#endif
          }
        }

        outScriptPos = pNext;

}

void    doFixups () {
        int     r, s, found, value, org;
        uint8  *src;

        // append strings
        addSymbol ("$$$STRINGS", outHeapPos, HEAP);
        memcpy (outHeapData + outHeapPos, outStringData, outStringPos);
        outHeapPos += outStringPos;

        // append heap relocs
        if (outHeapPos & 1) outHeapData [outHeapPos++] = 0;

        put_lsb16 (outHeapData + 0, outHeapPos);
        put_lsb16 (outHeapData + outHeapPos, HeapRelocs);
        outHeapPos += 2;
        memcpy (outHeapData + outHeapPos, HeapRelocData, HeapRelocs<<1);
        outHeapPos += HeapRelocs<<1;

        // append script relocs
        if (outScriptPos & 1) outScriptData [outScriptPos++] = 0;
        put_lsb16 (outScriptData + 0, outScriptPos);
        put_lsb16 (outScriptData + outScriptPos, ScriptRelocs);
        outScriptPos += 2;
        memcpy (outScriptData + outScriptPos, ScriptRelocData, ScriptRelocs<<1);
        outScriptPos += ScriptRelocs<<1;

        for (r=0; r<scReferences; r++) {
          found = 0;
          for (s=0; s<scSymbols; s++) {
            if (!strcmp (scReferenceData [r].name, scSymbolData [s].name)) {
              found = 1;
              break;
            }
          }
          if (!found) {
            printf ("unresolved reference: %s\n", scReferenceData [r].name);
            continue;
          }
          src = scReferenceData [r].refsource;
          value = scSymbolData [s].refpos;
          org = get_lsb16 (src);
          org += value;
          put_lsb16 (src, org);
        }
}
int     main (int argc, char **argv) {
        FILE   *out;
        int     x;
        uint8   resHdr [2];

        if (argc <= 1) {
           printf ("No script num specified.\n");
           exit (1);
        }
        init ();
        asmNum = strtol (argv [1], NULL, 10);
        sprintf (nameSCH, "%d.sch", asmNum);
        sprintf (nameSCS, "%d.scs", asmNum);
        sprintf (nameHEP, "%d.hep", asmNum);
        sprintf (nameSCR, "%d.scr", asmNum);
        fSCH = fopen (nameSCH, "rb");
        if (!fSCH) {
          perror (nameSCH);
          exit (1);
        }
        fSCS = fopen (nameSCS, "rb");
        if (!fSCS) {
          perror (nameSCS);
          exit (1);
        }
        HeapRelocData = malloc (65536);
        ScriptRelocData = malloc (65536);
        outStringData = malloc (65536);
        outHeapData   = malloc (65536);
        outScriptData = malloc (65536);
        put_lsb32 (outScriptData+2, 0);

        assembleHeap ();
        assembleScript ();
        doFixups ();

        out = fopen (nameHEP, "wb");
#ifdef _SCI11
        resHdr [0] = 0x91;
#else
		resHdr [0] = 0x11;
#endif
        resHdr [1] = 0x0;
        fwrite (resHdr, 2, 1, out);
        fwrite (outHeapData, outHeapPos, 1, out);
        fclose (out);
        out = fopen (nameSCR, "wb");
#ifdef _SCI11
        resHdr [0] = 0x82;
#else
		resHdr [0] = 0x02;
#endif
        resHdr [1] = 0x0;
        fwrite (resHdr, 2, 1, out);
        fwrite (outScriptData, outScriptPos, 1, out);
        fclose (out);
}
