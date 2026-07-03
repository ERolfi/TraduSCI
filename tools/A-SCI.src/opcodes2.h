#define sNONE 0
#define sBYTE 1
#define sWORD 2
#define sUBYT 3
#define sSBYT 4
#define sNEXT 5

uint8   opcodeArgType [256][4] = {
                { sNONE, sNONE, sNONE, sNONE },   // 00 bnot
                { sNONE, sNONE, sNONE, sNONE },   // 01 bnot
                { sNONE, sNONE, sNONE, sNONE },   // 02 add
                { sNONE, sNONE, sNONE, sNONE },   // 03 add
                { sNONE, sNONE, sNONE, sNONE },   // 04 sub
                { sNONE, sNONE, sNONE, sNONE },   // 06 mul
                { sNONE, sNONE, sNONE, sNONE },   // 08 div
                { sNONE, sNONE, sNONE, sNONE },   // 0a mod
                { sNONE, sNONE, sNONE, sNONE },   // 0c shr
                { sNONE, sNONE, sNONE, sNONE },   // 0e shl
                { sNONE, sNONE, sNONE, sNONE },   // 10 xor
                { sNONE, sNONE, sNONE, sNONE },   // 12 and
                { sNONE, sNONE, sNONE, sNONE },   // 14 or
                { sNONE, sNONE, sNONE, sNONE },   // 16 neg
                { sNONE, sNONE, sNONE, sNONE },   // 18 not
                { sNONE, sNONE, sNONE, sNONE },   // 1a eq?
                { sNONE, sNONE, sNONE, sNONE },   // 1c ne?
                { sNONE, sNONE, sNONE, sNONE },   // 1e gt?
                { sNONE, sNONE, sNONE, sNONE },   // 20 ge?
                { sNONE, sNONE, sNONE, sNONE },   // 22 lt?
                { sNONE, sNONE, sNONE, sNONE },   // 24 le?
                { sNONE, sNONE, sNONE, sNONE },   // 26 ugt?
                { sNONE, sNONE, sNONE, sNONE },   // 28 uge?
                { sNONE, sNONE, sNONE, sNONE },   // 2a ult?
                { sNONE, sNONE, sNONE, sNONE },   // 2c ule?
                { sSBYT, sNONE, sNONE, sNONE },   // 2e bt W/B
                { sSBYT, sNONE, sNONE, sNONE },   // 30 bnt W/B
                { sSBYT, sNONE, sNONE, sNONE },   // 32 jmp W/B
                { sSBYT, sNONE, sNONE, sNONE },   // 34 ldi W/B
                { sNONE, sNONE, sNONE, sNONE },   // 36 push
                { sSBYT, sNONE, sNONE, sNONE },   // 38 pushi W/B
                { sNONE, sNONE, sNONE, sNONE },   // 3a toss
                { sNONE, sNONE, sNONE, sNONE },   // 3c dup
                { sSBYT, sNONE, sNONE, sNONE },   // 3e link W/B
                { sSBYT, sWORD, sNONE, sNONE },   // 40 call W/B, W
                { sUBYT, sWORD, sNONE, sNONE },   // 42 callk W/B, W
                { sUBYT, sWORD, sNONE, sNONE },   // 44 callb W/B, W
                { sUBYT, sUBYT, sWORD, sNONE },   // 46 calle W/B, W, W
};

