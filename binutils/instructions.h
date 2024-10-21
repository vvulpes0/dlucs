enum ShortType {
	S_S3,   /* signed 3-bit [-3,+3] */
	S_U3,   /* unsigned 3-bit [1,7] */
	S_S3DE, /* 2*[-3,+3]: signed 3-bit, doubled (even) */
	S_NONE, /* unavailable: cannot accept short immediates */
};

enum AddrMode {
	AM_Register     =  1,
	AM_Immediate    =  2,
	AM_RegOrImm     =  3,
	AM_IndRegister  =  4,
	AM_IndImmediate =  8, 
	AM_IndRegOrImm  = 12,
};

struct InsMeta {
	char const * name;
	int narg;
	enum AddrMode op1mode;
	enum AddrMode op2mode;
	_Bool shortable;
	int short_type;
	int needs_dest;
	int force_mode;
	int opcode;
};

static struct InsMeta instrs[] = {
	{"adc",  2, AM_Register,    AM_RegOrImm,    1, S_S3,   1,0,0x00},
	{"add",  2, AM_Register,    AM_RegOrImm,    1, S_S3,   1,0,0x01},
	{"and",  2, AM_Register,    AM_RegOrImm,    1, S_S3,   1,0,0x02},
	{"asr",  2, AM_Register,    AM_RegOrImm,    1, S_U3,   1,0,0x03},
	{"call", 1, AM_RegOrImm,    0,              0, S_NONE, 0,0,0x04},
	{"cbs",  2, AM_Register,    AM_RegOrImm,    1, S_S3,   1,0,0x05},
	{"clz",  2, AM_Register,    AM_RegOrImm,    1, S_S3,   1,0,0x06},
	{"cmp",  2, AM_Register,    AM_RegOrImm,    1, S_S3,   1,0,0x07},
	{"ctz",  2, AM_Register,    AM_RegOrImm,    1, S_S3,   1,0,0x08},
	{"db",  -1, AM_Immediate,   0,              1, S_NONE, 0,0,  -1},
	{"dw",  -1, AM_Immediate,   0,              0, S_NONE, 0,0,  -1},
	{"flg",  2, AM_Register,    AM_RegOrImm,    0, S_NONE, 1,0,0x09},
	{"j",    1, AM_RegOrImm,    0,              0, S_NONE, 0,0,0x0A},
	{"ld",   2, AM_Register,    AM_RegOrImm,    1, S_S3,   1,0,0x0B},
	{"ld",   2, AM_Register,    AM_IndRegOrImm, 0, S_NONE, 1,0,0x0C},
	{"ld",   2, AM_IndRegister, AM_Register,    0, S_NONE, 1,1,0x1C},
	{"ldb",  2, AM_IndRegister, AM_Register,    0, S_NONE, 1,0,0x1C},
	{"lsl",  2, AM_Register,    AM_RegOrImm,    1, S_U3,   1,0,0x0D},
	{"lsr",  2, AM_Register,    AM_RegOrImm,    1, S_U3,   1,0,0x0E},
	{"neg",  2, AM_Register,    AM_RegOrImm,    1, S_S3,   1,0,0x0F},
	{"not",  2, AM_Register,    AM_RegOrImm,    1, S_S3,   1,0,0x10},
	{"offs", 2, AM_Register,    AM_RegOrImm,    1, S_S3DE, 1,0,0x11},
	{"or",   2, AM_Register,    AM_RegOrImm,    1, S_S3,   1,0,0x12},
	{"pop",  1, AM_Register,    0,              0, S_NONE, 1,0,0x13},
	{"push", 1, AM_Register,    0,              0, S_NONE, 0,0,0x14},
	{"ram", -2, 0,              0,              0, S_NONE, 0,0,   0},
	{"ret",  0, 0,              0,              0, S_NONE, 0,0,0x15},
	{"rlc",  2, AM_Register,    AM_RegOrImm,    1, S_U3,   1,0,0x16},
	{"rol",  2, AM_Register,    AM_RegOrImm,    1, S_U3,   1,0,0x17},
	{"ror",  2, AM_Register,    AM_RegOrImm,    1, S_U3,   1,0,0x18},
	{"rrc",  2, AM_Register,    AM_RegOrImm,    1, S_U3,   1,0,0x19},
	{"sbc",  2, AM_Register,    AM_RegOrImm,    1, S_S3,   1,0,0x1A},
	{"sex",  2, AM_Register,    AM_RegOrImm,    1, S_S3,   1,0,0x1B},
	{"sub",  2, AM_Register,    AM_RegOrImm,    1, S_S3,   1,0,0x1D},
	{"xcg",  2, AM_Register,    AM_Register,    0, S_NONE, 1,0,0x1E},
	{"xor",  2, AM_Register,    AM_RegOrImm,    1, S_S3,   1,0,0x1F},
};
