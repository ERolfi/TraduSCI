 uint8   opcodeRel8  [4] = {
                0x2f,   // bt
                0x31,   // bnt
                0x33,   // jmp
                0x41    // call
};

uint8   opcodeRel16 [4] = {
                0x2e,   // bt
                0x30,   // bnt
                0x32,   // jmp
                0x40    // call
};

#ifdef _SCI11
uint8   opcodeLength [256] =  {
                1,      // 00 bnot
                1,      // 01 bnot
                1,      // 02 add
                1,      // 03 add
                1,      // 04 sub
                1,      // 05 sub
                1,      // 06 mul
                1,      // 07 mul
                1,      // 08 div
                1,      // 09 div
                1,      // 0a mod
                1,      // 0b mod
                1,      // 0c shr
                1,      // 0d shr
                1,      // 0e shl
                1,      // 0f shl
                1,      // 10 xor
                1,      // 11 xor
                1,      // 12 and
                1,      // 13 and
                1,      // 14 or
                1,      // 15 or
                1,      // 16 neg
                1,      // 17 neg
                1,      // 18 not
                1,      // 19 not
                1,      // 1a eq?
                1,      // 1b eq?
                1,      // 1c ne?
                1,      // 1d ne?
                1,      // 1e gt?
                1,      // 1f gt?
                1,      // 20 ge?
                1,      // 21 ge?
                1,      // 22 lt?
                1,      // 23 lt?
                1,      // 24 le?
                1,      // 25 le?
                1,      // 26 ugt?
                1,      // 27 ugt?
                1,      // 28 uge?
                1,      // 29 uge?
                1,      // 2a ult?
                1,      // 2b ult?
                1,      // 2c ule?
                1,      // 2d ule?
                3,      // 2e bt W
                2,      // 2f bt B
                3,      // 30 bnt W
                2,      // 31 bnt B
                3,      // 32 jmp W
                2,      // 33 jmp B
                3,      // 34 ldi W
                2,      // 35 ldi B
                1,      // 36 push 
                1,      // 37 push 
                3,      // 38 pushi W
                2,      // 39 pushi B
                1,      // 3a toss 
                1,      // 3b toss 
                1,      // 3c dup  
                1,      // 3d dup  
                3,      // 3e link W
                2,      // 3f link B
                4,      // 40 call W B
                3,      // 41 call B B
                4,      // 42 callk W B
                3,      // 43 callk B B
                4,      // 44 callb W B
                3,      // 45 callb B B
                6,      // 46 calle W W B
                4,      // 47 calle B B B
                1,      // 48 ret
                1,      // 49 ret
                2,      // 4a send B
                2,      // 4b send B
                0,      // 4c ?    
                0,      // 4d ?    
                0,      // 4e ?    
                0,      // 4f ?    
                3,      // 50 class W
                2,      // 51 class B
                0,      // 52 ?    
                0,      // 53 ?    
                2,      // 54 self B
                2,      // 55 self B
                4,      // 56 super W B
                3,      // 57 super B B
                3,      // 58 &rest W
                2,      // 59 &rest B
                5,      // 5a lea W W
                3,      // 5b lea B B
                1,      // 5c selfID
                1,      // 5d selfID
                0,      // 5e ?    
                0,      // 5f ?    
                1,      // 60 pprev
                1,      // 61 pprev
                3,      // 62 pToa W
                2,      // 63 pToa B
                3,      // 64 aTop W
                2,      // 65 aTop B
                3,      // 66 pTos W
                2,      // 67 pTos B
                3,      // 68 sTop W
                2,      // 69 sTop B
                3,      // 6a ipToa W
                2,      // 6b ipToa B
                3,      // 6c dpToa W
                2,      // 6d dpToa B
                3,      // 6e ipTos W
                2,      // 6f ipTos B
                3,      // 70 dpTos W
                2,      // 71 dpTos B
                3,      // 72 lofsa W
                2,      // 73 lofsa B
                3,      // 74 lofss W
                2,      // 75 lofss B
                1,      // 76 push0
                1,      // 77 push0
                1,      // 78 push1
                1,      // 79 push1
                1,      // 7a push2
                1,      // 7b push2
                1,      // 7c pushSelf
                1,      // 7d pushSelf
                0,      // 7e ?
                0,      // 7f ?
                3,      // 80
                2,      // 81
                3,      // 82
                2,      // 83
                3,      // 84
                2,      // 85
                3,      // 86
                2,      // 87
                3,      // 88
                2,      // 89
                3,      // 8a
                2,      // 8b
                3,      // 8c
                2,      // 8d
                3,      // 8e
                2,      // 8f
                3,      // 90
                2,      // 91
                3,      // 92
                2,      // 93
                3,      // 94
                2,      // 95
                3,      // 96
                2,      // 97
                3,      // 99
                2,      // 99
                3,      // 9a
                2,      // 9b
                3,      // 9c
                2,      // 9d
                3,      // 9e
                2,      // 9f
                3,      // a0
                2,      // a1
                3,      // a2
                2,      // a3
                3,      // a4
                2,      // a5
                3,      // a6
                2,      // a7
                3,      // aa
                2,      // a9
                3,      // aa
                2,      // ab
                3,      // ac
                2,      // ad
                3,      // ae
                2,      // af
                3,      // b0
                2,      // b1
                3,      // b2
                2,      // b3
                3,      // b4
                2,      // b5
                3,      // b6
                2,      // b7
                3,      // bb
                2,      // b9
                3,      // ba
                2,      // bb
                3,      // bc
                2,      // bd
                3,      // be
                2,      // bf
                3,      // c0
                2,      // c1
                3,      // c2
                2,      // c3
                3,      // c4
                2,      // c5
                3,      // c6
                2,      // c7
                3,      // cc
                2,      // c9
                3,      // ca
                2,      // cb
                3,      // cc
                2,      // cd
                3,      // ce
                2,      // cf
                3,      // d0
                2,      // d1
                3,      // d2
                2,      // d3
                3,      // d4
                2,      // d5
                3,      // d6
                2,      // d7
                3,      // dd
                2,      // d9
                3,      // da
                2,      // db
                3,      // dc
                2,      // dd
                3,      // de
                2,      // df
                3,      // e0
                2,      // e1
                3,      // e2
                2,      // e3
                3,      // e4
                2,      // e5
                3,      // e6
                2,      // e7
                3,      // ee
                2,      // e9
                3,      // ea
                2,      // eb
                3,      // ec
                2,      // ed
                3,      // ee
                2,      // ef
                3,      // f0
                2,      // f1
                3,      // f2
                2,      // f3
                3,      // f4
                2,      // f5
                3,      // f6
                2,      // f7
                3,      // f8
                2,      // f9
                3,      // fa
                2,      // fb
                3,      // fc
                2,      // fd
                3,      // fe
                2       // ff
};
#endif

#ifdef _SCI32
uint8   opcodeLength [256] =  {
                1,      // 00 bnot
                1,      // 01 bnot
                1,      // 02 add
                1,      // 03 add
                1,      // 04 sub
                1,      // 05 sub
                1,      // 06 mul
                1,      // 07 mul
                1,      // 08 div
                1,      // 09 div
                1,      // 0a mod
                1,      // 0b mod
                1,      // 0c shr
                1,      // 0d shr
                1,      // 0e shl
                1,      // 0f shl
                1,      // 10 xor
                1,      // 11 xor
                1,      // 12 and
                1,      // 13 and
                1,      // 14 or
                1,      // 15 or
                1,      // 16 neg
                1,      // 17 neg
                1,      // 18 not
                1,      // 19 not
                1,      // 1a eq?
                1,      // 1b eq?
                1,      // 1c ne?
                1,      // 1d ne?
                1,      // 1e gt?
                1,      // 1f gt?
                1,      // 20 ge?
                1,      // 21 ge?
                1,      // 22 lt?
                1,      // 23 lt?
                1,      // 24 le?
                1,      // 25 le?
                1,      // 26 ugt?
                1,      // 27 ugt?
                1,      // 28 uge?
                1,      // 29 uge?
                1,      // 2a ult?
                1,      // 2b ult?
                1,      // 2c ule?
                1,      // 2d ule?
                3,      // 2e bt W
                2,      // 2f bt B
                3,      // 30 bnt W
                2,      // 31 bnt B
                3,      // 32 jmp W
                2,      // 33 jmp B
                3,      // 34 ldi W
                2,      // 35 ldi B
                1,      // 36 push 
                1,      // 37 push 
                3,      // 38 pushi W
                2,      // 39 pushi B
                1,      // 3a toss 
                1,      // 3b toss 
                1,      // 3c dup  
                1,      // 3d dup  
                3,      // 3e link W
                2,      // 3f link B
                5,      // 40 call W W
                4,      // 41 call B W
                5,      // 42 callk W W
                4,      // 43 callk B W
                5,      // 44 callb W W
                4,      // 45 callb B W
                7,      // 46 calle W W W
                5,      // 47 calle B B W
                1,      // 48 ret
                1,      // 49 ret
                3,      // 4a send W
                2,      // 4b send B
                0,      // 4c ?    
                0,      // 4d ?    
                0,      // 4e ?    
                0,      // 4f ?    
                3,      // 50 class W
                2,      // 51 class B
                0,      // 52 ?    
                0,      // 53 ?    
                2,      // 54 self B
                2,      // 55 self B
                5,      // 56 super W W
                4,      // 57 super B W
                3,      // 58 &rest W  ILLEGAL
                2,      // 59 &rest B
                5,      // 5a lea W W
                4,      // 5b lea B W
                1,      // 5c selfID
                1,      // 5d selfID
                0,      // 5e ?    
                0,      // 5f ?    
                1,      // 60 pprev
                1,      // 61 pprev
                3,      // 62 pToa W
                2,      // 63 pToa B
                3,      // 64 aTop W
                2,      // 65 aTop B
                3,      // 66 pTos W
                2,      // 67 pTos B
                3,      // 68 sTop W
                2,      // 69 sTop B
                3,      // 6a ipToa W
                2,      // 6b ipToa B
                3,      // 6c dpToa W
                2,      // 6d dpToa B
                3,      // 6e ipTos W
                2,      // 6f ipTos B
                3,      // 70 dpTos W
                2,      // 71 dpTos B
                3,      // 72 lofsa W
                2,      // 73 lofsa B
                3,      // 74 lofss W
                2,      // 75 lofss B
                1,      // 76 push0
                1,      // 77 push0
                1,      // 78 push1
                1,      // 79 push1
                1,      // 7a push2
                1,      // 7b push2
                1,      // 7c pushSelf
                0,      // 7d DEBUG: source name
                0,      // 7e DEBUG: source line number
                0,      // 7f ?
                3,      // 80
                2,      // 81
                3,      // 82
                2,      // 83
                3,      // 84
                2,      // 85
                3,      // 86
                2,      // 87
                3,      // 88
                2,      // 89
                3,      // 8a
                2,      // 8b
                3,      // 8c
                2,      // 8d
                3,      // 8e
                2,      // 8f
                3,      // 90
                2,      // 91
                3,      // 92
                2,      // 93
                3,      // 94
                2,      // 95
                3,      // 96
                2,      // 97
                3,      // 99
                2,      // 99
                3,      // 9a
                2,      // 9b
                3,      // 9c
                2,      // 9d
                3,      // 9e
                2,      // 9f
                3,      // a0
                2,      // a1
                3,      // a2
                2,      // a3
                3,      // a4
                2,      // a5
                3,      // a6
                2,      // a7
                3,      // aa
                2,      // a9
                3,      // aa
                2,      // ab
                3,      // ac
                2,      // ad
                3,      // ae
                2,      // af
                3,      // b0
                2,      // b1
                3,      // b2
                2,      // b3
                3,      // b4
                2,      // b5
                3,      // b6
                2,      // b7
                3,      // bb
                2,      // b9
                3,      // ba
                2,      // bb
                3,      // bc
                2,      // bd
                3,      // be
                2,      // bf
                3,      // c0
                2,      // c1
                3,      // c2
                2,      // c3
                3,      // c4
                2,      // c5
                3,      // c6
                2,      // c7
                3,      // cc
                2,      // c9
                3,      // ca
                2,      // cb
                3,      // cc
                2,      // cd
                3,      // ce
                2,      // cf
                3,      // d0
                2,      // d1
                3,      // d2
                2,      // d3
                3,      // d4
                2,      // d5
                3,      // d6
                2,      // d7
                3,      // dd
                2,      // d9
                3,      // da
                2,      // db
                3,      // dc
                2,      // dd
                3,      // de
                2,      // df
                3,      // e0
                2,      // e1
                3,      // e2
                2,      // e3
                3,      // e4
                2,      // e5
                3,      // e6
                2,      // e7
                3,      // ee
                2,      // e9
                3,      // ea
                2,      // eb
                3,      // ec
                2,      // ed
                3,      // ee
                2,      // ef
                3,      // f0
                2,      // f1
                3,      // f2
                2,      // f3
                3,      // f4
                2,      // f5
                3,      // f6
                2,      // f7
                3,      // f8
                2,      // f9
                3,      // fa
                2,      // fb
                3,      // fc
                2,      // fd
                3,      // fe
                2       // ff
};
#endif


uint8   opcodeArgs [256] =  {
                0,      // 00 bnot
                0,      // 01 bnot
                0,      // 02 add
                0,      // 03 add
                0,      // 04 sub
                0,      // 05 sub
                0,      // 06 mul
                0,      // 07 mul
                0,      // 08 div
                0,      // 09 div
                0,      // 0a mod
                0,      // 0b mod
                0,      // 0c shr
                0,      // 0d shr
                0,      // 0e shl
                0,      // 0f shl
                0,      // 10 xor
                0,      // 11 xor
                0,      // 12 and
                0,      // 13 and
                0,      // 14 or
                0,      // 15 or
                0,      // 16 neg
                0,      // 17 neg
                0,      // 18 not
                0,      // 19 not
                0,      // 1a eq?
                0,      // 1b eq?
                0,      // 1c ne?
                0,      // 1d ne?
                0,      // 1e gt?
                0,      // 1f gt?
                0,      // 20 ge?
                0,      // 21 ge?
                0,      // 22 lt?
                0,      // 23 lt?
                0,      // 24 le?
                0,      // 25 le?
                0,      // 26 ugt?
                0,      // 27 ugt?
                0,      // 28 uge?
                0,      // 29 uge?
                0,      // 2a ult?
                0,      // 2b ult?
                0,      // 2c ule?
                0,      // 2d ule?
                1,      // 2e bt W
                1,      // 2f bt B
                1,      // 30 bnt W
                1,      // 31 bnt B
                1,      // 32 jmp W
                1,      // 33 jmp B
                1,      // 34 ldi W
                1,      // 35 ldi B
                0,      // 36 push 
                0,      // 37 push 
                1,      // 38 pushi W
                1,      // 39 pushi B
                0,      // 3a toss 
                0,      // 3b toss 
                0,      // 3c dup  
                0,      // 3d dup  
                1,      // 3e link W
                1,      // 3f link B
                2,      // 40 call W W
                2,      // 41 call B W
                2,      // 42 callk W W
                2,      // 43 callk B W
                2,      // 44 callb W W
                2,      // 45 callb B W
                3,      // 46 calle W W W
                3,      // 47 calle B B W
                0,      // 48 ret
                0,      // 49 ret
                1,      // 4a send W
                1,      // 4b send B
                0,      // 4c ?    
                0,      // 4d ?    
                0,      // 4e ?    
                0,      // 4f ?    
                1,      // 50 class W
                1,      // 51 class B
                0,      // 52 ?    
                0,      // 53 ?    
                1,      // 54 self B
                1,      // 55 self B
                2,      // 56 super W W
                2,      // 57 super B W
                1,      // 58 &rest W
                1,      // 59 &rest B
                2,      // 5a lea W B
                2,      // 5b lea B B
                0,      // 5c selfID
                0,      // 5d selfID
                0,      // 5e ?    
                0,      // 5f ?    
                0,      // 60 pprev
                0,      // 61 pprev
                1,      // 62 pToa W
                1,      // 63 pToa B
                1,      // 64 aTop W
                1,      // 65 aTop B
                1,      // 66 pTos W
                1,      // 67 pTos B
                1,      // 68 sTop W
                1,      // 69 sTop B
                1,      // 6a ipToa W
                1,      // 6b ipToa B
                1,      // 6c dpToa W
                1,      // 6d dpToa B
                1,      // 6e ipTos W
                1,      // 6f ipTos B
                1,      // 70 dpTos W
                1,      // 71 dpTos B
                1,      // 72 lofsa W
                1,      // 73 lofsa B
                1,      // 74 lofss W
                1,      // 75 lofss B
                0,      // 76 push0
                0,      // 77 push0
                0,      // 78 push1
                0,      // 79 push1
                0,      // 7a push2
                0,      // 7b push2
                0,      // 7c pushSelf
                0,      // 7d DEBUG
                0,      // 7e DEBUG
                0,      // 7f DEBUG
                1,      // 80
                1,      // 81
                1,      // 82
                1,      // 83
                1,      // 84
                1,      // 85
                1,      // 86
                1,      // 87
                1,      // 88
                1,      // 89
                1,      // 8a
                1,      // 8b
                1,      // 8c
                1,      // 8d
                1,      // 8e
                1,      // 8f
                1,      // 90
                1,      // 91
                1,      // 92
                1,      // 93
                1,      // 94
                1,      // 95
                1,      // 96
                1,      // 97
                1,      // 99
                1,      // 99
                1,      // 9a
                1,      // 9b
                1,      // 9c
                1,      // 9d
                1,      // 9e
                1,      // 9f
                1,      // a0
                1,      // a1
                1,      // a2
                1,      // a3
                1,      // a4
                1,      // a5
                1,      // a6
                1,      // a7
                1,      // aa
                1,      // a9
                1,      // aa
                1,      // ab
                1,      // ac
                1,      // ad
                1,      // ae
                1,      // af
                1,      // b0
                1,      // b1
                1,      // b2
                1,      // b3
                1,      // b4
                1,      // b5
                1,      // b6
                1,      // b7
                1,      // bb
                1,      // b9
                1,      // ba
                1,      // bb
                1,      // bc
                1,      // bd
                1,      // be
                1,      // bf
                1,      // c0
                1,      // c1
                1,      // c2
                1,      // c3
                1,      // c4
                1,      // c5
                1,      // c6
                1,      // c7
                1,      // cc
                1,      // c9
                1,      // ca
                1,      // cb
                1,      // cc
                1,      // cd
                1,      // ce
                1,      // cf
                1,      // d0
                1,      // d1
                1,      // d2
                1,      // d3
                1,      // d4
                1,      // d5
                1,      // d6
                1,      // d7
                1,      // dd
                1,      // d9
                1,      // da
                1,      // db
                1,      // dc
                1,      // dd
                1,      // de
                1,      // df
                1,      // e0
                1,      // e1
                1,      // e2
                1,      // e3
                1,      // e4
                1,      // e5
                1,      // e6
                1,      // e7
                1,      // ee
                1,      // e9
                1,      // ea
                1,      // eb
                1,      // ec
                1,      // ed
                1,      // ee
                1,      // ef
                1,      // f0
                1,      // f1
                1,      // f2
                1,      // f3
                1,      // f4
                1,      // f5
                1,      // f6
                1,      // f7
                1,      // f8
                1,      // f9
                1,      // fa
                1,      // fb
                1,      // fc
                1,      // fd
                1,      // fe
                1       // ff
};

