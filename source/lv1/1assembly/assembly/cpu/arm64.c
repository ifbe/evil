//aarch64-elf-objdump -b binary -m aarch64 -D ~/kernel8.img
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


struct arm64_abs_scalar{
	u32 Rd :5;
	u32 Rn :5;
	u32 b10_b21_0x82e :12;
	u32 size :2;
	u32 b24_b28_0x1e :5;
	u32 U_0x0 :1;
	u32 b30_b31_0x1 :2;
};
struct arm64_abs_vector{
	u32 Rd :5;
	u32 Rn :5;
	u32 b10_b21_0x82e :12;
	u32 size :2;
	u32 b24_b28_0x0e :5;
	u32 U_0x0 :1;
	u32 Q :1;
	u32 b31_0x0 :1;
};
struct arm64_adc{
	u32 Rd :5;
	u32 Rn :5;
	u32 b10_b15_0x0 :6;
	u32 Rm :5;
	u32 b21_b28_0xd0 :8;
	u32 S_0x0 :1;
	u32 op_0x0 :1;
	u32 sf :1;
};
struct arm64_adcs{
	u32 Rd :5;
	u32 Rn :5;
	u32 b10_b15_0x0 :6;
	u32 Rm :5;
	u32 b21_b28_0xd0 :8;
	u32 S_0x1 :1;
	u32 op_0x0 :1;
	u32 sf :1;
};
struct arm64_add_imm{
	u32 Rd :5;
	u32 Rn :5;
	u32 imm12 :12;
	u32 shift :2;
	u32 b24_b28_0x11 :5;
	u32 S_0x0 :1;
	u32 op_0x0 :1;
	u32 sf :1;
};	//	Rd=Rn+imm
int arm64_add_imm_encode(struct arm64_add_imm* code, int o0, int i0, int imm, int s, int sf){
	if(imm >> 24)return 0;
	else if(0 == (imm >> 12)){	//hi12 = 0
		code->shift = 0;
		code->imm12 = imm;
	}
	else if(0 == (imm & 0xfff)){	//hi12 > 0, lo12 = 0
		code->shift = 1;
		code->imm12 = imm>>12;
	}
	else{	//hi12 > 0, lo12 > 0
		return 0;
	}

	code->Rd = o0;
	code->Rn = i0;
	code->b24_b28_0x11 = 0x11;
	code->S_0x0 = s;
	code->op_0x0 = 0;
	code->sf = sf;
	return 1;
}
struct arm64_add_regext{
	u32 Rd :5;
	u32 Rn :5;
	u32 imm3 :3;
	u32 option :3;
	u32 Rm :5;
	u32 b21_b28_0x59 :8;
	u32 S_0x0 :1;
	u32 op_0x0 :1;
	u32 sf :1;
};	//	Rd=Rn+Rm(ext)
int arm64_add_regext_encode(struct arm64_add_regext* code, int o0, int i0, int ix, int option, int sh, int s, int sf){
	code->Rd = o0;
	code->Rn = i0;
	code->Rm = ix;
	code->option = option;	//0=uxtb, 1=uxth, 2=uxtw, 3=uxtx, 4=sxtb, 5=sxth, 6=sxtw, 7=sxtx
	code->imm3 = sh;

	code->b21_b28_0x59 = 0x59;
	code->S_0x0 = s;
	code->op_0x0 = 0;
	code->sf = sf;
	return 1;
}
struct arm64_add_regshift{
	u32 Rd :5;
	u32 Rn :5;
	u32 imm6 :6;
	u32 Rm :5;
	u32 b21_0x0 :1;
	u32 shift :2;
	u32 b24_b28_0x0b :5;
	u32 S_0x0 :1;
	u32 op_0x0 :1;
	u32 sf :1;
};	//	Rd=Rn+Rm>>sh
int arm64_add_regshift_encode(struct arm64_add_regshift* code, int o0, int i0, int ix, int sh, int shtype, int s, int sf){
	code->Rd = o0;
	code->Rn = i0;
	code->Rm = ix;
	code->imm6 = sh;
	code->shift = shtype;	//0=lsl, 1=lsr, 2=asr, 3=undef

	code->b21_0x0 = 0;
	code->b24_b28_0x0b = 0xb;
	code->S_0x0 = s;
	code->op_0x0 = 0;
	code->sf = sf;
	return 1;
}
struct arm64_add_scalar{
	u32 Rd :5;
	u32 Rn :5;
	u32 b10_b15_0x21 :6;
	u32 Rm :5;
	u32 b21_0x0 :1;
	u32 size :2;
	u32 b24_b28_0x1e :5;
	u32 U_0x0 :1;
	u32 b30_b31_0x1 :2;
};
struct arm64_add_vector{
	u32 Rd :5;
	u32 Rn :5;
	u32 b10_b15_0x21 :6;
	u32 Rm :5;
	u32 b21_0x0 :1;
	u32 size :2;
	u32 b24_b28_0x1e :5;
	u32 U_0x0 :1;
	u32 Q :1;
	u32 b31_0x0 :1;
};
struct arm64_addg{
	u32 Xd :5;
	u32 Xn :5;
	u32 uimm4 :4;
	u32 b41_b15_0x0 :2;
	u32 uimm6 :6;
	u32 b22_b31_0x246 :10;
};
struct arm64_addp_sclar{
	u32 Rd :5;
	u32 Rn :5;
	u32 b10_b21_0xc6e :12;
	u32 size :2;
	u32 b24_b31_0x5e :8;
};
struct arm64_addp_vector{
	u32 Rd :5;
	u32 Rn :5;
	u32 b10_b15_0x2f :6;
	u32 Rm :5;
	u32 b21_0x1 :1;
	u32 size :2;
	u32 b24_b29_0x0e :6;
	u32 Q :1;
	u32 b31_0x0 :1;
};
struct arm64_adds_imm{
	u32 Rd :5;
	u32 Rn :5;
	u32 imm12 :12;
	u32 shift :2;
	u32 b24_b28_0x11 :5;
	u32 S_0x1 :1;
	u32 op_0x0 :1;
	u32 sf :1;
};
struct arm64_adds_regext{
	u32 Rd :5;
	u32 Rn :5;
	u32 imm3 :3;
	u32 option :3;
	u32 Rm :5;
	u32 b21_b28_0x59 :8;
	u32 S_0x1 :1;
	u32 op_0x0 :1;
	u32 sf :1;
};
struct arm64_adds_regshift{
	u32 Rd :5;
	u32 Rn :5;
	u32 imm6 :6;
	u32 Rm :5;
	u32 b21_0x0 :1;
	u32 size :2;
	u32 b24_b28_0x0b :5;
	u32 S_0x1 :1;
	u32 op_0x0 :1;
	u32 sf :1;
};
struct arm64_addv{
	u32 Rd :5;
	u32 Rn :5;
	u32 b10_b21_0xc6e :12;
	u32 size :2;
	u32 b24_b29_0x0e :6;
	u32 Q_0x0 :1;
	u32 b31_0x0 :1;
};
struct arm64_adr{
	u32 Rd :5;
	u32 immhi :19;
	u32 b24_b28_0x10 :5;
	u32 immlo :2;
	u32 op_0x0 :1;
};
struct arm64_adrp{
	u32 Rd :5;
	u32 immhi :19;
	u32 b24_b28_0x10 :5;
	u32 immlo :2;
	u32 op_0x1 :1;
};
struct arm64_and_imm{
	u32 Rd :5;
	u32 Rn :5;
	u32 imms :6;
	u32 immr :6;
	u32 N :1;
	u32 b23_b28_0x24 :6;
	u32 opc_0x0 :2;
	u32 sf :1;
};
struct arm64_and_regshift{
	u32 Rd :5;
	u32 Rn :5;
	u32 imm6 :6;
	u32 Rm :5;
	u32 N_0x0 :1;
	u32 shift :2;
	u32 b24_b28_0x0a :5;
	u32 opc_0x0 :2;
	u32 sf :1;
};
struct arm64_and_vector{
	u32 Rd :5;
	u32 Rn :5;
	u32 b10_b15_0x07 :6;
	u32 Rm :5;
	u32 b21_0x1 :1;
	u32 size :2;
	u32 b24_b29_0x0e :6;
	u32 Q :1;
	u32 b31_0x0 :1;
};
struct arm64_ands_imm{
	u32 Rd :5;
	u32 Rn :5;
	u32 imms :6;
	u32 immr :6;
	u32 N :1;
	u32 b23_b28_0x24 :6;
	u32 opc_0x3 :2;
	u32 sf :1;
};
struct arm64_ands_regshift{
	u32 Rd :5;
	u32 Rn :5;
	u32 imm6 :6;
	u32 Rm :5;
	u32 N_0x0 :1;
	u32 shift :2;
	u32 b24_b28_0x0a :5;
	u32 opc_0x3 :2;
	u32 sf :1;
};
struct arm64_asr_imm{
	u32 Rd :5;
	u32 Rn :5;
	u32 imms :6;	//[b010,b14]=0x1f, b15=x
	u32 immr :6;
	u32 N :1;
	u32 b23_b28_0x26 :6;
	u32 opc_0x0 :2;
	u32 sf :1;
};
struct arm64_asr_reg{
	u32 Rd :5;
	u32 Rn :5;
	u32 op2 :2;
	u32 b12_b15_0x2 :4;
	u32 Rm :5;
	u32 b21_b30_0x0d6 :10;
	u32 sf :1;
};
struct arm64_asrv{
	u32 Rd :5;
	u32 Rn :5;
	u32 op2 :2;
	u32 b12_b15_0x2 :4;
	u32 Rm :5;
	u32 b21_b30_0x0d6 :10;
	u32 sf :1;
};
//at is sys
//autda,autdza
//autdb,autdzb
//autia,autia1716,autiasp,autiaz,autiza
//autib,autib1716,autibsp,autibz,autizb
//axflag
struct arm64_b{
	u32 imm26 :26;
	u32 b26_b30_0x5 :5;
	u32 op_0x0 :1;
};
struct arm64_b_cond{
	u32 cond :4;
	u32 b4_0x0 :1;
	u32 imm19 :19;
	u32 b24_b31_0x54 :8;
};
//bfc
//bfi
//bfm
//bfxil
//bic
//bics
//bl
struct arm64_b_lr{		//call Rn
	u32 Rm :5;
	u32 Rn :5;
	u32 M :1;
	u32 A :1;
	u32 b12_b20_0x1f0 :9;
	u32 op_0x1 :2;
	u32 b23_0x0 :1;
	u32 Z_0x0 :1;
	u32 b25_b31_0x6b :7;
};
//blraa,blraaz,blrab,blrabz
struct arm64_b_r{		//jmp Rn
	u32 Rm :5;
	u32 Rn :5;
	u32 M :1;
	u32 A :1;
	u32 b12_b20_0x1f0 :9;
	u32 op_0x0 :2;
	u32 b23_0x0 :1;
	u32 Z_0x0 :1;
	u32 b25_b31_0x6b :7;
};
//braa,braaz,brab,brabz
struct arm64_brk{
	u32 b0_b4_0x0 :5;
	u32 imm16 :16;
	u32 b21_b31_0x6a1 :11;
};
struct arm64_bsl{		//bitwise select
	u32 Rd :5;
	u32 Rn :5;
	u32 b10_b15_0x7 :6;
	u32 Rm :5;
	u32 b21_0x1 :1;
	u32 opc2_0x1 :2;
	u32 b24_b29_0x2e :6;
	u32 Q :1;
	u32 b31_0x0 :1;
};
//bti
//cas,casa,casal,casl
//casb,casab,casalb,caslb
//cash,casah,casalh,caslh
//casp,caspa,caspal,caspl
struct arm64_cbnz{
	u32 Rt :5;
	u32 imm19 :19;
	u32 op_0x1 :1;
	u32 b25_b30_0x1a :6;
	u32 sf :1;
};
struct arm64_cbz{
	u32 Rt :5;
	u32 imm19 :19;
	u32 op_0x0 :1;
	u32 b25_b30_0x1a :6;
	u32 sf :1;
};
struct arm64_ccmn_imm{
	u32 nzcv :4;
	u32 b4_0x0 :1;
	u32 Rn :5;
	u32 b10_0x0 :1;
	u32 b11_0x1 :1;
	u32 cond :4;
	u32 imm5 :5;
	u32 b21_b29_0x1d2 :9;
	u32 op_0x0 :1;
	u32 sf :1;
};
struct arm64_ccmn_reg{
	u32 nzcv :4;
	u32 b4_0x0 :1;
	u32 Rn :5;
	u32 b10_0x0 :1;
	u32 b11_0x0 :1;
	u32 cond :4;
	u32 Rm :5;
	u32 b21_b29_0x1d2 :9;
	u32 op_0x0 :1;
	u32 sf :1;
};
struct arm64_ccmp_imm{
	u32 nzcv :4;
	u32 b4_0x0 :1;
	u32 Rn :5;
	u32 b10_0x0 :1;
	u32 b11_0x1 :1;
	u32 cond :4;
	u32 imm5 :5;
	u32 b21_b29_0x1d2 :9;
	u32 op_0x1 :1;
	u32 sf :1;
};
struct arm64_ccmp_reg{
	u32 nzcv :4;
	u32 b4_0x0 :1;
	u32 Rn :5;
	u32 b10_0x0 :1;
	u32 b11_0x0 :1;
	u32 cond :4;
	u32 Rm :5;
	u32 b21_b29_0x1d2 :9;
	u32 op_0x1 :1;
	u32 sf :1;
};
//cfinv
//cfp
//cinc
//cinv
//clrex
//cls
//clz
//cmn_imm is adds_imm
//cmn_regext is adds_regext
//cmn_regsh is adds_regsh
//cmp_imm: subs_imm.Rd=0x1f
//cmp_regext: subs_regext.Rd=0x1f
//cmp_regsh: subs_regsh.Rd=0x1f
//cmpp
//cmtst
//cneg
//cnt
//cpp
//crc32b,crc32h,crc32w,crc32x
//crc32cb,crc32ch,crc32cw,crc32cx
//csdb
//csel
//cset
//csetm
//csinc
//csinv
//csneg
//dc aliasof sys
//dcps1
//dcps2
//dcps3
struct arm64_dmb{
	u32 b0_b4_0x1f :5;
	u32 opc_0x1 :2;
	u32 b7_0x1 :1;
	u32 CRm :4;
	u32 b12_b31_0xd5033 :20;
};
struct arm64_drps{
	u32 b0_b31_0xd6bf03e0;
};
struct arm64_dsb{
	u32 b0_b4_0x1f :5;
	u32 opc_0x0 :2;
	u32 b7_0x1 :1;
	u32 CRm :4;
	u32 b12_b31_0xd5033 :20;
};
struct arm64_dup_element{
	u32 Rd :5;
	u32 Rn :5;
	u32 b10_b15_0x3 :6;
	u32 imm5 :5;
	u32 b21_b31_0x2f0 :11;
};
struct arm64_dup_general{
	u32 Rd :5;
	u32 Rn :5;
	u32 b10_b15_0x1 :6;
	u32 imm5 :5;
	u32 b21_b29_0x70 :9;
	u32 Q :1;
	u32 b31_0x0 :1;
};
//dvp aliasof sys
struct arm64_eon_regshift{
	u32 Rd :5;
	u32 Rn :5;
	u32 imm6 :6;
	u32 Rm :5;
	u32 N_0x1 :1;
	u32 shift :2;
	u32 b24_b28_0xa :5;
	u32 opc_0x2 :2;
	u32 sf :1;
};
struct arm64_eor3{
	u32 Rd :5;
	u32 Rn :5;
	u32 Ra :5;
	u32 b15_0x0 :1;
	u32 Rm :5;
	u32 b21_b21_0x670 :11;
};
struct arm64_eor_vector{
	u32 Rd :5;
	u32 Rn :5;
	u32 b10_b15_0x7 :6;
	u32 Rm :5;
	u32 b21_0x1 :1;
	u32 opc2_0x0 :2;
	u32 b24_b29_0x2e :6;
	u32 Q :1;
	u32 b31_0x0 :1;
};
struct arm64_eor_imm{
	u32 Rd :5;
	u32 Rn :5;
	u32 imms :6;
	u32 immr :6;
	u32 N :1;
	u32 b23_b28_0x24 :6;
	u32 opc_0x2 :2;
	u32 sf :1;
};
struct arm64_eor_regshift{
	u32 Rd :5;
	u32 Rn :5;
	u32 imm6 :6;
	u32 Rm :5;
	u32 N_0x0 :1;
	u32 shift :2;
	u32 b24_b28_0xa :5;
	u32 opc_0x2 :2;
	u32 sf :1;
};
struct arm64_eret{
	u32 op4_0x0 :5;
	u32 Rn_0x1f :5;
	u32 M_0x0 :1;
	u32 A_0x0 :1;
	u32 b12_b31_0xd69f0 :20;
};
//eretaa,eretab
struct arm64_esb{
	u32 b0_b4_0x1f :5;
	u32 op2_0x0 :3;
	u32 CRm_0x2 :4;
	u32 b12_b31_0xd5032 :20;
};
//ext
//extr
//fabd
//fabs
//facge
//facgt
//fadd
//faddp
//fcadd
//fccmp
//fccmpe
//fcmeq
//fcmge
//fcmgt
//fcmla
//fcmle
//fcmlt
//fcmp
//fcmpe
//fcsel
//fcvt
//fcvtas
//fcvtau
//fcvtl,fcvtl2
//fcvtms
//fcvtmu
//fcvtn,fcvtn2
//fcvtns
//fcvtnu
//fcvtps
//fcvtpu
//fcvtxn,fcvtxn2
//fcvtzs
//fcvtzu
//fdiv
//fjcvtzs
//fmadd
//fmax
//fmaxnm
//fmaxnmp
//fmaxnmv
//fmaxp
//fmaxv
//fmin
//fminnm
//fminnmp
//fminnmv
//fminp
//fminv
//fmla
//fmlal,fmlal2
//fmls
//fmlsl,fmlsl2
//fmov
//fmsub
//fmul
//fmulx
//fneg
//fnmadd
//fnmsub
//fnmul
//frecpe
//frecps
//frecpx
//frint32x
//frint32z
//frint64x
//frint64z
//frinta
//frinti
//frintm
//frintn
//frintp
//frintx
//frintz
//frsqrte
//frsqrts
//fsqrt
//fsub
//gmi
//hint
struct arm64_hlt{
	u32 b0_b4_0x0 :5;
	u32 imm16 :16;
	u32 b21_b31_0x6a2 :11;
};
struct arm64_hvc{
	u32 b0_b4_0x2 :5;
	u32 imm16 :16;
	u32 b21_b31_0x6a2 :11;
};
//ic aliasof sys
//ins_element aliasof mov_element
//ins_general aliasof mov_general
//irg
struct arm64_isb{
	u32 b0_b4_0x1f :5;
	u32 opc_0x2 :2;
	u32 CRm :4;
	u32 b12_b31_0xd5033 :20;
};
//ld1
//ld1r
//ld2
//ld2r
//ld3
//ld3r
//ld4
//ld4r
//ldadd,ldadda,ldaddal,ldaddl
//ldaddb,ldaddab,ldaddalb,ldaddlb
//ldaddh,ldaddah,ldaddalh,ldaddlh
//ldapr
//ldaprb
//ldaprh
//ldapur
//ldapurb
//ldapurh
//ldapursb
//ldapursh
//ldapursw
//ldar
//ldarb
//ldarh
//ldaxp
//ldaxr
//ldaxrb
//ldaxrh
//ldclr,ldclra,ldclral,ldclrl
//ldclrb,ldclrab,ldclralb,ldclrlb
//ldclrh,ldclrah,ldclralh,ldclrlh
//ldeor,ldeora,ldeoral,ldeorl
//ldeorb,ldeorab,ldeoralb,ldeorlb
//ldeorh,ldeorah,ldeoralh,ldeorlh
//ldg
//ldgv
//ldlar
//ldlarb
//ldlarh
//ldnp
//ldnp
//ldp
//ldpsw
struct arm64_ldr_imm_fp{
	u32 Rt :5;
	u32 Rn :5;
	u32 b10_b11_0x1 :2;
	u32 imm9 :9;
	u32 b21_0x0 :1;
	u32 opc_0xx1 :2;
	u32 b24_b29_0x3c :6;
	u32 size :2;
};
struct arm64_ldr_imm{
	u32 Rt :5;
	u32 Rn :5;
	u32 b10_b11_0x1 :2;
	u32 imm9 :9;
	u32 b21_0x0 :1;
	u32 opc_0x01 :2;
	u32 b24_b29_0x38 :6;
	u32 size_0x1x :2;
};
struct arm64_ldr_rel_fp{
	u32 Rt :5;
	u32 imm19 :19;
	u32 b24_b29_0x18 :6;
	u32 opc_0x0x :2;
};
struct arm64_ldr_rel{
	u32 Rt :5;
	u32 imm19 :19;
	u32 b24_b29_0x18 :6;
	u32 opc_0x0x :2;
};
struct arm64_ldr_reg_fp{
	u32 Rt :5;
	u32 Rn :5;
	u32 b10_b11_0x2 :2;
	u32 S :1;
	u32 option :3;
	u32 Rm :5;
	u32 b21_0x1 :1;
	u32 opc_0xx1 :2;
	u32 b24_b29_0x3c :6;
	u32 size :2;
};
struct arm64_ldr_reg{
	u32 Rt :5;
	u32 Rn :5;
	u32 b10_b11_0x2 :2;
	u32 S :1;
	u32 option :3;
	u32 Rm :5;
	u32 b21_0x1 :1;
	u32 opc_0xx1 :2;
	u32 b24_b29_0x38 :6;
	u32 size_0x1x :2;
};
//ldraa,ldrab
//ldrb_imm
//ldrb_reg
//ldrh_imm
//ldrh_reg
//ldrsb_imm
//ldrsb_reg
//ldrsh_imm
//ldrsh_reg
//ldrsw_imm
//ldrsw_rel
//ldrsw_reg
//ldset,ldseta,ldsetal,ldsetl
//ldsetb,ldsetab,ldsetalb,ldsetlb
//ldseth,ldsetah,ldsetalh,ldsetlh
//ldsmax,ldsmaxa,ldsmaxal,ldsmaxl
//ldsmaxb,ldsmaxab,ldsmaxalb,ldsmaxlb
//ldsmaxh,ldsmaxah,ldsmaxalh,ldsmaxlh
//ldsmin,ldsmina,ldsminal,ldsminl
//ldsminb,ldsminab,ldsminalb,ldsminlb
//ldsminh,ldsminah,ldsminalh,ldsminlh
//ldtr
//ldtrb
//ldtrh
//ldtrsb
//ldtrsh
//ldtrsw
//ldumax,ldumaxa,ldumaxal,ldumaxl
//ldumaxb,ldumaxab,ldumaxalb,ldumaxlh
//ldumaxh,ldumaxah,ldumaxalh,ldumaxlh
//ldumin,ldumina,lduminal,lduminl
//lduminb,lduminab,lduminalb,lduminlb
//lduminh,lduminah,lduminalh,lduminlh
//ldur_fp
//ldur
//ldurb
//ldursh
//ldursw
//ldxr
//ldxrb
struct arm64_lsl_reg{
	u32 Rd :5;
	u32 Rn :5;
	u32 op2_0x0 :2;
	u32 b12_b15_0x2 :4;
	u32 Rm :5;
	u32 b21_b30_0xd6 :10;
	u32 sf :1;
};
struct arm64_lsl_imm{
	u32 Rd :5;
	u32 Rn :5;
	u32 imms :6;
	u32 immr :6;
	u32 N :1;
	u32 b23_b28_0x26 :6;
	u32 opc_0x2 :2;
	u32 sf :1;
};
struct arm64_lsv{
	u32 Rd :5;
	u32 Rn :5;
	u32 op2_0x0 :2;
	u32 b12_b15_0x2 :4;
	u32 Rm :5;
	u32 b21_b30_0xd6 :10;
	u32 sf :1;
};
struct arm64_lsr_reg{
	u32 Rd :5;
	u32 Rn :5;
	u32 op2_0x1 :2;
	u32 b12_b15_0x2 :4;
	u32 Rm :5;
	u32 b21_b30_0xd6 :10;
	u32 sf :1;
};
struct arm64_lsr_imm{
	u32 Rd :5;
	u32 Rn :5;
	u32 imms :6;
	u32 immr :6;
	u32 N :1;
	u32 b23_b28_0x26 :6;
	u32 opc_0x2 :2;
	u32 sf :1;
};
struct arm64_lsrv{
	u32 Rd :5;
	u32 Rn :5;
	u32 op2_0x1 :2;
	u32 b12_b15_0x2 :4;
	u32 Rm :5;
	u32 b21_b30_0xd6 :10;
	u32 sf :1;
};
//madd
//mla_element
//mla_vector
//mls_element
//mls_vector
//mneg
//mov_sp aliasof add_imm
//mov_scalar
//mov_element
//mov_general
//mov_invimm
//mov_imm
//mov_vector
//mov_bitmaskimm
//mov_reg
//mov_general
//movi
//movk
//movn
//movz
//mrs
//msr_imm
//msr_reg
//msub
//mul_element
//mul_vector
struct arm64_mul{
	u32 Rd :5;
	u32 Rn :5;
	u32 Ra_1f :5;
	u32 o0 :1;
	u32 Rm :5;
	u32 b21_b30_0xd8 :10;
	u32 sf :1;
};
//mvn
//mvni
//neg_vector
//neg_regshift
//negs
//ngc
//ngcs
struct arm64_nop{
	u32 b0_b4_0x1f :5;
	u32 op2_0x0 :3;
	u32 CRm :4;
	u32 b12_b31_0xd5032 :20;
};
//not_vector
//orn_vector
//orn_regshift
//orr_vecimm
//orr_vecreg
//orr_imm
//orr_regshift
//pacda,pacdza
//pacdb,pacdzb
//pacga
//pacia,pacia1716,paciasp,paciaz,paciza
//pacib,pacib1716,pacibsp,pacibz,pacizb
//pmul
//pmull,pmull2
//prfm_imm
//prfm_literal
//prfm_reg
//prfm_unscaledoffset
//psb_csync
//pssbb
//raddhn,raddhn2
//rax1
//rbit_vector
//rbit
struct arm64_ret{
	u32 Rm_0x0 :5;
	u32 Rn :5;
	u32 M_0x0 :1;
	u32 A_0x0 :1;
	u32 b12_b15_0x0 :4;
	u32 b16_b20_0x1f :5;
	u32 op_0x2 :2;
	u32 b23_0x0 :1;
	u32 Z_0x0 :1;
	u32 b25_b31_0x6b :7;
};
//regaa,regab
//rev
//rev16_vector
//rev16
//rev32_vector
//rev32
//rev64
//rmif
struct arm64_ror_imm{
	u32 Rd :5;
	u32 Rn :5;
	u32 imms :6;
	u32 Rm :5;
	u32 b21_0x0 :1;
	u32 N :1;
	u32 b12_0x30_0x27 :8;
	u32 sf :1;
};
struct arm64_ror_reg{
	u32 Rd :5;
	u32 Rn :5;
	u32 op2_0x3 :2;
	u32 b12_b15_0x2 :4;
	u32 Rm :5;
	u32 b21_b30_0xd6 :10;
	u32 sf :1;
};
struct arm64_rorv{
	u32 Rd :5;
	u32 Rn :5;
	u32 op2_0x3 :2;
	u32 b12_b15_0x3 :4;
	u32 Rm :5;
	u32 b21_b30_0xd6 :10;
	u32 sf :1;
};
//rshrn,rshrn2
//rsubhn,rsubhn2
//saba
//sabal,sabal2
//sabd
//sabdl,sabdl2
//sadalp
//saddl,saddl2
//saddlp
//saddlv
//saddw,saddw2
//sb
//sbc
//sbcs
//sbfiz
//sbfm
//sbfx
//scvtf_vec_fp
//scvtf_vec_int
//scvtf_scalar_fp
//scvtf_scalar_int
//sdiv
//sdot_element
//sdot_vec
//setf8,setf16
//sev
//sevl
//sha1c
//sha1h
//sha1m
//sha1p
//sha1su0
//sha1su1
//sha256h2
//sha256h
//sha256su0
//sha256su1
//sha512h2
//sha512h
//sha512su0
//sha512su1
//shadd
struct arm64_shl_scalar{
	u32 Rd :5;
	u32 Rn :5;
	u32 b10_b15_0x15 :6;
	u32 immb :3;
	u32 immh :4;
	u32 b23_b31_0xbe :9;
};
struct arm64_shl_vector{
	u32 Rd :5;
	u32 Rn :5;
	u32 b10_b15_0x15 :6;
	u32 immb :3;
	u32 immh :4;
	u32 b23_b29_0x1e :7;
	u32 Q :1;
	u32 b31_0x0 :1;
};
//shll,shll2
//shrn,shrn2
//shsub
//sli
//sm3partw1
//sm3partw2
//sm3ss1
//sm3tt1a
//sm3tt1b
//sm3tt2a
//sm3tt2b
//sm4e
//sm4ekey
//smaddl
//smax
//smaxp
//smaxv
//smc
//smin
//sminp
//sminv
//smlal,smlal2
//smlsl,smlsl2
//smnegl
//smov
//smsubl
//smulh
//smull,smull2
//smull
//sqabs
//sqadd
//sqdmlal,sqdmlal2
//sqdmlsl,sqdmlsl2
//sqdmulh
//sqdmull,sqdmull2
//sqneg
//sqrdmlah
//sqrdmlsh
//sqrdmulh
//sqrshl
//sqrshrn,sqrshrn2
//sqrshrun,sqrshrun2
//sqshl_imm
//sqshlu
//sqshrn,sqshrn2
//sqshrun,sqshrun2
//sqsub
//sqxtn,sqxtn2
//sqxtun,sqxtun2
//srhadd
//sri
//srshl
//srshr
//srsra
//ssbb
//sshl
//sshll,sshll2
//sshr
//ssra
//ssubl,ssubl2
//ssubw,ssubw2
//st1
//st2
//st2g
//st3
//st4
//stadd,staddl
//staddb,staddlb
//staddh,staddlh
//stclr,stclrl
//stclrb,stclrlb
//stclrh,stclrlh
//steor,steorl
//steorb,steorlb
//steorh,steorlh
//stg
//stgp
//stgv
//stllr
//stllrb
//stllrh
//stlr
//stlrb
//stlrh
//stlur
//stlurb
//stlurh
//stlxp
//stlxr
//stlxrb
//stlxrh
//stnp
//stnp
//stp_fp
//stp
//str_fp
//str_imm
//str_regfp
//str_reg
//strb_imm
//strb_reg
//strh_imm
//strh_reg
//stset,stsetl
//stsetb,stsetlb
//stseth,stsetlh
//stsmax,stsmaxl
//stsmaxb,stsmaxlb
//stsmin,stsminl
//stsminb,stsminlb
//stsminh,stsminlh
//sttr
//sttrb
//sttrh
//stumax,stumaxl
//stumaxb,stumaxlb
//stumaxh,stumaxlh
//stumin,stuminl
//stuminb,stuminlh
//stuminh,stuminlh
//stur_fp
//stur
//sturb
//sturh
//stxp
//stxr
//stxrb
//stxrh
//stz2g
//stzg
struct arm64_sub_imm{
	u32 Rd :5;
	u32 Rn :5;
	u32 imm12 :12;
	u32 shift :2;
	u32 b24_b28_0x11 :5;
	u32 S_0x0 :1;
	u32 op_0x1 :1;
	u32 sf :1;
};	//	Rd=Rn-imm
int arm64_sub_imm_encode(struct arm64_sub_imm* code, int o0, int i0, int imm, int s, int sf){
	if(imm >> 24)return 0;
	else if(0 == (imm >> 12)){	//hi12 = 0
		code->shift = 0;
		code->imm12 = imm;
	}
	else if(0 == (imm & 0xfff)){	//hi12 > 0, lo12 = 0
		code->shift = 1;
		code->imm12 = imm>>12;
	}
	else{	//hi12 > 0, lo12 > 0
		return 0;
	}

	code->Rd = o0;
	code->Rn = i0;
	code->b24_b28_0x11 = 0x11;
	code->S_0x0 = s;
	code->op_0x1 = 1;
	code->sf = sf;
	return 1;
}
struct arm64_sub_regext{
	u32 Rd :5;
	u32 Rn :5;
	u32 imm3 :3;
	u32 option :3;
	u32 Rm :5;
	u32 b21_b28_0x59 :8;
	u32 S_0x0 :1;
	u32 op_0x1 :1;
	u32 sf :1;
};	//	Rd=Rn-Rm(ext)
int arm64_sub_regext_encode(struct arm64_sub_regext* code, int o0, int i0, int ix, int option, int sh, int s, int sf){
	code->Rd = o0;
	code->Rn = i0;
	code->Rm = ix;
	code->option = option;	//0=uxtb, 1=uxth, 2=uxtw, 3=uxtx, 4=sxtb, 5=sxth, 6=sxtw, 7=sxtx
	code->imm3 = sh;

	code->b21_b28_0x59 = 0x59;
	code->S_0x0 = s;
	code->op_0x1 = 1;
	code->sf = sf;
	return 1;
}
struct arm64_sub_regshift{
	u32 Rd :5;
	u32 Rn :5;
	u32 imm6 :6;
	u32 Rm :5;
	u32 b21_0x0 :1;
	u32 shift :2;
	u32 b24_b28_0x0b :5;
	u32 S_0x0 :1;
	u32 op_0x1 :1;
	u32 sf :1;
};	//	Rd=Rn-Rm>>sh
int arm64_sub_regshift_encode(struct arm64_sub_regshift* code, int o0, int i0, int ix, int sh, int shtype, int s, int sf){
	code->Rd = o0;
	code->Rn = i0;
	code->Rm = ix;
	code->imm6 = sh;
	code->shift = shtype;	//0=lsl, 1=lsr, 2=asr, 3=undef

	code->b21_0x0 = 0;
	code->b24_b28_0x0b = 0xb;
	code->S_0x0 = s;
	code->op_0x1 = 1;
	code->sf = sf;
	return 1;
}
struct arm64_sub_scalar{
	u32 Rd :5;
	u32 Rn :5;
	u32 b10_b15_0x21 :6;
	u32 Rm :5;
	u32 b21_0x1 :1;
	u32 size :2;
	u32 b24_b28_0x1e :5;
	u32 U_0x1 :1;
	u32 b30_b31_0x1 :2;
};
struct arm64_sub_vector{
	u32 Rd :5;
	u32 Rn :5;
	u32 b10_b15_0x21 :6;
	u32 Rm :5;
	u32 b21_0x1 :1;
	u32 size :2;
	u32 b24_b28_0x0e :5;
	u32 U_0x1 :1;
	u32 Q :1;
	u32 b31_0x0 :1;
};
//subg
//subhn,subhn2
//subp
//subps
//subs_imm: sub_imm + Sflag
//subs_regext: sub_regext + Sflag
//subs_regshift: sub_regshift + Sflag
//suqadd
struct arm64_svc{
	u32 b0_b4_0x1 :5;
	u32 imm16 :16;
	u32 b21_b31_0x6a0 :11;
};
//swp,swpa,swpal,swpl
//swpb,swpab,swpalb,swplb
//swph,swpah,swpalh,swplh
//sxtb
//sxth
//sxtl,sxtl2
//sxtw
//sys
//sysl
//




/*
0 EQ	Z=1	equal
1 NE	Z=0	nonequal
2 CS=HS	C=1	carry set
3 CC=LO C=0	carry clear
4 MI	N=1	minus
5 PL	N=0	positive or zero
6 VS	V=1	overflow set
7 VC	V=0	overflow clear
8 HI	C=1,Z=0	unsigned higher
9 LS	C=0,Z=1	unsigned lower or same
a GE	N=V	signed greater or equal
b LT	N!V	signed less
c GT	Z=0,N=V	signed greater
d LE	Z=1,N!V	signed less or equal
e AL always ?
f NV never ?
*/
static char* cond = "eq\0ne\0hs\0lo\0mi\0pl\0vs\0vc\0hi\0ls\0ge\0lt\0gt\0le\0al\0nv\0";


//v=0: return 0b0001
//v=1: return 0b0011
//v=2: retrun 0b0111
//v=3: return 0b1111
u32 table32(int v)
{
	int j;
	u32 tmp = 0;
	for(j=0;j<v+1;j++)tmp |= ((u32)1<<j);
	return tmp;
}
u32 rotateleft32(u32 in, int c)
{
	u32 tmp = in>>(32-c);
	return (in<<c) | tmp;
}
u64 table64(int v)
{
	int j;
	u64 tmp = 0;
	for(j=0;j<v+1;j++)tmp |= ((u64)1<<j);
	return tmp;
}
u64 rotateleft64(u64 in, int c)
{
	u64 tmp = in>>(64-c);
	return (in<<c) | tmp;
}




void* disasm_arm64_str(u32 code)
{
	switch(code&0xffdfffe0){
	case 0xd5100040:return "osdtrrx_el1";
	case 0xd5100080:return "dbgbvr0_el1";
	case 0xd51000a0:return "dbgbcr0_el1";
	case 0xd51000c0:return "dbgwvr0_el1";
	case 0xd51000e0:return "dbgwcr0_el1";
	case 0xd5100180:return "dbgbvr1_el1";
	case 0xd51001a0:return "dbgbcr1_el1";
	case 0xd51001c0:return "dbgwvr1_el1";
	case 0xd51001e0:return "dbgwcr1_el1";
	case 0xd5100200:return "mdccint_el1";
	case 0xd5100240:return "mdscr_el1";
	case 0xd5100280:return "dbgbvr2_el1";
	case 0xd51002a0:return "dbgbcr2_el1";
	case 0xd51002c0:return "dbgwvr2_el1";
	case 0xd51002e0:return "dbgwcr2_el1";
	case 0xd5100340:return "osdtrtx_el1";
	case 0xd5100380:return "dbgbvr3_el1";
	case 0xd51003a0:return "dbgbcr3_el1";
	case 0xd51003c0:return "dbgwvr3_el1";
	case 0xd51003e0:return "dbgwcr3_el1";
	case 0xd5100480:return "dbgbvr4_el1";
	case 0xd51004a0:return "dbgbcr4_el1";
	case 0xd51004c0:return "dbgwvr4_el1";
	case 0xd51004e0:return "dbgwcr4_el1";
	case 0xd5100580:return "dbgbvr5_el1";
	case 0xd51005a0:return "dbgbcr5_el1";
	case 0xd51005c0:return "dbgwvr5_el1";
	case 0xd51005e0:return "dbgwcr5_el1";
	case 0xd5100640:return "oseccr_el1";
	case 0xd5100680:return "dbgbvr6_el1";
	case 0xd51006a0:return "dbgbcr6_el1";
	case 0xd51006c0:return "dbgwvr6_el1";
	case 0xd51006e0:return "dbgwcr6_el1";
	case 0xd5100780:return "dbgbvr7_el1";
	case 0xd51007a0:return "dbgbcr7_el1";
	case 0xd51007c0:return "dbgwvr7_el1";
	case 0xd51007e0:return "dbgwcr7_el1";
	case 0xd5100880:return "dbgbvr8_el1";
	case 0xd51008a0:return "dbgbcr8_el1";
	case 0xd51008c0:return "dbgwvr8_el1";
	case 0xd51008e0:return "dbgwcr8_el1";
	case 0xd5100980:return "dbgbvr9_el1";
	case 0xd51009a0:return "dbgbcr9_el1";
	case 0xd51009c0:return "dbgwvr9_el1";
	case 0xd51009e0:return "dbgwcr9_el1";
	case 0xd5100a80:return "dbgbvr10_el1";
	case 0xd5100aa0:return "dbgbcr10_el1";
	case 0xd5100ac0:return "dbgwvr10_el1";
	case 0xd5100ae0:return "dbgwcr10_el1";
	case 0xd5100b80:return "dbgbvr11_el1";
	case 0xd5100ba0:return "dbgbcr11_el1";
	case 0xd5100bc0:return "dbgwvr11_el1";
	case 0xd5100be0:return "dbgwcr11_el1";
	case 0xd5100c80:return "dbgbvr12_el1";
	case 0xd5100ca0:return "dbgbcr12_el1";
	case 0xd5100cc0:return "dbgwvr12_el1";
	case 0xd5100ce0:return "dbgwcr12_el1";
	case 0xd5100d80:return "dbgbvr13_el1";
	case 0xd5100da0:return "dbgbcr13_el1";
	case 0xd5100dc0:return "dbgwvr13_el1";
	case 0xd5100de0:return "dbgwcr13_el1";
	case 0xd5100e80:return "dbgbvr14_el1";
	case 0xd5100ea0:return "dbgbcr14_el1";
	case 0xd5100ec0:return "dbgwvr14_el1";
	case 0xd5100ee0:return "dbgwcr14_el1";
	case 0xd5100f80:return "dbgbvr15_el1";
	case 0xd5100fa0:return "dbgbcr15_el1";
	case 0xd5100fc0:return "dbgwvr15_el1";
	case 0xd5100fe0:return "dbgwcr15_el1";
	case 0xd5101000:return "mdrar_el1";
	case 0xd5101080:return "oslar_el1";
	case 0xd5101180:return "oslsr_el1";
	case 0xd5101380:return "osdlr_el1";
	case 0xd5101480:return "dbgprcr_el1";
	case 0xd51078c0:return "dbgclaimset_el1";
	case 0xd51079c0:return "dbgclaimclr_el1";
	case 0xd5107ec0:return "dbgauthstatus_el1";
	case 0xd5120000:return "teecr32_el1";
	case 0xd5121000:return "teehbr32_el1";
	case 0xd5130100:return "mdccsr_el0";
	case 0xd5130400:return "dbgdtr_el0";
	case 0xd5130500:return "dbgdtrrx_el0";
	case 0xd5140700:return "dbgvcr32_el2";
	case 0xd5180000:return "midr_el1";
	case 0xd51800a0:return "mpidr_el1";
	case 0xd51800c0:return "revidr_el1";
	case 0xd51800e0:return "zidr_el1";
	case 0xd5180100:return "id_pfr0_el1";
	case 0xd5180120:return "id_pfr1_el1";
	case 0xd5180140:return "id_dfr0_el1";
	case 0xd5180160:return "id_afr0_el1";
	case 0xd5180180:return "id_mmfr0_el1";
	case 0xd51801a0:return "id_mmfr1_el1";
	case 0xd51801c0:return "id_mmfr2_el1";
	case 0xd51801e0:return "id_mmfr3_el1";
	case 0xd5180200:return "id_isar0_el1";
	case 0xd5180220:return "id_isar1_el1";
	case 0xd5180240:return "id_isar2_el1";
	case 0xd5180260:return "id_isar3_el1";
	case 0xd5180280:return "id_isar4_el1";
	case 0xd51802a0:return "id_isar5_el1";
	case 0xd51802c0:return "id_mmfr4_el1";
	case 0xd5180300:return "mvfr0_el1";
	case 0xd5180320:return "mvfr1_el1";
	case 0xd5180340:return "mvfr2_el1";
	case 0xd5180400:return "id_aa64pfr0_el1";
	case 0xd5180420:return "id_aa64pfr1_el1";
	case 0xd5180480:return "id_aa64zfr0_el1";
	case 0xd5180500:return "id_aa64dfr0_el1";
	case 0xd5180520:return "id_aa64dfr1_el1";
	case 0xd5180580:return "id_aa64afr0_el1";
	case 0xd51805a0:return "id_aa64afr1_el1";
	case 0xd5180600:return "id_aa64isar0_el1";
	case 0xd5180620:return "id_aa64isar1_el1";
	case 0xd5180700:return "id_aa64mmfr0_el1";
	case 0xd5180720:return "id_aa64mmfr1_el1";
	case 0xd5180740:return "id_aa64mmfr2_el1";
	case 0xd5181000:return "sctlr_el1";
	case 0xd5181020:return "actlr_el1";
	case 0xd5181040:return "cpacr_el1";
	case 0xd5181200:return "zcr_el1";
	case 0xd5182000:return "ttbr0_el1";
	case 0xd5182020:return "ttbr1_el1";
	case 0xd5182040:return "tcr_el1";
	case 0xd5182100:return "apiakeylo_el1";
	case 0xd5182120:return "apiakeyhi_el1";
	case 0xd5182140:return "apibkeylo_el1";
	case 0xd5182160:return "apibkeyhi_el1";
	case 0xd5182200:return "apdakeylo_el1";
	case 0xd5182220:return "apdakeyhi_el1";
	case 0xd5182240:return "apdbkeylo_el1";
	case 0xd5182260:return "apdbkeyhi_el1";
	case 0xd5182300:return "apgakeylo_el1";
	case 0xd5182320:return "apgakeyhi_el1";
	case 0xd5184000:return "spsr_el1";
	case 0xd5184020:return "elr_el1";
	case 0xd5184100:return "sp_el0";
	case 0xd5184200:return "spsel";
	case 0xd5184240:return "currentel";
	case 0xd5184260:return "pan";
	case 0xd5184280:return "uao";
	case 0xd5185100:return "afsr0_el1";
	case 0xd5185120:return "afsr1_el1";
	case 0xd5185200:return "esr_el1";
	case 0xd5185300:return "erridr_el1";
	case 0xd5185320:return "errselr_el1";
	case 0xd5185400:return "erxfr_el1";
	case 0xd5185420:return "erxctlr_el1";
	case 0xd5185440:return "erxstatus_el1";
	case 0xd5185460:return "erxaddr_el1";
	case 0xd5185500:return "erxmisc0_el1";
	case 0xd5185520:return "erxmisc1_el1";
	case 0xd5186000:return "far_el1";
	case 0xd5187400:return "par_el1";
	case 0xd5189900:return "pmscr_el1";
	case 0xd5189940:return "pmsicr_el1";
	case 0xd5189960:return "pmsirr_el1";
	case 0xd5189980:return "pmsfcr_el1";
	case 0xd51899a0:return "pmsevfr_el1";
	case 0xd51899c0:return "pmslatfr_el1";
	case 0xd51899e0:return "pmsidr_el1";
	case 0xd5189a00:return "pmblimitr_el1";
	case 0xd5189a20:return "pmbptr_el1";
	case 0xd5189a60:return "pmbsr_el1";
	case 0xd5189ae0:return "pmbidr_el1";
	case 0xd5189e20:return "pmintenset_el1";
	case 0xd5189e40:return "pmintenclr_el1";
	case 0xd518a200:return "mair_el1";
	case 0xd518a300:return "amair_el1";
	case 0xd518c000:return "vbar_el1";
	case 0xd518c020:return "rvbar_el1";
	case 0xd518c040:return "rmr_el1";
	case 0xd518c100:return "isr_el1";
	case 0xd518c120:return "disr_el1";
	case 0xd518d020:return "contextidr_el1";
	case 0xd518d080:return "tpidr_el1";
	case 0xd518e100:return "cntkctl_el1";
	case 0xd5190000:return "ccsidr_el1";
	case 0xd5190020:return "clidr_el1";
	case 0xd51900e0:return "aidr_el1";
	case 0xd51a0000:return "csselr_el1";
	case 0xd51b0020:return "ctr_el0";
	case 0xd51b00e0:return "dczid_el0";
	case 0xd51b4200:return "nzcv";
	case 0xd51b4220:return "daif";
	case 0xd51b42a0:return "dit";
	case 0xd51b4400:return "fpcr";
	case 0xd51b4420:return "fpsr";
	case 0xd51b4500:return "dspsr_el0";
	case 0xd51b4520:return "dlr_el0";
	case 0xd51b9c00:return "pmcr_el0";
	case 0xd51b9c20:return "pmcntenset_el0";
	case 0xd51b9c40:return "pmcntenclr_el0";
	case 0xd51b9c60:return "pmovsclr_el0";
	case 0xd51b9c80:return "pmswinc_el0";
	case 0xd51b9ca0:return "pmselr_el0";
	case 0xd51b9cc0:return "pmceid0_el0";
	case 0xd51b9ce0:return "pmceid1_el0";
	case 0xd51b9d00:return "pmccntr_el0";
	case 0xd51b9d20:return "pmxevtyper_el0";
	case 0xd51b9d40:return "pmxevcntr_el0";
	case 0xd51b9e00:return "pmuserenr_el0";
	case 0xd51b9e60:return "pmovsset_el0";
	case 0xd51bd040:return "tpidr_el0";
	case 0xd51bd060:return "tpidrro_el0";
	case 0xd51be000:return "cntfrq_el0";
	case 0xd51be020:return "cntpct_el0";
	case 0xd51be040:return "cntvct_el0";
	case 0xd51be200:return "cntp_tval_el0";
	case 0xd51be220:return "cntp_ctl_el0";
	case 0xd51be240:return "cntp_cval_el0";
	case 0xd51be300:return "cntv_tval_el0";
	case 0xd51be320:return "cntv_ctl_el0";
	case 0xd51be340:return "cntv_cval_el0";
	case 0xd51be800:return "pmevcntr0_el0";
	case 0xd51be820:return "pmevcntr1_el0";
	case 0xd51be840:return "pmevcntr2_el0";
	case 0xd51be860:return "pmevcntr3_el0";
	case 0xd51be880:return "pmevcntr4_el0";
	case 0xd51be8a0:return "pmevcntr5_el0";
	case 0xd51be8c0:return "pmevcntr6_el0";
	case 0xd51be8e0:return "pmevcntr7_el0";
	case 0xd51be900:return "pmevcntr8_el0";
	case 0xd51be920:return "pmevcntr9_el0";
	case 0xd51be940:return "pmevcntr10_el0";
	case 0xd51be960:return "pmevcntr11_el0";
	case 0xd51be980:return "pmevcntr12_el0";
	case 0xd51be9a0:return "pmevcntr13_el0";
	case 0xd51be9c0:return "pmevcntr14_el0";
	case 0xd51be9e0:return "pmevcntr15_el0";
	case 0xd51bea00:return "pmevcntr16_el0";
	case 0xd51bea20:return "pmevcntr17_el0";
	case 0xd51bea40:return "pmevcntr18_el0";
	case 0xd51bea60:return "pmevcntr19_el0";
	case 0xd51bea80:return "pmevcntr20_el0";
	case 0xd51beaa0:return "pmevcntr21_el0";
	case 0xd51beac0:return "pmevcntr22_el0";
	case 0xd51beae0:return "pmevcntr23_el0";
	case 0xd51beb00:return "pmevcntr24_el0";
	case 0xd51beb20:return "pmevcntr25_el0";
	case 0xd51beb40:return "pmevcntr26_el0";
	case 0xd51beb60:return "pmevcntr27_el0";
	case 0xd51beb80:return "pmevcntr28_el0";
	case 0xd51beba0:return "pmevcntr29_el0";
	case 0xd51bebc0:return "pmevcntr30_el0";
	case 0xd51bec00:return "pmevtyper0_el0";
	case 0xd51bec20:return "pmevtyper1_el0";
	case 0xd51bec40:return "pmevtyper2_el0";
	case 0xd51bec60:return "pmevtyper3_el0";
	case 0xd51bec80:return "pmevtyper4_el0";
	case 0xd51beca0:return "pmevtyper5_el0";
	case 0xd51becc0:return "pmevtyper6_el0";
	case 0xd51bece0:return "pmevtyper7_el0";
	case 0xd51bed00:return "pmevtyper8_el0";
	case 0xd51bed20:return "pmevtyper9_el0";
	case 0xd51bed40:return "pmevtyper10_el0";
	case 0xd51bed60:return "pmevtyper11_el0";
	case 0xd51bed80:return "pmevtyper12_el0";
	case 0xd51beda0:return "pmevtyper13_el0";
	case 0xd51bedc0:return "pmevtyper14_el0";
	case 0xd51bede0:return "pmevtyper15_el0";
	case 0xd51bee00:return "pmevtyper16_el0";
	case 0xd51bee20:return "pmevtyper17_el0";
	case 0xd51bee40:return "pmevtyper18_el0";
	case 0xd51bee60:return "pmevtyper19_el0";
	case 0xd51bee80:return "pmevtyper20_el0";
	case 0xd51beea0:return "pmevtyper21_el0";
	case 0xd51beec0:return "pmevtyper22_el0";
	case 0xd51beee0:return "pmevtyper23_el0";
	case 0xd51bef00:return "pmevtyper24_el0";
	case 0xd51bef20:return "pmevtyper25_el0";
	case 0xd51bef40:return "pmevtyper26_el0";
	case 0xd51bef60:return "pmevtyper27_el0";
	case 0xd51bef80:return "pmevtyper28_el0";
	case 0xd51befa0:return "pmevtyper29_el0";
	case 0xd51befc0:return "pmevtyper30_el0";
	case 0xd51befe0:return "pmccfiltr_el0";
	case 0xd51c0000:return "vpidr_el2";
	case 0xd51c00a0:return "vmpidr_el2";
	case 0xd51c1000:return "sctlr_el2";
	case 0xd51c1020:return "actlr_el2";
	case 0xd51c1100:return "hcr_el2";
	case 0xd51c1120:return "mdcr_el2";
	case 0xd51c1140:return "cptr_el2";
	case 0xd51c1160:return "hstr_el2";
	case 0xd51c11e0:return "hacr_el2";
	case 0xd51c1200:return "zcr_el2";
	case 0xd51c1320:return "sder32_el2";
	case 0xd51c2000:return "ttbr0_el2";
	case 0xd51c2020:return "ttbr1_el2";
	case 0xd51c2040:return "tcr_el2";
	case 0xd51c2100:return "vttbr_el2";
	case 0xd51c2140:return "vtcr_el2";
	case 0xd51c2200:return "vncr_el2";
	case 0xd51c2600:return "vsttbr_el2";
	case 0xd51c2640:return "vstcr_el2";
	case 0xd51c3000:return "dacr32_el2";
	case 0xd51c4000:return "spsr_el2";
	case 0xd51c4020:return "elr_el2";
	case 0xd51c4100:return "sp_el1";
	case 0xd51c4300:return "spsr_irq";
	case 0xd51c4320:return "spsr_abt";
	case 0xd51c4340:return "spsr_und";
	case 0xd51c4360:return "spsr_fiq";
	case 0xd51c5020:return "ifsr32_el2";
	case 0xd51c5100:return "afsr0_el2";
	case 0xd51c5120:return "afsr1_el2";
	case 0xd51c5200:return "esr_el2";
	case 0xd51c5260:return "vsesr_el2";
	case 0xd51c5300:return "fpexc32_el2";
	case 0xd51c6000:return "far_el2";
	case 0xd51c6080:return "hpfar_el2";
	case 0xd51c9900:return "pmscr_el2";
	case 0xd51ca200:return "mair_el2";
	case 0xd51ca300:return "amair_el2";
	case 0xd51cc000:return "vbar_el2";
	case 0xd51cc020:return "rvbar_el2";
	case 0xd51cc040:return "rmr_el2";
	case 0xd51cc120:return "vdisr_el2";
	case 0xd51cd020:return "contextidr_el2";
	case 0xd51cd040:return "tpidr_el2";
	case 0xd51ce060:return "cntvoff_el2";
	case 0xd51ce100:return "cnthctl_el2";
	case 0xd51ce200:return "cnthp_tval_el2";
	case 0xd51ce220:return "cnthp_ctl_el2";
	case 0xd51ce240:return "cnthp_cval_el2";
	case 0xd51ce300:return "cnthv_tval_el2";
	case 0xd51ce320:return "cnthv_ctl_el2";
	case 0xd51ce340:return "cnthv_cval_el2";
	case 0xd51ce400:return "cnthvs_tval_el2";
	case 0xd51ce420:return "cnthvs_ctl_el2";
	case 0xd51ce440:return "cnthvs_cval_el2";
	case 0xd51ce500:return "cnthps_tval_el2";
	case 0xd51ce520:return "cnthps_ctl_el2";
	case 0xd51ce540:return "cnthps_cval_el2";
	case 0xd51d1000:return "sctlr_el12";
	case 0xd51d1040:return "cpacr_el12";
	case 0xd51d1200:return "zcr_el12";
	case 0xd51d2000:return "ttbr0_el12";
	case 0xd51d2020:return "ttbr1_el12";
	case 0xd51d2040:return "tcr_el12";
	case 0xd51d4000:return "spsr_el12";
	case 0xd51d4020:return "elr_el12";
	case 0xd51d5100:return "afsr0_el12";
	case 0xd51d5120:return "afsr1_el12";
	case 0xd51d5200:return "esr_el12";
	case 0xd51d6000:return "far_el12";
	case 0xd51d9900:return "pmscr_el12";
	case 0xd51da200:return "mair_el12";
	case 0xd51da300:return "amair_el12";
	case 0xd51dc000:return "vbar_el12";
	case 0xd51dd020:return "contextidr_el12";
	case 0xd51de100:return "cntkctl_el12";
	case 0xd51de200:return "cntp_tval_el02";
	case 0xd51de220:return "cntp_ctl_el02";
	case 0xd51de240:return "cntp_cval_el02";
	case 0xd51de300:return "cntv_tval_el02";
	case 0xd51de320:return "cntv_ctl_el02";
	case 0xd51de340:return "cntv_cval_el02";
	case 0xd51e1000:return "sctlr_el3";
	case 0xd51e1020:return "actlr_el3";
	case 0xd51e1100:return "scr_el3";
	case 0xd51e1120:return "sder32_el3";
	case 0xd51e1140:return "cptr_el3";
	case 0xd51e1200:return "zcr_el3";
	case 0xd51e1320:return "mdcr_el3";
	case 0xd51e2000:return "ttbr0_el3";
	case 0xd51e2040:return "tcr_el3";
	case 0xd51e4000:return "spsr_el3";
	case 0xd51e4020:return "elr_el3";
	case 0xd51e4100:return "sp_el2";
	case 0xd51e5100:return "afsr0_el3";
	case 0xd51e5120:return "afsr1_el3";
	case 0xd51e5200:return "esr_el3";
	case 0xd51e6000:return "far_el3";
	case 0xd51ea200:return "mair_el3";
	case 0xd51ea300:return "amair_el3";
	case 0xd51ec000:return "vbar_el3";
	case 0xd51ec020:return "rvbar_el3";
	case 0xd51ec040:return "rmr_el3";
	case 0xd51ed040:return "tpidr_el3";
	case 0xd51fe200:return "cntps_tval_el1";
	case 0xd51fe220:return "cntps_ctl_el1";
	case 0xd51fe240:return "cntps_cval_el1";
	}
	return 0;
}
//[d5000000,d51f0000]
void disasm_arm64_msr(u32 code)
{
	switch(code){
	case 0xd500407f:{printf("uao	0\n");return;}
	case 0xd500409f:{printf("pan	0\n");return;}
	case 0xd50040bf:{printf("spsel	0\n");return;}

	case 0xd500417f:{printf("uao	1\n");return;}
	case 0xd500419f:{printf("pan	1\n");return;}
	case 0xd50041bf:{printf("spsel	1\n");return;}

	case 0xd503201f:{printf("nop\n");return;}
	case 0xd503203f:{printf("yield\n");return;}
	case 0xd503205f:{printf("wfe\n");return;}
	case 0xd503207f:{printf("wfi\n");return;}
	case 0xd503209f:{printf("sev\n");return;}
	case 0xd50320bf:{printf("sevl\n");return;}
	case 0xd50320ff:{printf("xpaclri\n");return;}
	case 0xd503211f:{printf("pacia1716\n");return;}
	case 0xd503215f:{printf("pacib1716\n");return;}
	case 0xd503219f:{printf("autia1716\n");return;}
	case 0xd50321df:{printf("autib1716\n");return;}
	case 0xd503221f:{printf("esb\n");return;}
	case 0xd503223f:{printf("psb	csync\n");return;}
	case 0xd503229f:{printf("csdb\n");return;}
	case 0xd503231f:{printf("paciaz\n");return;}
	case 0xd503233f:{printf("paciasp\n");return;}
	case 0xd503235f:{printf("pacibz\n");return;}
	case 0xd503237f:{printf("pacibsp\n");return;}
	case 0xd503239f:{printf("autiaz\n");return;}
	case 0xd50323bf:{printf("autiasp\n");return;}
	case 0xd50323df:{printf("autibz\n");return;}
	case 0xd50323ff:{printf("autibsp\n");return;}

	case 0xd503305f:{printf("clrex	0\n");return;}
	case 0xd503309f:{printf("dsb	0\n");return;}
	case 0xd50330bf:{printf("dmb	0\n");return;}
	case 0xd50330df:{printf("isb	0\n");return;}
	case 0xd503315f:{printf("clrex	0x1\n");return;}
	case 0xd503319f:{printf("dsb	oshld\n");return;}
	case 0xd50331bf:{printf("dmb	oshld\n");return;}
	case 0xd50331df:{printf("isb	0x1\n");return;}
	case 0xd503325f:{printf("clrex	0x2\n");return;}
	case 0xd503329f:{printf("dsb	oshst\n");return;}
	case 0xd50332bf:{printf("dmb	oshst\n");return;}
	case 0xd50332df:{printf("isb	0x2\n");return;}
	case 0xd503335f:{printf("clrex	0x3\n");return;}
	case 0xd503339f:{printf("dsb	osh\n");return;}
	case 0xd50333bf:{printf("dmb	osh\n");return;}
	case 0xd50333df:{printf("isb	0x3\n");return;}
	case 0xd503345f:{printf("clrex	0x4\n");return;}
	case 0xd503349f:{printf("dsb	0x4\n");return;}
	case 0xd50334bf:{printf("dmb	0x4\n");return;}
	case 0xd50334df:{printf("isb	0x4\n");return;}
	case 0xd503355f:{printf("clrex	0x5\n");return;}
	case 0xd503359f:{printf("dsb	nshld\n");return;}
	case 0xd50335bf:{printf("dmb	nshld\n");return;}
	case 0xd50335df:{printf("isb	0x5\n");return;}
	case 0xd503365f:{printf("clrex	0x6\n");return;}
	case 0xd503369f:{printf("dsb	nshst\n");return;}
	case 0xd50336bf:{printf("dmb	nshst\n");return;}
	case 0xd50336df:{printf("isb	0x6\n");return;}
	case 0xd503375f:{printf("clrex	0x7\n");return;}
	case 0xd503379f:{printf("dsb	nsh\n");return;}
	case 0xd50337bf:{printf("dmb	nsh\n");return;}
	case 0xd50337df:{printf("isb	0x7\n");return;}
	case 0xd503385f:{printf("clrex	0x8\n");return;}
	case 0xd503389f:{printf("dsb	0x8\n");return;}
	case 0xd50338bf:{printf("dmb	0x8\n");return;}
	case 0xd50338df:{printf("isb	0x8\n");return;}
	case 0xd503395f:{printf("clrex	0x9\n");return;}
	case 0xd503399f:{printf("dsb	ishld\n");return;}
	case 0xd50339bf:{printf("dmb	ishld\n");return;}
	case 0xd50339df:{printf("isb	0x9\n");return;}
	case 0xd5033a5f:{printf("clrex	0xa\n");return;}
	case 0xd5033a9f:{printf("dsb	ishst\n");return;}
	case 0xd5033abf:{printf("dmb	ishst\n");return;}
	case 0xd5033adf:{printf("isb	0xa\n");return;}
	case 0xd5033b5f:{printf("clrex	0xb\n");return;}
	case 0xd5033b9f:{printf("dsb	ish\n");return;}
	case 0xd5033bbf:{printf("dmb	ish\n");return;}
	case 0xd5033bdf:{printf("isb	0xb\n");return;}
	case 0xd5033c5f:{printf("clrex	0xc\n");return;}
	case 0xd5033c9f:{printf("dsb	0xc\n");return;}
	case 0xd5033cbf:{printf("dmb	0xc\n");return;}
	case 0xd5033cdf:{printf("isb	0xc\n");return;}
	case 0xd5033d5f:{printf("clrex	0xd\n");return;}
	case 0xd5033d9f:{printf("dsb	ld\n");return;}
	case 0xd5033dbf:{printf("dmb	ld\n");return;}
	case 0xd5033ddf:{printf("isb	0xd\n");return;}
	case 0xd5033e5f:{printf("clrex	0xe\n");return;}
	case 0xd5033e9f:{printf("dsb	st\n");return;}
	case 0xd5033ebf:{printf("dmb	st\n");return;}
	case 0xd5033edf:{printf("isb	0xe\n");return;}
	case 0xd5033f5f:{printf("clrex\n");return;}
	case 0xd5033f9f:{printf("dsb	sy\n");return;}
	case 0xd5033fbf:{printf("dmb	sy\n");return;}
	case 0xd5033fdf:{printf("isb\n");return;}

	case 0xd503405f:{printf("dit = 0x0\n");return;}
	case 0xd50340df:{printf("daifset = 0x0\n");return;}
	case 0xd50340ff:{printf("daifclr = 0x0\n");return;}
	case 0xd503415f:{printf("dit = 0x1\n");return;}
	case 0xd50341df:{printf("daifset = 0x1\n");return;}
	case 0xd50341ff:{printf("daifclr = 0x1\n");return;}
	case 0xd50342df:{printf("daifset = 0x2\n");return;}
	case 0xd50342ff:{printf("daifclr = 0x2\n");return;}
	case 0xd50343df:{printf("daifset = 0x3\n");return;}
	case 0xd50343ff:{printf("daifclr = 0x3\n");return;}
	case 0xd50344df:{printf("daifset = 0x4\n");return;}
	case 0xd50344ff:{printf("daifclr = 0x4\n");return;}
	case 0xd50345df:{printf("daifset = 0x5\n");return;}
	case 0xd50345ff:{printf("daifclr = 0x5\n");return;}
	case 0xd50346df:{printf("daifset = 0x6\n");return;}
	case 0xd50346ff:{printf("daifclr = 0x6\n");return;}
	case 0xd50347df:{printf("daifset = 0x7\n");return;}
	case 0xd50347ff:{printf("daifclr = 0x7\n");return;}
	case 0xd50348df:{printf("daifset = 0x8\n");return;}
	case 0xd50348ff:{printf("daifclr = 0x8\n");return;}
	case 0xd50349df:{printf("daifset = 0x9\n");return;}
	case 0xd50349ff:{printf("daifclr = 0x9\n");return;}
	case 0xd5034adf:{printf("daifset = 0xa\n");return;}
	case 0xd5034aff:{printf("daifclr = 0xa\n");return;}
	case 0xd5034bdf:{printf("daifset = 0xb\n");return;}
	case 0xd5034bff:{printf("daifclr = 0xb\n");return;}
	case 0xd5034cdf:{printf("daifset = 0xc\n");return;}
	case 0xd5034cff:{printf("daifclr = 0xc\n");return;}
	case 0xd5034ddf:{printf("daifset = 0xd\n");return;}
	case 0xd5034dff:{printf("daifclr = 0xd\n");return;}
	case 0xd5034edf:{printf("daifset = 0xe\n");return;}
	case 0xd5034eff:{printf("daifclr = 0xe\n");return;}
	case 0xd5034fdf:{printf("daifset = 0xf\n");return;}
	case 0xd5034fff:{printf("daifclr = 0xf\n");return;}
	}//switch

	switch(code&0xffffffe0){
	case 0xd5087100:{printf("ic	ialluis\n");return;}
	case 0xd5087500:{printf("ic	iallu\n");return;}
	case 0xd5088100:{printf("vmalle1os\n");return;}
	case 0xd5088300:{printf("vmalle1is\n");return;}
	case 0xd5088700:{printf("vmalle1\n");return;}
	case 0xd50c8100:{printf("alle2os\n");return;}
	case 0xd50c8180:{printf("alle1os\n");return;}
	case 0xd50c81c0:{printf("vmalls12e1os\n");return;}
	case 0xd50c8300:{printf("alle2is\n");return;}
	case 0xd50c8380:{printf("alle1is\n");return;}
	case 0xd50c83c0:{printf("vmalls12e1is\n");return;}
	case 0xd50c8700:{printf("alle2\n");return;}
	case 0xd50c8780:{printf("alle1\n");return;}
	case 0xd50c87c0:{printf("vmalls12e1\n");return;}
	case 0xd50e8100:{printf("alle3os\n");return;}
	case 0xd50e8300:{printf("alle3is\n");return;}
	case 0xd50e8700:{printf("alle3\n");return;}
	}

	char* str = 0;
	switch(code&0xffffffe0){
	case 0xd5087620:str = "ivac";break;
	case 0xd5087640:str = "isw";break;
	case 0xd5087800:str = "s1e1r";break;
	case 0xd5087820:str = "s1e1w";break;
	case 0xd5087840:str = "s1e0r";break;
	case 0xd5087860:str = "s1e0w";break;
	case 0xd5087900:str = "s1e1rp";break;
	case 0xd5087920:str = "s1e1wp";break;
	case 0xd5087a40:str = "csw";break;
	case 0xd5087e40:str = "cisw";break;
	case 0xd5088120:str = "vae1os";break;
	case 0xd5088140:str = "aside1os";break;
	case 0xd5088160:str = "vaae1os";break;
	case 0xd50881a0:str = "vale1os";break;
	case 0xd50881e0:str = "vaale1os";break;
	case 0xd5088220:str = "rvae1is";break;
	case 0xd5088260:str = "rvaae1is";break;
	case 0xd50882a0:str = "rvale1is";break;
	case 0xd50882e0:str = "rvaale1is";break;
	case 0xd5088320:str = "vae1is";break;
	case 0xd5088340:str = "aside1is";break;
	case 0xd5088360:str = "vaae1is";break;
	case 0xd50883a0:str = "vale1is";break;
	case 0xd50883e0:str = "vaale1is";break;
	case 0xd5088520:str = "rvae1os";break;
	case 0xd5088560:str = "rvaae1os";break;
	case 0xd50885a0:str = "rvale1os";break;
	case 0xd50885e0:str = "rvaale1os";break;
	case 0xd5088620:str = "rvae1";break;
	case 0xd5088660:str = "rvaae1";break;
	case 0xd50886a0:str = "rvale1";break;
	case 0xd50886e0:str = "rvaale1";break;
	case 0xd5088720:str = "vae1";break;
	case 0xd5088740:str = "aside1";break;
	case 0xd5088760:str = "vaae1";break;
	case 0xd50887a0:str = "vale1";break;
	case 0xd50887e0:str = "vaale1";break;
	case 0xd50b7420:str = "zva";break;
	case 0xd50b7520:str = "ivau";break;
	case 0xd50b7a20:str = "cvac";break;
	case 0xd50b7b20:str = "cvau";break;
	case 0xd50b7c20:str = "cvap";break;
	case 0xd50b7e20:str = "civac";break;
	case 0xd50c7800:str = "s1e2r";break;
	case 0xd50c7820:str = "s1e2w";break;
	case 0xd50c7880:str = "s12e1r";break;
	case 0xd50c78a0:str = "s12e1w";break;
	case 0xd50c78c0:str = "s12e0r";break;
	case 0xd50c78e0:str = "s12e0w";break;
	case 0xd50c8020:str = "ipas2e1is";break;
	case 0xd50c8040:str = "ripas2e1is";break;
	case 0xd50c80a0:str = "ipas2le1is";break;
	case 0xd50c80c0:str = "ripas2le1is";break;
	case 0xd50c8120:str = "vae2os";break;
	case 0xd50c81a0:str = "vale2os";break;
	case 0xd50c8220:str = "rvae2is";break;
	case 0xd50c82a0:str = "rvale2is";break;
	case 0xd50c8320:str = "vae2is";break;
	case 0xd50c83a0:str = "vale2is";break;
	case 0xd50c8400:str = "ipas2e1os";break;
	case 0xd50c8420:str = "ipas2e1";break;
	case 0xd50c8440:str = "ripas2e1";break;
	case 0xd50c8460:str = "ripas2e1os";break;
	case 0xd50c8480:str = "ipas2le1os";break;
	case 0xd50c84a0:str = "ipas2le1";break;
	case 0xd50c84c0:str = "ripas2le1";break;
	case 0xd50c84e0:str = "ripas2le1os";break;
	case 0xd50c8520:str = "rvae2os";break;
	case 0xd50c85a0:str = "rvale2os";break;
	case 0xd50c8620:str = "rvae2";break;
	case 0xd50c86a0:str = "rvale2";break;
	case 0xd50c8720:str = "vae2";break;
	case 0xd50c87a0:str = "vale2";break;
	case 0xd50e7800:str = "s1e3r";break;
	case 0xd50e7820:str = "s1e3w";break;
	case 0xd50e8120:str = "vae3os";break;
	case 0xd50e81a0:str = "vale3os";break;
	case 0xd50e8220:str = "rvae3is";break;
	case 0xd50e82a0:str = "rvale3is";break;
	case 0xd50e8320:str = "vae3is";break;
	case 0xd50e83a0:str = "vale3is";break;
	case 0xd50e8520:str = "rvae3os";break;
	case 0xd50e85a0:str = "rvale3os";break;
	case 0xd50e8620:str = "rvae3";break;
	case 0xd50e86a0:str = "rvale3";break;
	case 0xd50e8720:str = "vae3";break;
	case 0xd50e87a0:str = "vale3";break;
	}

	u8 r0 = code&0x1f;
	if(str){
		printf("msr	%s = x%d\n", str, r0);
		return;
	}

	str = disasm_arm64_str(code);
	if(str){
		printf("msr	%s = x%d\n", str, r0);
		return;
	}

	u8 s0 = (code >> 5) & 0x7;
	u8 s1 = (code >> 8) & 0xf;
	u8 s2 = (code >>12) & 0xf;
	u8 s3 = (code >>16) & 0x7;
	u8 s4 = (code >>19) & 0x3;
	printf("msr	s%d_%d_c%d_c%d_%d = x%d\n", s4,s3,s2,s1,s0, r0);
}
//[d5200000,d53f0000]
void disasm_arm64_mrs(u32 code)
{
	char* str = 0;
	u8 r0 = code&0x1f;

	str = disasm_arm64_str(code);
	if(str){
		printf("mrs	x%d = %s\n", r0, str);
		return;
	}

	u8 s0 = (code >> 5) & 0x7;
	u8 s1 = (code >> 8) & 0xf;
	u8 s2 = (code >>12) & 0xf;
	u8 s3 = (code >>16) & 0x7;
	u8 s4 = (code >>19) & 0x3;
	printf("mrs	x%d = s%d_%d_c%d_c%d_%d\n", r0, s4,s3,s2,s1,s0);
}




void disasm_arm64_100x(u32 addr, u32 code)
{
	switch((code>>22)&0xf){
//PC-rel. addressing
	case 0x0:
	case 0x1:
	case 0x2:
	case 0x3:{
		u8 r0 = code&0x1f;
		int val = (code>>5)&0x7ffff;
		if(code&0x800000)val -= 0x80000;
		val = (val<<2)+((code>>29)&3);
		if(0x10000000 == (code&0x9f000000)){
			printf("adr	x%d = 0x%x (0x%x+0x%x)\n", r0, addr+val, addr,val);
			return;
		}
		if(0x90000000 == (code&0x9f000000)){
			printf("adrp	x%d = 0x%x (0x%x+0x%x)\n", r0,
				(addr&0xfffff000)+(val<<12),
				(addr&0xfffff000),(val<<12)
			);
			return;
		}
		break;
	}

//Add/subtract (immediate)
	case 0x4:
	case 0x5:{
		u8 r0 = code&0x1f;
		u8 r1 = (code>>5)&0x1f;
		u32 val = (code>>10)&0xfff;

		switch(code&0xff000000){
		case 0x11000000:{
			printf("add	");

			if(0x1f == r0)printf("wsp");
			else printf("w%d", r0);
			printf(" = ");

			if(0x1f == r1)printf("wsp");
			else printf("w%d", r1);

			if(0 == val){printf("\n");return;}
			printf(" + ");

			if(code & 0x400000)printf("0x%x<<12\n", val);
			else printf("0x%x\n", val);
			return;
		}
		case 0x31000000:{
			if(0x1f == r0)printf("cmn	");
			else printf("adds	w%d = ", r0);

			if(0x1f == r1)printf("wsp");
			else printf("w%d", r1);
			printf(" + ");

			if(code & 0x400000)printf("0x%x<<12\n", val);
			else printf("0x%x\n", val);
			return;
		}
		case 0x51000000:{
			printf("sub	");

			if(0x1f == r0)printf("wsp");
			else printf("w%d", r0);
			printf(" = ");

			if(0x1f == r1)printf("wsp - ");
			else printf("w%d - ", r1);

			if(code & 0x400000)printf("0x%x<<12\n", val);
			else printf("0x%x\n", val);
			return;
		}
		case 0x71000000:{
			if(0x1f == r0)printf("cmp	");
			else printf("subs	w%d = ", r0);

			if(0x1f == r1)printf("wsp");
			else printf("w%d", r1);
			printf(" - ");

			if(code & 0x400000)printf("0x%x<<12\n", val);
			else printf("0x%x\n", val);
			return;
		}
		case 0x91000000:{
			printf("add	");

			if(0x1f == r0)printf("sp");
			else printf("x%d", r0);
			printf(" = ");

			if(0x1f == r1)printf("sp");
			else printf("x%d", r1);

			if(0 == val){printf("\n");return;}
			printf(" + ");

			if(code & 0x400000)printf("0x%x<<12\n", val);
			else printf("0x%x\n", val);
			return;
		}
		case 0xb1000000:{
			if(0x1f == r0)printf("cmn	");
			else printf("adds	x%d = ", r0);

			if(0x1f == r1)printf("sp");
			else printf("x%d", r1);
			printf(" + ");

			if(code & 0x400000)printf("0x%x<<12\n", val);
			else printf("0x%x\n", val);
			return;
		}
		case 0xd1000000:{
			printf("sub	");

			if(0x1f == r0)printf("sp");
			else printf("x%d", r0);
			printf(" = ");

			if(0x1f == r1)printf("sp");
			else printf("x%d", r1);
			printf(" - ");

			if(code & 0x400000)printf("0x%x<<12\n", val);
			else printf("0x%x\n", val);
			return;
		}
		case 0xf1000000:{
			if(0x1f == r0)printf("cmp	");
			else printf("subs	x%d = ", r0);

			if(0x1f == r1)printf("sp");
			else printf("x%d", r1);
			printf(" - ");

			if(code & 0x400000)printf("0x%x<<12\n", val);
			else printf("0x%x\n", val);
			return;
		}
		}
		break;
	}

//Add/subtract (immediate, with tags)
	case 0x6:
	case 0x7:{
		u8 r0 = code&0x1f;
		u8 r1 = (code>>5)&0x1f;
		u8 uimm4 = (code>>10)&0xf;
		u32 uimm6 = ((code>>16)&0x3f)*16;
		if(0x91 == (code>>24)){
			printf("addg	tag = 0x%x, x%d = x%d + 0x%x + carry\n", uimm4, r0, r1, uimm6);
			return;
		}
		if(0x91 == (code>>24)){
			printf("subg	tag = %d, x%d = x%d + 0x%x + carry\n", uimm4, r0, r1, uimm6);
			return;
		}
		break;
	}

//Logical (immediate)
	case 0x8:
	case 0x9:{
		u8 r0 = code&0x1f;
		u8 r1 = (code>>5)&0x1f;

		switch(code>>24){
		//and
		case 0x12:{
			printf("and	w%d = w%d & ", r0, r1);

			u8 bit10 = (code>>10)&0x1f;
			u8 bit16 = (code>>16)&0x1f;
			u32 mask = table32(bit10);
			//printf("(%x,%x,%x)",bit10,bit16,mask);
			//if(0x000000 == (code&0xff0000)){
				mask = rotateleft32(mask, 0x20-bit16);
			//}
			printf("0x%x\n", mask);
			return;
		}
		case 0x92:{
			printf("and	x%d = x%d & ", r0, r1);

			u8 bit10 = (code>>10)&0x3f;
			u8 bit16 = (code>>16)&0x3f;
			u64 mask = table64(bit10);
			if(0x400000 != (code&0xff0000)){
				mask = rotateleft64(mask, 0x80-bit16);
			}
			printf("0x%llx\n", mask);
			return;
		}

		//orr
		case 0x32:{
			if(0x1f == r0)printf("mov	wsp");
			else printf("orr	w%d", r0);
			printf(" = ");

			if(0x1f != r1)printf("w%d | ", r1);

			u8 bit10 = (code>>10)&0x1f;
			u8 bit16 = (code>>16)&0x1f;
			u32 mask = table32(bit10);
			//printf("(%x,%x,%x)",bit10,bit16,mask);
			//if(0x000000 == (code&0xff0000)){
				mask = rotateleft32(mask, 0x20-bit16);
			//}
			printf("0x%x\n", mask);
			return;
		}
		case 0xb2:{
			if(0x1f == r0)printf("mov	sp");
			else printf("orr	x%d", r0);
			printf(" = ");

			if(0x1f != r1)printf("x%d | ", r1);

			u8 bit10 = (code>>10)&0x3f;
			u8 bit16 = (code>>16)&0x3f;
			u64 mask = table64(bit10);
			if(0x400000 != (code&0xff0000)){
				mask = rotateleft64(mask, 0x80-bit16);
			}
			printf("0x%llx\n", mask);
			return;
		}

		//eor
		case 0x52:{
			printf("eor	w%d = w%d | ", r0, r1);

			u8 bit10 = (code>>10)&0x1f;
			u8 bit16 = (code>>16)&0x1f;
			u32 mask = table32(bit10);
			//printf("(%x,%x,%x)",bit10,bit16,mask);
			//if(0x000000 == (code&0xff0000)){
				mask = rotateleft32(mask, 0x20-bit16);
			//}
			printf("0x%x\n", mask);
			return;
		}
		case 0xd2:{
			printf("eor	x%d = x%d | ", r0, r1);

			u8 bit10 = (code>>10)&0x3f;
			u8 bit16 = (code>>16)&0x3f;
			u64 mask = table64(bit10);
			if(0x400000 != (code&0xff0000)){
				mask = rotateleft64(mask, 0x80-bit16);
			}
			printf("0x%llx\n", mask);
			return;
		}

		//ands
		case 0x72:{
			if(0x1f == r0)printf("tst	w%d & ", r1);
			else printf("ands	w%d = w%d & ", r0, r1);

			u8 bit10 = (code>>10)&0x1f;
			u8 bit16 = (code>>16)&0x1f;
			u32 mask = table32(bit10);
			//printf("(%x,%x,%x)",bit10,bit16,mask);
			//if(0x000000 == (code&0xff0000)){
				mask = rotateleft32(mask, 0x20-bit16);
			//}
			printf("0x%x\n", mask);
			return;
		}
		case 0xf2:{
			if(0x1f == r0)printf("tst	x%d & ", r1);
			else printf("ands	x%d = x%d & ", r0, r1);

			u8 bit10 = (code>>10)&0x3f;
			u8 bit16 = (code>>16)&0x3f;
			u64 mask = table64(bit10);
			if(0x400000 != (code&0xff0000)){
				mask = rotateleft64(mask, 0x80-bit16);
			}
			printf("0x%llx\n", mask);
			return;
		}
		}//switch(code>>24)

		break;
	}

//Move wide (immediate)
	case 0xa:
	case 0xb:{
		u8 rd = code&0x1f;
		u8 hw = ((code>>21)&3)<<4;
		u32 imm16 = (code>>5)&0xffff;
		switch(code>>24){
		//movn
		case 0x12:{
			printf("movn	w%d = ~(0x%x<<%d)\n", rd, imm16, hw);
			return;
		}
		case 0x92:{
			printf("movn	x%d = ~(0x%x<<%d)\n", rd, imm16, hw);
			return;
		}
		//movz
		case 0x52:{
			printf("movz	w%d = 0x%x<<%d\n", rd, imm16, hw);
			return;
		}
		case 0xd2:{
			printf("movz	x%d = 0x%x<<%d\n", rd, imm16, hw);
			return;
		}
		//movk
		case 0x72:{
			printf("movk	w%d.[%d,%d] = 0x%x\n", rd, hw,hw+15, imm16);
			return;
		}
		case 0xf2:{
			printf("movk	x%d.[%d,%d] = 0x%x\n", rd, hw,hw+15, imm16);
			return;
		}
		}//switch(code>>24)

		break;
	}

//Bitfield
	case 0xc:
	case 0xd:{
		u8 rd = code&0x1f;
		u8 rn = (code>>5)&0x1f;
		u8 imms = (code>>10)&0x3f;
		u8 immr = (code>>16)&0x3f;

		switch(code&0xffc00000){
		case 0x13000000:{
			if(immr<=imms)printf("sbfm	w%d = w%d.[%d,%d]\n", rd, rn, immr, imms);
			else printf("sbfm	w%d.[%d,63] = w%d.[0,%d]\n", rd, 32-immr, rn, imms);
			return;
		}
		case 0x93400000:{
			if(immr<=imms)printf("sbfm	x%d = x%d.[%d,%d]\n", rd, rn, immr, imms);
			else printf("sbfm	x%d.[%d,63] = x%d.[0,%d]\n", rd, 64-immr, rn, imms);
			return;
		}
		//bfm
		case 0x33000000:{
			if(immr<=imms)printf("bfm	w%d = w%d.[%d,%d]\n", rd, rn, immr, imms);
			else printf("bfm	w%d.[%d,%d] = w%d.[0,%d]\n", rd,32-immr,32-immr+imms, rn,imms);
			return;
		}
		case 0xb3400000:{
			if(immr<=imms)printf("bfm	x%d = x%d.[%d,%d]\n", rd, rn, immr, imms);
			else printf("bfm	x%d.[%d,%d] = x%d.[0,%d]\n", rd,64-immr,64-immr+imms, rn,imms);
			return;
		}
		//ubfm
		case 0x53000000:{
			if(immr<=imms)printf("ubfm	w%d = w%d.[%d,%d]\n", rd, rn, immr, imms);
			else printf("ubfm	w%d.[%d,63] = w%d.[0,%d]\n", rd, 32-immr, rn, imms);
			return;
		}
		case 0xd3400000:{
			if(immr<=imms)printf("ubfm	x%d = x%d.[%d,%d]\n", rd, rn, immr, imms);
			else printf("ubfm	x%d.[%d,63] = x%d.[0,%d]\n", rd, 64-immr, rn, imms);
			return;
		}
		}
		break;
	}

//Extract
	case 0xe:
	case 0xf:{
		u8 rd = code&0x1f;
		u8 rn = (code>>5)&0x1f;
		u8 lsb = (code>>10)&0x3f;
		u8 rm = (code>>16)&0x1f;
		if(0x13800000 == (code&0xfff00000)){
			if(rn == rm)printf("ror	w%d = ror(w%d,%d)\n", rd, rn, lsb);
			else printf("extr	w%d = (w%d,w%d)>>%d\n", rd, rn,rm, lsb);
			return;
		}
		if(0x93c00000 == (code&0xffe00000)){
			if(rn == rm)printf("ror	x%d = ror(x%d,%d)\n", rd, rn, lsb);
			else printf("extr	x%d = (x%d,x%d)>>%d\n", rd, rn,rm, lsb);
			return;
		}
		break;
	}
	}

	printf("unknown	data-imm\n");
}
void disasm_arm64_101x(u32 addr, u32 code)
{
	switch(code>>24){
	case 0x54:{
		u8 c = code&0xf;
		int off = ((code>>5)&0x7ffff)<<2;
		if(code&0x800000)off -= 0x200000;

		printf("b.%s	", cond+c*3);
		printf("pc = 0x%x (0x%x+0x%x)\n", addr+off, addr,off);
		return;
	}//Conditional branch (immediate)

	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:{
		int off = (code&0x03ffffff)<<2;
		if(off > 0x08000000)off -= 0x10000000;
		printf("b	pc = 0x%x (0x%x+0x%x)\n", addr+off, addr,off);
		return;
	}
	case 0x94:
	case 0x95:
	case 0x96:
	case 0x97:{
		int off = (code&0x03ffffff)<<2;
		if(off > 0x08000000)off -= 0x10000000;
		printf("bl	lr = 0x%x, pc = 0x%x (0x%x+0x%x)\n", addr+4, addr+off, addr,off);
		return;
	}//Unconditional branch (immediate)

	case 0x34:{
		u8 r0 = code&0x1f;
		int val = (code>>5)&0x7ffff;
		if(code&0x800000)val -= 0x80000;
		val = val<<2;
		printf("cbz	if(0==w%d)b 0x%x (0x%x+0x%x)\n", r0, addr+val, addr,val);
		return;
	}
	case 0x35:{
		u8 r0 = code&0x1f;
		int val = (code>>5)&0x7ffff;
		if(code&0x800000)val -= 0x80000;
		val = val<<2;
		printf("cbnz	if(w%d)b 0x%x (0x%x+0x%x)\n", r0, addr+val, addr,val);
		return;
	}
	case 0xb4:{
		u8 r0 = code&0x1f;
		int val = (code>>5)&0x7ffff;
		if(code&0x800000)val -= 0x80000;
		val = val<<2;
		printf("cbz	if(0==x%d)b 0x%x (0x%x+0x%x)\n", r0, addr+val, addr,val);
		return;
	}
	case 0xb5:{
		u8 r0 = code&0x1f;
		int val = (code>>5)&0x7ffff;
		if(code&0x800000)val -= 0x80000;
		val = val<<2;
		printf("cbnz	if(x%d)b 0x%x (0x%x+0x%x)\n", r0, addr+val, addr,val);
		return;
	}//Compare and branch (immediate)

	case 0x36:{
		u8 r0 = code&0x1f;
		u8 bit = (code>>19)&0x1f;
		int val = (code>>5)&0x3fff;
		if(val&0x2000)val -= 0x4000;
		val = val<<2;
		printf("tbz	if(0==w%d.bit%d)b 0x%x (0x%x+0x%x)\n", r0,bit, addr+val, addr,val);
		return;
	}
	case 0x37:{
		u8 r0 = code&0x1f;
		u8 bit = (code>>19)&0x1f;
		int val = (code>>5)&0x3fff;
		if(val&0x2000)val -= 0x4000;
		val = val<<2;
		printf("tbnz	if(w%d.bit%d)b 0x%x (0x%x+0x%x)\n", r0,bit, addr+val, addr,val);
		return;
	}
	case 0xb6:{
		u8 r0 = code&0x1f;
		u8 bit = (code>>19)&0x1f;
		bit += 32;
		int val = (code>>5)&0x3fff;
		if(val&0x2000)val -= 0x4000;
		val = val<<2;
		printf("tbz	if(0==x%d.bit%d)b 0x%x (0x%x+0x%x)\n", r0,bit, addr+val, addr,val);
		return;
	}
	case 0xb7:{
		u8 r0 = code&0x1f;
		u8 bit = (code>>19)&0x1f;
		bit += 32;
		int val = (code>>5)&0x3fff;
		if(val&0x2000)val -= 0x4000;
		val = val<<2;
		printf("tbnz	if(x%d.bit%d)b 0x%x (0x%x+0x%x)\n", r0,bit, addr+val, addr,val);
		return;
	}//Test and branch (immediate)

	case 0xd4:{
		u8 ophi = (code>>21)&0x7;
		u8 oplo = code&0x1f;
		int imm16 = (code>>5)&0xffff;
		if(0 == ophi){
			if(1 == oplo){printf("svc	0x%x\n",imm16);return;}
			if(2 == oplo){printf("hvc	0x%x\n",imm16);return;}
			if(3 == oplo){printf("smc	0x%x\n",imm16);return;}
		}
		if(1 == ophi){
			if(0 == oplo){printf("brk	0x%x\n",imm16);return;}
		}
		if(2 == ophi){
			if(0 == oplo){printf("hlt	0x%x\n",imm16);return;}
		}
		if(5 == ophi){
			if(1 == oplo){printf("dcps1	0x%x\n",imm16);return;}
			if(2 == oplo){printf("dcps2	0x%x\n",imm16);return;}
			if(3 == oplo){printf("dcps3	0x%x\n",imm16);return;}
		}
		break;
	}

	case 0xd5:{
		if(0xd5000000 == (code&0xffe00000)){
			disasm_arm64_msr(code);
			return;
		}
		if(0xd5200000 == (code&0xffe00000)){
			disasm_arm64_mrs(code);
			return;
		}
		break;
	}

	case 0xd6:
	case 0xd7:{
		u8 r0 = code&0x1f;
		u8 r5 = (code>>5)&0x1f;
		u8 op10 = (code>>10)&0x3f;
		u8 op16 = (code>>16)&0x1f;
		u8 op21 = (code>>21)&0xf;
		if(0x1f != op16)break;

		switch(op21){
		case 0x0:{
			if((0 == op10)&&(0x00 == r0)){
				printf("br	pc = x%d\n", r5);
				return;
			}
			if((2 == op10)&&(0x1f == r0)){
				printf("braa	keya\n");
				return;
			}
			if((3 == op10)&&(0x1f == r0)){
				printf("braa	keyb\n");
				return;
			}
		}
		case 0x1:{
			if((0 == op10)&&(0x00 == r0)){
				printf("blr	pc = x%d\n", r5);
				return;
			}
			if((2 == op10)&&(0x1f == r0)){
				printf("blraa	keya\n");
				return;
			}
			if((3 == op10)&&(0x1f == r0)){
				printf("blraa	keyb\n");
				return;
			}
		}
		case 0x2:{
			if((0 == op10)&&(0x00 == r0)){
				printf("ret	pc = x%d\n", r5);
				return;
			}
			if((2 == op10)&&(0x1f == r5)&&(0x1f == r0)){
				printf("retaa\n");
				return;
			}
			if((3 == op10)&&(0x1f == r5)&&(0x1f == r0)){
				printf("retab\n");
				return;
			}
		}
		case 0x4:{
			if((0 == op10)&&(0x00 == r0)){
				printf("eret	pc = x%d\n", r5);
				return;
			}
			if((2 == op10)&&(0x1f == r5)&&(0x1f == r0)){
				printf("eretaa\n");
				return;
			}
			if((3 == op10)&&(0x1f == r5)&&(0x1f == r0)){
				printf("eretab\n");
				return;
			}
		}
		case 0x5:{
			if((0 == op10)&&(0x1f == r5)&&(0x00 == r0)){
				printf("drps\n");
				return;
			}
		}
		case 0x8:{
			if(2 == op10){
				printf("braa	keya,reg\n");
				return;
			}
			if(3 == op10){
				printf("braa	keyb,reg\n");
				return;
			}
		}
		case 0x9:{
			if(2 == op10){
				printf("blraa	keya,reg\n");
				return;
			}
			if(3 == op10){
				printf("blraa	keyb,reg\n");
				return;
			}
		}
		break;
		}//switch
	break;
	}//Unconditional branch (register)

	}//switch

	//other undefined
	printf("unknown	system-inst\n");
}
void disasm_arm64_x1x0(u32 addr, u32 code)
{/*
	if(0x08000000 == (code&0xff000000)){
		u8 val = code&0x1f;
		u8 adr = (code>>5)&0x1f;
		u8 sts = (code>>16)&0x1f;
		printf("ldxrb	w%d = [x%d].byte\n", val, adr);
		printf("stxrb	[x%d].byte = w%d, w%d = status\n", adr, val, sts);
	}*/
	if(0x18000000 == (code&0xff000000)){
		u8 r0 = code&0x1f;
		int off = ((code>>5)&0x3ffff)<<2;
		printf("ldr	w%d = [0x%x]\n", r0, addr+off);
		return;
	}
	if(0x58000000 == (code&0xff000000)){
		u8 r0 = code&0x1f;
		int off = ((code>>5)&0x7ffff)<<2;
		printf("ldr	x%d = [0x%x]\n", r0, addr+off);
		return;
	}
	if(0xb8000000 == (code&0xff000000)){
		u8 r0 = code&0x1f;
		u8 r1 = (code>>5)&0x1f;

		switch(code&0xc00000){
		case 0x000000:{
			int off = (code>>12)&0xff;
			if(code&0x100000)off -= 256;

			switch(code&0xc00){
			case 0x000:printf("stur	[x%d %+d] = w%d\n", r1, off, r0);return;
			case 0x400:printf("str	[x%d] %+d = w%d\n", r1, off, r0);return;
			case 0x800:printf("sttr	[x%d %+d] = w%d\n", r1, off, r0);return;
			case 0xc00:printf("str!	[x%d %+d] = w%d\n", r1, off, r0);return;
			}
			break;
		}
		case 0x400000:{
			int off = (code>>12)&0xff;
			if(code&0x100000)off -= 256;

			switch(code&0xc00){
			case 0x000:printf("ldur	w%d == [x%d %+d]\n", r0, r1, off);return;
			case 0x400:printf("ldr	w%d == [x%d] %+d\n", r0, r1, off);return;
			case 0x800:printf("ldtr	w%d == [x%d %+d]\n", r0, r1, off);return;
			case 0xc00:printf("ldr!	w%d == [x%d %+d]\n", r0, r1, off);return;
			}
			break;
		}
		}
	}
	if(0xa8000000 == (code&0xff000000)){
		u8 r0 = code&0x1f;
		u8 r1 = (code>>10)&0x1f;
		u8 rr = (code>>5)&0x1f;
		int off = (code>>15)&0x3f;
		if(code&0x200000)off -= 0x40;
		off *= 8;

		switch(code&0xc00000){
		case 0x000000:printf("stnp	[x%d %+d] = x%d,x%d\n", rr,off, r0,r1);return;
		case 0x400000:printf("ldnp	x%d,x%d = [x%d %+d]\n", r0,r1, rr,off);return;
		case 0x800000:printf("stp	[x%d] = x%d,x%d, x%d += %d\n", rr, r0,r1, rr,off);return;
		case 0xc00000:printf("ldp	x%d,x%d = [x%d], x%d += %d\n", r0,r1, rr, rr,off);return;
		}
	}
	if(0xa9000000 == (code&0xff000000)){
		u8 r0 = code&0x1f;
		u8 r1 = (code>>10)&0x1f;
		u8 rr = (code>>5)&0x1f;
		int off = (code>>15)&0x3f;
		if(code&0x200000)off -= 0x40;
		off *= 8;

		switch(code&0xc00000){
		case 0x000000:printf("stp	[x%d %+d] = x%d,x%d\n", rr,off, r0,r1);return;
		case 0x400000:printf("ldp	x%d,x%d = [x%d %+d]\n", r0,r1, rr,off);return;
		case 0x800000:printf("stp!	x%d += 0x%x, [x%d] = x%d,x%d\n", rr,off, rr, r0,r1);return;
		case 0xc00000:printf("ldp!	x%d += 0x%x, x%d,x%d = [x%d]\n", rr,off, r0,r1, rr);return;
		}
	}
	if(0xb9000000 == (code&0xff000000)){
		u8 r0 = code&0x1f;
		u8 r1 = (code>>5)&0x1f;
		int off = (code>>10)&0xfff;
		switch(code&0xc00000){
		case 0x000000:printf("str	[x%d %+d] = w%d\n", r1, off<<2, r0);return;
		case 0x400000:printf("ldr	w%d = [x%d %+d]\n", r0, r1, off<<2);return;
		case 0x800000:printf("ldrsw	x%d = [x%d %+d]\n", r0, r1, off<<2);return;
		}
	}
	if(0xf8000000 == (code&0xff000000)){
		u8 r0 = code&0x1f;
		u8 rr = (code>>5)&0x1f;

		switch(code&0xe00000){
		case 0x000000:{
			int val = (code>>12)&0xff;
			if(code&0x100000)val -= 256;

			switch(code&0xc00){
			case 0x000:printf("stur	[x%d %+d] = x%d\n", rr, val, r0);return;
			case 0x400:printf("str	[x%d] = x%d, x%d += %d\n", rr, r0, rr,val);return;
			case 0x800:printf("sttr	[x%d %+d] = x%d\n", rr,val, r0);return;
			case 0xc00:printf("str!	x%d += 0x%x, [x%d] = x%d\n", rr,val, rr, r0);return;
			}
		}
		case 0x400000:{
			int val = (code>>12)&0xff;
			if(code&0x100000)val -= 256;

			switch(code&0xc00){
			case 0x000:printf("ldur	x%d = [x%d %+d]\n", r0, rr, val);return;
			case 0x400:printf("ldr	x%d = [x%d], x%d += %d\n", r0, rr, rr,val);return;
			case 0x800:printf("ldtr	x%d = [x%d %+d]\n", r0, rr,val);return;
			case 0xc00:printf("ldr!	x%d += 0x%x, x%d = [x%d]\n", rr,val, r0, rr);return;
			}
		}
		}
	}
	if(0xf9000000 == (code&0xff000000)){
		u8 r0 = code&0x1f;
		u8 rr = (code>>5)&0x1f;
		int off = (code>>10)&0xfff;
		if(code&0x400000){
			printf("ldr	x%d = [x%d %+d]\n", r0, rr, off*8);
		}
		else{
			printf("str	[x%d %+d] = x%d\n", rr, off*8, r0);
		}
		return;
	}
	if(0xfd000000 == (code&0xff000000)){
		u8 r0 = code&0x1f;
		u8 r1 = (code>>5)&0x1f;
		int val = ((code>>10)&0xfff)<<3;
		if(code & 0x400000){
			printf("ldr	d%d = [x%d + %d]\n", r0, r1,val);
		}
		else{
			printf("str	[x%d + %d] = d%d\n", r1,val, r0);
		}
		return;
	}
	printf("unknown	load,store\n");
}
void disasm_arm64_x101(u32 addr, u32 code)
{
//-----------------mov-------------------
/*
	else if(0xaa0003e0 == (code&0xffc003e0)){
		printf("mov	x%d = x%d\n", code&0x1f, (code>>16)&0x1f);
	}
	else if(0x2a0003e0 == (code&0xffc003e0)){
		printf("mov	w%d = w%d\n", code&0x1f, (code>>16)&0x1f);
	}
	else if(0xd2800000 == (code&0xfff00000)){
		printf("mov	x%d = %d\n", (code&0x1f), (code>>5)&0x7fff);
	}
	else if(0x52800000 == (code&0xfff00000)){
		printf("mov	w%d = %d\n", (code&0x1f), (code>>5)&0x7fff);
	}
*/
	u8 op0 = (code>>30)&1;
	u8 op1 = (code>>28)&1;
	u8 op2 = (code>>21)&0xf;
	u8 op3 = (code>>10)&0x3f;
	//Data-processing (2 source)
	if((0 == op0)&&(1 == op1)&&(6 == op2)){
		u8 rd = code&0x1f;
		u8 rn = (code>>5)&0x1f;
		u8 rm = (code>>15)&0x1f;

		u8 sz = 'w';
		if(code>>31)sz = 'x';

		u8 op = (code>>10)&0x3f;
		switch(op){
		case 0:{
			if(4 == (code>>29)){
				printf("subp	%c%d = %c%d / %c%d\n", sz,rd, sz,rn, sz,rm);
				return;
			}
			if(5 == (code>>29)){
				printf("subps	%c%d = %c%d / %c%d\n", sz,rd, sz,rn, sz,rm);
				return;
			}
			break;
		}
		case 2:{
			if(0 == (code&0x20000000)){
				printf("udiv	%c%d = %c%d / %c%d\n", sz,rd, sz,rn, sz,rm);
				return;
			}
			break;
		}
		case 3:{
			if(0 == (code&0x20000000)){
				printf("sdiv	%c%d = %c%d / %c%d\n", sz,rd, sz,rn, sz,rm);
				return;
			}
			break;
		}
		case 4:{
			if(0x9ac01000 == (code&0xffe0fc00)){
				printf("irg	%c%d = %c%d ? %c%d\n", sz,rd, sz,rn, sz,rm);
				return;
			}
			break;
		}
		case 5:{
			if(0x9ac01400 == (code&0xffe0fc00)){
				printf("gmi	%c%d = %c%d ? %c%d\n", sz,rd, sz,rn, sz,rm);
				return;
			}
			break;
		}
		case 8:{
			if(0 == (code&0x20000000)){
				printf("lsl	%c%d = %c%d << %c%d\n", sz,rd, sz,rn, sz,rm);
				return;
			}
			break;
		}
		case 9:{
			if(0 == (code&0x20000000)){
				printf("lsr	%c%d = %c%d >> %c%d\n", sz,rd, sz,rn, sz,rm);
				return;
			}
			break;
		}
		case 0xa:{
			if(0 == (code&0x20000000)){
				printf("asr	%c%d = asr(%c%d, %c%d)\n", sz,rd, sz,rn, sz,rm);
				return;
			}
			break;
		}
		case 0xb:{
			if(0 == (code&0x20000000)){
				printf("ror	%c%d = ror(%c%d, %c%d)\n", sz,rd, sz,rn, sz,rm);
				return;
			}
			break;
		}
		case 0xc:{
			if(0x9ac03000 == (code&0xffe0fc00)){
				printf("ror	%c%d = ror(%c%d, %c%d)\n", sz,rd, sz,rn, sz,rm);
				return;
			}
			break;
		}
		case 0x10:{
			if(0x1ac04000 == (code&0xffe0fc00)){
				printf("crc32b	w%d = w%d ? w%d", rd, rn, rm);
				return;
			}
			break;
		}
		case 0x11:{
			if(0x1ac04400 == (code&0xffe0fc00)){
				printf("crc32h	w%d = w%d ? w%d", rd, rn, rm);
				return;
			}
			break;
		}
		case 0x12:{
			if(0x1ac04800 == (code&0xffe0fc00)){
				printf("crc32w	w%d = w%d ? w%d", rd, rn, rm);
				return;
			}
			break;
		}
		case 0x13:{
			if(0x9ac04c00 == (code&0xffe0fc00)){
				printf("crc32x	w%d = w%d ? x%d", rd, rn, rm);
				return;
			}
			break;
		}
		case 0x14:{
			if(0x1ac05000 == (code&0xffe0fc00)){
				printf("crc32cb	w%d = w%d ? w%d", rd, rn, rm);
				return;
			}
			break;
		}
		case 0x15:{
			if(0x1ac05400 == (code&0xffe0fc00)){
				printf("crc32ch	w%d = w%d ? w%d", rd, rn, rm);
				return;
			}
			break;
		}
		case 0x16:{
			if(0x1ac05800 == (code&0xffe0fc00)){
				printf("crc32cw	w%d = w%d ? w%d", rd, rn, rm);
				return;
			}
			break;
		}
		case 0x17:{
			if(0x9ac05c00 == (code&0xffe0fc00)){
				printf("crc32cx	w%d = w%d ? x%d", rd, rn, rm);
				return;
			}
			break;
		}
		}
	}
	//Data-processing (1 source)
	if((1 == op0)&&(1 == op1)&&(6 == op2)){
		u8 rd = code&0x1f;
		u8 rn = (code>>5)&0x1f;

		u8 sz = 'w';
		if(code>>31)sz = 'x';

		if(0x5ac00000 == (code&0xfffffc00)){
			printf("rbit	w%d = revbit(w%d)\n", rd, rn);
			return;
		}
		if(0xdac00000 == (code&0xfffffc00)){
			printf("rbit	x%d = revbit(x%d)\n", rd, rn);
			return;
		}
		if(0x5ac00400 == (code&0xfffffc00)){
			printf("rev16	w%d.0123 = w%d.1032\n", rd ,rn);
			return;
		}
		if(0xdac00400 == (code&0xfffffc00)){
			printf("rev16	x%d.01234567 = x%d.10325476\n", rd, rn);
			return;
		}
		if(0x5ac00800 == (code&0xfffffc00)){
			printf("rev	w%d.0123 = w%d.3210\n", rd, rn);
			return;
		}
		if(0xdac00c00 == (code&0xfffffc00)){
			printf("rev	w%d.01234567 = w%d.76543210\n", rd, rn);
			return;
		}
		if(0x5ac01000 == (code&0xfffffc00)){
			printf("clz	w%d = how_many_lead_zero(w%d)\n", rd, rn);
			return;
		}
		if(0xdac01000 == (code&0xfffffc00)){
			printf("clz	x%d = how_many_lead_zero(x%d)\n", rd, rn);
			return;
		}
	}
	if(0 == op1){
		//Logical (shifted register)
		if(op2 < 4){
			u8 r0 = code&0x1f;
			u8 r1 = (code>>5)&0x1f;

			u8 sz = 'w';
			if(code>>31)sz = 'x';

			u8 lo = (code>>21)&1;
			u8 hi = ((code>>29)&3)<<1;
			switch(hi | lo){
			case 0:printf("and	%c%d = %c%d & ", sz,r0, sz,r1);break;
			case 1:printf("bic	%c%d = %c%d & ~",sz,r0, sz,r1);break;
			case 2:{
				if(0x1f == r1)printf("mov	%c%d = ", sz,r0);
				else printf("orr	%c%d = %c%d | ", sz,r0, sz,r1);
				break;
			}
			case 3:printf("orn	%c%d = %c%d | ~",sz,r0, sz,r1);break;
			case 4:printf("eor	%c%d = %c%d ^ ", sz,r0, sz,r1);break;
			case 5:printf("eon	%c%d = %c%d ^ ~",sz,r0, sz,r1);break;
			case 6:printf("ands	%c%d = %c%d & ", sz,r0, sz,r1);break;
			case 7:printf("bics	%c%d = %c%d & ~",sz,r0, sz,r1);break;
			}

			u8 sh = (code>>10)&0x3f;
			u8 r2 = (code>>16)&0x1f;
			u8 op = (code>>22)&3;
			if(0 == op){
				if(0 == sh)printf("%c%d\n", sz,r2);
				else printf("lsl(%c%d,%d)\n", sz,r2, sh);
				return;
			}
			if(1 == op){
				printf("lsr(%c%d,%d)\n", sz,r2, sh);
				return;
			}
			if(2 == op){
				printf("asr(%c%d,%d)\n", sz,r2, sh);
				return;
			}
			if(3 == op){
				printf("ror(%c%d,%d)\n", sz,r2, sh);
				return;
			}
		}
		//Add/subtract (shifted register)
		if(8 == (op2 & 9)){
			u8 r0 = code&0x1f;
			u8 r1 = (code>>5)&0x1f;

			u8 sz = 'w';
			if(code>>31)sz = 'x';

			switch((code>>29)&3){
			case 0:{
				printf("add	%c%d = %c%d + ", sz,r0, sz,r1);
				break;
			}
			case 1:{
				if(0x1f == r0)printf("cmn	%c%d + ", sz,r1);
				else printf("adds	%c%d = %c%d + ", sz,r0, sz,r1);
				break;
			}
			case 2:{
				printf("sub	%c%d = %c%d - ", sz,r0, sz,r1);
				break;
			}
			case 3:{
				if(0x1f == r0)printf("cmp	%c%d - ", sz,r1);
				else if(0x1f == r1)printf("negs	%c%d = - ", sz,r0);
				else printf("subs	%c%d = %c%d - ", sz,r0, sz,r1);
				break;
			}
			}

			u8 sh = (code>>10)&0x3f;
			u8 r2 = (code>>16)&0x1f;
			u8 op = (code>>22)&3;
			if(0 == op){
				if(0 == sh)printf("%c%d\n", sz,r2);
				else printf("lsl(%c%d,%d)\n", sz,r2, sh);
				return;
			}
			if(1 == op){
				printf("lsr(%c%d,%d)\n", sz,r2, sh);
				return;
			}
			if(2 == op){
				printf("asr(%c%d,%d)\n", sz,r2, sh);
				return;
			}
			printf("unknown\n");
			return;
		}
		//Add/subtract (extended register)
		if(9 == (op2 & 9)){
			u8 r0 = code&0x1f;
			u8 r1 = (code>>5)&0x1f;

			u8 sz = 'w';
			if(code>>31)sz = 'x';

			switch((code>>29)&3){
			case 0:{
				printf("add	");

				if(0x1f == r0){
					if('x'==sz)printf("sp = ");
					else printf("wsp = ");
				}
				else printf("%c%d = ", sz,r0);

				if(0x1f == r1){
					if('x'==sz)printf("sp + ");
					else printf("wsp + ");
				}
				else printf("%c%d + ", sz,r1);

				break;
			}
			case 1:{
				if(0x1f == r0)printf("cmn	%c%d + ", sz,r1);
				else printf("adds	%c%d = %c%d + ", sz,r0, sz,r1);
				break;
			}
			case 2:{
				printf("sub	");

				if(0x1f == r0){
					if('x'==sz)printf("sp = ");
					else printf("wsp = ");
				}
				else printf("%c%d = ", sz,r0);

				if(0x1f == r1){
					if('x'==sz)printf("sp - ");
					else printf("wsp - ");
				}
				else printf("%c%d + ", sz,r1);

				break;
			}
			case 3:{
				if(0x1f == r0)printf("cmp	%c%d - ", sz,r1);
				else if(0x1f == r1)printf("negs	%c%d = - ", sz,r0);
				else printf("subs	%c%d = %c%d - ", sz,r0, sz,r1);
				break;
			}
			}

			u8 sh = (code>>10)&0x7;
			u8 op = (code>>13)&0x7;
			u8 r2 = (code>>16)&0x1f;
			switch(op){
			case 0:printf("uxtb(%c%d,%d)\n", sz,r2, sh);return;
			case 1:printf("uxth(%c%d,%d)\n", sz,r2, sh);return;
			case 2:{
				if((0x1f == r1)&&(0 == (code>>31))){
					if(0 == sh)printf("%c%d\n", sz,r2);
					else printf("lsl(%c%d,%d)\n", sz,r2, sh);
				}
				else printf("uxtw(%c%d,%d)\n", sz,r2, sh);
				return;
			}
			case 3:{
				if((0x1f == r1)&&(code>>31)){
					if(0 == sh)printf("%c%d\n", sz,r2);
					else printf("lsl(%c%d,%d)\n", sz,r2, sh);
				}
				else printf("uxtx(%c%d,%d)\n", sz,r2, sh);
				return;
			}
			case 4:printf("sxtb(%c%d,%d)\n", sz,r2, sh);return;
			case 5:printf("sxth(%c%d,%d)\n", sz,r2, sh);return;
			case 6:printf("sxtw(%c%d,%d)\n", sz,r2, sh);return;
			case 7:printf("sxtx(%c%d,%d)\n", sz,r2, sh);return;
			}
		}
	}
	if(1 == op1){
		if(0 == op2){
			//Add/subtract (with carry)
			if(0 == op3){
				u8 rd = code&0x1f;
				u8 rn = (code>>5)&0x1f;
				u8 rm = (code>>16)&0x1f;
				switch(code>>29){
				case 0:{
					printf("adc	w%d = w%d + w%d + carry\n", rd, rn, rm);
					return;
				}
				case 1:{
					printf("adcs	w%d = w%d + w%d + carry\n", rd, rn, rm);
					return;
				}
				case 2:{
					printf("sbc	w%d = w%d + ~w%d + carry\n", rd, rn, rm);
					return;
				}
				case 3:{
					printf("sbcs	w%d = w%d + ~w%d + carry\n", rd, rn, rm);
					return;
				}
				case 4:{
					printf("adc	x%d = x%d + x%d + carry\n", rd, rn, rm);
					return;
				}
				case 5:{
					printf("adcs	x%d = x%d + x%d + carry\n", rd, rn, rm);
					return;
				}
				case 6:{
					printf("sbc	x%d = x%d + ~x%d + carry\n", rd, rn, rm);
					return;
				}
				case 7:{
					printf("sbcs	x%d = x%d + ~x%d + carry\n", rd, rn, rm);
					return;
				}
				}
			}
			//Rotate right into flags
			if(1 == (op3&0x1f)){
				if(0xba000400 == (code&0xffe007c10)){
					u8 rn = (code>>5)&0x1f;
					u8 imm6 = (code>>15)&0x3f;
					printf("rmif	x%d, %d\n", rn, imm6);
					return;
				}
			}
			//Evaluate into flags
			if(2 == (op3&0x0f)){
				u8 rn = (code>>5)&0x1f;
				if(0x3a00080d == (code&0xfffffc1f)){
					printf("setf8	w%d\n", rn);
					return;
				}
				if(0x3a00480d == (code&0xfffffc1f)){
					printf("setf16	w%d\n", rn);
					return;
				}
			}
		}//0==op2

		if(2 == op2){
			//Conditional compare (register)
			if(0 == (op3 & 2)){
				u8 rn = (code>>5)&0x1f;
				u8 rm = (code>>16)&0x1f;

				switch(code>>29){
				case 1:printf("ccmn	w%d,w%d\n", rn, rm);return;
				case 3:printf("ccmp	w%d,w%d\n", rn, rm);return;
				case 5:printf("ccmn	x%d,x%d\n", rn, rm);return;
				case 7:printf("ccmp	x%d,x%d\n", rn, rm);return;
				}
			}
			//Conditional compare (immediate)
			if(op3 & 2){
				u8 rn = (code>>5)&0x1f;
				u8 imm5 = (code>>16)&0x1f;

				switch(code>>29){
				case 1:printf("ccmn	w%d, %d\n", rn, imm5);return;
				case 3:printf("ccmp	w%d, %d\n", rn, imm5);return;
				case 5:printf("ccmn	x%d, %d\n", rn, imm5);return;
				case 7:printf("ccmp	x%d, %d\n", rn, imm5);return;
				}
			}
		}

		//Conditional select
		if(4 == op2){
			u8 rd = code&0x1f;
			u8 rn = (code>>5)&0x1f;
			u8 cd = (code>>12)&0xf;
			u8 rm = (code>>16)&0x1f;

			u8 aa = (code>>30)<<1;
			u8 bb = (code>>10)&1;
			switch(aa | bb){
			case 0:printf("csel	w%d = %s ? w%d : w%d\n", rd, cond+3*cd, rn, rm);return;
			case 1:printf("csinc	w%d = %s ? w%d : (w%d+1)\n", rd, cond+3*cd, rn, rm);return;
			case 2:printf("csinv	w%d = %s ? w%d : ~w%d\n", rd, cond+3*cd, rn, rm);return;
			case 3:printf("csneg	w%d = %s ? w%d : (~w%d+1)\n", rd, cond+3*cd, rn, rm);return;
			case 4:printf("csel	x%d = %s ? x%d : x%d\n", rd, cond+3*cd, rn, rm);return;
			case 5:printf("csinc	x%d = %s ? x%d : (x%d+1)\n", rd, cond+3*cd, rn, rm);return;
			case 6:printf("csinv	x%d = %s ? x%d : ~x%d\n", rd, cond+3*cd, rn, rm);return;
			case 7:printf("csneg	x%d = %s ? x%d : (~x%d+1)\n", rd, cond+3*cd, rn, rm);return;
			}
		}

		//Data-processing (3 source)
		if(op2 & 0x8){
			u8 rd = code&0x1f;
			u8 rn = (code>>5)&0x1f;
			u8 ra = (code>>10)&0x1f;
			u8 rm = (code>>16)&0x1f;
			//madd
			if(0x1b000000 == (code&0xff008000)){
				if(0x1f == ra)printf("mul	w%d = w%d * w%d\n", rd, rn, rm);
				else printf("madd	w%d = w%d + (w%d * w%d)\n", rd, ra, rn, rm);
				return;
			}
			if(0x9b000000 == (code&0xff008000)){
				if(0x1f == ra)printf("mul	x%d = x%d * x%d\n", rd, rn, rm);
				else printf("madd	x%d = x%d + (x%d * x%d)\n", rd, ra, rn, rm);
				return;
			}
			//msub
			if(0x1b008000 == (code&0xff008000)){
				if(0x1f == ra)printf("mneg	w%d = - (w%d * w%d)\n", rd, rn, rm);
				else printf("msub	w%d = w%d - (w%d * w%d)\n", rd, ra, rn, rm);
				return;
			}
			if(0x9b008000 == (code&0xff008000)){
				if(0x1f == ra)printf("mneg	x%d = - (x%d * x%d)\n", rd, rn, rm);
				else printf("msub	x%d = x%d - (x%d * x%d)\n", rd, ra, rn, rm);
				return;
			}

			//smaddl
			if(0x9b200000 == (code&0xffe08000)){
				if(0x1f == ra)printf("smull	x%d = smul(w%d, w%d)\n", rd, rn, rm);
				else printf("smaddl	x%d = x%d + smul(w%d, %d)\n", rd, ra, rn, rm);
				return;
			}
			//smsubl
			if(0x9b208000 == (code&0xffe08000)){
				if(0x1f == ra)printf("smnegl	x%d = - smul(w%d, w%d)\n", rd, rn, rm);
				else printf("smaddl	x%d = x%d - smul(w%d, %d)\n", rd, ra, rn, rm);
				return;
			}
			//smulh
			if(0x9b208000 == (code&0xffe08000)){
				printf("smulh	x%d = smul(x%d, x%d)>>64\n", rd, rn, rm);
				return;
			}

			//umaddl
			if(0x9ba00000 == (code&0xffe08000)){
				if(0x1f == ra)printf("umull	x%d = umul(w%d, w%d)\n", rd, rn, rm);
				else printf("umaddl	x%d = x%d + umul(w%d, %d)\n", rd, ra, rn, rm);
				return;
			}
			//umsubl
			if(0x9ba08000 == (code&0xffe08000)){
				if(0x1f == ra)printf("umnegl	x%d = - umul(w%d, w%d)\n", rd, rn, rm);
				else printf("umaddl	x%d = x%d - umul(w%d, %d)\n", rd, ra, rn, rm);
				return;
			}
			//umulh
			if(0x9bc00000 == (code&0xffe08000)){
				printf("umulh	x%d = umul(x%d, x%d)>>64\n", rd, rn, rm);
				return;
			}
		}
	}
	//
	printf("unknown	data-reg\n");
}
void disasm_arm64_x111(u32 addr, u32 code)
{
	printf("unknown	data-fp\n");
}
void disasm_arm64_one(u32 code, u64 rip)
{
	printf("%8llx:	%08x	", rip, code);

	switch((code>>25) & 0xf){
	//100x: Data Processing -- Immediate
	case 0x8:
	case 0x9:{
		disasm_arm64_100x(rip, code);
		break;
	}
	//101x: Branches, Exception Generating and System instructions
	case 0xa:
	case 0xb:{
		disasm_arm64_101x(rip, code);
		break;
	}
	//x1x0: Loads and Stores
	case 0x4:
	case 0x6:
	case 0xc:
	case 0xe:{
		disasm_arm64_x1x0(rip, code);
		break;
	}
	//x101: Data Processing -- Register
	case 0x5:
	case 0xd:{
		disasm_arm64_x101(rip, code);
		break;
	}
	//x111: Data Processing -- Scalar Floating-Point and Advanced SIMD
	case 0x7:
	case 0xf:{
		disasm_arm64_x111(rip, code);
		break;
	}
	default:printf("unknown\n");
	}
}
void disasm_arm64_all(u8* buf, int len, int rip)
{
	int j;
	u32 code;
	for(j=0;j<len;j+=4){
		code = *(u32*)(buf+j);
		disasm_arm64_one(code, rip+j);
	}
}
void disasm_arm64(int argc, char** argv)
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
	disasm_arm64_all(buf, ret, 0);

release:
	free(buf);
theend:
	close(fd);
}


u32 arm64_str2data(u8* p){
	char* buf = (char*)p;
	if( ('0' == buf[0]) && ('x' == buf[1]) )return strtol((char*)buf+2, 0, 16);
	else return atoi((char*)buf);
}
void str2type(u8* p, int* t0, int* t1){
	if( ('l'==p[0]) && ('s'==p[1]) && ('l'==p[2]) ){
		*t0 = 1;
		*t1 = 0;
	}
	if( ('l'==p[0]) && ('s'==p[1]) && ('r'==p[2]) ){
		*t0 = 1;
		*t1 = 1;
	}
	if( ('a'==p[0]) && ('s'==p[1]) && ('r'==p[2]) ){
		*t0 = 1;
		*t1 = 2;
	}

	if( ('u'==p[0]) && ('x'==p[1]) && ('t'==p[2]) && ('b'==p[3]) ){
		*t0 = 2;
		*t1 = 0;
	}
	if( ('u'==p[0]) && ('x'==p[1]) && ('t'==p[2]) && ('h'==p[3]) ){
		*t0 = 2;
		*t1 = 1;
	}
	if( ('u'==p[0]) && ('x'==p[1]) && ('t'==p[2]) && ('w'==p[3]) ){
		*t0 = 2;
		*t1 = 2;
	}
	if( ('u'==p[0]) && ('x'==p[1]) && ('t'==p[2]) && ('x'==p[3]) ){
		*t0 = 2;
		*t1 = 3;
	}
	if( ('s'==p[0]) && ('x'==p[1]) && ('t'==p[2]) && ('b'==p[3]) ){
		*t0 = 2;
		*t1 = 4;
	}
	if( ('s'==p[0]) && ('x'==p[1]) && ('t'==p[2]) && ('h'==p[3]) ){
		*t0 = 2;
		*t1 = 5;
	}
	if( ('s'==p[0]) && ('x'==p[1]) && ('t'==p[2]) && ('w'==p[3]) ){
		*t0 = 2;
		*t1 = 6;
	}
	if( ('s'==p[0]) && ('x'==p[1]) && ('t'==p[2]) && ('x'==p[3]) ){
		*t0 = 2;
		*t1 = 7;
	}
}


struct offlen{
	u8 off;
	u8 len;
}__attribute__((packed));
void assembly_arm64_orr(u8* buf, int len, struct offlen* tab, int cnt)
{
	tab = &tab[1];
	cnt = cnt-1;

	u8* dst0b = buf+tab[0].off;
	int dst0l = tab[0].len;
	u8* src0b = buf+tab[2].off;
	int src0l = tab[2].len;

	int sf = -1;
	if( ('x'==dst0b[0]) && ('x'==src0b[0]) )sf = 1;
	if( ('w'==dst0b[0]) && ('w'==src0b[0]) )sf = 0;

	u32 bin = 0;
	if(sf >= 0){
		int d0 = atoi((char*)dst0b+1);
		int s0 = atoi((char*)src0b+1);
		bin |= (d0<<0);
		bin |= (0x1f<<5);
		bin |= (s0<<16);
		bin |= 0x2a000000;
		bin |= ((u64)sf)<<31;
		disasm_arm64_one(bin, 0);
	}
}
void assembly_arm64_add(u8* ptr, int len, struct offlen* tab, int cnt)
{
	int what = 0;
	if(strncmp((char*)ptr+tab[0].off, "adds", 4) == 0){
		what = 1;
	}
	if(strncmp((char*)ptr+tab[0].off, "cmn", 3) == 0){
		what = 2;
	}
	tab = &tab[1];
	cnt = cnt-1;

	u8* buf0 = ptr+tab[0].off;
	int len0 = tab[0].len;
	u8* buf1 = ptr+tab[2].off;
	int len1 = tab[2].len;
	u8* buf2 = ptr+tab[2].off;
	int len2 = tab[2].len;
	u8* buf3 = ptr+tab[3].off;
	int len3 = tab[3].len;
	u8* buf4 = ptr+tab[4].off;
	int len4 = tab[4].len;
	u8* buf5 = ptr+tab[5].off;
	int len5 = tab[5].len;
	u8* buf6 = ptr+tab[6].off;
	int len6 = tab[6].len;
	u8* buf7 = ptr+tab[7].off;
	int len7 = tab[7].len;
	u8* buf8 = ptr+tab[8].off;
	int len8 = tab[8].len;

	int type = -1;
	int subt = -1;
	if(2 == what){
		type = 3;
	}
	else{
		if( ('x'==buf0[0]) && ('x'==buf2[0]) ){
			if( (buf4[0]>='0') && (buf4[0]<='9') )type = 0;
			if('x'==buf4[0])type = 1;
			str2type(buf4, &type, &subt);
		}
		if( ('w'==buf0[0]) && ('w'==buf2[0]) ){
			if( (buf4[0]>='0') && (buf4[0]<='9') )type = 0;
			if('w'==buf4[0])type = 1;
			str2type(buf4, &type, &subt);
		}
	}
	printf("type=%d,subt=%d\n", type, subt);

	u32 code = 0;
	if(type == 0){		//		x0 = x1 + x2
		int d0 = atoi((char*)buf0+1);
		int s0 = atoi((char*)buf2+1);
		int imm = arm64_str2data(buf4);
		printf("imm=%x\n", imm);
		arm64_add_imm_encode((void*)&code, d0, s0, imm, (what==1), ('x'==buf0[0]) );
	}
	if(type == 1){
		int d0 = atoi((char*)buf0+1);
		int s0 = atoi((char*)buf2+1);
		int s1;

		int shdata = 0;
		int shtype = 0;
		if(subt >= 0){		//		x0 = x1 + asr ( x2 , 4 )
			s1 = atoi((char*)buf6+1);
			shtype = subt;
			shdata = arm64_str2data(buf8);
		}
		else{		//		x0 = x1 + x2 << 8
			s1 = atoi((char*)buf4+1);
			if(cnt == 7){
				if('<' == buf5[0])shtype = 0;
				if('>' == buf5[0])shtype = 1;
				shdata = arm64_str2data(buf6);
			}
		}
		arm64_add_regshift_encode((void*)&code, d0, s0, s1, shdata, shtype, (what==1), ('x'==buf0[0]) );
	}
	if(type == 2){		//		x0 = x1 + uxtw ( x2 , 9 )
		int d0 = atoi((char*)buf0+1);
		int s0 = atoi((char*)buf2+1);
		int s1 = atoi((char*)buf6+1);
		int shift = 0;
		if(cnt > 7)shift = arm64_str2data(buf8);
		arm64_add_regext_encode((void*)&code, d0, s0, s1, subt, shift, (what==1), ('x'==buf0[0]) );
	}
	if(type == 3){		//		x3 + x7
		int s0 = atoi((char*)buf0+1);
		int s1 = atoi((char*)buf2+1);
		int is64 = ('x'==buf0[0]);
		arm64_add_regext_encode((void*)&code, 0x1f, s0, s1, 2+is64, 0, 1, is64 );
	}

	disasm_arm64_one(code, 0);
}
void assembly_arm64_sub(u8* ptr, int len, struct offlen* tab, int cnt)
{
	int what = 0;
	if(strncmp((char*)ptr+tab[0].off, "subs", 4) == 0){
		what = 1;
	}
	if(strncmp((char*)ptr+tab[0].off, "cmp", 3) == 0){
		what = 2;
	}
	tab = &tab[1];
	cnt = cnt-1;

	u8* buf0 = ptr+tab[0].off;
	int len0 = tab[0].len;
	u8* buf1 = ptr+tab[2].off;
	int len1 = tab[2].len;
	u8* buf2 = ptr+tab[2].off;
	int len2 = tab[2].len;
	u8* buf3 = ptr+tab[3].off;
	int len3 = tab[3].len;
	u8* buf4 = ptr+tab[4].off;
	int len4 = tab[4].len;
	u8* buf5 = ptr+tab[5].off;
	int len5 = tab[5].len;
	u8* buf6 = ptr+tab[6].off;
	int len6 = tab[6].len;
	u8* buf7 = ptr+tab[7].off;
	int len7 = tab[7].len;
	u8* buf8 = ptr+tab[8].off;
	int len8 = tab[8].len;

	int type = -1;
	int subt = -1;
	if(2 == what){
		type = 3;
	}
	else{
		if( ('x'==buf0[0]) && ('x'==buf2[0]) ){
			if( (buf4[0]>='0') && (buf4[0]<='9') )type = 0;
			if('x'==buf4[0])type = 1;
			str2type(buf4, &type, &subt);
		}
		if( ('w'==buf0[0]) && ('w'==buf2[0]) ){
			if( (buf4[0]>='0') && (buf4[0]<='9') )type = 0;
			if('w'==buf4[0])type = 1;
			str2type(buf4, &type, &subt);
		}
	}
	printf("type=%d,subt=%d\n", type, subt);

	u32 code = 0;
	if(type == 0){		//x0 = x1 + x2
		int d0 = atoi((char*)buf0+1);
		int s0 = atoi((char*)buf2+1);
		int imm = arm64_str2data(buf4);
		printf("imm=%x\n", imm);
		arm64_sub_imm_encode((void*)&code, d0, s0, imm, (what==1), ('x'==buf0[0]) );
	}
	if(type == 1){
		int d0 = atoi((char*)buf0+1);
		int s0 = atoi((char*)buf2+1);
		int s1;

		int shdata = 0;
		int shtype = 0;
		if(subt >= 0){		//x0 = x1 + asr ( x2 , 4 )
			s1 = atoi((char*)buf6+1);
			shtype = subt;
			shdata = arm64_str2data(buf8);
		}
		else{		//		x0 = x1 + x2 << 8
			s1 = atoi((char*)buf4+1);
			if(cnt == 7){
				if('<' == buf5[0])shtype = 0;
				if('>' == buf5[0])shtype = 1;
				shdata = arm64_str2data(buf6);
			}
		}
		arm64_sub_regshift_encode((void*)&code, d0, s0, s1, shdata, shtype, (what==1), ('x'==buf0[0]) );
	}
	if(type == 2){		//x0 = x1 + uxtw ( x2 , 9 )
		int d0 = atoi((char*)buf0+1);
		int s0 = atoi((char*)buf2+1);
		int s1 = atoi((char*)buf6+1);
		int shift = 0;
		if(cnt > 7)shift = arm64_str2data(buf8);
		arm64_sub_regext_encode((void*)&code, d0, s0, s1, subt, shift, (what==1), ('x'==buf0[0]) );
	}
	if(type == 3){		//x3 - x7
		int s0 = atoi((char*)buf0+1);
		int s1 = atoi((char*)buf2+1);
		int is64 = ('x'==buf0[0]);
		arm64_sub_regext_encode((void*)&code, 0x1f, s0, s1, 2+is64, 0, 1, is64 );
	}

	disasm_arm64_one(code, 0);
}
void assembly_compile_arm64(u8* buf, int len, struct offlen* tab, int cnt)
{
	int j;
/*	for(j=0;j<cnt;j++){
		printf("%d: %.*s\n", j, tab[j].len, buf+tab[j].off);
	}
*/
	if( (strncmp((char*)buf+tab[0].off, "mov", 3) == 0) |
		(strncmp((char*)buf+tab[0].off, "orr", 3) == 0) ){
		assembly_arm64_orr(buf, len, tab, cnt);
	}
	if( (strncmp((char*)buf+tab[0].off, "add", 3) == 0) |
		(strncmp((char*)buf+tab[0].off, "adds", 4) == 0) |
		(strncmp((char*)buf+tab[0].off, "cmn", 3) == 0) ){
		assembly_arm64_add(buf, len, tab, cnt);
	}
	if( (strncmp((char*)buf+tab[0].off, "sub", 3) == 0) |
		(strncmp((char*)buf+tab[0].off, "subs", 4) == 0) |
		(strncmp((char*)buf+tab[0].off, "cmp", 3) == 0) ){
		assembly_arm64_sub(buf, len, tab, cnt);
	}
}