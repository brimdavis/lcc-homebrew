%{
#define INTTMP 0x03f0
#define INTVAR 0x0300
#define FLTTMP 0x0e
#define FLTVAR 0xf0

/* Platform specific assembler directives.  These macros allow  generation
   of the majority of the code that differs between the ARM and GNU assembler
   formats for ARM assembler.  The remainder of differences are dealt with
   on a case-by-case basis within code generation.                           */


#ifdef __riscos__

#define ASM_ALIGN          "\tALIGN\t4\n"
#define ASM_EXPORT         "\tEXPORT\t%s\n"
#define ASM_ALABEL         "%a"
#define ASM_SYMBOL_NUM     "|L..%d|"
#define ASM_SYMBOL_NAME    "|L..%s|"
#define ASM_CONST_WORD     "\tdcd\t"
#define ASM_CONST_BYTE     "\tdcb\t"
#define ASM_COMMENT        "\t; "
#define ASM_SPACE          "\t%%\t"
#define ASM_SDIVIDE        "\t|x$divide|"
#define ASM_UDIVIDE        "\t|x$udivide|"
#define ASM_SREMAINDER     "\t|x$remainder|"
#define ASM_UREMAINDER     "\t|x$uremainder|"


#else

#define ASM_ALIGN          "\t.align\t4\n"
#define ASM_EXPORT         "\t.global\t%s\n"
#define ASM_ALABEL         "%a:"
#define ASM_SYMBOL_NUM     ".L%d"
#define ASM_SYMBOL_NAME    ".L%s"
#define ASM_CONST_WORD     "\t.word\t"
#define ASM_CONST_BYTE     "\t.byte\t"
#define ASM_COMMENT        "\t@ "
#define ASM_SPACE          "\t.space\t"
#define ASM_SDIVIDE        "\t__divsi3"
#define ASM_SREMAINDER     "\t__modsi3"

#endif


#define savelist        (0xd800) /* fp, ip, lr, pc */
#define reslist         (0xa800) /* fp, sp, pc */

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#define readsreg(p) \
        (generic((p)->op) == INDIR && (p)->kids[0]->op == VREG + P)

#define setsrc(d) ((d) && (d)->x.regnode && \
        (d)->x.regnode->set == src->x.regnode->set && \
        (d)->x.regnode->mask&src->x.regnode->mask)

#define relink(a, b) ((b)->x.prev = (a), (a)->x.next = (b))

#include "c.h"

#define NODEPTR_TYPE Node
#define OP_LABEL(p) ((p)->op)
#define LEFT_CHILD(p) ((p)->kids[0])
#define RIGHT_CHILD(p) ((p)->kids[1])
#define STATE_LABEL(p) ((p)->x.state)

static void address(Symbol, Symbol, long);
static void blkfetch(int, int, int, int);
static void blkloop(int, int, int, int, int, int[]);
static void blkstore(int, int, int, int);
static void defaddress(Symbol);
static void defconst(int, int, Value);
static void defstring(int, char *);
static void defsymbol(Symbol);
static void doarg(Node);
static void emit2(Node);
static void export(Symbol);
static void clobber(Node);
static void function(Symbol, Symbol [], Symbol [], int);
static void global(Symbol);
static void import(Symbol);
static void local(Symbol);
static void progbeg(int, char **);
static void progend(void);
static void segment(int);
static void space(int);
static void target(Node);
static int  imm(Node);
static int  imm_const(unsigned);
static int  bitcount(unsigned);
static void arm_blkcopy(int, int, int, int, int);
static void arm_mov(int, int);
static void arm_add(int, int, int);
static char *reglist(unsigned);

static Symbol ireg[32], freg[32];
static Symbol iregw, fregw;
static int cseg;
static int sizeisave;
static int bigargs;

%}
%start stmt
%term CNSTF4=4113 CNSTF8=8209
%term CNSTI1=1045 CNSTI2=2069 CNSTI4=4117
%term CNSTP4=4119
%term CNSTU1=1046 CNSTU2=2070 CNSTU4=4118
%term ARGB=41
%term ARGF4=4129 ARGF8=8225
%term ARGI4=4133
%term ARGP4=4135
%term ARGU4=4134
%term ASGNB=57
%term ASGNF4=4145 ASGNF8=8241
%term ASGNI1=1077 ASGNI2=2101 ASGNI4=4149
%term ASGNP4=4151
%term ASGNU1=1078 ASGNU2=2102 ASGNU4=4150
%term INDIRB=73
%term INDIRF4=4161 INDIRF8=8257
%term INDIRI1=1093 INDIRI2=2117 INDIRI4=4165
%term INDIRP4=4167
%term INDIRU1=1094 INDIRU2=2118 INDIRU4=4166
%term CVFF4=4209 CVFF8=8305
%term CVFI4=4213
%term CVIF4=4225 CVIF8=8321
%term CVII1=1157 CVII2=2181 CVII4=4229
%term CVIU1=1158 CVIU2=2182 CVIU4=4230
%term CVPU4=4246
%term CVUI1=1205 CVUI2=2229 CVUI4=4277
%term CVUP4=4279
%term CVUU1=1206 CVUU2=2230 CVUU4=4278
%term NEGF4=4289 NEGF8=8385
%term NEGI4=4293
%term CALLB=217
%term CALLF4=4305 CALLF8=8401
%term CALLI4=4309
%term CALLP4=4311
%term CALLU4=4310
%term CALLV=216
%term RETF4=4337 RETF8=8433
%term RETI4=4341
%term RETP4=4343
%term RETU4=4342
%term RETV=248
%term ADDRGP4=4359
%term ADDRFP4=4375
%term ADDRLP4=4391
%term ADDF4=4401 ADDF8=8497
%term ADDI4=4405
%term ADDP4=4407
%term ADDU4=4406
%term SUBF4=4417 SUBF8=8513
%term SUBI4=4421
%term SUBP4=4423
%term SUBU4=4422
%term LSHI4=4437
%term LSHU4=4438
%term MODI4=4453
%term MODU4=4454
%term RSHI4=4469
%term RSHU4=4470
%term BANDI4=4485
%term BANDU4=4486
%term BCOMI4=4501
%term BCOMU4=4502
%term BORI4=4517
%term BORU4=4518
%term BXORI4=4533
%term BXORU4=4534
%term DIVF4=4545 DIVF8=8641
%term DIVI4=4549
%term DIVU4=4550
%term MULF4=4561 MULF8=8657
%term MULI4=4565
%term MULU4=4566
%term EQF4=4577 EQF8=8673
%term EQI4=4581
%term EQU4=4582
%term GEF4=4593 GEF8=8689
%term GEI4=4597
%term GEU4=4598
%term GTF4=4609 GTF8=8705
%term GTI4=4613
%term GTU4=4614
%term LEF4=4625 LEF8=8721
%term LEI4=4629
%term LEU4=4630
%term LTF4=4641 LTF8=8737
%term LTI4=4645
%term LTU4=4646
%term NEF4=4657 NEF8=8753
%term NEI4=4661
%term NEU4=4662
%term JUMPV=584
%term LABELV=600
%term LOADB=233
%term LOADF4=4321
%term LOADF8=8417
%term LOADI1=1253
%term LOADI2=2277
%term LOADI4=4325
%term LOADP4=4327
%term LOADU1=1254
%term LOADU2=2278
%term LOADU4=4326
%term VREGP=711
%%
stmt: reg  ""

reg:  INDIRI1(VREGP)     "# read register\n"
reg:  INDIRU1(VREGP)     "# read register\n"

reg:  INDIRI2(VREGP)     "# read register\n"
reg:  INDIRU2(VREGP)     "# read register\n"

reg:  INDIRF4(VREGP)     "# read register\n"
reg:  INDIRI4(VREGP)     "# read register\n"
reg:  INDIRP4(VREGP)     "# read register\n"
reg:  INDIRU4(VREGP)     "# read register\n"

reg:  INDIRF8(VREGP)     "# read register\n"

stmt: ASGNI1(VREGP,reg)  "# write register\n"
stmt: ASGNU1(VREGP,reg)  "# write register\n"

stmt: ASGNI2(VREGP,reg)  "# write register\n"
stmt: ASGNU2(VREGP,reg)  "# write register\n"

stmt: ASGNF4(VREGP,reg)  "# write register\n"
stmt: ASGNI4(VREGP,reg)  "# write register\n"
stmt: ASGNP4(VREGP,reg)  "# write register\n"
stmt: ASGNU4(VREGP,reg)  "# write register\n"

stmt: ASGNF8(VREGP,reg)  "# write register\n"

con: CNSTI1  "%a"
con: CNSTU1  "%a"

con: CNSTI2  "%a"
con: CNSTU2  "%a"

con: CNSTI4  "%a"
con: CNSTU4  "%a"
con: CNSTP4  "%a"

reg: con     "# arbitrary constant\n" 2
reg: ADDRGP4 "\tldr\t%c, [pc, #0]\n\tmov\tpc, pc\n\t.word\t%a\n"  2
reg: ADDRFP4 "# arbitrary offset\n"   2
reg: ADDRLP4 "# arbitrary offset\n"   2

rc:  reg    "%0"
rc:  CNSTI1 " #%a"
rc:  CNSTU1 " #%a"
rc:  CNSTI2 " #%a"  imm(a)
rc:  CNSTU2 " #%a"  imm(a)
rc:  CNSTI4 " #%a"  imm(a)
rc:  CNSTU4 " #%a"  imm(a)
rc:  CNSTP4 " #%a"  imm(a)
rc5: CNSTI1 " #%a"  range(a,1,32)
rc5: CNSTU1 " #%a"  range(a,1,32)
rc5: CNSTI2 " #%a"  range(a,1,32)
rc5: CNSTU2 " #%a"  range(a,1,32)
rc5: CNSTI4 " #%a"  range(a,1,32)
rc5: CNSTU4 " #%a"  range(a,1,32)
rc5: reg    "%0"
rc: LSHI4(reg,rc5)  "%0, asl %1"
rc: LSHU4(reg,rc5)  "%0, lsl %1"
rc: RSHI4(reg,rc5)  "%0, asr %1"
rc: RSHU4(reg,rc5)  "%0, lsr %1"

bra: ADDRGP4  "%a"
bra: con      "%0"

reg: CVII4(reg)  "\tmov\t%c, %0, lsl#8*(4-%a)\n\tmov\t%c, %c, asr#8*(4-%a)\n"  1
reg: CVII4(INDIRI1(reg))  "\tldrb\t%c, [%0, #0]\n\tmov\t%c, %c, lsl#24\n\tmov\t%c, %c, asr#24\n"  2
reg: CVII4(INDIRI2(reg))  "\tldrb\tip, [%0, #1]\n\tldrb\t%c, [%0, #0]\n\tadd\t%c, %c, ip, lsl#8\n\tmov\t%c, %c, lsl#16\n\tmov\t%c, %c, asr#16\n"  8
reg: CVUI4(reg)  "\tand\t%c, %0, #0xff\n"  (a->syms[0]->u.c.v.i == 1 ? 1 : LBURG_MAX)
reg: CVUI4(reg)  "\tbic\t%c, %0, #0xff000000\n\tand\t%c, %c, #0xff0000\n"  2
reg: CVUI4(INDIRU1(reg))  "\tldrb\t%c, [%0, #0]\n"  2
reg: CVUI4(INDIRU2(reg))  "\tldrb\tip, [%0, #1]\n\tldrb\t%c, [%0, #0]\n\tadd\t%c, %c, ip, lsl#8\n"  6
reg: CVUU4(reg)  "\tand\t%c, %0, #0xff\n"  (a->syms[0]->u.c.v.i == 1 ? 1 : LBURG_MAX)
reg: CVUU4(reg)  "\tbic\t%c, %0, #0xff000000\n\tand\t%c, %c, #0xff0000\n"  2
reg: CVUU4(INDIRU1(reg))  "\tldrb\t%c, [%0, #0]\n"  2
reg: CVUU4(INDIRU2(reg))  "\tldrb\tip, [%0, #1]\n\tldrb\t%c, [%0, #0]\n\tadd\t%c, %c, ip, lsl#8\n"  6

reg: CVFF4(reg)  "%0"               notarget(a)
reg: CVFF4(reg)  "\tmvfs\t%c, %0\n" move(a)
reg: CVFF8(reg)  "%0"               notarget(a)
reg: CVFF8(reg)  "\tmvfd\t%c, %0\n" move(a)

reg: CVFI4(reg)  "\tfix\t%c, %0\n"   1
reg: CVIF4(reg)  "\tflts\t%c, %0\n"  1
reg: CVIF8(reg)  "\tfltd\t%c, %0\n"  1

reg: INDIRI1(reg)     "\tldrb\t%c, [%0, #0]\n\tmov\t%c, %c, lsl#24\n\tmov\t%c, %c, asr#24\n"  4
reg: INDIRU1(reg)     "\tldrb\t%c, [%0, #0]\n"  2
reg: INDIRI2(reg)     "\tldrb\tip, [%0, #1]\n\tldrb\t%c, [%0, #0]\n\tadd\t%c, %c, ip, lsl#8\n\tmov\t%c, %c, lsl#16\n\tmov\t%c, %c, asr#16\n"  8
reg: INDIRU2(reg)     "\tldrb\tip, [%0, #1]\n\tldrb\t%c, [%0, #0]\n\tadd\t%c, %c, ip, lsl#8\n"  6
reg: INDIRI4(reg)     "\tldr\t%c, [%0, #0]\n"   2
reg: INDIRU4(reg)     "\tldr\t%c, [%0, #0]\n"   2
reg: INDIRP4(reg)     "\tldr\t%c, [%0, #0]\n"   2
reg: INDIRF4(reg)     "\tldfs\t%c, [%0, #0]\n"  2
reg: INDIRF8(reg)     "\tldfd\t%c, [%0, #0]\n"  2

stmt: ASGNI1(reg,reg)  "\tstrb\t%1, [%0, #0]\n"  2
stmt: ASGNU1(reg,reg)  "\tstrb\t%1, [%0, #0]\n"  2
stmt: ASGNI2(reg,reg)  "\tstrb\t%1, [%0, #0]\n\tmov\tip, %1, lsr#8\n\tstrb\tip, [%0, #1]\n"   5
stmt: ASGNU2(reg,reg)  "\tstrb\t%1, [%0, #0]\n\tmov\tip, %1, lsr#8\n\tstrb\tip, [%0, #1]\n"   5
stmt: ASGNI4(reg,reg)  "\tstr\t%1, [%0, #0]\n"   2
stmt: ASGNU4(reg,reg)  "\tstr\t%1, [%0, #0]\n"   2
stmt: ASGNP4(reg,reg)  "\tstr\t%1, [%0, #0]\n"   2
stmt: ASGNF4(reg,reg)  "\tstfs\t%1, [%0, #0]\n"  2
stmt: ASGNF8(reg,reg)  "\tstfd\t%1, [%0, #0]\n"  2
stmt: ASGNB(reg,INDIRB(reg))  "# asgnb %0 %1\n"        3

reg: LOADI1(reg)  "\tmov\t%c, %0\n"     move(a)
reg: LOADU1(reg)  "\tmov\t%c, %0\n"     move(a)
reg: LOADI2(reg)  "\tmov\t%c, %0\n"     move(a)
reg: LOADU2(reg)  "\tmov\t%c, %0\n"     move(a)
reg: LOADI4(reg)  "\tmov\t%c, %0\n"     move(a)
reg: LOADP4(reg)  "\tmov\t%c, %0\n"     move(a)
reg: LOADU4(reg)  "\tmov\t%c, %0\n"     move(a)
reg: LOADF4(reg)  "\tmvfs\t%c, %0\n"    move(a)
reg: LOADF8(reg)  "\tmvfd\t%c, %0\n"    move(a)

reg: NEGI4(reg)   "\trsb\t%c, %0, #0\n"  1
reg: NEGF4(reg)   "\tmnfs\t%c, %0\n"     1
reg: NEGF8(reg)   "\tmnfd\t%c, %0\n"     1

reg: BCOMI4(rc)      "\tmvn\t%c, %0\n"      1
reg: BCOMU4(rc)      "\tmvn\t%c, %0\n"      1
reg: BANDI4(reg,rc)  "\tand\t%c, %0, %1\n"  1
reg: BANDU4(reg,rc)  "\tand\t%c, %0, %1\n"  1
reg: BORI4(reg,rc)   "\torr\t%c, %0, %1\n"  1
reg: BORU4(reg,rc)   "\torr\t%c, %0, %1\n"  1
reg: BXORI4(reg,rc)  "\teor\t%c, %0, %1\n"  1
reg: BXORU4(reg,rc)  "\teor\t%c, %0, %1\n"  1

reg: LSHI4(reg,rc)  "\tmov\t%c, %0, asl %1\n"  1
reg: LSHU4(reg,rc)  "\tmov\t%c, %0, lsl %1\n"  1
reg: RSHI4(reg,rc)  "\tmov\t%c, %0, asr %1\n"  1
reg: RSHU4(reg,rc)  "\tmov\t%c, %0, lsr %1\n"  1

reg: ADDI4(reg,rc)   "\tadd\t%c, %0, %1\n"   1
reg: ADDP4(reg,rc)   "\tadd\t%c, %0, %1\n"   1
reg: ADDU4(reg,rc)   "\tadd\t%c, %0, %1\n"   1
reg: ADDF4(reg,reg)  "\tadfs\t%c, %0, %1\n"  1
reg: ADDF8(reg,reg)  "\tadfd\t%c, %0, %1\n"  1

reg: SUBI4(reg,rc)   "\tsub\t%c, %0, %1\n"   1
reg: SUBP4(reg,rc)   "\tsub\t%c, %0, %1\n"   1
reg: SUBU4(reg,rc)   "\tsub\t%c, %0, %1\n"   1
reg: SUBF4(reg,reg)  "\tsufs\t%c, %0, %1\n"  1
reg: SUBF8(reg,reg)  "\tsufd\t%c, %0, %1\n"  1

reg: MULI4(reg,reg)  "# mul\n"               1
reg: MULU4(reg,reg)  "# mul\n"               1
reg: MULF4(reg,reg)  "\tfmls\t%c, %0, %1\n"  1
reg: MULF8(reg,reg)  "\tmufd\t%c, %0, %1\n"  1

reg: DIVI4(reg,reg)  "\tbl\t__divsi3\n"    2
reg: DIVU4(reg,reg)  "\tbl\t|x$udivide|\n"   2
reg: DIVF4(reg,reg)  "\tfdvs\t%c, %0, %1\n"  1
reg: DIVF8(reg,reg)  "\tdvfd\t%c, %0, %1\n"  1

reg: MODI4(reg,reg)  "\tbl\t__modsi3\n"    1              
reg: MODU4(reg,reg)  "\tbl\t|x$uremainder|\n"   1                  

stmt: LABELV    "%a:\n"

stmt: JUMPV(bra)  "\tb\t%0\n"        1
stmt: JUMPV(reg)  "\tmov\tpc, %0\n"  1

stmt: EQI4(reg,rc)  "\tcmp\t%0, %1\n\tbeq\t%a\n"   2
stmt: EQU4(reg,rc)  "\tcmp\t%0, %1\n\tbeq\t%a\n"   2
stmt: GEI4(reg,rc)  "\tcmp\t%0, %1\n\tbge\t%a\n"   2
stmt: GEU4(reg,rc)  "\tcmp\t%0, %1\n\tbhs\t%a\n"   2
stmt: GTI4(reg,rc)  "\tcmp\t%0, %1\n\tbgt\t%a\n"   2
stmt: GTU4(reg,rc)  "\tcmp\t%0, %1\n\tbhi\t%a\n"   2
stmt: LEI4(reg,rc)  "\tcmp\t%0, %1\n\tble\t%a\n"   2
stmt: LEU4(reg,rc)  "\tcmp\t%0, %1\n\tbls\t%a\n"   2
stmt: LTI4(reg,rc)  "\tcmp\t%0, %1\n\tblt\t%a\n"   2
stmt: LTU4(reg,rc)  "\tcmp\t%0, %1\n\tblo\t%a\n"   2
stmt: NEI4(reg,rc)  "\tcmp\t%0, %1\n\tbne\t%a\n"   2
stmt: NEU4(reg,rc)  "\tcmp\t%0, %1\n\tbne\t%a\n"   2

stmt: EQF4(reg,reg)  "\tcmf\t%0, %1\n\tbeq\t%a\n"  2
stmt: EQF8(reg,reg)  "\tcmf\t%0, %1\n\tbeq\t%a\n"  2
stmt: LEF4(reg,reg)  "\tcmfe\t%0, %1\n\tble\t%a\n" 2
stmt: LEF8(reg,reg)  "\tcmfe\t%0, %1\n\tble\t%a\n" 2
stmt: LTF4(reg,reg)  "\tcmfe\t%0, %1\n\tblt\t%a\n" 2
stmt: LTF8(reg,reg)  "\tcmfe\t%0, %1\n\tblt\t%a\n" 2
stmt: GEF4(reg,reg)  "\tcmfe\t%0, %1\n\tbge\t%a\n" 2
stmt: GEF8(reg,reg)  "\tcmfe\t%0, %1\n\tbge\t%a\n" 2
stmt: GTF4(reg,reg)  "\tcmfe\t%0, %1\n\tbgt\t%a\n" 2
stmt: GTF8(reg,reg)  "\tcmfe\t%0, %1\n\tbgt\t%a\n" 2
stmt: NEF4(reg,reg)  "\tcmf\t%0, %1\n\tbne\t%a\n"  2
stmt: NEF8(reg,reg)  "\tcmf\t%0, %1\n\tbne\t%a\n"  2

stmt: ARGI4(reg)         "# arg\n"  1
stmt: ARGP4(reg)         "# arg\n"  1
stmt: ARGU4(reg)         "# arg\n"  1
stmt: ARGF4(reg)         "# arg\n"  1
stmt: ARGF8(reg)         "# arg\n"  1
stmt: ARGB(INDIRB(reg))  "# argb %0\n"  1

stmt: CALLV(bra)   "\tbl\t%0\n"  1
reg:  CALLI4(bra)  "\tbl\t%0\n"  1
reg:  CALLP4(bra)  "\tbl\t%0\n"  1
reg:  CALLU4(bra)  "\tbl\t%0\n"  1
reg:  CALLF4(bra)  "\tbl\t%0\n"  1
reg:  CALLF8(bra)  "\tbl\t%0\n"  1

stmt: CALLV(reg)   "\tmov\tlr, pc\n\tmov\tpc, %0\n"  2
reg:  CALLI4(reg)  "\tmov\tlr, pc\n\tmov\tpc, %0\n"  2
reg:  CALLP4(reg)  "\tmov\tlr, pc\n\tmov\tpc, %0\n"  2
reg:  CALLU4(reg)  "\tmov\tlr, pc\n\tmov\tpc, %0\n"  2
reg:  CALLF4(reg)  "\tmov\tlr, pc\n\tmov\tpc, %0\n"  2
reg:  CALLF8(reg)  "\tmov\tlr, pc\n\tmov\tpc, %0\n"  2

stmt: RETV(reg)   "# ret\n"  1
stmt: RETI4(reg)  "# ret\n"  1
stmt: RETP4(reg)  "# ret\n"  1
stmt: RETU4(reg)  "# ret\n"  1
stmt: RETF4(reg)  "# ret\n"  1
stmt: RETF8(reg)  "# ret\n"  1

spill: ADDRLP4  "%a+%F"
stmt: ASGNI1(spill,reg)  "\tldr\tip, [pc, #0]\n\tmov\tpc, pc\n\t.word\t%0\n\tstrb\t%1, [sp, ip]\n"
stmt: ASGNU1(spill,reg)  "\tldr\tip, [pc, #0]\n\tmov\tpc, pc\n\t.word\t%0\n\tstrb\t%1, [sp, ip]\n"
stmt: ASGNI2(spill,reg)  "\tldr\tip, [pc, #0]\n\tmov\tpc, pc\n\t.word\t%0\n\tadd\tip, ip, sp\n\tstr\ta1, [sp, #-4]!\n\tstrb\t%1, [ip, #0]\n\tmov\ta1, %1, lsr#8\n\tstrb\ta1, [ip, #1]\n\tldr\ta1, [sp], #4\n"
stmt: ASGNU2(spill,reg)  "\tldr\tip, [pc, #0]\n\tmov\tpc, pc\n\t.word\t%0\n\tadd\tip, ip, sp\n\tstr\ta1, [sp, #-4]!\n\tstrb\t%1, [ip, #0]\n\tmov\ta1, %1, lsr#8\n\tstrb\ta1, [ip, #1]\n\tldr\ta1, [sp], #4\n"
stmt: ASGNI4(spill,reg)  "\tldr\tip, [pc, #0]\n\tmov\tpc, pc\n\t.word\t%0\n\tstr\t%1, [sp, ip]\n"
stmt: ASGNP4(spill,reg)  "\tldr\tip, [pc, #0]\n\tmov\tpc, pc\n\t.word\t%0\n\tstr\t%1, [sp, ip]\n"
stmt: ASGNU4(spill,reg)  "\tldr\tip, [pc, #0]\n\tmov\tpc, pc\n\t.word\t%0\n\tstr\t%1, [sp, ip]\n"
stmt: ASGNF4(spill,reg)  "\tldr\tip, [pc, #0]\n\tmov\tpc, pc\n\t.word\t%0\n\tadd\tip, ip, sp\n\tstfs\t%1, [ip, #0]\n"
stmt: ASGNF8(spill,reg)  "\tldr\tip, [pc, #0]\n\tmov\tpc, pc\n\t.word\t%0\n\tadd\tip, ip, sp\n\tstfd\t%1, [ip, #0]\n"
%%
static void progend(void) {
#ifdef __riscos__
        print(ASM_ALIGN);
        print("\tEND\n");
#else
        print("\t.ident \"lcc ARM backend\"\n");
#endif
}
static void progbeg(argc, argv) int argc; char *argv[]; {
        int i;

        {
                union {
                        char c;
                        int i;
                } u;
                u.i = 0;
                u.c = 1;
                swap = ((int)(u.i == 1)) != ((int)IR->little_endian);
        }
        parseflags(argc, argv);
        for (i = 0; i < 8; i ++)
                freg[i] = mkreg("f%d", i, 1, FREG);
        for (i = 0; i < 16; i++) {
                if (i < 4)
                        ireg[i] = mkreg(stringf("a%d", i + 1), i, 1, IREG);
                else if (i < 10)
                        ireg[i] = mkreg(stringf("v%d", i - 3), i, 1, IREG);
                else
                        ireg[i] = mkreg("", i, 1, IREG);
        }
        ireg[10]->x.name = "sl";
        ireg[11]->x.name = "fp";
        ireg[12]->x.name = "ip";
        ireg[13]->x.name = "sp";
        ireg[14]->x.name = "lr";
        ireg[15]->x.name = "pc";
        fregw = mkwildcard(freg);
        iregw = mkwildcard(ireg);
        tmask[IREG] = INTTMP; tmask[FREG] = FLTTMP;
        vmask[IREG] = INTVAR; vmask[FREG] = FLTVAR;
}
static Symbol rmap(int opk) {
        switch (optype(opk)) {
        case I: case U: case P: case B:
                return iregw;
        case F:
                return fregw;
        default:
                return 0;
        }
}
static void target(p) Node p; {
        int n;

        assert(p);
        switch (specific(p->op)) {
        case CALL+F:
                setreg(p, freg[0]);     break;
        case CALL+I: case CALL+U: case CALL+P:
                setreg(p, ireg[0]);     break;
        case RET+F:
                rtarget(p, 0, freg[0]); break;
        case RET+I: case RET+U: case RET+P:
                rtarget(p, 0, ireg[0]);
                p->kids[0]->x.registered = 1;
                break;
        case ARG+I: case ARG+U: case ARG+P:
                n = p->x.argno;
                if (n < 4) {
                        rtarget(p, 0, ireg[n]);
                        setreg(p, ireg[n]);
                }
                break;
        case DIV+I: case DIV+U: case MOD+I: case MOD+U:
                setreg(p, ireg[0]);
                rtarget(p, 1, ireg[0]);
                rtarget(p, 0, ireg[1]);
                break;
        }
}
static void clobber(p) Node p; {
        int n;

        assert(p);
        switch (specific(p->op)) {
        case CALL+F: case CALL+I: case CALL+U: case CALL+P: case CALL+V:
                spill(0xf, FREG, p);
                break;
        }
}
static int imm(p) Node p; {
        unsigned long v;
        Symbol s = p->syms[0];

        switch (specific(p->op)) {
                case CNST+I:  v = s->u.c.v.i; break;
                case CNST+U:  v = s->u.c.v.u; break;
                case CNST+P:  if (s->u.c.v.p == 0) return 0;
                default:  return LBURG_MAX;
        }
        return imm_const(v);
}
static int imm_const(v) unsigned v; {
        int i;

        for (i = 0; i < 32; i += 2) {
                v = (v << 2) + (v >> 30);
                if (v < 0x100) return 0;
        }
        return LBURG_MAX;
}
static void emit2(p) Node p; {
        int src, dst, n, tmp;

        switch (specific(p->op)) {
        case ARG+I: case ARG+U: case ARG+P:
                n = p->x.argno;
                if (n >= 4) {
                        src = getregnum(p->x.kids[0]);
                        print("\tstr\t%s, [sp, #%d]\n",
                                ireg[src]->x.name, n * 4 - 16);
                }
                break;
        case ARG+F:
                n = p->x.argno;
                src = getregnum(p->x.kids[0]);
                if (opsize(p->op) == 4) {
                        print("\tstfs\t%s, [sp, #%d]\n",
                                freg[src]->x.name, n * 4 - 16);
                        if (n < 4)
                                print("\tldr\t%s, [sp, #%d]\n",
                                        ireg[n]->x.name, n * 4 - 16);
                } else {
                        print("\tstfd\t%s, [sp, #%d]\n",
                                freg[src]->x.name, n * 4 - 16);
                        if (n < 4)
                                print("\tldr\t%s, [sp, #%d]\n",
                                        ireg[n]->x.name, n * 4 - 16);
                        if (n < 3)
                                print("\tldr\t%s, [sp, #%d]\n",
                                        ireg[n + 1]->x.name,
                                        (n + 1) * 4 - 16);
                }
                break;
        case ARG+B:
                {
                        int size, offset;
                        n = p->syms[RX]->u.c.v.i;
                        size = roundup(p->syms[0]->u.c.v.i, 4);
                        offset = max(16 - n, 0);
                        if (offset < size)
                                arm_blkcopy(13, n - 16 + offset + 16,
                                        getregnum(p->x.kids[0]), offset,
                                        size - offset);
                        if (p->x.argno < 4) {
                                tmp = min(p->x.argno + (size - 1) / 4, 3);
                                if (tmp > p->x.argno)
                                        print("\tldmia\t%s, {%s-%s}\n",
                                                ireg[getregnum(p->x.kids[0])]->x.name,
                                                ireg[p->x.argno]->x.name,
                                                ireg[tmp]->x.name);
                                else
                                        print("\tldr\t%s, [%s, #0]\n",
                                                ireg[p->x.argno]->x.name,
                                                ireg[getregnum(p->x.kids[0])]->x.name);
                        }
                }
                break;
        case ASGN+B:
                arm_blkcopy(getregnum(p->x.kids[0]), 0,
                        getregnum(p->x.kids[1]), 0,
                        p->syms[0]->u.c.v.i);
                break;
        case CNST+I: case CNST+U: case CNST+P:
                dst = getregnum(p);
                arm_mov(dst, p->syms[0]->u.c.v.i);
                break;
        case ADDRF+P:
                arm_add(getregnum(p), 11, 4 - (bigargs ? 0 : sizeisave) +
                        p->syms[0]->x.offset);
                break;
        case ADDRL+P:
                arm_add(getregnum(p), 13, framesize + p->syms[0]->x.offset);
                break;
        case MUL+I: case MUL+U:
                src = getregnum(p->x.kids[0]);
                n = getregnum(p->x.kids[1]);
                dst = getregnum(p);
                if (src == dst && dst == n) {
                        print("\tmov\tip, %s\n", ireg[src]->x.name);
                        src = 12;
                }
                if (src == dst) {
                        tmp = src; src = n; n = tmp;
                }
                print("\tmul\t%s, %s, %s\n", ireg[dst]->x.name,
                        ireg[src]->x.name, ireg[n]->x.name);
                break;
        }
}

static void arm_mov(dest, con) int dest; int con; {
        int inv = bitcount(con) > 16, n = 24;
        unsigned v;

        if (inv) con = ~con;
        while (n > 0 && (con & (0xc0 << n)) == 0)
                n -= 2;
        v = con & (0xff << n);
        if (inv) {
                print("\tmvn\t%s, #%d\n", ireg[dest]->x.name, v);
                con = (~con) - (~v);
        } else {
                print("\tmov\t%s, #%d\n", ireg[dest]->x.name, v);
                con -= v;
        }
        arm_add(dest, dest, con);
}


static void arm_add(dest, src, con) int dest, src; int con; {
        int n = 0;

        if (con == 0 && dest == src)
                return;
        if (con == 0) {
                print("\tmov\t%s, %s\n", ireg[dest]->x.name,
                        ireg[src]->x.name);
                return;
        }
        do {
                for (; (con & 3) == 0; con >>= 2, n += 2);
                if (!(con & 0x100)) {
                        print("\tadd\t%s, %s, #0x%x\n",
                                ireg[dest]->x.name, ireg[src]->x.name,
                                (con & 0xff) << n);
                        con -= con & 0xff;
                } else {
                        print("\tsub\t%s, %s, #0x%x\n",
                                ireg[dest]->x.name, ireg[src]->x.name,
                                (-con & 0xff) << n);
                        con += -con & 0xff;
                }
                src = dest;
        } while (con);
}

static char *reglist(mask) unsigned mask; {
        int i, first = 1;
        static char list[16 * 4 + 1];

        list[0] = '\0';
        for (i = 0; i < 16; i++) {
                if (mask & (1<<i)) {
                        if (first)
                                first = 0;
                        else
                                strcat(list, ", ");
                        strcat(list, ireg[i]->x.name);
                }
        }
        return list;
}


static void doarg(Node p) {
        assert(p && p->syms[0]);
        p->syms[RX] = intconst(mkactual(4, p->syms[0]->u.c.v.i));
        p->x.argno = p->syms[RX]->u.c.v.i / 4;
}
static void local(Symbol p) {
        if (askregvar(p, rmap(ttob(p->type))) == 0)
                mkauto(p);
}
static void function(Symbol f, Symbol caller[], Symbol callee[], int ncalls)
{
        int i, sizefsave, varargs, dumpmask = 0, reg = 0;

        for (i = 0; callee[i]; i++)
                ;
        varargs = variadic(f->type)
                || i > 0 && strcmp(callee[i-1]->name,
                        "__builtin_va_alist") == 0;
        usedmask[0] = usedmask[1] = 0;
        freemask[0] = freemask[1] = ~(unsigned)0;
        offset = maxoffset = maxargoffset = 0;
        for (i = 0; callee[i]; i++) {
                Symbol p = callee[i], q = caller[i];
                int size = roundup(q->type->size, 4);
                assert(q);
                if (isfloat(p->type) || reg >= 4) {
                        p->x.offset = q->x.offset = offset;
                        p->x.name = q->x.name = stringd(offset);
                        p->sclass = q->sclass = AUTO;
                        dumpmask |= 0xffff>>(15 - reg);
                        if (p->type->size == 8)
                                dumpmask |= 1<<(reg + 1);
                }
                else if (p->addressed || varargs || isstruct(p->type)) {
                                p->x.offset = offset;
                                p->x.name = stringd(offset);
                                p->sclass = q->sclass = AUTO;
                                dumpmask |= 0xffff>>(16 - reg - size / 4);
                } else {
                        q->type = p->type;
                        p->sclass = q->sclass = REGISTER;
                        if (askregvar(p, rmap(ttob(p->type))))
                                q->x.name = ireg[reg]->x.name;
                        else {
                                p->sclass = q->sclass = AUTO;
                                p->x.offset = offset;
                                p->x.name = stringd(offset);
                                dumpmask |= 0xffff>>(15 - reg);
                        }
                }
                offset += size;
                reg += size / 4;
        }
        dumpmask &= 0xf;
        bigargs = (varargs || reg > 4) ? 1 : 0;
        assert(!caller[i]);
        offset = 0;
        gencode(caller, callee);
        maxargoffset = max(roundup(maxargoffset, 4) - 16, 0);
        sizefsave = 12 * bitcount(usedmask[FREG] & 0xf0);
        sizeisave = 16 + 4 * bitcount(usedmask[IREG] & 0x3f0) +
                4 * bitcount(dumpmask);
        framesize = roundup(sizefsave + sizeisave + maxargoffset + maxoffset,
                4);
        segment(CODE);
        if (glevel) {
                print(ASM_ALIGN);
                defstring(strlen(f->x.name) + 1, f->x.name);
                print(ASM_ALIGN);
                print("\tdcd\t&%x\n", 0xff000000 | ((strlen(f->x.name) + 4) &
~3));
        }
        print(ASM_ALIGN);
#ifdef __riscos__
        print("|%s|\n", f->x.name);
#else
        print("%s:\n", f->x.name);
#endif

        /* APCS Function header to save registers */
        print("\tmov\tip, sp\n");
        if (bigargs) {
                print("\tstmfd\tsp!, {a1-a4}\n");
                print("\tstmfd\tsp!, {%s}\n",
                        reglist((usedmask[IREG] & 0x3f0) | savelist));
        } else {
                print("\tstmfd\tsp!, {%s}\n",
                        reglist(dumpmask | (usedmask[IREG] & 0x3f0)
                                | savelist));
        }
        for (i = 7; i >= 4; i--)
                if (usedmask[FREG] & (1<<i))
                        print("\tstfe\t%s, [sp, #-12]!\n", freg[i]->x.name);
        print("\tsub\tfp, ip, #%d\n", bigargs ? 20 : 4);


       /* Output APCS stack overflow check */
#ifdef __riscos__
        if (framesize + 16 > 256) {
                arm_add(12, 13, -(framesize + 16));
                print("\tcmp\tip, sl\n");
                print("\tbllt\t|x$stack_overflow_1|\n");
        } else {
                print("\tcmp\tsp, sl\n");
                print("\tbllt\t|x$stack_overflow|\n");
        }
#endif

        if (framesize - sizefsave - sizeisave > 0)
                arm_add(13, 13, -(framesize - sizefsave - sizeisave));
        for (i = 0; i < 4 && callee[i]; i++)
                if (caller[i]->sclass == REGISTER &&
                        callee[i]->sclass == REGISTER &&
                        caller[i]->type == callee[i]->type)
                        print("\tmov\t%s, %s\n", callee[i]->x.name,
                                caller[i]->x.name);
        i = framesize;
        framesize -= sizefsave + sizeisave;
        emitcode();
        framesize = i;
        for (i = 4; i <= 7; i++)
                if (usedmask[FREG] & (1<<i))
                        print("\tldfe\t%s, [fp, #-%d]\n", freg[i]->x.name,
                                sizeisave - 4 * bitcount(dumpmask) +
                                (8 - i) * 12 - 4);

        /* Restore saved registers used by function */
        print("\tldmea\tfp, {%s}^\n",
                reglist((usedmask[IREG] & 0x3f0) | reslist));
}


/**
 * Output a constant inline in assmebler of the given type and value
 */
static void defconst(int suffix, int size, Value v) {
        /* float */
        if (suffix == F && size == 4) {
                float f = v.d;
                print(ASM_CONST_WORD "0x%x" ASM_COMMENT "float %f\n", *(unsigned *)&f, f); return;
        }
        /* double */
        else if (suffix == F && size == 8) {
                double d = v.d;
                unsigned *p = (unsigned *)&d;
                print(ASM_CONST_WORD "0x%x, 0x%x" ASM_COMMENT "double %f\n", p[swap], p[!swap], d);
        }
        /* pointer */
        else if (suffix == P)
                print(ASM_CONST_WORD "0x%x\n", v.p);
        /* byte */
        else if (size == 1)
                print(ASM_CONST_BYTE "%d\n", suffix == I ? (signed char)v.i : (unsigned char)v.u);
        /* short */
        else if (size == 2)
                print("\tdcw\t%d\n", suffix == I ? (short)v.i : (unsigned short)v.u);
        /* int */        
        else if (size == 4)
                print(ASM_CONST_WORD "%d\n", suffix == I ? (int)v.i : (unsigned)v.u);
}


/**
 * Output address of a given symbol
 */
static void defaddress(Symbol p) {
        print(ASM_CONST_WORD "%s\n", p->x.name);
}


/**
 * Output a counted string suitable for assembler
 */
static void defstring(int n, char *str) {
        int mode, oldmode = 0;
        char *s, c;

#ifdef __riscos__
        print("\t=\t");
#else
        print("\t.ascii \"");
#endif
        for (s = str; s < str + n; s++) {
                c = (*s) & 0xff;
                mode = (isprint(c) && c != '\"' && c != '\\') ? 1 : 0;
#ifdef __riscos__
                if (oldmode && !mode)
                        print("\",");
                else if (!oldmode && mode)
                        print("\"");
                print(mode ? "%c" : "&%x", c);
                if (!mode && s < str + n - 1)
                        print(",");
#else
                if (mode)
                        print("%c", c);
                else
                        print("\\%d%d%d", c >> 6, (c >> 3) & 3, c & 3);
#endif
                oldmode = mode;
        }
#ifdef __riscos__
        if (mode)
#endif
                print("\"");
        print("\n");
}


static void export(Symbol p) {
        print(ASM_EXPORT, p->x.name);
}


static void import(Symbol p) {
#if 0
#ifdef __riscos__
        print("\tIMPORT\t|%s|\n", p->x.name);
#endif
#endif
}


static void defsymbol(Symbol p) {
        if (p->scope >= LOCAL && p->sclass == STATIC)
                p->x.name = stringf(ASM_SYMBOL_NUM, genlabel(1));
        else if (p->generated)
                p->x.name = stringf(ASM_SYMBOL_NAME, p->name);
        else
                assert(p->scope != CONSTANTS || isint(p->type) || isptr(p->type)),
                p->x.name = p->name;
}


static void address(Symbol q, Symbol p, long n) {
        q->x.offset = p->x.offset + n;
        if (p->scope == GLOBAL
        || p->sclass == STATIC || p->sclass == EXTERN)
                q->x.name = stringf("%s%s%d", p->x.name,
                        n >= 0 ? "+" : "", n);
        else
                q->x.name = stringf("%d", q->x.offset);
}


static void global(Symbol p) {
#ifdef __riscos__
        if (p->u.seg == BSS) {
                if (p->sclass == STATIC || Aflag >= 2) {
                        if (cseg != BSS)
                                print("\tAREA\t|C$$zidata|, DATA, NOINIT\n");
                        if (p->type->align > 1)
                                print("\tALIGN\t%d\n", p->type->align);
                        print("%s\n\t%%\t%d\n", p->x.name, p->type->size);
                } else {
                        print("\tAREA\t%s, COMMON, NOINIT\n", p->x.name);
                        if (p->type->align > 1)
                                print("\tALIGN\t%d\n", p->type->align);
                        print("\t%%\t%d\n", p->type->size);
                }
                print("\tAREA\t|C$$code|, CODE, READONLY\n");
        } else {
                if (p->u.seg == DATA)
                        print("\tAREA\t|C$$data|, DATA\n");
                if (p->type->align > 1)
                        print("\tALIGN\t%d\n", p->type->align);
                print("%s\n", p->x.name);
        }
#else
        assert(p->u.seg);
        if (!p->generated) {
                print(".type %s,#%s\n", p->x.name,
                          isfunc(p->type) ? "function" : "object");
                if (p->type->size > 0)
                        print(".size %s,%d\n", p->x.name, p->type->size);

                /* else
                         prevg = p; */
        }
        if (p->u.seg == BSS && p->sclass == STATIC)
                print(".local %s\n.common %s,%d,%d\n", p->x.name, p->x.name,
                          p->type->size, p->type->align);
        else if (p->u.seg == BSS && Aflag >= 2)
                print(".align %d\n%s:.skip %d\n", p->type->align, p->x.name,
                          p->type->size);
        else if (p->u.seg == BSS)
                print(".common %s,%d,%d\n", p->x.name, p->type->size, p->type->align);
        else
                print(".align %d\n%s:\n", p->type->align, p->x.name);
#endif
}


static void segment(int n) {
#ifdef __riscos__
        if (cseg == n) return;
#endif
        cseg = n;
        switch (n) {
#ifdef __riscos__
        case CODE: case LIT:
                print("\tAREA\t|C$$code|, CODE, READONLY\n"); break;
        case BSS:  print("\tAREA\t|C$$zidata|, DATA, NOINIT\n"); break;
#else
        case CODE: print(".text\n");   break;
        case BSS:  print(".bss\n");    break;
        case DATA: print(".data\n");   break;
        case LIT:  print(".section\t.rodata\n"); break;
#endif
        }
}


static void space(int n) {
        if (cseg != BSS)
                print(ASM_SPACE "%d\n", n);
}


static void arm_blkcopy(int dreg, int doffset, int sreg, int soffset, int size)
{
        print("\tstmfd\tsp!, {a1-a4}\n");
        arm_add(0, dreg, doffset);
        arm_add(1, sreg, soffset);
        arm_mov(2, size);
        print("\tbl\tmemcpy\n");
        print("\tldmfd\tsp!, {a1-a4}\n");
}


static void blkloop(int dreg, int doff, int sreg, int soff, int size, int tmps[]) {}
static void blkfetch(int size, int off, int reg, int tmp) {}
static void blkstore(int size, int off, int reg, int tmp) {}


static int bitcount(unsigned mask) {
        unsigned i, n = 0;

        for (i = 1; i; i <<= 1)
                if (mask & i)
                        n++;
        return n;
}


Interface armIR = 
{
  1, 1, 0,    /* char */
  2, 2, 0,    /* short */
  4, 4, 0,    /* int */
  4, 4, 0,    /* long */
  4, 4, 0,    /* long long */
  4, 4, 1,    /* float */
  8, 4, 1,    /* double */
  8, 4, 1,    /* long double */
  4, 4, 0,    /* T * */
  0, 4, 0,    /* struct */

  1,          /* little_endian */
  1,          /* mulops_calls */
  0,          /* wants_callb */
  1,          /* wants_argb */
  1,          /* left_to_right */
  0,          /* wants_dag */
  0,          /* unsigned_char */ 

  address,

  blockbeg,
  blockend,

  defaddress,
  defconst,
  defstring,
  defsymbol,

  emit,
  export,
  function,
  gen,

  global,
  import,
  local,

  progbeg,
  progend,

  segment,
  space,

  0, 0, 0, 0, 0, 0, 0,

  {
          1,      /* max_unaligned_load */
          rmap,
          blkfetch, blkstore, blkloop,
          _label,
          _rule,
          _nts,
          _kids,
          _string,
          _templates,
          _isinstruction,
          _ntname,
          emit2,
          doarg,
          target,
          clobber,

  }
};
