#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<errno.h>
#include<sys/stat.h>
#include<sys/types.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define s8 signed char
#define s16 signed short
#define s32 signed int
#define s64 signed long long
#ifndef O_BINARY
	#define O_BINARY 0x0
#endif
int hexstr2u32(void* str, void* dat);


/*
x0 zero Hard-wired zero —
x1 ra Return address Caller
x2 sp Stack pointer Callee
x3 gp Global pointer —
x4 tp Thread pointer —
x5 t0 Temporary/alternate link register Caller
x6–7 t1–2 Temporaries Caller
x8 s0/fp Saved register/frame pointer Callee
x9 s1 Saved register Callee
x10–11 a0–1 Function arguments/return values Caller
x12–17 a2–7 Function arguments Caller
x18–27 s2–11 Saved registers Callee
x28–31 t3–6 Temporaries Caller
*/
static char* regname[32] = {
"zero",
"ra",
"sp",
"gp",
"tp",
"t0",	//5
"t1",
"t2",
"s0",	//8
"s1",
"a0",	//10
"a1",
"a2",
"a3",
"a4",
"a5",
"a6",	//16
"a7",
"s2",	//18
"s3",
"s4",
"s5",
"s6",
"s7",
"s8",
"s9",
"s10",
"s11",
"t3",	//28
"t4",
"t5",
"t6"
};
/*
f0–7 ft0–7 FP temporaries Caller
f8–9 fs0–1 FP saved registers Callee
f10–11 fa0–1 FP arguments/return values Caller
f12–17 fa2–7 FP arguments Caller
f18–27 fs2–11 FP saved registers Callee
f28–31 ft8–11 FP temporaries Caller
*/


struct rv64r{
	u32 opcode :7;
	u32     rd :5;
	u32 funct3 :3;
	u32    rs1 :5;
	u32    rs2 :5;
	u32 funct7 :7;
};
struct rv64i{
	u32 opcode :7;
	u32     rd :5;
	u32 funct3 :3;
	u32    rs1 :5;
	u32    imm :12;
};
struct rv64s{
	u32  opcode :7;
	u32 imm_lo5 :5;
	u32  funct3 :3;
	u32     rs1 :5;
	u32     rs2 :5;
	u32 imm_hi7 :7;
};
struct rv64u{
	u32 opcode :7;
	u32     rd :5;
	u32    imm :20;
};
//addi
//li: addi.rs1=r0
//mv: addi.imm=0


/*
//RV32I Base Instruction Set
imm[31:12] rd 0110111 LUI
imm[31:12] rd 0010111 AUIPC
imm[20|10:1|11|19:12] rd 1101111 JAL
imm[11:0] rs1 000 rd 1100111 JALR
imm[12|10:5] rs2 rs1 000 imm[4:1|11] 1100011 BEQ
imm[12|10:5] rs2 rs1 001 imm[4:1|11] 1100011 BNE
imm[12|10:5] rs2 rs1 100 imm[4:1|11] 1100011 BLT
imm[12|10:5] rs2 rs1 101 imm[4:1|11] 1100011 BGE
imm[12|10:5] rs2 rs1 110 imm[4:1|11] 1100011 BLTU
imm[12|10:5] rs2 rs1 111 imm[4:1|11] 1100011 BGEU
imm[11:0] rs1 000 rd 0000011 LB
imm[11:0] rs1 001 rd 0000011 LH
imm[11:0] rs1 010 rd 0000011 LW
imm[11:0] rs1 100 rd 0000011 LBU
imm[11:0] rs1 101 rd 0000011 LHU
imm[11:5] rs2 rs1 000 imm[4:0] 0100011 SB
imm[11:5] rs2 rs1 001 imm[4:0] 0100011 SH
imm[11:5] rs2 rs1 010 imm[4:0] 0100011 SW
imm[11:0] rs1 000 rd 0010011 ADDI
imm[11:0] rs1 010 rd 0010011 SLTI
imm[11:0] rs1 011 rd 0010011 SLTIU
imm[11:0] rs1 100 rd 0010011 XORI
imm[11:0] rs1 110 rd 0010011 ORI
imm[11:0] rs1 111 rd 0010011 ANDI
0000000 shamt rs1 001 rd 0010011 SLLI
0000000 shamt rs1 101 rd 0010011 SRLI
0100000 shamt rs1 101 rd 0010011 SRAI
0000000 rs2 rs1 000 rd 0110011 ADD
0100000 rs2 rs1 000 rd 0110011 SUB
0000000 rs2 rs1 001 rd 0110011 SLL
0000000 rs2 rs1 010 rd 0110011 SLT
0000000 rs2 rs1 011 rd 0110011 SLTU
0000000 rs2 rs1 100 rd 0110011 XOR
0000000 rs2 rs1 101 rd 0110011 SRL
0100000 rs2 rs1 101 rd 0110011 SRA
0000000 rs2 rs1 110 rd 0110011 OR
0000000 rs2 rs1 111 rd 0110011 AND
0000 pred succ 00000 000 00000 0001111 FENCE
0000 0000 0000 00000 001 00000 0001111 FENCE.I
000000000000 00000 000 00000 1110011 ECALL
000000000001 00000 000 00000 1110011 EBREAK
csr rs1 001 rd 1110011 CSRRW
csr rs1 010 rd 1110011 CSRRS
csr rs1 011 rd 1110011 CSRRC
csr zimm 101 rd 1110011 CSRRWI
csr zimm 110 rd 1110011 CSRRSI
csr zimm 111 rd 1110011 CSRRCI
//funct7 rs2 rs1 funct3 rd opcode R-type
//imm[11:0] rs1 funct3 rd opcode I-type
//imm[11:5] rs2 rs1 funct3 imm[4:0] opcode S-type
//RV64I Base Instruction Set (in addition to RV32I)
imm[11:0] rs1 110 rd 0000011 LWU
imm[11:0] rs1 011 rd 0000011 LD
imm[11:5] rs2 rs1 011 imm[4:0] 0100011 SD
000000 shamt rs1 001 rd 0010011 SLLI
000000 shamt rs1 101 rd 0010011 SRLI
010000 shamt rs1 101 rd 0010011 SRAI
imm[11:0] rs1 000 rd 0011011 ADDIW
0000000 shamt rs1 001 rd 0011011 SLLIW
0000000 shamt rs1 101 rd 0011011 SRLIW
0100000 shamt rs1 101 rd 0011011 SRAIW
0000000 rs2 rs1 000 rd 0111011 ADDW
0100000 rs2 rs1 000 rd 0111011 SUBW
0000000 rs2 rs1 001 rd 0111011 SLLW
0000000 rs2 rs1 101 rd 0111011 SRLW
0100000 rs2 rs1 101 rd 0111011 SRAW
//RV32M Standard Extension
0000001 rs2 rs1 000 rd 0110011 MUL
0000001 rs2 rs1 001 rd 0110011 MULH
0000001 rs2 rs1 010 rd 0110011 MULHSU
0000001 rs2 rs1 011 rd 0110011 MULHU
0000001 rs2 rs1 100 rd 0110011 DIV
0000001 rs2 rs1 101 rd 0110011 DIVU
0000001 rs2 rs1 110 rd 0110011 REM
0000001 rs2 rs1 111 rd 0110011 REMU
//RV64M Standard Extension (in addition to RV32M)
0000001 rs2 rs1 000 rd 0111011 MULW
0000001 rs2 rs1 100 rd 0111011 DIVW
0000001 rs2 rs1 101 rd 0111011 DIVUW
0000001 rs2 rs1 110 rd 0111011 REMW
0000001 rs2 rs1 111 rd 0111011 REMUW
//RV32A Standard Extension
00010 aq rl 00000 rs1 010 rd 0101111 LR.W
00011 aq rl rs2 rs1 010 rd 0101111 SC.W
00001 aq rl rs2 rs1 010 rd 0101111 AMOSWAP.W
00000 aq rl rs2 rs1 010 rd 0101111 AMOADD.W
00100 aq rl rs2 rs1 010 rd 0101111 AMOXOR.W
01100 aq rl rs2 rs1 010 rd 0101111 AMOAND.W
01000 aq rl rs2 rs1 010 rd 0101111 AMOOR.W
10000 aq rl rs2 rs1 010 rd 0101111 AMOMIN.W
10100 aq rl rs2 rs1 010 rd 0101111 AMOMAX.W
11000 aq rl rs2 rs1 010 rd 0101111 AMOMINU.W
11100 aq rl rs2 rs1 010 rd 0101111 AMOMAXU.W
//funct7 rs2 rs1 funct3 rd opcode R-type
rs3 funct2 rs2 rs1 funct3 rd opcode R4-type
//imm[11:0] rs1 funct3 rd opcode I-type
//imm[11:5] rs2 rs1 funct3 imm[4:0] opcode S-type
//RV64A Standard Extension (in addition to RV32A)
00010 aq rl 00000 rs1 011 rd 0101111 LR.D
00011 aq rl rs2 rs1 011 rd 0101111 SC.D
00001 aq rl rs2 rs1 011 rd 0101111 AMOSWAP.D
00000 aq rl rs2 rs1 011 rd 0101111 AMOADD.D
00100 aq rl rs2 rs1 011 rd 0101111 AMOXOR.D
01100 aq rl rs2 rs1 011 rd 0101111 AMOAND.D
01000 aq rl rs2 rs1 011 rd 0101111 AMOOR.D
10000 aq rl rs2 rs1 011 rd 0101111 AMOMIN.D
10100 aq rl rs2 rs1 011 rd 0101111 AMOMAX.D
11000 aq rl rs2 rs1 011 rd 0101111 AMOMINU.D
11100 aq rl rs2 rs1 011 rd 0101111 AMOMAXU.D
//RV32F Standard Extension
imm[11:0] rs1 010 rd 0000111 FLW
imm[11:5] rs2 rs1 010 imm[4:0] 0100111 FSW
rs3 00 rs2 rs1 rm rd 1000011 FMADD.S
rs3 00 rs2 rs1 rm rd 1000111 FMSUB.S
rs3 00 rs2 rs1 rm rd 1001011 FNMSUB.S
rs3 00 rs2 rs1 rm rd 1001111 FNMADD.S
0000000 rs2 rs1 rm rd 1010011 FADD.S
0000100 rs2 rs1 rm rd 1010011 FSUB.S
0001000 rs2 rs1 rm rd 1010011 FMUL.S
0001100 rs2 rs1 rm rd 1010011 FDIV.S
0101100 00000 rs1 rm rd 1010011 FSQRT.S
0010000 rs2 rs1 000 rd 1010011 FSGNJ.S
0010000 rs2 rs1 001 rd 1010011 FSGNJN.S
0010000 rs2 rs1 010 rd 1010011 FSGNJX.S
0010100 rs2 rs1 000 rd 1010011 FMIN.S
0010100 rs2 rs1 001 rd 1010011 FMAX.S
1100000 00000 rs1 rm rd 1010011 FCVT.W.S
1100000 00001 rs1 rm rd 1010011 FCVT.WU.S
1110000 00000 rs1 000 rd 1010011 FMV.X.W
1010000 rs2 rs1 010 rd 1010011 FEQ.S
1010000 rs2 rs1 001 rd 1010011 FLT.S
1010000 rs2 rs1 000 rd 1010011 FLE.S
1110000 00000 rs1 001 rd 1010011 FCLASS.S
1101000 00000 rs1 rm rd 1010011 FCVT.S.W
1101000 00001 rs1 rm rd 1010011 FCVT.S.WU
1111000 00000 rs1 000 rd 1010011 FMV.W.X
//funct7 rs2 rs1 funct3 rd opcode R-type
//rs3 funct2 rs2 rs1 funct3 rd opcode R4-type
imm[11:0] rs1 funct3 rd opcode I-type
imm[11:5] rs2 rs1 funct3 imm[4:0] opcode S-type
//RV64F Standard Extension (in addition to RV32F)
1100000 00010 rs1 rm rd 1010011 FCVT.L.S
1100000 00011 rs1 rm rd 1010011 FCVT.LU.S
1101000 00010 rs1 rm rd 1010011 FCVT.S.L
1101000 00011 rs1 rm rd 1010011 FCVT.S.LU
//RV32D Standard Extension
imm[11:0] rs1 011 rd 0000111 FLD
imm[11:5] rs2 rs1 011 imm[4:0] 0100111 FSD
rs3 01 rs2 rs1 rm rd 1000011 FMADD.D
rs3 01 rs2 rs1 rm rd 1000111 FMSUB.D
rs3 01 rs2 rs1 rm rd 1001011 FNMSUB.D
rs3 01 rs2 rs1 rm rd 1001111 FNMADD.D
0000001 rs2 rs1 rm rd 1010011 FADD.D
0000101 rs2 rs1 rm rd 1010011 FSUB.D
0001001 rs2 rs1 rm rd 1010011 FMUL.D
0001101 rs2 rs1 rm rd 1010011 FDIV.D
0101101 00000 rs1 rm rd 1010011 FSQRT.D
0010001 rs2 rs1 000 rd 1010011 FSGNJ.D
0010001 rs2 rs1 001 rd 1010011 FSGNJN.D
0010001 rs2 rs1 010 rd 1010011 FSGNJX.D
0010101 rs2 rs1 000 rd 1010011 FMIN.D
0010101 rs2 rs1 001 rd 1010011 FMAX.D
0100000 00001 rs1 rm rd 1010011 FCVT.S.D
0100001 00000 rs1 rm rd 1010011 FCVT.D.S
1010001 rs2 rs1 010 rd 1010011 FEQ.D
1010001 rs2 rs1 001 rd 1010011 FLT.D
1010001 rs2 rs1 000 rd 1010011 FLE.D
1110001 00000 rs1 001 rd 1010011 FCLASS.D
1100001 00000 rs1 rm rd 1010011 FCVT.W.D
1100001 00001 rs1 rm rd 1010011 FCVT.WU.D
1101001 00000 rs1 rm rd 1010011 FCVT.D.W
1101001 00001 rs1 rm rd 1010011 FCVT.D.WU
RV64D Standard Extension (in addition to RV32D)
1100001 00010 rs1 rm rd 1010011 FCVT.L.D
1100001 00011 rs1 rm rd 1010011 FCVT.LU.D
1110001 00000 rs1 000 rd 1010011 FMV.X.D
1101001 00010 rs1 rm rd 1010011 FCVT.D.L
1101001 00011 rs1 rm rd 1010011 FCVT.D.LU
1111001 00000 rs1 000 rd 1010011 FMV.D.X
*/


void disasm_riscv64_one(u32 code, u64 rip)
{
	printf("%8llx:	%08x	", rip, code);
	struct rv64r* p = (struct rv64r*)&code;
	switch(p->opcode){
/*
imm[11:0] rs1 000 rd 0000011 LB		//rv32i
imm[11:0] rs1 001 rd 0000011 LH
imm[11:0] rs1 010 rd 0000011 LW
imm[11:0] rs1 100 rd 0000011 LBU
imm[11:0] rs1 101 rd 0000011 LHU
imm[11:0] rs1 110 rd 0000011 LWU	//rv64i
imm[11:0] rs1 011 rd 0000011 LD
*/
	case 0x03:
		switch(p->funct3){
		case 0:
			printf("lb	rd=%x,rs1=%x,rs2=%x,funct7=%x",p->rd,p->rs1,p->rs2,p->funct7);
			break;
		case 1:
			printf("lh	rd=%x,rs1=%x,rs2=%x,funct7=%x",p->rd,p->rs1,p->rs2,p->funct7);
			break;
		case 2:
			printf("lw	rd=%x,rs1=%x,rs2=%x,funct7=%x",p->rd,p->rs1,p->rs2,p->funct7);
			break;
		case 3:
			printf("ld	rd=%x,rs1=%x,rs2=%x,funct7=%x",p->rd,p->rs1,p->rs2,p->funct7);
			break;
		case 4:
			printf("lbu	rd=%x,rs1=%x,rs2=%x,funct7=%x",p->rd,p->rs1,p->rs2,p->funct7);
			break;
		case 5:
			printf("lhu	rd=%x,rs1=%x,rs2=%x,funct7=%x",p->rd,p->rs1,p->rs2,p->funct7);
			break;
		case 6:
			printf("lwu	rd=%x,rs1=%x,rs2=%x,funct7=%x",p->rd,p->rs1,p->rs2,p->funct7);
			break;
		default:
			printf("opcode=%x,rd=%x,funct3=%x,rs1=%x,rs2=%x,funct7=%x",p->opcode,p->rd,p->funct3,p->rs1,p->rs2,p->funct7);
		}
		break;
/*
imm[11:0] rs1 000 rd 0010011 ADDI
imm[11:0] rs1 010 rd 0010011 SLTI
imm[11:0] rs1 011 rd 0010011 SLTIU
imm[11:0] rs1 100 rd 0010011 XORI
imm[11:0] rs1 110 rd 0010011 ORI
imm[11:0] rs1 111 rd 0010011 ANDI
0000000 shamt rs1 001 rd 0010011 SLLI
0000000 shamt rs1 101 rd 0010011 SRLI
0100000 shamt rs1 101 rd 0010011 SRAI
*/
	case 0x13:
		switch(p->funct3){
		case 0:
			if( (0 == p->rs1) && (0 == p->rs2) ){
				printf("li	%s = 0x%x",
					regname[p->rd],
					(p->funct7<0x40) ? (32*p->funct7) : (0x80-p->funct7)*32
				);
			}
			else if( (0 == p->funct7) && (0 == p->rs2) ){
				printf("mv	%s = %s",
					regname[p->rd],
					regname[p->rs1]
				);
			}
			else if(0 == p->rs2){
				printf("addi	%s = %s %c 0x%x",
					regname[p->rd],
					regname[p->rs1],
					(p->funct7<0x40) ? '+' : '-',
					(p->funct7<0x40) ? (32*p->funct7) : (0x80-p->funct7)*32
				);
			}
			else{
				printf("addi	%s = %s + %s %c 0x%x",
					regname[p->rd],
					regname[p->rs1],
					regname[p->rs2],
					(p->funct7<0x40) ? '+' : '-',
					(p->funct7<0x40) ? (32*p->funct7) : (0x80-p->funct7)*32
				);
			}
			break;
		case 1:
			printf("slli	rd=%x,rs1=%x,rs2=%x,funct7=%x",p->rd,p->rs1,p->rs2,p->funct7);
			break;
		case 2:
			printf("slti	rd=%x,rs1=%x,rs2=%x,funct7=%x",p->rd,p->rs1,p->rs2,p->funct7);
			break;
		case 3:
			printf("sltiu	rd=%x,rs1=%x,rs2=%x,funct7=%x",p->rd,p->rs1,p->rs2,p->funct7);
			break;
		case 4:
			printf("xori	rd=%x,rs1=%x,rs2=%x,funct7=%x",p->rd,p->rs1,p->rs2,p->funct7);
			break;
		case 5:
			printf("sr?i	rd=%x,rs1=%x,rs2=%x,funct7=%x",p->rd,p->rs1,p->rs2,p->funct7);
			break;
		case 6:
			printf("ori	rd=%x,rs1=%x,rs2=%x,funct7=%x",p->rd,p->rs1,p->rs2,p->funct7);
			break;
		case 7:
			printf("andi	rd=%x,rs1=%x,rs2=%x,funct7=%x",p->rd,p->rs1,p->rs2,p->funct7);
			break;
		}
		break;
/*
imm[11:5] rs2 rs1 000 imm[4:0] 0100011 SB	//rv32i
imm[11:5] rs2 rs1 001 imm[4:0] 0100011 SH
imm[11:5] rs2 rs1 010 imm[4:0] 0100011 SW
imm[11:5] rs2 rs1 011 imm[4:0] 0100011 SD	//rv64i
*/
	case 0x23:
		switch(p->funct3){
		case 0:
			printf("sb	rd=%x,rs1=%x,rs2=%x,funct7=%x",p->rd,p->rs1,p->rs2,p->funct7);
			break;
		case 1:
			printf("sh	rd=%x,rs1=%x,rs2=%x,funct7=%x",p->rd,p->rs1,p->rs2,p->funct7);
			break;
		case 2:
			printf("sw	rd=%x,rs1=%x,rs2=%x,funct7=%x",p->rd,p->rs1,p->rs2,p->funct7);
			break;
		case 3:
			printf("sd	rd=%x,rs1=%x,rs2=%x,funct7=%x",p->rd,p->rs1,p->rs2,p->funct7);
			break;
		default:
			printf("opcode=%x,rd=%x,funct3=%x,rs1=%x,rs2=%x,funct7=%x",p->opcode,p->rd,p->funct3,p->rs1,p->rs2,p->funct7);
		}
		break;
	case 0x33:
		switch(p->funct3){
/*
0000000 rs2 rs1 000 rd 0110011 ADD		//rv32i
0100000 rs2 rs1 000 rd 0110011 SUB
0000001 rs2 rs1 000 rd 0110011 MUL		//RV32M
*/
		case 0:
			if(0 == p->funct7){
				printf("add	rd=%x,rs1=%x,rs2=%x",p->rd,p->rs1,p->rs2);
			}
			else if(0x20 == p->funct7){
				printf("sub	rd=%x,rs1=%x,rs2=%x",p->rd,p->rs1,p->rs2);
			}
			else if(1 == p->funct7){
				printf("mul	rd=%x,rs1=%x,rs2=%x",p->rd,p->rs1,p->rs2);
			}
			else{
				printf("?	rd=%x,rs1=%x,rs2=%x,funct7=%x",p->rd,p->rs1,p->rs2,p->funct7);
			}
			break;
/*
0000000 rs2 rs1 001 rd 0110011 SLL		//rv32i
0000001 rs2 rs1 001 rd 0110011 MULH		//RV32M
*/
		case 1:
			if(0 == p->funct7){
				printf("sll	rd=%x,rs1=%x,rs2=%x",p->rd,p->rs1,p->rs2);
			}
			else if(1 == p->funct7){
				printf("mulh	rd=%x,rs1=%x,rs2=%x",p->rd,p->rs1,p->rs2);
			}
			else{
				printf("?	rd=%x,rs1=%x,rs2=%x,funct7=%x",p->rd,p->rs1,p->rs2,p->funct7);
			}
			break;
/*
0000000 rs2 rs1 010 rd 0110011 SLT		//rv32i
0000001 rs2 rs1 010 rd 0110011 MULHSU		//RV32M
*/
		case 2:
			if(0 == p->funct7){
				printf("slt	rd=%x,rs1=%x,rs2=%x",p->rd,p->rs1,p->rs2);
			}
			else if(1 == p->funct7){
				printf("mulhsu	rd=%x,rs1=%x,rs2=%x",p->rd,p->rs1,p->rs2);
			}
			else{
				printf("?	rd=%x,rs1=%x,rs2=%x,funct7=%x",p->rd,p->rs1,p->rs2,p->funct7);
			}
			break;
/*
0000000 rs2 rs1 011 rd 0110011 SLTU		//rv32i
0000001 rs2 rs1 011 rd 0110011 MULHU		//RV32M
*/
		case 3:
			if(0 == p->funct7){
				printf("sltu	rd=%x,rs1=%x,rs2=%x",p->rd,p->rs1,p->rs2);
			}
			else if(1 == p->funct7){
				printf("mulhu	rd=%x,rs1=%x,rs2=%x",p->rd,p->rs1,p->rs2);
			}
			else{
				printf("?	rd=%x,rs1=%x,rs2=%x,funct7=%x",p->rd,p->rs1,p->rs2,p->funct7);
			}
			break;
/*
0000000 rs2 rs1 100 rd 0110011 XOR		//rv32i
0000001 rs2 rs1 100 rd 0110011 DIV		//RV32M
*/
		case 4:
			if(0 == p->funct7){
				printf("xor	rd=%x,rs1=%x,rs2=%x",p->rd,p->rs1,p->rs2);
			}
			else if(1 == p->funct7){
				printf("div	rd=%x,rs1=%x,rs2=%x",p->rd,p->rs1,p->rs2);
			}
			else{
				printf("?	rd=%x,rs1=%x,rs2=%x,funct7=%x",p->rd,p->rs1,p->rs2,p->funct7);
			}
			break;
/*
0000000 rs2 rs1 101 rd 0110011 SRL		//rv32i
0100000 rs2 rs1 101 rd 0110011 SRA		//rv32i
0000001 rs2 rs1 101 rd 0110011 DIVU		//RV32M
*/
		case 5:
			if(0 == p->funct7){
				printf("srl	rd=%x,rs1=%x,rs2=%x",p->rd,p->rs1,p->rs2);
			}
			else if(0x20 == p->funct7){
				printf("sra	rd=%x,rs1=%x,rs2=%x",p->rd,p->rs1,p->rs2);
			}
			else if(1 == p->funct7){
				printf("divu	rd=%x,rs1=%x,rs2=%x",p->rd,p->rs1,p->rs2);
			}
			else{
				printf("?	rd=%x,rs1=%x,rs2=%x,funct7=%x",p->rd,p->rs1,p->rs2,p->funct7);
			}
			break;
/*
0000000 rs2 rs1 110 rd 0110011 OR		//rv32i
0000001 rs2 rs1 110 rd 0110011 REM		//RV32M
*/
		case 6:
			if(0 == p->funct7){
				printf("or	rd=%x,rs1=%x,rs2=%x",p->rd,p->rs1,p->rs2);
			}
			else if(1 == p->funct7){
				printf("rem	rd=%x,rs1=%x,rs2=%x",p->rd,p->rs1,p->rs2);
			}
			else{
				printf("?	rd=%x,rs1=%x,rs2=%x,funct7=%x",p->rd,p->rs1,p->rs2,p->funct7);
			}
			break;
/*
0000000 rs2 rs1 111 rd 0110011 AND		//rv32i
0000001 rs2 rs1 111 rd 0110011 REMU		//RV32M
*/
		case 7:
			if(0 == p->funct7){
				printf("and	rd=%x,rs1=%x,rs2=%x",p->rd,p->rs1,p->rs2);
			}
			else if(1 == p->funct7){
				printf("remu	rd=%x,rs1=%x,rs2=%x",p->rd,p->rs1,p->rs2);
			}
			else{
				printf("?	rd=%x,rs1=%x,rs2=%x,funct7=%x",p->rd,p->rs1,p->rs2,p->funct7);
			}
			break;
		}
		break;
/*
imm[11:0] rs1 000 rd 1100111 JALR
*/
	case 0x67:
		if(0x8067 == code){
			printf("ret");
		}
		else{
			printf("opcode=%x,rd=%x,funct3=%x,rs1=%x,rs2=%x,funct7=%x",p->opcode,p->rd,p->funct3,p->rs1,p->rs2,p->funct7);
		}
		break;
/*
imm[20|10:1|11|19:12] rd 1101111 JAL
*/
	case 0x6f:
		printf("opcode=%x,rd=%x,funct3=%x,rs1=%x,rs2=%x,funct7=%x",p->opcode,p->rd,p->funct3,p->rs1,p->rs2,p->funct7);
		break;
	default:
		printf("opcode=%x,rd=%x,funct3=%x,rs1=%x,rs2=%x,funct7=%x",p->opcode,p->rd,p->funct3,p->rs1,p->rs2,p->funct7);
	}
	printf("\n");
}
void disasm_riscv64_all(u8* buf, int len, int rip)
{
	int j;
	u32 code;
	for(j=0;j<len;j+=4){
		code = *(u32*)(buf+j);
		disasm_riscv64_one(code, rip+j);
	}
}
void disasm_riscv64(int argc, char** argv)
{
	u32 at = 0;
	u32 sz = 0;
	if(argc < 2)return;
	if(argc > 2)hexstr2u32(argv[2], &at);
	if(argc > 3)hexstr2u32(argv[3], &sz);
	if(0 == sz)sz = 0x1000000;

	int fd = open(argv[1] , O_RDONLY|O_BINARY);
	if(fd <= 0){
		printf("errno=%d@open\n", errno);
		return;
	}

	u8* buf = malloc(sz);
	if(0 == buf){
		printf("errno=%d@malloc\n", errno);
		goto theend;
	}

	int ret = lseek(fd, at, SEEK_SET);
	if(ret < 0){
		printf("errno=%d@lseek\n", errno);
		goto release;
	}

	ret = read(fd, buf, sz);
	if(ret <= 0){
		printf("errno=%d@read\n", errno);
		goto release;
	}
	disasm_riscv64_all(buf, ret, 0);

release:
	free(buf);
theend:
	close(fd);
}




struct offlen{
	u8 off;
	u8 len;
}__attribute__((packed));
void assembly_riscv64_mov(u8* buf, int len, struct offlen* tab, int cnt)
{
	printf("dst=%.*s, src=%.*s\n", tab[0].len, buf+tab[0].off, tab[2].len, buf+tab[2].off);
}
void assembly_riscv64_add(u8* buf, int len, struct offlen* tab, int cnt)
{
	printf("dst=%.*s, src0=%.*s, src1=%.*s\n", tab[0].len, buf+tab[0].off, tab[2].len, buf+tab[2].off, tab[4].len, buf+tab[4].off);
}
void assembly_compile_riscv64(u8* buf, int len, struct offlen* tab, int cnt)
{
	int j;
/*	for(j=0;j<cnt;j++){
		printf("%d: %.*s\n", j, tab[j].len, buf+tab[j].off);
	}
*/
	if(0 == strncmp((char*)buf+tab[0].off, "mov", 3)){
		assembly_riscv64_mov(buf, len, &tab[1], cnt-1);
	}
	if(0 == strncmp((char*)buf+tab[0].off, "add", 3)){
		assembly_riscv64_add(buf, len, &tab[1], cnt-1);
	}
}

//riscv64-elf-as test.s -o test.o
//riscv64-elf-gcc -march=rv64i -mabi=lp64 -c test.c