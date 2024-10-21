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
	{"adc",  2, AM_Register,    AM_RegOrImm,    1, S_S3,   1,0,0x15},
	{"add",  2, AM_Register,    AM_RegOrImm,    1, S_S3,   1,0,0x14},
	{"and",  2, AM_Register,    AM_RegOrImm,    1, S_S3,   1,0,0x10},
	{"asr",  2, AM_Register,    AM_RegOrImm,    1, S_U3,   1,0,0x1E},
	{"call", 1, AM_RegOrImm,    0,              0, S_NONE, 0,0,0x0F},
	{"cbs",  2, AM_Register,    AM_RegOrImm,    1, S_S3,   1,0,0x0A},
	{"clz",  2, AM_Register,    AM_RegOrImm,    1, S_S3,   1,0,0x08},
	{"cmp",  2, AM_Register,    AM_RegOrImm,    1, S_S3,   1,0,0x06},
	{"ctz",  2, AM_Register,    AM_RegOrImm,    1, S_S3,   1,0,0x09},
	{"db",  -1, AM_Immediate,   0,              1, S_NONE, 0,0,  -1},
	{"dw",  -1, AM_Immediate,   0,              0, S_NONE, 0,0,  -1},
	{"flg",  2, AM_Register,    AM_RegOrImm,    0, S_NONE, 1,0,0x0B},
	{"j",    1, AM_RegOrImm,    0,              0, S_NONE, 0,0,0x0E},
	{"ld",   2, AM_Register,    AM_RegOrImm,    1, S_S3,   1,0,0x00},
	{"ld",   2, AM_Register,    AM_IndRegOrImm, 0, S_NONE, 1,0,0x01},
	{"ld",   2, AM_IndRegister, AM_Register,    0, S_NONE, 1,1,0x02},
	{"ldb",  2, AM_IndRegister, AM_Register,    0, S_NONE, 1,0,0x02},
	{"lsl",  2, AM_Register,    AM_RegOrImm,    1, S_U3,   1,0,0x1D},
	{"lsr",  2, AM_Register,    AM_RegOrImm,    1, S_U3,   1,0,0x1C},
	{"neg",  2, AM_Register,    AM_RegOrImm,    1, S_S3,   1,0,0x03},
	{"not",  2, AM_Register,    AM_RegOrImm,    1, S_S3,   1,0,0x13},
	{"offs", 2, AM_Register,    AM_RegOrImm,    1, S_S3DE, 1,0,0x05},
	{"or",   2, AM_Register,    AM_RegOrImm,    1, S_S3,   1,0,0x11},
	{"pop",  1, AM_Register,    0,              0, S_NONE, 1,0,0x0D},
	{"push", 1, AM_Register,    0,              0, S_NONE, 0,0,0x0C},
	{"ram", -2, 0,              0,              0, S_NONE, 0,0,  -1},
	{"ret",  0, 0,              0,              0, S_NONE, 0,0,0x1F},
	{"rlc",  2, AM_Register,    AM_RegOrImm,    1, S_U3,   1,0,0x1B},
	{"rol",  2, AM_Register,    AM_RegOrImm,    1, S_U3,   1,0,0x19},
	{"ror",  2, AM_Register,    AM_RegOrImm,    1, S_U3,   1,0,0x18},
	{"rrc",  2, AM_Register,    AM_RegOrImm,    1, S_U3,   1,0,0x1A},
	{"sbc",  2, AM_Register,    AM_RegOrImm,    1, S_S3,   1,0,0x17},
	{"sex",  2, AM_Register,    AM_RegOrImm,    1, S_S3,   1,0,0x04},
	{"sub",  2, AM_Register,    AM_RegOrImm,    1, S_S3,   1,0,0x16},
	{"xcg",  2, AM_Register,    AM_Register,    0, S_NONE, 1,0,0x07},
	{"xor",  2, AM_Register,    AM_RegOrImm,    1, S_S3,   1,0,0x12},
};
