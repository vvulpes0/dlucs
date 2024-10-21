#include <stdio.h>

#define l_0      ( 0UL<<21)
#define l_1      ( 1UL<<21)
#define l_pc     ( 2UL<<21)
#define l_alu    ( 4UL<<21)
#define l_mem    ( 5UL<<21)
#define l_i      ( 6UL<<21)
#define l_f      ( 7UL<<21)
#define starA    ( 1UL<<20)
#define incPC    ( 1UL<<19)
#define incA     ((1UL<<18)|(1UL<<17))
#define decA     ( 1UL<<18)
#define swab     ( 1UL<<16)
#define ack      ( 1UL<<15)
#define rs       ( 0UL<<13)
#define rd       ( 2UL<<13)
#define sp       ( 1UL<<13)
#define cplA     ( 1UL<<12)
#define cplB     ( 1UL<<11)
#define cplD     ( 1UL<<10)
#define nand     (l_alu | ( 0UL<< 6))
#define adc      (l_alu | ( 1UL<< 6))
#define add      (l_alu | ( 2UL<< 6))
#define addp1    (l_alu | ( 3UL<< 6))
#define xor      (l_alu | ( 4UL<< 6))
#define lsl      (l_alu | ( 5UL<< 6))
#define lsr      (l_alu | ( 6UL<< 6))
#define asr      (l_alu | ( 7UL<< 6))
#define rol      (l_alu | ( 8UL<< 6))
#define rlc      (l_alu | ( 9UL<< 6))
#define ror      (l_alu | (10UL<< 6))
#define rrc      (l_alu | (11UL<< 6))
#define cbs      (l_alu | (12UL<< 6))
#define fd       (l_alu | (13UL<< 6))
#define rev      (l_alu | (14UL<< 6))
#define s3bf     (l_alu | (15UL<< 6))
#define sub      (addp1|     cplB)
#define sbc      (adc  |     cplB)
#define and      (nand |          cplD)
#define or       (nand |cplA|cplB)
#define nor      (nand |cplA|cplB|cplD)
#define s_pc     ( 1UL<< 5)
#define s_mem    ( 1UL<< 4)
#define s_a      ( 1UL<< 3)
#define s_b      ( 1UL<< 2)
#define s_f      ( 1UL    )

#define l_rs     (( 3UL<<21) | rs)
#define l_rd     (( 3UL<<21) | rd)
#define l_sp     (( 3UL<<21) | sp)
#define s_rs     (( 1UL<< 1) | rs)
#define s_rd     (( 1UL<< 1) | rd)
#define s_sp     (( 1UL<< 1) | sp)

#define uop0  (incPC | l_sp | s_a)
#define adc1  (incPC)
#define add1  (incPC)
#define and1  (incPC)
#define asr1  (incPC | l_1 | s_a | s_b)
#define call1 (incPC | l_0 | s_b | decA)
#define cbs1  (incPC)
#define clz1  (incPC)
#define cmp1  (incPC)
#define ctz1  (incPC)
#define flg1  (incPC)
#define j1    (incPC)
#define ld1   (incPC)
#define ldi1  (incPC)
#define lsl1  (incPC | l_1 | s_a | s_b)
#define lsr1  (incPC | l_1 | s_a | s_b)
#define neg1  (incPC)
#define not1  (incPC)
#define offs1 (incPC)
#define or1   (incPC)
#define pop1  (incPC)
#define push1 (incPC | l_0 | s_b | decA)
#define ret1  (incPC)
#define rlc1  (incPC | l_1 | s_a | s_b)
#define rol1  (incPC | l_1 | s_a | s_b)
#define ror1  (incPC | l_1 | s_a | s_b)
#define rrc1  (incPC | l_1 | s_a | s_b)
#define sbc1  (incPC)
#define sex1  (incPC | l_1 | s_a | s_b)
#define sti1  (incPC)
#define sub1  (incPC)
#define xcg1  (incPC)
#define xor1  (incPC)

#define nomode (l_sp|s_a), \
               (l_0|s_b|decA), \
               (l_pc|s_mem|starA|swab|decA), \
               (l_pc|s_mem|starA), \
               (or|s_sp), \
               (l_1|s_a|s_b), \
               (lsl|s_a|s_b), \
               (lsl|s_pc|ack)

static unsigned long ins_adc[32] = {
	/* ADC :: Reg-Reg */
	uop0,adc1,
	l_rs  | s_b,
	l_rd  | s_a,
	adc   | s_rd | s_f,
	0UL,
	0UL,
	0UL,
	/* ADC :: Short Immediate */
	uop0,adc1,
	l_i   | s_a,
	s3bf  | s_b,
	l_rd  | s_a,
	adc   | s_rd | s_f,
	0UL,
	0UL,
	/* ADC :: Long Immediate */
	uop0,adc1,
	l_mem | s_b   | incPC,
	l_mem | s_a   | incPC | swab,
	or    | s_b,
	l_rd  | s_a,
	adc   | s_rd | s_f,
	0UL,

	nomode,
};

static unsigned long ins_add[32] = {
	/* ADD :: Reg-Reg */
	uop0,add1,
	l_rs  | s_b,
	l_rd  | s_a,
	add   | s_rd | s_f,
	0UL,
	0UL,
	0UL,
	/* ADD :: Short Immediate */
	uop0,add1,
	l_i   | s_a,
	s3bf  | s_b,
	l_rd  | s_a,
	add   | s_rd | s_f,
	0UL,
	0UL,
	/* ADD :: Long Immediate */
	uop0,add1,
	l_mem | s_b   | incPC,
	l_mem | s_a   | incPC | swab,
	or    | s_b,
	l_rd  | s_a,
	add   | s_rd | s_f,
	0UL,

	nomode,
};

static unsigned long ins_and[32] = {
	/* AND :: Reg-Reg */
	uop0,and1,
	l_rs  | s_b,
	l_rd  | s_a,
	and   | s_rd | s_f,
	0UL,
	0UL,
	0UL,
	/* AND :: Short Immediate */
	uop0,and1,
	l_i   | s_a,
	s3bf  | s_b,
	l_rd  | s_a,
	and   | s_rd | s_f,
	0UL,
	0UL,
	/* AND :: Long Immediate */
	uop0,and1,
	l_mem | s_b   | incPC,
	l_mem | s_a   | incPC | swab,
	or    | s_b,
	l_rd  | s_a,
	and   | s_rd | s_f,
	0UL,

	nomode,
};

static unsigned long ins_asr[32] = {
	/* ASR :: Reg-Reg */
	uop0,asr1,
	l_rs  | s_b,
	l_rd  | s_a,
	asr   | s_rd | s_f,
	0UL,
	0UL,
	0UL,
	/* ASR :: Short Immediate */
	uop0,asr1,
	addp1 | s_a | s_b,
	addp1 | s_b,
	l_i   | s_a,
	and   | s_b,
	l_rd  | s_a,
	asr   | s_rd | s_f,
	/* ASR :: Long Immediate */
	uop0,asr1,
	l_mem | s_b | incPC,
	l_mem | s_a | incPC | swab,
	or    | s_b,
	l_rd  | s_a,
	asr   | s_rd | s_f,
	0UL,

	nomode,
};

static unsigned long ins_call[32] = {
	/* CALL :: Register */
	uop0,call1,
	l_pc | s_mem | starA | swab | decA,
	l_pc | s_mem | starA,
	or   | s_sp,
	l_rs | s_pc,
	0UL,
	0UL,
	/* CALL :: Short Immediate */
	nomode,
	/* CALL :: Long Immediate */
	uop0,call1,
	l_pc  | s_mem | starA | swab | decA,
	l_pc  | s_mem | starA,
	or    | s_sp,
	l_mem | s_a   | incPC,
	l_mem | s_b   | incPC | swab,
	or    | s_pc,

	nomode,
};

static unsigned long ins_cbs[32] = {
	/* CBS :: Reg-Reg */
	uop0,cbs1,
	l_rs | s_a,
	cbs  | s_rd | s_f,
	0UL,
	0UL,
	0UL,
	0UL,
	/* CBS :: Short Immediate */
	uop0,cbs1,
	l_i  | s_a,
	s3bf | s_a,
	cbs  | s_rd | s_f,
	0UL,
	0UL,
	0UL,
	/* CBS :: Long Immediate */
	uop0,cbs1,
	l_mem | s_b  | incPC,
	l_mem | s_a  | incPC | swab,
	or    | s_a,
	cbs   | s_rd | s_f,
	0UL,
	0UL,

	nomode,
};

static unsigned long ins_clz[32] = {
	/* CLZ :: Reg-Reg */
	uop0,clz1,
	l_rs | s_a,
	fd   | s_a  | cplD,
	cbs  | s_rd | s_f,
	0UL,
	0UL,
	0UL,
	/* CLZ :: Short Immediate */
	uop0,clz1,
	l_i  | s_a,
	s3bf | s_a,
	fd   | s_a  | cplD,
	cbs  | s_rd | s_f,
	0UL,
	0UL,
	/* CLZ :: Long Immediate */
	uop0,clz1,
	l_mem | s_b  | incPC,
	l_mem | s_a  | incPC | swab,
	or    | s_a,
	fd    | s_a  | cplD,
	cbs   | s_rd | s_f,
	0UL,

	nomode,
};

static unsigned long ins_cmp[32] = {
	/* CMP :: Reg-Reg */
	uop0,cmp1,
	l_rs  | s_b,
	l_rd  | s_a,
	sub   | s_f,
	0UL,
	0UL,
	0UL,
	/* CMP :: Short Immediate */
	uop0,cmp1,
	l_i   | s_a,
	s3bf  | s_b,
	l_rd  | s_a,
	sub   | s_f,
	0UL,
	0UL,
	/* CMP :: Long Immediate */
	uop0,cmp1,
	l_mem | s_b | incPC,
	l_mem | s_a | incPC | swab,
	or    | s_b,
	l_rd  | s_a,
	sub   | s_f,
	0UL,

	nomode,
};

static unsigned long ins_ctz[32] = {
	/* CTZ :: Reg-Reg */
	uop0,ctz1,
	l_rs | s_a,
	rev  | s_a,
	fd   | s_a  | cplD,
	cbs  | s_rd | s_f,
	0UL,
	0UL,
	/* CTZ :: Short Immediate */
	uop0,ctz1,
	l_i  | s_a,
	s3bf | s_a,
	rev  | s_a,
	fd   | s_a  | cplD,
	cbs  | s_rd | s_f,
	0UL,
	/* CTZ :: Long Immediate */
	uop0,ctz1,
	l_mem | s_b  | incPC,
	l_mem | s_a  | incPC | swab,
	or    | s_a,
	rev   | s_a,
	fd    | s_a  | cplD,
	cbs   | s_rd | s_f,

	nomode,
};

static unsigned long ins_flg[32] = {
	/* FLG :: Reg-Reg */
	uop0,flg1,
	l_rs | s_b,
	l_f  | s_a,
	rev  | s_a,
	rev  | s_rd,
	s3bf | s_f,
	0UL,
	/* FLG :: Short Immediate */
	nomode,
	/* FLG :: Long Immediate */
	uop0,flg1,
	l_mem | s_b | incPC,
	l_f   | s_a | incPC,
	rev   | s_a,
	rev   | s_rd,
	s3bf  | s_f,
	0UL,

	nomode,
};

static unsigned long ins_j[32] = {
	/* J :: Register */
	uop0,j1,
	l_rs | s_pc,
	0UL,
	0UL,
	0UL,
	0UL,
	0UL,
	/* J :: Short Immediate */
	nomode,
	/* J :: Long Immediate */
	uop0,j1,
	l_mem | s_a | incPC,
	l_mem | s_b | incPC | swab,
	or    | s_pc,
	0UL,
	0UL,
	0UL,

	nomode,
};

static unsigned long ins_ld[32] = {
	/* LD :: Reg-Reg */
	uop0,ld1,
	l_rs | s_a | s_b,
	or   | s_rd,
	0UL,
	0UL,
	0UL,
	0UL,
	/* LD :: Short Immediate */
	uop0,ld1,
	l_i  | s_a,
	s3bf | s_rd,
	0UL,
	0UL,
	0UL,
	0UL,
	/* LD :: Long Immediate */
	uop0,ld1,
	l_mem | s_b | incPC,
	l_mem | s_a | incPC | swab,
	or    | s_rd,
	0UL,
	0UL,
	0UL,

	nomode,
};

static unsigned long ins_ldi[32] = {
	/* LDI :: Reg-Reg */
	uop0,ldi1,
	l_rs  | s_a,
	l_mem | s_b | starA | incA,
	l_mem | s_a | starA | swab,
	or    | s_rd,
	0UL,
	0UL,
	/* LDI :: Short Immediate */
	nomode,
	/* LDI :: Long Immediate */
	uop0,ldi1,
	l_mem | s_b | incPC,
	l_mem | s_a | incPC | swab,
	or    | s_a,
	l_mem | s_b | starA | incA,
	l_mem | s_a | starA | swab,
	or    | s_rd,

	nomode,
};

static unsigned long ins_lsl[32] = {
	/* LSL :: Reg-Reg */
	uop0,lsl1,
	l_rs | s_b,
	l_rd | s_a,
	lsl  | s_rd | s_f,
	0UL,
	0UL,
	0UL,
	/* LSL :: Short Immediate */
	uop0,lsl1,
	addp1 | s_a | s_b,
	addp1 | s_b,
	l_i   | s_a,
	and   | s_b,
	l_rd  | s_a,
	lsl   | s_rd | s_f,
	/* LSL :: Long Immediate */
	uop0,lsl1,
	l_mem | s_b | incPC,
	l_mem | s_a | incPC | swab,
	or    | s_b,
	l_rd  | s_a,
	lsl   | s_rd | s_f,
	0UL,

	nomode,
};

static unsigned long ins_lsr[32] = {
	/* LSR :: Reg-Reg */
	uop0,lsr1,
	l_rs | s_b,
	l_rd | s_a,
	lsr  | s_rd | s_f,
	0UL,
	0UL,
	0UL,
	/* LSR :: Short Immediate */
	uop0,lsr1,
	addp1 | s_a | s_b,
	addp1 | s_b,
	l_i   | s_a,
	and   | s_b,
	l_rd  | s_a,
	lsr   | s_rd | s_f,
	/* LSR :: Long Immediate */
	uop0,lsr1,
	l_mem | s_b | incPC,
	l_mem | s_a | incPC | swab,
	or    | s_b,
	l_rd  | s_a,
	lsr   | s_rd | s_f,
	0UL,

	nomode,
};

static unsigned long ins_neg[32] = {
	/* NEG :: Reg-Reg */
	uop0,neg1,
	l_rs | s_b,
	l_0  | s_a,
	sub  | s_rd | s_f,
	0UL,
	0UL,
	0UL,
	/* NEG :: Short Immediate */
	uop0,neg1,
	l_i  | s_a,
	s3bf | s_b,
	l_0  | s_a,
	sub  | s_rd | s_f,
	0UL,
	0UL,
	/* NEG :: Long Immediate */
	uop0,neg1,
	l_mem | s_b | incPC,
	l_mem | s_a | incPC | swab,
	or    | s_b,
	l_0   | s_a,
	sub   | s_rd | s_f,
	0UL,

	nomode,
};

static unsigned long ins_not[32] = {
	/* NOT :: Reg-Reg */
	uop0,not1,
	l_rs | s_a  | s_b,
	nor  | s_rd | s_f,
	0UL,
	0UL,
	0UL,
	0UL,
	/* NOT :: Short Immediate */
	uop0,not1,
	l_i  | s_a,
	s3bf | s_a  | s_b,
	nor  | s_rd | s_f,
	0UL,
	0UL,
	0UL,
	/* NOT :: Long Immediate */
	uop0,not1,
	l_mem | s_b  | incPC,
	l_mem | s_a  | incPC | swab,
	nor   | s_rd | s_f,
	0UL,
	0UL,
	0UL,

	nomode,
};

static unsigned long ins_offs[32] = {
	/* OFFS :: Reg-Reg */
	uop0,offs1,
	l_rs | s_a,
	l_pc | s_b,
	add  | s_rd,
	0UL,
	0UL,
	0UL,
	/* OFFS :: Short Immediate */
	uop0,offs1,
	l_i  | s_a,
	s3bf | s_a | s_b,
	add  | s_a,
	l_pc | s_b,
	add  | s_rd,
	0UL,
	/* OFFS :: Long Immediate */
	uop0,offs1,
	l_mem | s_b | incPC,
	l_mem | s_a | incPC | swab,
	or    | s_a,
	l_pc  | s_b,
	add   | s_rd,
	0UL,

	nomode,
};

static unsigned long ins_or[32] = {
	/* OR :: Reg-Reg */
	uop0,or1,
	l_rs | s_b,
	l_rd | s_a,
	or   | s_rd | s_f,
	0UL,
	0UL,
	0UL,
	/* OR :: Short Immediate */
	uop0,or1,
	l_i  | s_a,
	s3bf | s_b,
	l_rd | s_a,
	or   | s_rd | s_f,
	0UL,
	0UL,
	/* OR :: Long Immediate */
	uop0,or1,
	l_mem | s_b  | incPC,
	l_mem | s_a  | incPC | swab,
	or    | s_b,
	l_rd  | s_a,
	or    | s_rd | s_f,
	0UL,

	nomode,
};

static unsigned long ins_pop[32] = {
	/* POP :: Register */
	uop0,pop1,
	l_mem | s_b  | starA | incA,
	l_mem | s_a  | starA | swab,
	or    | s_rd,
	l_sp  | s_a,
	l_1   | s_b  | incA,
	add   | s_sp,
	/* POP :: Short Immediate */
	nomode,
	/* POP :: Long Immediate */
	nomode,

	nomode,
};

static unsigned long ins_push[32] = {
	/* PUSH :: Reg-Reg */
	uop0,push1,
	l_rs | s_mem | starA | swab | decA,
	l_rs | s_mem | starA,
	or   | s_sp,
	0UL,
	0UL,
	0UL,
	/* PUSH :: Short Immediate */
	nomode,
	/* PUSH :: Long Immediate */
	nomode,

	nomode,
};

static unsigned long ins_ret[32] = {
	/* RET :: Zero-Address */
	uop0,ret1,
	l_mem | s_b  | starA | incA,
	l_mem | s_a  | starA | swab,
	or    | s_pc,
	l_sp  | s_a,
	l_1   | s_b  | incA,
	add   | s_sp,
	/* RET :: Short Immediate */
	nomode,
	/* RET :: Long Immediate */
	nomode,

	nomode,
};

static unsigned long ins_rlc[32] = {
	/* RLC :: Reg-Reg */
	uop0,rlc1,
	l_rs | s_b,
	l_rd | s_a,
	rlc  | s_rd | s_f,
	0UL,
	0UL,
	0UL,
	/* RLC :: Short Immediate */
	uop0,rlc1,
	addp1 | s_a  | s_b,
	addp1 | s_b,
	l_i   | s_a,
	and   | s_b,
	l_rd  | s_a,
	rlc   | s_rd | s_f,
	/* RLC :: Long Immediate */
	uop0,rlc1,
	l_mem | s_b  | incPC,
	l_mem | s_a  | incPC | swab,
	or    | s_b,
	l_rd  | s_a,
	rlc   | s_rd | s_f,
	0UL,

	nomode,
};

static unsigned long ins_rol[32] = {
	/* ROL :: Reg-Reg */
	uop0,rol1,
	l_rs | s_b,
	l_rd | s_a,
	rol  | s_rd | s_f,
	0UL,
	0UL,
	0UL,
	/* ROL :: Short Immediate */
	uop0,rol1,
	addp1 | s_a | s_b,
	addp1 | s_b,
	l_i   | s_a,
	and   | s_b,
	l_rd  | s_a,
	rol   | s_rd | s_f,
	/* ROL :: Long Immediate */
	uop0,rol1,
	l_mem | s_b  | incPC,
	l_mem | s_a  | incPC | swab,
	or    | s_b,
	l_rd  | s_a,
	rol   | s_rd | s_f,
	0UL,

	nomode,
};

static unsigned long ins_ror[32] = {
	/* ROR :: Reg-Reg */
	uop0,ror1,
	l_rs | s_b,
	l_rd | s_a,
	ror  | s_rd | s_f,
	0UL,
	0UL,
	0UL,
	/* ROR :: Short Immediate */
	uop0,ror1,
	addp1 | s_a  | s_b,
	addp1 | s_b,
	l_i   | s_a,
	and   | s_b,
	l_rd  | s_a,
	ror   | s_rd | s_f,
	/* ROR :: Long Immediate */
	uop0,ror1,
	l_mem | s_b  | incPC,
	l_mem | s_a  | incPC | swab,
	or    | s_b,
	l_rd  | s_a,
	ror   | s_rd | s_f,
	0UL,

	nomode,
};

static unsigned long ins_rrc[32] = {
	/* RRC :: Reg-Reg */
	uop0,rrc1,
	l_rs | s_b,
	l_rd | s_a,
	rrc  | s_rd | s_f,
	0UL,
	0UL,
	0UL,
	/* RRC :: Short Immediate */
	uop0,rrc1,
	addp1 | s_a | s_b,
	addp1 | s_b,
	l_i   | s_a,
	and   | s_b,
	l_rd  | s_a,
	rrc   | s_rd | s_f,
	/* RRC :: Long Immediate */
	uop0,rrc1,
	l_mem | s_b  | incPC,
	l_mem | s_a  | incPC | swab,
	or    | s_b,
	l_rd  | s_a,
	rrc   | s_rd | s_f,
	0UL,

	nomode,
};

static unsigned long ins_sbc[32] = {
	/* SBC :: Reg-Reg */
	uop0,sbc1,
	l_rs | s_b,
	l_rd | s_a,
	sbc  | s_rd | s_f,
	0UL,
	0UL,
	0UL,
	/* SBC :: Short Immediate */
	uop0,sbc1,
	l_i  | s_a,
	s3bf | s_b,
	l_rd | s_a,
	sbc  | s_rd | s_f,
	0UL,
	0UL,
	/* SBC :: Long Immediate */
	uop0,sbc1,
	l_mem | s_b  | incPC,
	l_mem | s_a  | incPC | swab,
	or    | s_b,
	l_rd  | s_a,
	sbc   | s_rd | s_f,
	0UL,

	nomode,
};

static unsigned long ins_sex[32] = {
	/* SEX :: Reg-Reg */
	uop0,sex1,
	lsl  | s_a  | s_b,
	lsl  | s_b,
	l_rs | s_a,
	lsl  | s_a,
	asr  | s_rd | s_f,
	0UL,
	/* SEX :: Short Immediate */
	uop0,sex1,
	l_i  | s_a,
	s3bf | s_a,
	l_0  | s_b,
	asr  | s_rd | s_f,
	0UL,
	0UL,
	/* SEX :: Long Immediate */
	uop0,sex1,
	lsl   | s_a  | s_b,
	lsl   | s_b,
	l_mem | s_a  | incPC,
	lsl   | s_a  | incPC,
	asr   | s_rd | s_f,
	0UL,

	nomode,
};

static unsigned long ins_sti[32] = {
	/* STI :: Byte (Register slot) */
	uop0,sti1,
	l_rd | s_a,
	l_rs | s_mem | starA | incA,
	0UL,
	0UL,
	0UL,
	0UL,
	/* STI :: Word (Short Immediate slot) */
	uop0,sti1,
	l_rd | s_a,
	l_rs | s_mem | starA | incA,
	l_rs | s_mem | starA | swab,
	0UL,
	0UL,
	0UL,
	/* STI :: Word (Long Immediate slot) */
	uop0,sti1,
	l_rd | s_a,
	l_rs | s_mem | starA | incA,
	l_rs | s_mem | starA | swab,
	0UL,
	0UL,
	0UL,

	nomode,
};

static unsigned long ins_sub[32] = {
	/* SUB :: Reg-Reg */
	uop0,sub1,
	l_rs | s_b,
	l_rd | s_a,
	sub  | s_rd | s_f,
	0UL,
	0UL,
	0UL,
	/* SUB :: Short Immediate */
	uop0,sub1,
	l_i  | s_a,
	s3bf | s_b,
	l_rd | s_a,
	sub  | s_rd | s_f,
	0UL,
	0UL,
	/* SUB :: Long Immediate */
	uop0,sub1,
	l_mem | s_b  | incPC,
	l_mem | s_a  | incPC | swab,
	or    | s_b,
	l_rd  | s_a,
	sub   | s_rd | s_f,
	0UL,

	nomode,
};

static unsigned long ins_xcg[32] = {
	/* XCG :: Reg-Reg */
	uop0,xcg1,
	l_rd | s_a,
	l_rs | s_b,
	xor  | s_a,
	xor  | s_rs | s_b,
	xor  | s_rd,
	0UL,
	/* XCG :: Short Immediate */
	nomode,
	/* XCG :: Long Immediate */
	nomode,

	nomode,
};

static unsigned long ins_xor[32] = {
	/* XOR :: Reg-Reg */
	uop0,xor1,
	l_rs | s_b,
	l_rd | s_a,
	xor  | s_rd | s_f,
	0UL,
	0UL,
	0UL,
	/* XOR :: Short Immediate */
	uop0,xor1,
	l_i  | s_a,
	s3bf | s_b,
	l_rd | s_a,
	xor  | s_rd | s_f,
	0UL,
	0UL,
	/* XOR :: Long Immediate */
	uop0,xor1,
	l_mem | s_b | incPC,
	l_mem | s_a | incPC | swab,
	or    | s_b,
	l_rd  | s_a,
	xor   | s_rd | s_f,
	0UL,

	nomode,
};

static unsigned long *ops[] = {
	ins_ld,   ins_ldi,  ins_sti, ins_neg,
	ins_sex,  ins_offs, ins_cmp, ins_xcg,
	ins_clz,  ins_ctz,  ins_cbs, ins_flg,
	ins_push, ins_pop,  ins_j,   ins_call,
	ins_and,  ins_or,   ins_xor, ins_not,
	ins_add,  ins_adc,  ins_sub, ins_sbc,
	ins_ror,  ins_rol,  ins_rrc, ins_rlc,
	ins_lsr,  ins_lsl,  ins_asr, ins_ret,
};

int
main(void)
{
	size_t i;
	size_t j;
	size_t addr = 0;
	printf("v3.0 hex words addressed");
	for (i = 0; i < sizeof(ops)/sizeof(*ops); ++i) {
		for (j = 0; j < sizeof(ins_ld)/sizeof(*ins_ld); ++j) {
			if (j%8 == 0) printf("\n%03zx:",addr);
			printf(" %06lx",ops[i][j]);
			++addr;
		}
	}
	printf("\n");
	return 0;
}
