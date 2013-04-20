%{

//
// <yard.md>
//
// experimental lcc target description for YARD-1 processor
//
// unfinished, buggy, and not working yet
//

//
// derived from original lcc mips.md and arm.md targets,
// which are covered under the original LCC license
//

//
// YARD-1 target specific code is provided under BSD 2-clause license:
//
//---------------------------------------------------------------
//
// <yard.md> copyright (c) 2012, Brian Davis
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright 
//      notice, this list of conditions and the following disclaimer in the 
//      documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
//-----------------------------------------------------------------


//
// register masks
//
#define INTTMP 0x00f0
#define INTVAR 0x0f00

#define FLTTMP 0x00f0
#define FLTVAR 0x0f00

#define ARG_NUM_REGS     4    
#define LAST_SAVED_REG  12    


//
// stack space needed for pc & fp save
//
#define SIZE_PC_FP_SAVE 8


//
// FIXME: these defs clash with default Microsoft includes in VC2005+
//
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#include <ctype.h>

//
// standard lcc boilerplate
//
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

static char *reglist(unsigned);

static Symbol ireg[32], freg[32];                                                                                      

static Symbol iregw, fregw;

static int  cseg;

//
// YARD support functions
//
static int bitcount(unsigned mask); 
static int alu_const(long value, int logicals); 
static int find_single_bit_set( unsigned long value );

static int cost_alu_const(Node p, int logicals); 
static int cost_pow2(Node p); 


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

%term  LOADB=233
%term  LOADF4=4321
%term  LOADF8=8417
%term  LOADI1=1253
%term  LOADI2=2277
%term  LOADI4=4325
%term  LOADP4=4327
%term  LOADU1=1254
%term  LOADU2=2278
%term  LOADU4=4326

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

!
! ca5 : encoded arithmetic constants
!
ca5  :  CNSTI1  "%a"  cost_alu_const(a,0)
ca5  :  CNSTU1  "%a"  cost_alu_const(a,0)

ca5  :  CNSTI2  "%a"  cost_alu_const(a,0)
ca5  :  CNSTU2  "%a"  cost_alu_const(a,0)

ca5  :  CNSTI4  "%a"  cost_alu_const(a,0)
ca5  :  CNSTU4  "%a"  cost_alu_const(a,0)
ca5  :  CNSTP4  "%a"  cost_alu_const(a,0)


!
! cg5 : encoded logical constants ( including NOT variants )
!
cg5  :  CNSTI1  "%a"  cost_alu_const(a,1)
cg5  :  CNSTU1  "%a"  cost_alu_const(a,1)

cg5  :  CNSTI2  "%a"  cost_alu_const(a,1)
cg5  :  CNSTU2  "%a"  cost_alu_const(a,1)

cg5  :  CNSTI4  "%a"  cost_alu_const(a,1)
cg5  :  CNSTU4  "%a"  cost_alu_const(a,1)
cg5  :  CNSTP4  "%a"  cost_alu_const(a,1)


!
! c12 : signed 12 bit IMM12 constants
!
c12  :  CNSTI1  "%a"  range(a,-2048,2047)
c12  :  CNSTU1  "%a"  range(a,-2048,2047)

c12  :  CNSTI2  "%a"  range(a,-2048,2047)
c12  :  CNSTU2  "%a"  range(a,-2048,2047)

c12  :  CNSTI4  "%a"  range(a,-2048,2047)
c12  :  CNSTU4  "%a"  range(a,-2048,2047)
c12  :  CNSTP4  "%a"  range(a,-2048,2047)


!
! c32 : 32 bit LDI constants
!
c32  :  CNSTI1  "%a"  1
c32  :  CNSTU1  "%a"  1

c32  :  CNSTI2  "%a"  1
c32  :  CNSTU2  "%a"  1

c32  :  CNSTI4  "%a"  1
c32  :  CNSTU4  "%a"  1
c32  :  CNSTP4  "%a"  1


!
! constant register loads 
!
reg  :  cg5     " mov %c,#%0    ; reg:cg5\n"              1
reg  :  c12     " imm12 #%0\n mov %c,imm   ; reg:c12\n"   2
reg  :  c32     " imm #%0\n mov %c,imm   ; reg:c32\n"     4


!
! logical ops
!
reg  : BANDI4(reg,reg)   "@ mov %c,%0\n and %c,%1 \n"                1
reg  : BANDI4(reg,cg5)   "@ mov %c,%0\n and %c,#%1 \n"               1
reg  : BANDI4(reg,c12)   "@ mov %c,%0\n imm12 #%1 \n and %c,imm\n"   2
reg  : BANDI4(reg,c32)   "@ mov %c,%0\n imm #%1   \n and %c,imm\n"   3

reg  : BANDU4(reg,reg)   "@ mov %c,%0\n and %c,%1\n"                 1
reg  : BANDU4(reg,cg5)   "@ mov %c,%0\n and %c,#%1\n"                1
reg  : BANDU4(reg,c12)   "@ mov %c,%0\n imm12 #%1 \n and %c,imm\n"   2
reg  : BANDU4(reg,c32)   "@ mov %c,%0\n imm #%1   \n and %c,imm\n"   3

reg  : BORI4(reg,reg)    "@ mov %c,%0\n or %c,%1  \n"                1
reg  : BORI4(reg,cg5)    "@ mov %c,%0\n or %c,#%1  \n"               1
reg  : BORI4(reg,c12)    "@ mov %c,%0\n imm12 #%1 \n or %c,imm\n"    2
reg  : BORI4(reg,c32)    "@ mov %c,%0\n imm #%1   \n or %c,imm\n"    3

reg  : BORU4(reg,reg)    "@ mov %c,%0\n or %c,%1\n"                  1
reg  : BORU4(reg,cg5)    "@ mov %c,%0\n or %c,#%1\n"                 1
reg  : BORU4(reg,c12)    "@ mov %c,%0\n imm12 #%1 \n or %c,imm\n"    2
reg  : BORU4(reg,c32)    "@ mov %c,%0\n imm #%1   \n or %c,imm\n"    3

reg  : BXORI4(reg,reg)   "@ mov %c,%0\n xor %c,%1 \n"                1
reg  : BXORI4(reg,cg5)   "@ mov %c,%0\n xor %c,#%1 \n"               1
reg  : BXORI4(reg,c12)   "@ mov %c,%0\n imm12 #%1 \n xor %c,imm\n"   2
reg  : BXORI4(reg,c32)   "@ mov %c,%0\n imm #%1   \n xor %c,imm\n"   3

reg  : BXORU4(reg,reg)   "@ mov %c,%0\n xor %c,%1\n"                 1
reg  : BXORU4(reg,cg5)   "@ mov %c,%0\n xor %c,#%1\n"                1
reg  : BXORU4(reg,c12)   "@ mov %c,%0\n imm12 #%1 \n xor %c,imm\n"   2
reg  : BXORU4(reg,c32)   "@ mov %c,%0\n imm #%1   \n xor %c,imm\n"   3

! 
! note, no "? mov" prefix is needed for complement, because move.not has independent source/dest
! 
reg  : BCOMI4(reg)       " mov.not %c,%0\n"   1
reg  : BCOMU4(reg)       " mov.not %c,%0\n"   1


!
! logicals again, this time around with rules to emit .not variants for ~register_operand
!
reg  : BANDI4(reg,BCOMI4(reg))   "@ mov %c,%0\n and.not %c,%1\n"     0
reg  : BANDI4(reg,BCOMI4(cg5))   "@ mov %c,%0\n and.not %c,#%1\n"    0
reg  : BANDI4(reg,BCOMU4(reg))   "@ mov %c,%0\n and.not %c,%1\n"     0
reg  : BANDI4(reg,BCOMU4(cg5))   "@ mov %c,%0\n and.not %c,#%1\n"    0

reg  : BANDU4(reg,BCOMI4(reg))   "@ mov %c,%0\n and.not %c,%1\n"     0
reg  : BANDU4(reg,BCOMI4(cg5))   "@ mov %c,%0\n and.not %c,#%1\n"    0
reg  : BANDU4(reg,BCOMU4(reg))   "@ mov %c,%0\n and.not %c,%1\n"     0
reg  : BANDU4(reg,BCOMU4(cg5))   "@ mov %c,%0\n and.not %c,#%1\n"    0

reg  : BORI4(reg,BCOMI4(reg))    "@ mov %c,%0\n or.not %c,%1\n"      0
reg  : BORI4(reg,BCOMI4(cg5))    "@ mov %c,%0\n or.not %c,#%1\n"     0
reg  : BORI4(reg,BCOMU4(reg))    "@ mov %c,%0\n or.not %c,%1\n"      0
reg  : BORI4(reg,BCOMU4(cg5))    "@ mov %c,%0\n or.not %c,#%1\n"     0

reg  : BORU4(reg,BCOMI4(reg))    "@ mov %c,%0\n or.not %c,%1\n"      0
reg  : BORU4(reg,BCOMI4(cg5))    "@ mov %c,%0\n or.not %c,#%1\n"     0
reg  : BORU4(reg,BCOMU4(reg))    "@ mov %c,%0\n or.not %c,%1\n"      0
reg  : BORU4(reg,BCOMU4(cg5))    "@ mov %c,%0\n or.not %c,#%1\n"     0

reg  : BXORI4(reg,BCOMI4(reg))   "@ mov %c,%0\n xor.not %c,%1\n"     0
reg  : BXORI4(reg,BCOMI4(cg5))   "@ mov %c,%0\n xor.not %c,#%1\n"    0
reg  : BXORI4(reg,BCOMU4(reg))   "@ mov %c,%0\n xor.not %c,%1\n"     0
reg  : BXORI4(reg,BCOMU4(cg5))   "@ mov %c,%0\n xor.not %c,#%1\n"    0

reg  : BXORU4(reg,BCOMI4(reg))   "@ mov %c,%0\n xor.not %c,%1\n"     0
reg  : BXORU4(reg,BCOMI4(cg5))   "@ mov %c,%0\n xor.not %c,#%1\n"    0
reg  : BXORU4(reg,BCOMU4(reg))   "@ mov %c,%0\n xor.not %c,%1\n"     0
reg  : BXORU4(reg,BCOMU4(cg5))   "@ mov %c,%0\n xor.not %c,#%1\n"    0



!
! shift ops
!
!
! TODO: add support for variable shifts 
!
! TODO: add flag to control whether core has multi-bit constant shifts
!

!
! FIXME: is anything besides CNSTI4 actually needed here?
!
sc5  : CNSTI1            " #%a"  range(a,0,31)
sc5  : CNSTU1            " #%a"  range(a,0,31)
sc5  : CNSTI2            " #%a"  range(a,0,31)
sc5  : CNSTU2            " #%a"  range(a,0,31)
sc5  : CNSTI4            " #%a"  range(a,0,31)
sc5  : CNSTU4            " #%a"  range(a,0,31)

!                        
! not used - full shifts needed in hardware                       
!                        
!reg  : LSHI4(reg,sc5)    "? mov %c,%0\n lsl %c,%1\n"  1 
!reg  : LSHU4(reg,sc5)    "? mov %c,%0\n lsl %c,%1\n"  1 
!reg  : RSHI4(reg,sc5)    "? mov %c,%0\n asr %c,%1\n"  1 
!reg  : RSHU4(reg,sc5)    "? mov %c,%0\n lsr %c,%1\n"  1 


!
! FIXME: hack to support constant shifts of 1 & 2 bits with one-bit hardware
!
sc1  : CNSTI1            " #%a"  range(a,1,1)
sc1  : CNSTU1            " #%a"  range(a,1,1)
sc1  : CNSTI2            " #%a"  range(a,1,1)
sc1  : CNSTU2            " #%a"  range(a,1,1)
sc1  : CNSTI4            " #%a"  range(a,1,1)
sc1  : CNSTU4            " #%a"  range(a,1,1)

sc2  : CNSTI1            " #%a"  range(a,2,2)
sc2  : CNSTU1            " #%a"  range(a,2,2)
sc2  : CNSTI2            " #%a"  range(a,2,2)
sc2  : CNSTU2            " #%a"  range(a,2,2)
sc2  : CNSTI4            " #%a"  range(a,2,2)
sc2  : CNSTU4            " #%a"  range(a,2,2)

reg  : LSHI4(reg,sc1)    "? mov %c,%0\n lsl %c,#1\n"              1
reg  : LSHU4(reg,sc1)    "? mov %c,%0\n lsl %c,#1\n"              1
reg  : RSHI4(reg,sc1)    "? mov %c,%0\n asr %c,#1\n"              1
reg  : RSHU4(reg,sc1)    "? mov %c,%0\n lsr %c,#1\n"              1

reg  : LSHI4(reg,sc2)    "? mov %c,%0\n lsl %c,#1\n lsl %c,#1\n"  2
reg  : LSHU4(reg,sc2)    "? mov %c,%0\n lsl %c,#1\n lsl %c,#1\n"  2
reg  : RSHI4(reg,sc2)    "? mov %c,%0\n asr %c,#1\n asr %c,#1\n"  2
reg  : RSHU4(reg,sc2)    "? mov %c,%0\n lsr %c,#1\n lsr %c,#1\n"  2

!
! catch any remaining shifts & print error in emit2
!

! unsupported constant shifts
reg  : LSHI4(reg,sc5)    "#\n"  4
reg  : LSHU4(reg,sc5)    "#\n"  4
reg  : RSHI4(reg,sc5)    "#\n"  4
reg  : RSHU4(reg,sc5)    "#\n"  4
                        
! unsupported variable shifts
reg  : LSHI4(reg,reg)    "#\n"  4
reg  : LSHU4(reg,reg)    "#\n"  4
reg  : RSHI4(reg,reg)    "#\n"  4
reg  : RSHU4(reg,reg)    "#\n"  4
                        
                        
!
! arithmetic ops
!
reg  : ADDI4(reg,reg)    "@ mov %c,%0\n add %c,%1\n"                 1
reg  : ADDI4(reg,ca5)    "@ mov %c,%0\n add %c,#%1\n"                1
reg  : ADDI4(reg,c12)    "@ mov %c,%0\n imm12 #%1\n add %c,imm\n"    2
reg  : ADDI4(reg,c32)    "@ mov %c,%0\n imm #%1  \n add %c,imm\n"    3

reg  : ADDP4(reg,reg)    "@ mov %c,%0\n add %c,%1\n"                 1
reg  : ADDP4(reg,ca5)    "@ mov %c,%0\n add %c,#%1\n"                1
reg  : ADDP4(reg,c12)    "@ mov %c,%0\n imm12 #%1\n add %c,imm\n"    2
reg  : ADDP4(reg,c32)    "@ mov %c,%0\n imm #%1  \n add %c,imm\n"    3

reg  : ADDU4(reg,reg)    "@ mov %c,%0\n add %c,%1\n"                 1
reg  : ADDU4(reg,ca5)    "@ mov %c,%0\n add %c,#%1\n"                1
reg  : ADDU4(reg,c12)    "@ mov %c,%0\n imm12 #%1\n add %c,imm\n"    2
reg  : ADDU4(reg,c32)    "@ mov %c,%0\n imm #%1  \n add %c,imm\n"    3

!     
! SUB is non-commutative; reg,reg fixups have to be handled in emit2()
!     
reg  : SUBI4(reg,reg)    "#\n"                                       1
reg  : SUBP4(reg,reg)    "#\n"                                       1
reg  : SUBU4(reg,reg)    "#\n"                                       1

!     
! can reg-constant operations still use patterns ???
!     
reg  : SUBI4(reg,ca5)    "? mov %c,%0\n sub %c, #%1\n"               1
reg  : SUBI4(reg,c12)    "? mov %c,%0\n imm12 #%1\n sub %c,imm\n"    2
reg  : SUBI4(reg,c32)    "? mov %c,%0\n imm #%1  \n sub %c,imm\n"    3

reg  : SUBP4(reg,ca5)    "? mov %c,%0\n sub %c, #%1\n"               1
reg  : SUBP4(reg,c12)    "? mov %c,%0\n imm12 #%1\n sub %c,imm\n"    2
reg  : SUBP4(reg,c32)    "? mov %c,%0\n imm #%1  \n sub %c,imm\n"    3

reg  : SUBU4(reg,ca5)    "? mov %c,%0\n sub %c, #%1\n"               1
reg  : SUBU4(reg,c12)    "? mov %c,%0\n imm12 #%1\n sub %c,imm\n"    2
reg  : SUBU4(reg,c32)    "? mov %c,%0\n imm #%1  \n sub %c,imm\n"    3

!     
! FIXME: double check "? mov" sense for rsub with register source
!     
reg  : SUBI4(ca5,reg)    "? mov %c,%1\n rsub %c, #%0\n"              1
reg  : SUBI4(c12,reg)    "? mov %c,%1\n imm12 #%0\n rsub %c,imm\n"   2
reg  : SUBI4(c32,reg)    "? mov %c,%1\n imm #%0  \n rsub %c,imm\n"   3

reg  : SUBP4(ca5,reg)    "? mov %c,%1\n rsub %c, #%0\n"              1
reg  : SUBP4(c12,reg)    "? mov %c,%1\n imm12 #%0\n rsub %c,imm\n"   2
reg  : SUBP4(c32,reg)    "? mov %c,%1\n imm #%0  \n rsub %c,imm\n"   3

reg  : SUBU4(ca5,reg)    "? mov %c,%1\n rsub %c, #%0\n"              1
reg  : SUBU4(c12,reg)    "? mov %c,%1\n imm12 #%0\n rsub %c,imm\n"   2
reg  : SUBU4(c32,reg)    "? mov %c,%1\n imm #%0  \n rsub %c,imm\n"   3


!
! negate has one operand, integer type only
!
reg  : NEGI4(reg)        "? mov %c,%0\n neg %c\n"     1

!
! extra negate pattern to catch complement-and-add-one emitted by lcc for negation of an unsigned 
! spotted this technique in <pdp11.md> target
!
cc1  : CNSTI4                  " #%a"  range(a,1,1)
cc1  : CNSTU4                  " #%a"  range(a,1,1)
reg  : ADDU4(BCOMU4(reg),cc1)  "? mov %c,%0\n neg %c\n"     1


!
! TODO: multiply/divide 
!
reg  : MULI4(reg,reg)    " lbsr __muls\n"   2
reg  : MULU4(reg,reg)    " lbsr __mulu\n"   2

reg  : DIVI4(reg,reg)    " lbsr __divs\n"   8
reg  : DIVU4(reg,reg)    " lbsr __divu\n"   8

reg  : MODI4(reg,reg)    " lbsr __mods\n"   8
reg  : MODU4(reg,reg)    " lbsr __modu\n"   8


!
! TODO: floating point ops
!
reg  : ADDF4(reg,reg)    " fp.add.s %c, %0, %1\n"  1
reg  : ADDF8(reg,reg)    " fp.add.d %c, %0, %1\n"  1

reg  : SUBF4(reg,reg)    " fp.sub.s %c, %0, %1\n"  1
reg  : SUBF8(reg,reg)    " fp.sub.d %c, %0, %1\n"  1

reg  : NEGF4(reg)        " fp.neg.s %c, %0\n"      1
reg  : NEGF8(reg)        " fp.neg.d %c, %0\n"      1

reg  : MULF4(reg,reg)    " fp.mul.s %c, %0, %1\n"  1
reg  : MULF8(reg,reg)    " fp.mul.d %c, %0, %1\n"  1

reg  : DIVF4(reg,reg)    " fp.div.s %c, %0, %1\n"  1
reg  : DIVF8(reg,reg)    " fp.div.d %c, %0, %1\n"  1

!
! register-register sign/zero extensions, needed for char/short register variables
!
reg  : CVII4(reg)        " ext.sb %c,%0\n"  ( a->syms[0]->u.c.v.i == 1 ? 1 : LBURG_MAX )
reg  : CVII4(reg)        " ext.sw %c,%0\n"  ( a->syms[0]->u.c.v.i == 2 ? 1 : LBURG_MAX )

reg  : CVUI4(reg)        " ext.ub %c,%0\n"  ( a->syms[0]->u.c.v.i == 1 ? 1 : LBURG_MAX )
reg  : CVUI4(reg)        " ext.uw %c,%0\n"  ( a->syms[0]->u.c.v.i == 2 ? 1 : LBURG_MAX )
                                              
reg  : CVUU4(reg)        " ext.ub %c,%0\n"  ( a->syms[0]->u.c.v.i == 1 ? 1 : LBURG_MAX )
reg  : CVUU4(reg)        " ext.uw %c,%0\n"  ( a->syms[0]->u.c.v.i == 2 ? 1 : LBURG_MAX )


!
! TODO: floating point conversions
!
reg  : CVFF4(reg)        " fp.cvt.s.d f%c,f%0\n"                   1
reg  : CVFF8(reg)        " fp.cvt.d.s f%c,f%0\n"                   1

reg  : CVIF4(reg)        " fp.mtc %0,f%c\n fp.cvt.s.w f%c,f%c\n"     2
reg  : CVIF8(reg)        " fp.mtc %0,f%c\n fp.cvt.d.w f%c,f%c\n"     2

reg  : CVFI4(reg)        " fp.trunc.s f2,f%0,%c\n fp.mfc %c,f2\n"  (a->syms[0]->u.c.v.i==4?2:LBURG_MAX)
reg  : CVFI4(reg)        " fp.trunc.d f2,f%0,%c\n fp.mfc %c,f2\n"  (a->syms[0]->u.c.v.i==8?2:LBURG_MAX)


!
! labels 
!
stmt : LABELV           "\n%a:\n"


!
! addressing modes and constants
!

!
! address constants      
!
!  FIXME: acon c32 rule seems to act strange, maybe redefine here without refencing c32
!
acon  : c32              "%0"      
acon  : ADDRGP4          "%a"      

!
! offset indirect 
!
aimm : acon             " imm #%0   ; aimm:acon\n"      1

addr : ADDI4(reg,aimm)  " .imm(%0)"     1
addr : ADDU4(reg,aimm)  " .imm(%0)"     1
addr : ADDP4(reg,aimm)  " .imm(%0)"     1

!
! load address constant into register
!
reg  : acon             " imm #%0   ; reg:acon\n mov %c,imm\n"  3

!
! plain register indirect
!
addr : reg              "(%0)"

!
! stack modes
!
! TODO: catch stack offset > 60 and build with .imm sequence like aimm
!
!
addr : ADDRFP4          "%a+%F(sp)"        

!
! ADDRLP4 needs a "-8" to account for SIZE_PC_FP_SAVE ( offset from top of stack frame to end of locals )
!
! note: ARM target avoided an offset by changing framesize just before the emitcode() call, 
!       such that sp+framesize' pointed just above the locals within the stack frame- not sure 
!       how different offset for ADDRFP was handled
!
addr : ADDRLP4          "%a+%F-8(sp)"        
     
!
! lea rule
!
reg  : addr             " lea %c,%0   ; reg:addr\n"  2


!
! memory accesses
!

!
! basic loads
!
reg  :  INDIRI1(addr)            " ld.b %c,%0\n"   1
reg  :  INDIRU1(addr)            " ld.ub %c,%0\n"  1

reg  :  INDIRI2(addr)            " ld.w %c,%0\n"   1
reg  :  INDIRU2(addr)            " ld.uw %c,%0\n"  1

reg  :  INDIRI4(addr)            " ld.q %c,%0\n"   1
reg  :  INDIRU4(addr)            " ld.q %c,%0\n"   1
                                                  
reg  :  INDIRP4(addr)            " ld.q %c,%0\n"   1


!
! loads with size conversion
!
reg  :  CVII4(INDIRI1(addr))     " ld.b %c,%0\n"   1
reg  :  CVII4(INDIRI2(addr))     " ld.w %c,%0\n"   1

reg  :  CVUU4(INDIRU1(addr))     " ld.ub %c,%0\n"  1
reg  :  CVUU4(INDIRU2(addr))     " ld.uw %c,%0\n"  1

reg  :  CVUI4(INDIRU1(addr))     " ld.ub %c,%0\n"  1
reg  :  CVUI4(INDIRU2(addr))     " ld.uw %c,%0\n"  1

     
!
! stores
!
stmt :  ASGNI1(addr,reg)         " st.b %1,%0\n"   1
stmt :  ASGNU1(addr,reg)         " st.b %1,%0\n"   1

stmt :  ASGNI2(addr,reg)         " st.w %1,%0\n"   1
stmt :  ASGNU2(addr,reg)         " st.w %1,%0\n"   1

stmt :  ASGNI4(addr,reg)         " st.q %1,%0\n"   1
stmt :  ASGNU4(addr,reg)         " st.q %1,%0\n"   1

stmt :  ASGNP4(addr,reg)         " st.q %1,%0\n"   1


!
! TODO: floating point load/store
!
reg  :  INDIRF4(addr)     " mips_l.s f%c,%0\n"     1
reg  :  INDIRF8(addr)     " mips_l.d f%c,%0\n"     1

stmt :  ASGNF4(addr,reg)  " mips_s.s f%1,%0\n"     1
stmt :  ASGNF8(addr,reg)  " mips_s.d f%1,%0\n"     1


!
! register moves
!
reg  : LOADI1(reg)    " mov %c,%0\n"  move(a)
reg  : LOADU1(reg)    " mov %c,%0\n"  move(a)

reg  : LOADI2(reg)    " mov %c,%0\n"  move(a)
reg  : LOADU2(reg)    " mov %c,%0\n"  move(a)

reg  : LOADI4(reg)    " mov %c,%0\n"  move(a)
reg  : LOADU4(reg)    " mov %c,%0\n"  move(a)

reg  : LOADP4(reg)    " mov %c,%0\n"  move(a)


!
! TODO: floating point register moves
!
reg  : LOADF4(reg)    " mips_mov.s f%c,f%0\n"      move(a)
reg  : LOADF8(reg)    " mips_mov.d f%c,f%0\n"      move(a)


!
! conditionals
!

!
! look for reg-zero comparisons
!
ccz  : CNSTI4         ""                           range(a,0,0)
ccz  : CNSTU4         ""                           range(a,0,0)

stmt : EQU4(reg,ccz)  " when.z   %0\n bra %a\n"    1
stmt : NEU4(reg,ccz)  " when.nz  %0\n bra %a\n"    1
stmt : GTU4(reg,ccz)  " when.nz  %0\n bra %a\n"    1

stmt : EQI4(reg,ccz)  " when.z   %0\n bra %a\n"    1
stmt : NEI4(reg,ccz)  " when.nz  %0\n bra %a\n"    1
stmt : GTI4(reg,ccz)  " when.gtz %0\n bra %a\n"    1 
stmt : GEI4(reg,ccz)  " when.pl  %0\n bra %a\n"    1
stmt : LEI4(reg,ccz)  " when.lez %0\n bra %a\n"    1
stmt : LTI4(reg,ccz)  " when.mi  %0\n bra %a\n"    1 

!
! reg-reg comparisons
!
stmt : EQI4(reg,reg)  " when.eq %0,%1\n bra %a\n"  2
stmt : EQU4(reg,reg)  " when.eq %0,%1\n bra %a\n"  2

stmt : NEI4(reg,reg)  " when.ne %0,%1\n bra %a\n"  2
stmt : NEU4(reg,reg)  " when.ne %0,%1\n bra %a\n"  2

stmt : GEI4(reg,reg)  " when.ge %0,%1\n bra %a\n"  2
stmt : GEU4(reg,reg)  " when.hs %0,%1\n bra %a\n"  2

stmt : GTI4(reg,reg)  " when.gt %0,%1\n bra %a\n"  2 
stmt : GTU4(reg,reg)  " when.hi %0,%1\n bra %a\n"  2

stmt : LEI4(reg,reg)  " when.le %0,%1\n bra %a\n"  2
stmt : LEU4(reg,reg)  " when.ls %0,%1\n bra %a\n"  2

stmt : LTI4(reg,reg)  " when.lt %0,%1\n bra %a\n"  2 
stmt : LTU4(reg,reg)  " when.lo %0,%1\n bra %a\n"  2


!
! skip-on-bit conditional patterns :  ( reg AND single_bit_mask ) == 0 
! integer to bit number conversion done with emit2
!
! inspired by similar technique used in <pdp11.md> target 
!
cp2  : CNSTI4         "%a"         cost_pow2(a)
cp2  : CNSTU4         "%a"         cost_pow2(a)

stmt : EQI4(BANDI4(reg,cp2),ccz)   "# skip.bs %0,#bitnum(%1)\n bra %a\n"
stmt : EQU4(BANDU4(reg,cp2),ccz)   "# skip.bs %0,#bitnum(%1)\n bra %a\n"

stmt : NEI4(BANDI4(reg,cp2),ccz)   "# skip.bc %0,#bitnum(%1)\n bra %a\n"
stmt : NEU4(BANDU4(reg,cp2),ccz)   "# skip.bc %0,#bitnum(%1)\n bra %a\n"


!
! TODO: floating point conditionals
!
stmt: EQF4(reg,reg)  " when.fp.eq %0, %1\n bra %a\n"   2
stmt: EQF8(reg,reg)  " when.fp.eq %0, %1\n bra %a\n"   2

stmt: LEF4(reg,reg)  " when.fp.le %0, %1\n bra %a\n"  2
stmt: LEF8(reg,reg)  " when.fp.le %0, %1\n bra %a\n"  2

stmt: LTF4(reg,reg)  " when.fp.lt %0, %1\n bra %a\n"  2
stmt: LTF8(reg,reg)  " when.fp.lt %0, %1\n bra %a\n"  2

stmt: GEF4(reg,reg)  " when.fp.ge %0, %1\n bra %a\n"  2
stmt: GEF8(reg,reg)  " when.fp.ge %0, %1\n bra %a\n"  2

stmt: GTF4(reg,reg)  " when.fp.gt %0, %1\n bra %a\n"  2
stmt: GTF8(reg,reg)  " when.fp.gt %0, %1\n bra %a\n"  2

stmt: NEF4(reg,reg)  " when.fp.ne %0, %1\n bra %a\n"   2
stmt: NEF8(reg,reg)  " when.fp.ne %0, %1\n bra %a\n"   2


!
! TODO: argument calling convention
!
stmt : ARGI4(reg)         "# arg\n"  1
stmt : ARGP4(reg)         "# arg\n"  1
stmt : ARGU4(reg)         "# arg\n"  1
stmt : ARGF4(reg)         "# arg\n"  1
stmt : ARGF8(reg)         "# arg\n"  1

stmt : ARGB(INDIRB(reg))       "# argb %0\n"  1
stmt : ASGNB(reg,INDIRB(reg))  "# asgnb %0 %1\n"  1


!
! branches, calls, and returns
!
baddr  : ADDRGP4       "%a"
baddr  : c32           "%0"

!
! FIXME : change to lbra once assembler supports it
!
stmt : JUMPV(baddr)  " bra %0\n"    1

stmt : JUMPV(reg)    " jmp  (%0)\n"  1


!
! FIXME : change to lbsr once assembler supports it
!
stmt : CALLV(baddr)    " bsr %0\n"     1
reg  : CALLI4(baddr)   " bsr %0\n"     1
reg  : CALLP4(baddr)   " bsr %0\n"     1
reg  : CALLU4(baddr)   " bsr %0\n"     1
reg  : CALLF4(baddr)   " bsr %0\n"     1
reg  : CALLF8(baddr)   " bsr %0\n"     1

stmt : CALLV(reg)      " jsr (%0)\n"   1
reg  : CALLI4(reg)     " jsr (%0)\n"   1
reg  : CALLP4(reg)     " jsr (%0)\n"   1
reg  : CALLU4(reg)     " jsr (%0)\n"   1
reg  : CALLF4(reg)     " jsr (%0)\n"   1
reg  : CALLF8(reg)     " jsr (%0)\n"   1

stmt : RETV(reg)       "# ret\n"       1
stmt : RETI4(reg)      "# ret\n"       1
stmt : RETP4(reg)      "# ret\n"       1
stmt : RETU4(reg)      "# ret\n"       1
stmt : RETF4(reg)      "# ret\n"       1
stmt : RETF8(reg)      "# ret\n"       1
     

%%

//
// set global swap flag if byte ordering of host differs from target
//
static void detect_endian_swap( void )
{
  union 
    {
      char c;
      int i;
    } u;

  u.i = 0;
  u.c = 1;

  swap = ((int)(u.i == 1)) != IR->little_endian;
}


//
// write assembly code header 
//
// TODO: add filename info, command line args to header
//
static void write_header( void )
{
  print
  (
    ";\n"
    "; incomplete and experimental YARD backend for lcc\n"
    ";\n"
    "\n"
  );

}

//
// write rudimentary cstart stub
//
static void write_cstart( void )
{
  print
  (
    ";\n"
    "; simple absolute cstart code for tiny embedded startup\n"
    ";\n"
    " org $100\n"
    "cstart_1:\n"
    " mov sp,#$1000\n"
    " bsr main\n"
    "\n"
    ";; rts\n"
    "\n"
    "; jump to reset vector \n"
    " mov r0,#0\n"
    " jmp (r0)\n"
    "\n"
  );
}


//
//
//
static void progbeg(int argc, char *argv[]) 
{
  int i;

  detect_endian_swap();

  parseflags(argc, argv);

  write_header();
  write_cstart();

  ///
  /// old MIPS code: global register
  ///
  /// for (i = 0; i < argc; i++)
  ///         if (strncmp(argv[i], "-G", 2) == 0)
  ///                 gnum = atoi(argv[i] + 2);
  ///
  
  //            
  // integer registers           
  //            
  for (i = 0; i < 16; i++)
    ireg[i]  = mkreg("r%d", i, 1, IREG);


  ireg[12]->x.name = "fp";
  ireg[13]->x.name = "sp";
  ireg[14]->x.name = "imm";

  //            
  // float/double registers           
  //            
  for (i = 0; i < 12; i += 1)
    freg[i] = mkreg("f%d", i, 1, FREG);

  ///
  /// TODO: floating point support
  ///
  ///  for (i = 0; i < 12; i += 2)
  ///    freg[i] = mkreg("g%d", i, 3, FREG);

  //            
  // wildcards
  //            
  iregw = mkwildcard(ireg);
  fregw = mkwildcard(freg);

  //
  // temporary and variable register masks           
  //
  tmask[IREG] = INTTMP; 
  tmask[FREG] = FLTTMP;

  vmask[IREG] = INTVAR; 
  vmask[FREG] = FLTVAR;

  ///
  /// old MIPS code: double and block move register groups
  ///
  ///    d2= mkreg("2", 2, 3, IREG);
  ///    blkreg = mkreg("4", 4, 0xf, IREG);  // register quadruplet used for block moves

}


//
//
//
static void progend(void) 
{
  print("\n");
  print(" .align 2\n");
  print(" end\n");
}

//
//
//
static Symbol rmap(int opk) 
{
  switch (optype(opk)) 
  {

    case I: case U: case P: case B:
      return iregw;

    case F:
      return fregw;

    default:
      return 0;
  }
}

//
//
//
static void target(Node p)
{
  int n;

  assert(p);

  switch (specific(p->op)) 
  {

    case CALL+I: 
    case CALL+U: 
    case CALL+P:
      setreg(p, ireg[0]);     
      break;

    case CALL+F:
      setreg(p, freg[0]);     
      break;

    case RET+I: 
    case RET+U: 
    case RET+P:
      rtarget(p, 0, ireg[0]);
      p->kids[0]->x.registered = 1;
      break;

    case RET+F:
      rtarget(p, 0, freg[0]); 
      break;

    case ARG+I: 
    case ARG+U: 
    case ARG+P:
      n = p->x.argno;
      if (n < ARG_NUM_REGS) 
      {
        rtarget(p, 0, ireg[n]);
        setreg(p, ireg[n]);
      }
      break;

    case ARG+F:
      n = p->x.argno;

      if (opsize(p->op) == 4) 
      {
        if (n < ARG_NUM_REGS) 
        {
          rtarget(p, 0, freg[n]);
          setreg(p, freg[n]);
        }
      } 
      else 
      {
        //
        // TODO: double register args
        //
        print( "\n .warn \"Double register args not implemented yet\"\n" );
      }

      break;

    case MUL+I: case MUL+U: 
    case DIV+I: case DIV+U: 
    case MOD+I: case MOD+U:
      setreg(p, ireg[0]);
      rtarget(p, 1, ireg[0]);
      rtarget(p, 0, ireg[1]);
      break;
 
    ///
    /// old MIPS code: block move registers
    ///
    ///    case ASGN+B: rtarget(p->kids[1], 0, blkreg); break;
    ///    case ARG+B:  rtarget(p->kids[0], 0, blkreg); break;

  }
}

//
// clobber
//
static void clobber(Node p) 
{
  int n;

  assert(p);
  switch (specific(p->op)) 
    {
      case CALL+F: 
      case CALL+I: 
      case CALL+U: 
      case CALL+P: 
      case CALL+V:
        spill(INTTMP, IREG, p);

// 
//  FIXME: hack to force no floating point saves 
//  without disabling this line, after gencode call in function(), 
//  usedmask[FREG] is set to 0x000000f0 in a test program without any floats
// 
////    spill(FLTTMP, FREG, p);

        break;

    }
}


////
//// ARM version of clobber
////
//static void clobber(Node p) {
//        int n;
//
//        assert(p);
//        switch (specific(p->op)) {
//
//        case CALL+F: case CALL+I: case CALL+U: case CALL+P: case CALL+V:
//                spill(FLTTMP, FREG, p);
//                break;
//
//        }
//}
//

////
//// MIPS version of clobber
////
////static void clobber(Node p) 
//{
//  assert(p);
//  switch (specific(p->op)) 
//    {
//      case CALL+F:
//        spill(INTTMP | INTRET, IREG, p);
//        spill(FLTTMP,          FREG, p);
//        break;
//
//      case CALL+I: case CALL+P: case CALL+U:
//        spill(INTTMP,          IREG, p);
//        spill(FLTTMP | FLTRET, FREG, p);
//        break;
//
//      case CALL+V:
//        spill(INTTMP | INTRET, IREG, p);
//        spill(FLTTMP | FLTRET, FREG, p);
//        break;
//    }
//}
//

//
// 
//
static void emit2(Node p) 
{
  int src, src0, src1, dst, n;
  int size, offset;

  switch (specific(p->op)) 
  {
    case ARG+I: 
    case ARG+U: 
    case ARG+P:
      n   = p->x.argno;
      src = getregnum(p->x.kids[0]);

      if (n >= ARG_NUM_REGS) 
        print(" st.q %s, %d(sp)  ; ARG+I|U|P\n",ireg[src]->x.name, n * 4);

      break;

    //
    // TODO : double argno/address offset code
    //
    case ARG+F:
      n   = p->x.argno;
      src = getregnum(p->x.kids[0]);

      if (opsize(p->op) == 4) 
      {
        if (n >= ARG_NUM_REGS) 
          print(" st.q %s, %d(sp)\n",ireg[src]->x.name, n * 4);
      } 

      else 
      {
        print( "\n .error \"Double args not implemented yet\"\n" );
      }

      break;

    //
    // TODO : ARG+B block copy
    //
    case ARG+B:
      n = p->syms[RX]->u.c.v.i;
      size = roundup(p->syms[0]->u.c.v.i, 4);
      offset = max(16 - n, 0);

      print( "\n .error \"ARG+B block copy not implemented yet\"\n" );
  
      break;
  
    //
    // TODO : ASGN+B block copy
    //
    case ASGN+B:
      print( "\n .error \"ASGN+B block copy not implemented yet\"\n" );

      break;


    //
    // unsupported shifts
    //
    case LSH+I: case LSH+U: case RSH+I: case RSH+U:
      print( "\n .error \"Multi-bit (>2) and variable shifts not supported yet\"\n" );

      break;

    //
    // non-commutative reg-reg subtracts are fixed up here if there's a two-operand overwrite
    //
    case SUB+I: case SUB+U: 
      dst  = getregnum( p );
      src0 = getregnum( p->x.kids[0] );
      src1 = getregnum( p->x.kids[1] );

      print( ";; emit2 SUB: %s = %s - %s\n", ireg[dst]->x.name, ireg[src0]->x.name, ireg[src1]->x.name );

      //
      // fixup any operand overwrites by either swapping operands or switching to rsub
      //
      if ( dst == src0 )
      {
        print(" sub %s,%s\n",  ireg[dst]->x.name, ireg[src1]->x.name );
      }

      else if ( dst == src1 )
      {
        print(" rsub %s,%s\n", ireg[dst]->x.name, ireg[src0]->x.name );
      }

      else 
      {
        print(" mov %s,%s\n sub %s,%s\n", ireg[dst]->x.name, ireg[src0]->x.name, ireg[dst]->x.name, ireg[src1]->x.name );
      }

      break;


    //
    // emit2 is only called for EQ/NE by the following rules matching power-of-two constants
    // in order to compute the bit number for the skip from the power-of-two mask
    //
    // 
    // !
    // ! skip-on-bit conditional patterns :  ( reg AND single_bit_mask ) == 0 
    // ! integer to bit number conversion done with emit2
    // !
    // ! inspired by similar technique used in <pdp11.md> target 
    // !
    // cp2  : CNSTI4         "%a"         cost_pow2(a)
    // cp2  : CNSTU4         "%a"         cost_pow2(a)
    // 
    // stmt : EQI4(BANDI4(reg,cp2),ccz)   "# skip.bs %0,#bitnum(%1)\n bra %a\n"
    // stmt : EQU4(BANDU4(reg,cp2),ccz)   "# skip.bs %0,#bitnum(%1)\n bra %a\n"
    // 
    // stmt : NEI4(BANDI4(reg,cp2),ccz)   "# skip.bc %0,#bitnum(%1)\n bra %a\n"
    // stmt : NEU4(BANDU4(reg,cp2),ccz)   "# skip.bc %0,#bitnum(%1)\n bra %a\n"
    // 
    //
    case EQ+I: 
    case EQ+U:
      // TODO: add sanity check here for proper EQ arguments

      src = getregnum( p->x.kids[0] );
      n   = p->kids[0]->kids[1]->syms[0]->u.c.v.u;

      print(" skip.bs %s,#%d\n bra L%s\n", ireg[src]->x.name, find_single_bit_set(n), p->syms[0]->name );
//.L    print(" skip.bs %s,#%d\n bra .L%s\n", ireg[src]->x.name, find_single_bit_set(n), p->syms[0]->name );
                                                                                     
      break;

    case NE+I: 
    case NE+U:
      // TODO: add sanity check here for proper NE arguments

      src = getregnum( p->x.kids[0] );
      n   = p->kids[0]->kids[1]->syms[0]->u.c.v.u;

      print(" skip.bc %s,#%d\n bra L%s\n", ireg[src]->x.name, find_single_bit_set(n), p->syms[0]->name );
//.L  print(" skip.bc %s,#%d\n bra .L%s\n", ireg[src]->x.name, find_single_bit_set(n), p->syms[0]->name );

      break;

  }
}

//
// old ARM function to print comma separated list of registers for load/store multiple
//
static char *reglist(mask) unsigned mask; 
{
  int i;
  int first = 1;
  static char list[16 * 4 + 1];

  list[0] = '\0';

  for (i = 0; i < 16; i++) 
  {
    if (mask & (1<<i)) 
    {
      if (first)
        first = 0;
      else
        strcat(list, ", ");

      strcat(list, ireg[i]->x.name);
    }
  }

  return list;
}


//
//  ARM based doarg, should look at MIPS version
//
static void doarg(Node p) {
        assert(p && p->syms[0]);
        p->syms[RX] = intconst(mkactual(4, p->syms[0]->u.c.v.i));
        p->x.argno = p->syms[RX]->u.c.v.i / 4;
}


//
//
//
static void local(Symbol p) 
{
  if (askregvar(p, rmap(ttob(p->type))) == 0)
    mkauto(p);
}


//
// emit code for function
//
static void function(Symbol f, Symbol caller[], Symbol callee[], int ncalls)
{
  int i, j;
  int varargs;
  int sizeisave, sizefsave;
  int dumpmask = 0;
  int reg      = 0;

  //
  // count args
  //
  for (i = 0; callee[i]; i++)
  { ; }

  varargs = variadic(f->type) || ( i > 0 ) && ( strcmp(callee[i-1]->name,"__builtin_va_alist") == 0 );

  //
  // initialize masks and stack offsets
  //
  usedmask[IREG] = usedmask[FREG] = 0;
  freemask[IREG] = freemask[FREG] = ~(unsigned)0;

  offset = maxoffset = maxargoffset = 0;

  //
  // loop through arguments
  //   compute offsets
  //   set dumpmask for args that need to be in memory
  //
  for (i = 0; callee[i]; i++) 
  {
    Symbol p = callee[i];
    Symbol q = caller[i];
    int size = roundup(q->type->size, 4);

    assert(q);

    //
    // FIXME: floating point arguments
    //
    if (isfloat(p->type) || reg >= ARG_NUM_REGS) 
    {
      p->x.offset = q->x.offset = offset;
      p->x.name   = q->x.name   = stringd(offset);
      p->sclass   = q->sclass   = AUTO;

      dumpmask |= 0xffff>>(15 - reg);

      if (p->type->size == 8)
        dumpmask |= 1<<(reg + 1);
    }

    //
    // args required in memory for address reference, varargs, and structures
    //
    else if (p->addressed || varargs || isstruct(p->type)) 
    {
      p->x.offset = offset;
      p->x.name   = stringd(offset);
      p->sclass   = q->sclass = AUTO;

      dumpmask |= 0xffff>>(16 - reg - size / 4);

    } 

    //
    //
    //
    else 
    {
      q->type   = p->type;
      p->sclass = q->sclass = REGISTER;

      if (askregvar(p, rmap(ttob(p->type))))
        q->x.name = ireg[reg]->x.name;
      else 
      {
        p->sclass   = q->sclass = AUTO;
        p->x.offset = offset;
        p->x.name   = stringd(offset);

        dumpmask |= 0xffff>>(15 - reg);
      }
    }

    offset += size;
    reg += size / 4;
  }

  //
  // remove non-argument registers from dumpmask
  //
  dumpmask &= 0xf;

  //
  // ??? check for arg list mismatch ???
  //
  assert(!caller[i]);

  //
  //
  //
  offset = 0;
  gencode(caller, callee);

  //
  // compute stack space required
  //
  maxargoffset = roundup(maxargoffset, 4);
  sizefsave    = 8 * bitcount(usedmask[FREG] & 0xf0);
  sizeisave    = 4 * bitcount(usedmask[IREG] & INTVAR);

  //
  // total frame size, with room for FP + return address
  //
  framesize = roundup( SIZE_PC_FP_SAVE + sizefsave + sizeisave + maxargoffset + maxoffset, 4);

  print(";\n");
  print("; usedmask[IREG]=%x, usedmask[FREG]=%x\n", usedmask[IREG], usedmask[FREG] );
  print("; framesize = %d, sizeisave = %d, sizefsave = %d, maxargoffset = %d, maxoffset = %d\n", framesize, sizeisave, sizefsave, maxargoffset, maxoffset);
  print(";\n");

  // 
  // Stack frame: intended layout, generated code doesn't quite match yet...
  // 
  // 
  // $FFFF_FFFF
  // 
  //           +  arg # N-1        +
  //           +  ...              +
  //           +  arg # 1          +
  //           +  arg # 0          +    caller's frame  ( space always reserved for arg 0..3 )
  //
  //           ---------------------
  //
  //           +  return address   +    ( always allocated but not used in leaf functions )
  //
  //  fp ->    +  saved fp         +    ( space allocated, fp not currently used )
  //
  //           +  locals           +  
  //           +  ...              +    maxoffset
  //           +  locals           +  
  //
  //           +  reg spill        +    sizeisave
  //           +  ...              +       +
  //           +  reg spill        +    sizefsave
  //
  //           +  outgoing args    +    
  //           +  ...              +    maxargoffset
  //  sp ->    +  outgoing args    +    
  //
  //           ---------------------
  //
  // $0000_0000
  //
  //
  
  segment(CODE);
  

  print(" .align 2\n");
  print("\n%s:\n", f->x.name);

  print("\n");

  //
  // stack offset
  //
  if ( alu_const(framesize,0) == 0 )
  {
    print(" sub sp,#%d\n", framesize);
  }
  else
  {
    print(" imm #%d\n", framesize);
    print(" sub sp,imm\n");
  }

  print("\n");

  //
  // FIXME: pop return address off hardware stack for non-leaf functions
  //
  if (ncalls)
  {
    print(";;\n");
    print(";; FIXME: when supported, pop return address off HW return stack for non-leaf routines\n");
    print(";; st.q rs, %d(sp)\n", framesize-4 );
    print(";;\n");
    print("\n");
  }


  //
  // dumpmask arg registers are copied to stack frame
  //
  print("; dumpmask arg copy \n");

  for ( i = 0; i < ARG_NUM_REGS; i++)
  {
    if ( dumpmask & (1<<i) )
    {
      print(" st.q %s, %d(sp)\n", ireg[i]->x.name, framesize + 4 * i );
    }
  }

  print("\n");


  //
  // save caller-saved registers
  //
  print("; caller-saved registers\n");

  for ( j = i = 0; i <= LAST_SAVED_REG; i++)
  {
    if ( (usedmask[IREG] & INTVAR) & (1<<i) )
    {
      print(" st.q %s, %d(sp)\n", ireg[i]->x.name, framesize - SIZE_PC_FP_SAVE - maxoffset - sizeisave + 4 * j );
      j++;
    }
  }

  //
  // TODO: replace old arm floating point save code
  //
  //        for (i = 7; i >= 4; i--)
  //                if (usedmask[FREG] & (1<<i))
  //                        print("\tstfe\t%s, [sp, #-12]!\n", freg[i]->x.name);

  print("\n");


  //
  // copy register args to register variables 
  //
  print("; copy reg args\n");

  for (i = 0; ( i < ARG_NUM_REGS ) && callee[i]; i++)
  {
    if ( caller[i]->sclass == REGISTER &&
         callee[i]->sclass == REGISTER &&
         caller[i]->type   == callee[i]->type
       )
       print(" mov %s, %s\n", callee[i]->x.name, caller[i]->x.name);
  }

  print("\n");


  //
  // 
  //
  print("; function body\n");
  emitcode();
  print("\n");

  //
  // Restore caller saved registers 
  //
  print("; restore caller-saved registers\n");

  //
  // TODO: replace old arm floating point restore code
  //
  //        for (i = 4; i <= 7; i++)
  //                if (usedmask[FREG] & (1<<i))
  //                        print("\tldfe\t%s, [fp, #-%d]\n", freg[i]->x.name,
  //                                sizeisave - 4 * bitcount(dumpmask) +
  //                                (8 - i) * 12 - 4);

  for ( j = i = 0; i <= LAST_SAVED_REG; i++)
  {
    if ( (usedmask[IREG] & INTVAR) & (1<<i) )
    {
      print(" ld.q %s, %d(sp)\n", ireg[i]->x.name, framesize - SIZE_PC_FP_SAVE - maxoffset - sizeisave + 4 * j );
      j++;
    }
  }


  print("\n");

  //
  // FIXME: restore saved return address for non-leaf functions
  //
  if (ncalls)
  {
    print(";;\n");
    print(";; FIXME: when supported, restore saved return address from software stack for non-leaf routines\n");
    print(";; ld.q rs, %d(sp)\n", framesize-4 );
    print(";;\n");
  }

  //
  // clean up stack frame
  //
  if ( alu_const(framesize,0) == 0 )
  {
    print(" add sp,#%d\n", framesize);
  }
  else if ( framesize <= 64 )
  {
    print(" lea sp, %d(sp)\n", framesize);
  }
  else
  {
    print(" imm #%d\n", framesize);
    print(" add sp,imm\n");
  }

  print("\n");

  //
  // return
  //
  print("\n");
  print(" rts\n");
  print("\n");
}


//
//  Output an inline constant of the given type and value
//
static void defconst(int suffix, int size, Value v) 
{
  /* float */
  if (suffix == F && size == 4) 
    {
      float f = v.d;
      print( " dc.q $%x    ; float %f\n", *(unsigned *)&f, f); return;
    }

  /* double */
  else if (suffix == F && size == 8) 
    {
      double d = v.d;
      unsigned *p = (unsigned *)&d;
      print( " dc.q $%x, $%x    ; double %f\n", p[swap], p[!swap], d);
    }

  /* pointer */
  else if (suffix == P)
    print( " dc.q $%x\n", v.p);

  /* byte */
  else if (size == 1)
    print( " dc.b %d\n", suffix == I ? (signed char)v.i : (unsigned char)v.u);

  /* short */
  else if (size == 2)
    print(" dc.w %d\n", suffix == I ? (short)v.i : (unsigned short)v.u);

  /* int */        
  else if (size == 4)
    print( " dc.q %d\n", suffix == I ? (int)v.i : (unsigned)v.u);
}


//
// Output address of a given symbol
//
static void defaddress(Symbol p) 
{
  print( " dc.q  %s\n", p->x.name);
}


//
// Output a string constant
//
// creates a string using dc.b with a list of hex values for chars,
// thus avoiding escape char problems with dc.s/dc.z strings
//
static void defstring(int n, char *str) 
{
  char *s;
  int first = 1;

  print(" dc.b ");

  //
  // first loop prints comma-separated hex values for dc.b
  //
  for (s = str; s < str + n; s++) 
    {
      if ( !first ) print(",");

      print("$%x", (*s) & 0xff );
      first = 0;
    }

  //
  // second loop prints string again as a comment with control characters replaced by '~'
  //
  //
  print("  ; \"");

  for (s = str; s < str + n; s++) 
    {
      print("%c", isprint(*s) ? (*s) : '~' );
    }

  print("\"\n\n");

}

//
//
//
static void export(Symbol p) 
{
  print( "\n .global %s\n", p->x.name);
}


//
//
//
static void import(Symbol p) 
{
  if (!isfunc(p->type))
    print(" .extern %s %d\n", p->name, p->type->size);
}


//
//
//
static void defsymbol(Symbol p) 
{
  if (p->scope >= LOCAL && p->sclass == STATIC)
    p->x.name = stringf( "L%d", genlabel(1));
//    p->x.name = stringf( ".L%d", genlabel(1));

  else if (p->generated)
    p->x.name = stringf( "L%s", p->name);
//    p->x.name = stringf( ".L%s", p->name);

  else
  {
    assert( p->scope != CONSTANTS || isint(p->type) || isptr(p->type) ); 
    p->x.name = p->name;
  }
}


//
//
//
static void address(Symbol q, Symbol p, long n) {
        q->x.offset = p->x.offset + n;
        if (p->scope == GLOBAL
        || p->sclass == STATIC || p->sclass == EXTERN)
                q->x.name = stringf("%s%s%d", p->x.name,
                        n >= 0 ? "+" : "", n);
        else
                q->x.name = stringf("%d", q->x.offset);
}


//
//
//
static void global(Symbol p) 
{
  assert(p->u.seg);

  if (p->u.seg == BSS && p->sclass == STATIC)
    print("\n%s: .common %d,%d\n .local %s\n ", p->x.name, p->type->size, p->type->align, p->x.name);

  else if (p->u.seg == BSS && Aflag >= 2)
    print("\n .align %d\n%s:\n .reserve %d\n", p->type->align, p->x.name, p->type->size);

  else if (p->u.seg == BSS)
    print("\n%s: .common %d,%d\n", p->x.name, p->type->size, p->type->align);

  else
    print(" .align %d\n\n%s:\n", p->type->align, p->x.name);


  if (!p->generated) 
  {
    print(" .type %s,#%s\n", p->x.name, isfunc(p->type) ? "function" : "object");
    if (p->type->size > 0)
      print(" .size %s,%d\n", p->x.name, p->type->size);

  }

}


//
//
//
static void segment(int n) 
{
  cseg = n;

  switch (n) 
  {
    case CODE : print(" .section text\n");    break;
    case BSS  : print(" .section bss\n");     break;
    case DATA : print(" .section data\n");    break;
    case LIT  : print(" .section rodata\n");  break;
  }
}


//
//
//
static void space(int n) 
{
  if (cseg != BSS)  print( " %d\n", n);
}


//
// TODO: block copy functions
//
static void blkloop(int dreg, int doff, int sreg, int soff, int size, int tmps[]) 
{
  print( "\n .error \"Block copy functions not implemented yet\"\n" );
}

static void blkfetch(int size, int off, int reg, int tmp) 
{
  print( "\n .error \"Block copy functions not implemented yet\"\n" );
}

static void blkstore(int size, int off, int reg, int tmp) 
{
  print( "\n .error \"Block copy functions not implemented yet\"\n" );
}


//////////////////////////////////////////////////////
//
// misc. support routines
//
//////////////////////////////////////////////////////

//
//
//
static int bitcount(unsigned mask) 
{
  unsigned i, n = 0;

  for (i = 1; i; i <<= 1)
  {
    if (mask & i) n++;
  }
  return n;
}

//
// alu_const
//   returns zero cost if constant value fits into the YARD encoded immediate format
//
//   logicals = 0   checks Arithmetic constants : 5 bit signed, 2^N, and (2^N)-1
//   logicals = 1   checks Logical constants    : above plus NOT variants
//
//
static int alu_const(long value, int logicals) 
{
  int i;
  unsigned bitset;
  //
  // loop to test for a match to the encoded immediates
  //   first iteration tests against normal value
  //   if 'logicals' is 1, second iteration tests against complemented value
  //
  for ( i = 0; i <= logicals; i++ )
    {
      if ( ( value >= -16 ) && ( value <= 15 ) ) return 0;

      for ( bitset = 0x80000000; bitset != 0x00000000; bitset = bitset >> 1 )
        {
          if ( ( (unsigned) value ) == bitset   )   return 0;

          if ( ( (unsigned) value ) == (bitset-1) ) return 0;
        }
      value = ~value;
    }

  return LBURG_MAX;
}


//
// find_single_bit_set
//   returns log2(value) if a single bit is set, else returns -1
//
static int find_single_bit_set( unsigned long value )
{
  int i;
  unsigned bitset;

  //
  // loop to test for power of two
  //
  bitset = 0x80000000;

  for ( i = 31; i >= 0; i-- )
  {
    if ( value == bitset ) { return i; }
    bitset = bitset >> 1;
  }

  return -1;
}

//////////////////////////////////////////////////////
//
// cost functions, used by rules, test Node structure
//
//////////////////////////////////////////////////////

//
// cost function tests for a power-of-two constant
//
static int cost_pow2(Node p) 
  {
    int i;
    unsigned bitset;
    unsigned long value;

    Symbol s = p->syms[0];

    switch (specific(p->op)) 
      {
        case ADDRF+P :
        case ADDRL+P : value = s->x.offset; break;
        case CNST+I  : value = s->u.c.v.i;  break;
        case CNST+U  : value = s->u.c.v.u;  break;

// FIXME - should P values be handled similarly, or check for null like other targets?
//        case CNST+P  : value = s->u.c.v.p; break;
        case CNST+P  : if (s->u.c.v.p == 0) return 0;

        default      : return LBURG_MAX;
      }

    if ( find_single_bit_set(value) >= 0 ) { return 0; }

    return LBURG_MAX;
}

//
// cost_alu_const
//   returns low cost if Node constant fits into the YARD encoded immediate format
//
//   logicals = 0   checks Arithmetic constants : 5 bit signed, 2^N, and (2^N)-1
//   logicals = 1   checks Logical constants    : above plus NOT variants
//
//
static int cost_alu_const(Node p, int logicals) 
  {
    int i;
    unsigned bitset;

// FIXME: signed/unsigned constant matching problems
//    unsigned long value;
    long value;

    Symbol s = p->syms[0];

    switch (specific(p->op)) 
      {
        case ADDRF+P :
        case ADDRL+P : value = s->x.offset; break;
        case CNST+I  : value = s->u.c.v.i;  break;
        case CNST+U  : value = s->u.c.v.u;  break;

// FIXME - should P values be handled similarly, or check for null like other targets?
//        case CNST+P  : value = s->u.c.v.p; break;
        case CNST+P  : if (s->u.c.v.p == 0) return 0;

        default      : return LBURG_MAX;
      }

    return alu_const(value, logicals);
}

//////////////////////////////////////////////////////

//
// FIXME: stab routines from MIPS target
//
static void stabinit(char *, int, char *[]);
static void stabline(Coordinate *);
static void stabsym(Symbol);

static char *currentfile;

/* stabinit - initialize stab output */
static void stabinit(char *file, int argc, char *argv[]) {
        if (file) {
                print(".file 2,\"%s\"\n", file);
                currentfile = file;
        }
}

/* stabline - emit stab entry for source coordinate *cp */
static void stabline(Coordinate *cp) {
        if (cp->file && cp->file != currentfile) {
                print(".file 2,\"%s\"\n", cp->file);
                currentfile = cp->file;
        }
        print(".loc 2,%d\n", cp->y);
}

/* stabsym - output a stab entry for symbol p */
static void stabsym(Symbol p) {
        if (p == cfunc && IR->stabline)
                (*IR->stabline)(&p->src);
}

//////////////////////////////////////////////////////

//
//
//
Interface yardIR =  
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

  0,          /* little_endian */
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

  0, 0, 0, stabinit, stabline, stabsym, 0,

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


