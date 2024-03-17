// mads++.cpp : Defines the entry point for the application.
//

// ReSharper disable CppDefaultCaseNotHandledInSwitchStatement
#include "mads++.h"

#include "color.hpp"	// https://github.com/aafulei/color-console


using namespace std;

// Define the variables

// File access
ofstream	dotListFile;		// Assembler .lst file output
ofstream	dotLabFile;			// Assembler .lab file output
ofstream	dotHFile;			// Assembler .h file output -hc[:filename] parameter CC65 header file
ofstream	dotMEAFile;			// Assembler .mea file output -hm[:filename] parameter MADS header file
ofstream	dotObjectFile;		// where the assembled data is written to

char label_type = 'V';			// What type is the label: Procedure, Variable, Constant

BYTE		pass;				// Which assembly pass are we currently on
BYTE		status;				// Final assembly status: 0 = ok, 1 = warnings, 2 = stop on error
BYTE		memType;
BYTE		optDefault;			// Assembler options to restore on every pass

BYTE		opt = (BYTE)(opt_H | opt_O);		// OPT default value
t_Attrib	atr = __RW;							// ATTRIBUTE default value Read/Write
t_Attrib	atrDefault = __U;

const string emptyString = {};

BYTE		asize = (BYTE)8;
BYTE		isize = (BYTE)8;
BYTE		longa;
BYTE		longi;

BYTE		margin = (BYTE)32;

BYTE		obxFillValue = (BYTE)0xFF;

unsigned int __link_stack_pointer_old, __link_stack_address_old, __link_proc_vars_adr_old;
unsigned int __link_stack_pointer, __link_stack_address, __link_proc_vars_adr;

int bank, blok, proc_lokal, fill, proc_idx, anonymous_idx;
int whi_idx, while_nr, ora_nr, test_nr, test_idx, sym_idx, org_ofset;
int hea_i, rel_ofs, wyw_idx, array_idx, buf_i, rept_cnt, skip_idx;
int proc_nr, lc_nr, local_nr, rel_idx, smb_idx, var_id, usi_idx;
int line, line_err, line_all, line_add, _rept_ile, ext_idx, extn_idx;
int pag_idx, end_idx, pub_idx, var_idx, ifelse, ds_empty;

int segment = 0;
int size_idx = 1;
int addres = -0xFFFF;
int zpvar = 0x80;

int5 nul;

string	while_name, test_name, lst_string, lst_header, etyArray;
string	path, name, t, global_name, proc_name, def_label;
string	end_string;
string	warning_mes;
string	filenameH;
string	filenameHEA;
string	filenameLST;
string	filenameOBX;	// Filename for the .OBX output.  The binary file of the assembler!
string	filenameLAB;	// Filename for the .LAB output
string	filenameMAC;
string	filenameASM;
string	macro_nr, local_name, warning_old;

infiniteEntry	infinite = {};
runIniEntry		runini = {};
attributeEntry	attribute = {};
regOptiConfig	registerOptimisation = {};
structureConfig	structure = { false, false, false, 0, 0, 0, -1 };
tStructUsed		struct_used = {};
tArrayUsed		array_used = {};
tExtUsed		ext_used = {};
tDotReloc		dotRELOC = {};
tDotLink		dotLINK = {};
tBulkUpdatePublic	bulkUpdatePublic = {};

tBinaryFile		binary_file = {};
tRaw			raw = {};
tHeaOffset		hea_offset = {};
tEnumeration	enumeration = {};

bool defaultZero = false;		// default to zeros
bool xasmStyle = false;			// combining commands through ':'
bool ReadEnum = false;			// for reading labels @dma(narrow|dma)
bool VerifyProc = false;
bool ExProcTest = false;		// required to eliminate 'Unreferenced procedures'
bool NoAllocVar = false;		// hold off on allocating .VAR variables
bool code6502 = false;
bool unused_label = false;
bool regAXY_opty = false;		// OPT R+- registry optimization  MW?,MV?
bool mae_labels = false;		// OPT ?+- MAE ?labels
bool undeclared = false;
bool variable = false;
bool klamra_used = false;
bool noWarning = false;
bool lst_off = false;
bool macro = false;
bool labFirstCol = false;
bool test_symbols = false;
bool overflow = false;
bool block_record = false;
bool FOX_ripit = false;
bool blocked = false;
bool rel_used = false;
bool put_used = false;
bool exclude_proc = false;
bool mne_used = false;
bool data_out = false;
bool aray = false;
bool dta_used = false;
bool pisz = false;
bool rept = false;
bool rept_run = false;
bool empty = false;
bool reloc = false;
bool branch = false;
bool isVector = false;
bool silent = false;
bool bez_lst = false;
bool icl_used = false;
bool isComment = false;
bool case_used = false;
bool full_name = false;
bool proc = false;
bool run_macro = false;
bool loop_used = false;
bool org = false;
bool over = false;
bool open_ok = false;
bool list_lab = false;		// List defined labels in a .lab file
bool list_dotH = false;
bool list_dotMEA = false;
bool list_mac = false;
bool next_pass = false;
bool BranchTest = false;

bool first_lst = false;
bool first_org = true;
bool if_test = true;
bool hea = true;

bool TestWhileOpt = true;

bool skip_xsm = false;
bool skip_use = false;
bool skip_hlt = false;

bool macroCmd = false;


// t256i imes;			// array with indexes to messages (proc INITIALIZE)
t256i tCRC16;			// table for CRC codes 16           (proc INITIALIZE)
t256c tCRC32;			// table for CRC 32 codes           (proc INITIALIZE)


_strArray		reptPar;	// global array for parameters passed to .REPT

_bckAdr						t_bck;		// a variable storing 2 addresses back
m64kb						hash;		// hashed mnemonics and addressing modes
m64kb						t_get;		// buffer for .GET and .PUT directives
m64kb						t_lnk;		// buffer for the .LINK directive
c64kb						t_tmp;		// buffer for data entered into .ARRAY
m64kb						t_ins;		// buffer for reading files via INS, maximum file length is 64KB
m4kb						t_buf;		// memory buffer for writing
t256b						t_zpv;		// 256 tags for variables placed on page zero
_intArray					t_hea;		// remember the block length
_strArray					t_lin;		// stores listing lines joined by '\' (broken into lines)
_strArray					t_mac;		// remember lines with macros, procedures, .REPT loops
_strArray					t_par;		// remember the parameter names of the .PROC procedures
_strArray					t_pth;		// remember the search paths for INS and ICL
_strArray					t_sym;		// remember new symbols for SDX (blk update new i_settd 'i_settd')
_bolArray					t_els;		// remember occurrences of #ELSE in #IF blocks
vector<forwardEntry>		t_skp;		// array storing the forward address
vector<segInfo>				t_seg;		// remember the segments
vector<localAreasEntry>		t_loc;		// remember the names of the .LOCAL areas
vector<messageEntry>		messages;	// MESSAGES - error and warning messages
vector<pubTab>				t_pub;		// remember the names of the .PUBLIC labels
vector<variableEntry>		t_var;		// remember the names of the .VAR labels
vector<endEntry>			t_end;		// remember the order in which the .MACRO, .PROC, .LOCAL, .STRUCT, .ARRAY etc. directives are called.
vector<argumentSizeEntry>	t_siz;		// remember the argument sizes
vector<madsHeaderEntry>		t_mad;		// remember the labels for the *.HEA header file (-hm)
vector<pageEntry>			t_pag;		// remember the parameters for the .PAGE directive
vector<usingLabels>			t_usi;		// remember the parameters for the .USE [.USING] directive
vector<externalLabel>		t_ext;		// remember the addresses of external labels
vector<extLabelDeclEntry>	t_extn;		// remember external label declarations
vector<arrayEntry>			t_arr;		// remember the parameters of the .ARRAY arrays
vector<relocatableLabel>	t_rel;		// remember relocatable SDX label addresses
vector<relocSmb>			t_smb;		// remember the relocatable SDX symbol addresses
vector<reptEntry>			t_rep;		// remember the parameters of the .REPT loop
vector<procedureEntry>		t_prc;		// remember the parameters of the .PROC procedure
vector<labels>				t_lab;		// remember the labels
vector<triggered>			t_wyw;		// remember lines with invoked macros, helpful when displaying lines with errors
vector<structureEntry>		t_str;		// remember the names of .STRUCT structures
vector<if_else_endif_stack_item> if_stos;	// stack for .IF .ELSE .ENDIF operations

BYTE pass_end = (BYTE)2;	// By default there are 2 passes

// Build sets to quickly test things

set AllowedDotOperators = set::of(range(_or, _array), eos);

set AllowBinaryChars = set::of(range('0', '1'), eos);
set AllowDecimalChars = set::of(range('0', '9'), eos);
set AllowHexChars = set::of(range('0', '9'), range('A', 'F'), eos);

set AllowDirectiveChars = set::of('.', '#', eos);

set AllowLineEndings = set::of(0, 9, '\\', ';', ' ', eos);
set AllowWhiteSpaces = set::of(' ', '\t', '\r', '\n', eos);

set AllowLettersChars = set::of(range('A', 'Z'), eos);
set AllowMacroParamChars = set::of(range('A', 'Z'), '_', '@', eos);
set AllowLabelFirstChars = set::of(range('A', 'Z'), '_', '@', '?', eos);
set AllowExpressionChars = set::of(range('A', 'Z'), '_', '@', '?', ':', eos);
set AllowLineFirstChars = set::of(range('A', 'Z'), '_', '@', '?', ':', '.', '+', '-', '=', '*', eos);

set AllowMacroChars = set::of(range('A', 'Z'), range('0', '9'), '_', '@', eos);
set AllowLabelChars = set::of(range('A', 'Z'), range('0', '9'), '_', '@', '?', '.', eos);

set AllowQuotes = set::of('\'', '\"', eos);
set AllowStringBrackets = set::of('[', '(', eos);
set AllowBrackets = set::of('[', '(', '{', eos);

set AllowDirectorySeparators = set::of('/', '\\', eos);

set AssemblyAbort = set::of(
	ERROR_EXTRA_CHARS_ON_LINE,
	ERROR_NO_MATCHING_BRACKET, 
	ERROR_NEED_PARENTHESIS, 
	ERROR_ILLEGAL_ADDR_MODE_65XX,
	ERROR_MISSING_CURLY_CLOSING_BRK,
	MSG_STRING_ERROR, 
	WARN_ILLEGAL_CHARACTER, 
	WARN_NO_ORG_SPECIFIED,
	ERROR_ILLEGAL_INSTRUCTION,
	ERROR_VALUE_OUT_OF_RANGE,
	ERROR_LABEL_NAME_REQUIRED,
	WARN_MISSING_DOT_END, 
	MSG_CANT_OPEN_CREATE_FILE,
	ERROR_UNEXPECTED_EOL,
	ERROR_FILE_IS_TOO_SHORT,
	ERROR_FILE_IS_TOO_LONG,
	ERROR_MISSING_DOT_LOCAL,
	ERROR_USER_ERROR, 
	MSG_RESERVED_BLANK,
	ERROR_CANT_REPEAT_DIRECTIVE,
	WARN_IMPROPER_NR_OF_PARAMS,
	WARN_INCOMPATIBLE_TYPES, 
	WARN_BAD_PARAMETER_TYPE,
	ERROR_MISSING_DOT_ENDS,
	ERROR_NO_RECURSIVE_STRUCTURES,
	ERROR_IMPROPER_SYNTAX,
	ERROR_CONSTANT_EXP_SUBRANGE_BOUND,
	ERROR_INFINITE_RECURSION,
	WARN_UNDEFINED_SYMBOL,
	ERROR_INCORRECT_HEADER_FOR_FILE,
	ERROR_ZP_RELOC_BLOCK,
	ERROR_VAR_ADDR_OUT_OF_RANGE,
	WARN_SEGMENT_X_ERROR_AT_Y,
	ERROR_USE_SQUARE_BRACKET,
	ERROR_MISSING_DOT_ENDE, 
	ERROR_MULTILINE_ARG_NOT_SUPPORTED,
	eos);

set AllowedOperands = set::of('=', '<', '>', '!', eos);

set AllowedLineBreaks = set::of(0, ' ', '\t', eos);

set SetOfPlusMinus = set::of('+', '-', eos);

set ValidWriteCode = set::of(0x0e, 0x06, 0x1e, 0x16, 0x2e, 0x26, 0x3e, 0x36, 0x4e, 0x46, 0x5e, 0x56, 0x6e, 0x66, 0x7e, 0x76, 0xce, 0xc6, 0xde, 0xd6, 0xee, 0xe6, 0xfe, 0xf6, 0x8d, 0x85, 0x9d, 0x99, 0x95, 0x81, 0x91, 0x8e, 0x86, 0x96, 0x8c, 0x84, 0x94, 0x64, 0x9c, eos);
set ValidReadCode = set::of(0x6d, 0x65, 0x7d, 0x79, 0x75, 0x61, 0x71, 0x2d, 0x25, 0x3d, 0x39, 0x35, 0x21, 0x31, 0x2c, 0x24, 0xcd, 0xc5, 0xdd, 0xd9, 0xd5, 0xc1, 0xd1, 0xec, 0xe4, 0xcc, 0xc4, 0x4d, 0x45, 0x5d, 0x59, 0x55, 0x41, 0x51, 0x6c, 0xad, 0xa5, 0xbd, 0xb9, 0xb5, 0xa1, 0xb1, 0xae, 0xa6, 0xbe, 0xb6, 0xac, 0xa4, 0xbc, 0xb4, 0x0d, 0x05, 0x1d, 0x19, 0x15, 0x01, 0x11, 0xed, 0xe5, 0xfd, 0xf9, 0xf5, 0xe1, 0xf1, eos);

static constexpr char prior[17] = { 'D','E','&','|','^','/','*','%','+','=','A','B','<','C','>','F','G' };

// was indexed from 1..23
static const tCardinal maska[23] = {
	0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x100,0x200,0x400,0x800,0x1000,0x2000,
	0x4000,0x8000,0x10000,0x20000,0x40000,0x80000,0x100000,0x200000,0x400000
};

// was indexed from 1..48
static const tByte addycja[48] = {
	0,0,4,8,8,12,16,20,20,24,28,44,
	0,0,8,4,8,16,20,24,24,32,32,32,
	4,0,8,4,8,16,20,24,24,32,32,32,
	0,0,0,133,22,8,0,240,0,0,216,0
};

// was index from 1..207
static const tByte addycja_16[207] = {
	0,0,0,2,2,4,6,8,8,12,14,16,17,18,20,20,22,24,28,30,32,34,48,
	0,0,0,2,2,8,6,4,8,16,14,16,17,18,24,24,22,32,32,30,32,34,48,
	0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,220,
	0,0,0,0,0,0,0,0,0,0,16,0,0,0,0,0,0,0,0,0,32,144,48,
	0,0,0,0,0,0,0,101,0,8,0,0,0,0,16,0,0,0,24,0,0,0,0,
	0,0,0,0,0,0,0,0,0,56,0,0,0,0,16,0,0,0,58,0,0,0,0,
	0,0,0,0,0,0,0,146,0,0,0,0,114,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,140,0,0,0,148,0,0,0,0,156,0,0,0,164,0,0,0,0,
	0,0,0,0,0,204,0,0,0,212,0,0,0,0,220,0,0,0,228,0,0,0,0
};

// machine code of illegal mnemonic in first address (CPU 6502)
// index from 96..118
static const t_ads m6502ill[23] = {
	// Next are all illegal
	{0x03, 0x000006E5}, // 96 ASO
	{0x23, 0x000006E5}, // 97 RLN
	{0x43, 0x000006E5}, // 98 LSE
	{0x63, 0x000006E5}, // 99 RRD
	{0x83, 0x00000125}, // 100 SAX
	{0x9F, 0x80000365}, // 101 LAX
	{0xC3, 0x000006E5}, // 102 DCP
	{0xE3, 0x000006E5}, // 103 ISB
	{0x03, 0x00000008}, // 104 ANC
	{0x43, 0x00000008}, // 105 ALR
	{0x63, 0x00000008}, // 106 ARR
	{0x83, 0x00000008}, // 107 ANE Unstable
	{0xA3, 0x00000008}, // 108 ANX Unstable
	{0xC3, 0x00000008}, // 109 SBX
	{0xA3, 0x00000200}, // 110 LAS Unstable
	{0x83, 0x00000240}, // 111 SHA Unstable
	{0x83, 0x00000200}, // 112 SHS Unstable
	{0x86, 0x00000200}, // 113 SHX
	{0x80, 0x00000400}, // 114 SHY
	{0x1A, 0x00000000}, // 115 NPO
	{0x40, 0x0000008C}, // 116 DOP
	{0x00, 0x00000420}, // 117 TOP
	{0x02, 0x00000000}, // 118 CIM
};

// Machine code mnemonic in first addressing (CPU 6502)
// indexed from 0.55
static const t_ads m6502[56] =
{
	{0xA1, 0x000006ED}, // LDA
	{0x9E, 0x8000032C}, // LDX
	{0x9C, 0x800004AC}, // LDY
	{0x81, 0x000006E5}, // STA
	{0x82, 0x00000124}, // STX
	{0x80, 0x000000A4}, // STY
	{0x61, 0x000006ED}, // ADC
	{0x21, 0x000006ED}, // AND
	{0x02, 0x000004B4}, // ASL
	{0xE1, 0x000006ED}, // SBC
	{0x14, 0x00000020}, // JSR
	{0x40, 0x00000820}, // JMP
	{0x42, 0x000004B4}, // LSR
	{0x01, 0x000006ED}, // ORA
	{0xC1, 0x000006ED}, // CMP
	{0xBC, 0x8000002C}, // CPY
	{0xDC, 0x8000002C}, // CPX
	{0xC2, 0x000004A4}, // DEC
	{0xE2, 0x000004A4}, // INC
	{0x41, 0x000006ED}, // EOR
	{0x22, 0x000004B4}, // ROL
	{0x62, 0x000004B4}, // ROR
	{OP_BRK_IMP,	0x00000000}, // BRK
	{OP_CLC_IMP,	0x00000000}, // CLC
	{OP_CLI_IMP,	0x00000000}, // CLI
	{OP_CLV_IMP,	0x00000000}, // CLV
	{OP_CLD_IMP,	0x00000000}, // CLD
	{OP_PHP_IMP,	0x00000000}, // PHP
	{OP_PLP_IMP,	0x00000000}, // PLP
	{OP_PHA_IMP,	0x00000000}, // PHA
	{OP_PLA_IMP,	0x00000000}, // PLA
	{OP_RTI_IMP,	0x00000000}, // RTI
	{OP_RTS_IMP,	0x00000000}, // RTS
	{OP_SEC_IMP,	0x00000000}, // SEC
	{OP_SEI_IMP,	0x00000000}, // SEI
	{OP_SED_IMP,	0x00000000}, // SED
	{OP_INY_IMP,	0x00000000}, // INY
	{OP_INX_IMP,	0x00000000}, // INX
	{OP_DEY_IMP,	0x00000000}, // DEY
	{OP_DEX_IMP,	0x00000000}, // DEX
	{OP_TXA_IMP,	0x00000000}, // TXA
	{OP_TYA_IMP,	0x00000000}, // TYA
	{OP_TXS_IMP,	0x00000000}, // TXS
	{OP_TAY_IMP,	0x00000000}, // TAY
	{OP_TAX_IMP,	0x00000000}, // TAX
	{OP_TSX_IMP,	0x00000000}, // TSX
	{OP_NOP_IMP,	0x00000000}, // NOP
	{OP_BPL_REL,	0x00000002}, // BPL
	{OP_BMI_REL,	0x00000002}, // BMI
	{OP_BNE_REL,	0x00000002}, // BNE
	{OP_BCC_REL,	0x00000002}, // BCC
	{OP_BCS_REL,	0x00000002}, // BCS
	{OP_BEQ_REL,	0x00000002}, // BEQ
	{OP_BVC_REL,	0x00000002}, // BVC
	{OP_BVS_REL,	0x00000002}, // BVS
	{OP_BIT_BASE,	0x00000024}, // BIT (zp | abs)
};

// mnemonic machine code in first addressing (CPU 65816)
// index from 0..93
static const t_ads m65816[94] = {
	{0xA1, 0x000F7EE9}, // LDA
	{0x9E, 0x800282A0}, // LDX
	{0x9C, 0x800442A0}, // LDY
	{0x81, 0x000F7E69}, // STA
	{0x82, 0x00008220}, // STX
	{0x80, 0x00004220}, // STY
	{0x61, 0x000F7EE9}, // ADC
	{0x21, 0x000F7EE9}, // AND
	{0x02, 0x00044320}, // ASL
	{0xE1, 0x000F7EE9}, // SBC
	{0x20, 0x40400600}, // JSR
	{0x4C, 0x20700600}, // JMP
	{0x42, 0x00044320}, // LSR
	{0x01, 0x000F7EE9}, // ORA
	{0xC1, 0x000F7EE9}, // CMP
	{0xBC, 0x800002A0}, // CPY
	{0xDC, 0x800002A0}, // CPX
	{0x3A, 0x02044320}, // DEC
	{0x1A, 0x01044320}, // INC
	{0x41, 0x000F7EE9}, // EOR
	{0x22, 0x00044320}, // ROL
	{0x62, 0x00044320}, // ROR
	{0x00, 0x00000000}, // BRK
	{0x18, 0x00000000}, // CLC
	{0x58, 0x00000000}, // CLI
	{0xB8, 0x00000000}, // CLV
	{0xD8, 0x00000000}, // CLD
	{0x08, 0x00000000}, // PHP
	{0x28, 0x00000000}, // PLP
	{0x48, 0x00000000}, // PHA
	{0x68, 0x00000000}, // PLA
	{0x40, 0x00000000}, // RTI
	{0x60, 0x00000000}, // RTS
	{0x38, 0x00000000}, // SEC
	{0x78, 0x00000000}, // SEI
	{0xF8, 0x00000000}, // SED
	{0xC8, 0x00000000}, // INY
	{0xE8, 0x00000000}, // INX
	{0x88, 0x00000000}, // DEY
	{0xCA, 0x00000000}, // DEX
	{0x8A, 0x00000000}, // TXA
	{0x98, 0x00000000}, // TYA
	{0x9A, 0x00000000}, // TXS
	{0xA8, 0x00000000}, // TAY
	{0xAA, 0x00000000}, // TAX
	{0xBA, 0x00000000}, // TSX
	{0xEA, 0x00000000}, // NOP
	{0x10, 0x00000002}, // BPL
	{0x30, 0x00000002}, // BMI
	{0xD0, 0x00000002}, // BNE
	{0x90, 0x00000002}, // BCC
	{0xB0, 0x00000002}, // BCS
	{0xF0, 0x00000002}, // BEQ
	{0x50, 0x00000002}, // BVC
	{0x70, 0x00000002}, // BVS
	{0x24, 0x100442A0}, // BIT
	{0x64, 0x08044220}, // STZ
	{0xDE, 0x80000080}, // SEP
	{0xBE, 0x80000080}, // REP
	{0x10, 0x00000220}, // TRB
	{0x00, 0x00000220}, // TSB
	{0x80, 0x00000012}, // BRA
	{0xFE, 0x80000080}, // COP
	{0x54, 0x00000004}, // MVN
	{0x44, 0x00000004}, // MVP
	{0x60, 0x00000010}, // PER = PEA rell (push effective address relative)
	{0xC3, 0x00001000}, // PEI = PEA (zp) (push effective address indirect)
	{0x62, 0x04001090}, // PEA = PEA
	{0x8B, 0x00000000}, // PHB
	{0x0B, 0x00000000}, // PHD
	{0x4B, 0x00000000}, // PHK
	{0xDA, 0x00000000}, // PHX
	{0x5A, 0x00000000}, // PHY
	{0xAB, 0x00000000}, // PLB
	{0x2B, 0x00000000}, // PLD
	{0xFA, 0x00000000}, // PLX
	{0x7A, 0x00000000}, // PLY
	{0x6B, 0x00000000}, // RTL
	{0xDB, 0x00000000}, // STP
	{0x5B, 0x00000000}, // TCD
	{0x1B, 0x00000000}, // TCS
	{0x7B, 0x00000000}, // TDC
	{0x3B, 0x00000000}, // TSC
	{0x9B, 0x00000000}, // TXY
	{0xBB, 0x00000000}, // TYX
	{0xCB, 0x00000000}, // WAI
	{0x42, 0x00000000}, // WDM
	{0xEB, 0x00000000}, // XBA
	{0xFB, 0x00000000}, // XCE
	{0x3A, 0x00000000}, // DEA
	{0x1A, 0x00000000}, // INA
	{0x14, 0x00000400}, // JSL
	{0x4E, 0x00000400}, // JML
	{0x80, 0x00000010}, // BRL
};


#ifdef _WIN32
const char PathDelim = '\\';
#else
const char PathDelim = '/';
#endif

constexpr int pass_max = 20; // maximum possible number of assembly passes

constexpr BYTE __equ		= static_cast<BYTE>(0x80);       // pseudo command codes
constexpr BYTE __opt		= static_cast<BYTE>(0x81);
constexpr BYTE __org		= static_cast<BYTE>(0x82);
constexpr BYTE __ins		= static_cast<BYTE>(0x83);
constexpr BYTE __end		= static_cast<BYTE>(0x84);
constexpr BYTE __dta		= static_cast<BYTE>(0x85);
constexpr BYTE __icl		= static_cast<BYTE>(0x86);
constexpr BYTE __run		= static_cast<BYTE>(0x87);       // RUN
constexpr BYTE __nmb		= static_cast<BYTE>(0x88);
constexpr BYTE __ini		= static_cast<BYTE>(0x89);       // INI-RUN = 2  !!! necessarily !!!
//  __bot    = 0x8a;       // BOT currently not programmed
constexpr BYTE __rmb		= static_cast<BYTE>(0x8b);
constexpr BYTE __lmb		= static_cast<BYTE>(0x8c);
constexpr BYTE __ert		= static_cast<BYTE>(0x8d);
//  __ift    = 0x8e;       // -> .IF
//  __els    = 0x8f;       // -> .ELSE
//  __eif    = 0x90;       // -> .ENDIF
//  __eli    = 0x91;       // -> .ELSEIF
constexpr BYTE __smb		= static_cast<BYTE>(0x92);
constexpr BYTE __blk		= static_cast<BYTE>(0x93);
constexpr BYTE __ext		= static_cast<BYTE>(0x94);
constexpr BYTE __set		= static_cast<BYTE>(0x95);

constexpr BYTE __cpbcpd	= static_cast<BYTE>(0x97);       // macro command codes __cpbcpd..__jskip
constexpr BYTE __adbsbb	= static_cast<BYTE>(0x98);
constexpr BYTE __phrplr	= static_cast<BYTE>(0x99);
constexpr BYTE __adwsbw	= static_cast<BYTE>(0x9A);
constexpr BYTE __BckSkp	= static_cast<BYTE>(0x9B);
constexpr BYTE __inwdew	= static_cast<BYTE>(0x9C);
constexpr BYTE __addsub	= static_cast<BYTE>(0x9D);
constexpr BYTE __movaxy	= static_cast<BYTE>(0x9E);
constexpr BYTE __jskip		= static_cast<BYTE>(0x9F);       // end of macro command codes

constexpr BYTE __macro		= static_cast<BYTE>(0xA0);       // directive codes, order according to the table 'MAC' + 0xA0
constexpr BYTE __if		= static_cast<BYTE>(0xA1);
constexpr BYTE __endif		= static_cast<BYTE>(0xA2);
constexpr BYTE __endm		= static_cast<BYTE>(0xA3);
constexpr BYTE __exit		= static_cast<BYTE>(0xA4);
constexpr BYTE __error		= static_cast<BYTE>(0xA5);
constexpr BYTE __else		= static_cast<BYTE>(0xA6);
constexpr BYTE __print		= static_cast<BYTE>(0xA7);       // = __echo
constexpr BYTE __proc		= static_cast<BYTE>(0xA8);
constexpr BYTE __endp		= static_cast<BYTE>(0xA9);
constexpr BYTE __elseif	= static_cast<BYTE>(0xAA);
constexpr BYTE __local		= static_cast<BYTE>(0xAB);
constexpr BYTE __endl		= static_cast<BYTE>(0xAC);
constexpr BYTE __rept		= static_cast<BYTE>(0xAD);
constexpr BYTE __endr		= static_cast<BYTE>(0xAE);

constexpr BYTE __byte		= static_cast<BYTE>(0xAF);       // codes for .BYTE, .WORD, .LONG, .DWORD
constexpr BYTE __word		= static_cast<BYTE>(0xB0);      // they follow each other, !!! necessarily !!!
constexpr BYTE __long		= static_cast<BYTE>(0xB1);
constexpr BYTE __dword		= static_cast<BYTE>(0xB2);

constexpr BYTE __byteValue = static_cast<BYTE>((int)__byte - 1);     // will replace operations (...-__BYTE+1)

constexpr BYTE __struct	= static_cast<BYTE>(0xB3);
constexpr BYTE __ends		= static_cast<BYTE>(0xB4);
constexpr BYTE __ds		= static_cast<BYTE>(0xB5);
constexpr BYTE __symbol	= static_cast<BYTE>(0xB6);
constexpr BYTE __fl		= static_cast<BYTE>(0xB7);
constexpr BYTE __array		= static_cast<BYTE>(0xB8);
constexpr BYTE __enda		= static_cast<BYTE>(0xB9);
constexpr BYTE __get		= static_cast<BYTE>(0xBA);
constexpr BYTE __put		= static_cast<BYTE>(0xBB);
constexpr BYTE __sav		= static_cast<BYTE>(0xBC);
constexpr BYTE __pages		= static_cast<BYTE>(0xBD);
constexpr BYTE __endpg		= static_cast<BYTE>(0xBE);
constexpr BYTE __reloc		= static_cast<BYTE>(0xBF);
constexpr BYTE __dend		= static_cast<BYTE>(0xC0); // replaces the directives .ENDL, .ENDP, .ENDS, .ENDM, .ENDR etc.
constexpr BYTE __link		= static_cast<BYTE>(0xC1);
constexpr BYTE __extrn		= static_cast<BYTE>(0xC2); // equivalent of the EXT pseudo-command
constexpr BYTE __public	= static_cast<BYTE>(0xC3); // equivalent to .GLOBAL, .GLOBL

constexpr BYTE __reg		= static_cast<BYTE>(0xC4); //__REG, __VAR necessarily in this order
constexpr BYTE __var		= static_cast<BYTE>(0xC5);

constexpr BYTE __or		= static_cast<BYTE>(0xC6); // ORG
constexpr BYTE __by		= static_cast<BYTE>(0xC7);
constexpr BYTE __he		= static_cast<BYTE>(0xC8);
constexpr BYTE __wo		= static_cast<BYTE>(0xC9);
constexpr BYTE __en		= static_cast<BYTE>(0xCA);
constexpr BYTE __sb		= static_cast<BYTE>(0xCB);

constexpr BYTE __while		= static_cast<BYTE>(0xCC);
constexpr BYTE __endw		= static_cast<BYTE>(0xCD);
constexpr BYTE __test		= static_cast<BYTE>(0xCE);
constexpr BYTE __endt		= static_cast<BYTE>(0xCF);
constexpr BYTE __using		= static_cast<BYTE>(0xD0);
constexpr BYTE __ifndef	= static_cast<BYTE>(0xD1);
constexpr BYTE __nowarn	= static_cast<BYTE>(0xD2);
constexpr BYTE __def		= static_cast<BYTE>(0xD3);
constexpr BYTE __ifdef		= static_cast<BYTE>(0xD4);
constexpr BYTE __align		= static_cast<BYTE>(0xD5);
constexpr BYTE __zpvar		= static_cast<BYTE>(0xD6);
constexpr BYTE __enum		= static_cast<BYTE>(0xD7);
constexpr BYTE __ende		= static_cast<BYTE>(0xD8);
constexpr BYTE __cb		= static_cast<BYTE>(0xD9);
constexpr BYTE __segdef	= static_cast<BYTE>(0xDA);
constexpr BYTE __segment	= static_cast<BYTE>(0xDB);
constexpr BYTE __endseg	= static_cast<BYTE>(0xDC);
constexpr BYTE __dbyte		= static_cast<BYTE>(0xDD);
constexpr BYTE __xget		= static_cast<BYTE>(0xDE);
constexpr BYTE __define	= static_cast<BYTE>(0xDF);
constexpr BYTE __undef		= static_cast<BYTE>(0xE0);
constexpr BYTE __a			= static_cast<BYTE>(0xE1);
constexpr BYTE __i			= static_cast<BYTE>(0xE2);
constexpr BYTE __ai		= static_cast<BYTE>(0xE3);
constexpr BYTE __ia		= static_cast<BYTE>(0xE4);
constexpr BYTE __longa		= static_cast<BYTE>(0xE5);
constexpr BYTE __longi		= static_cast<BYTE>(0xE6);
constexpr BYTE __cbm		= static_cast<BYTE>(0xE7);
constexpr BYTE __bi		= static_cast<BYTE>(0xE8);
constexpr BYTE __over		= static_cast<BYTE>(__bi); // end of directive codes


//  __switch = 0xE9;	// not software
//  __case   = 0xEA;
constexpr BYTE __telse		= static_cast<BYTE>(0xEB);
constexpr BYTE __cycle		= static_cast<BYTE>(0xEC);
		   
constexpr BYTE __blkSpa	= static_cast<BYTE>(0xED);
constexpr BYTE __blkRel	= static_cast<BYTE>(0xEE);
constexpr BYTE __blkEmp	= static_cast<BYTE>(0xEF);
		   
constexpr BYTE __nill		= static_cast<BYTE>(0xF0);
constexpr BYTE __addEqu	= static_cast<BYTE>(0xF1);
constexpr BYTE __addSet	= static_cast<BYTE>(0xF2);
constexpr BYTE __xasm		= static_cast<BYTE>(0xF3);

// !!! of 0xF4 starts with __id_
// !!! it's over !!!

set AllowedTypeChars = set::of(range(__byte, __dword), eos);
set AllowedTypeExtChars = set::of(range(__byte, __dword), __proc, eos);
set LoopingDirective = set::of(range(__byte, __dword), __def, __print, __sav, __get, __xget, __put, __he, __by, __wo, __sb, __fl, __bi, __cb, __cbm, __dbyte, eos);
set AllDirectives = set::of(range(__macro, __over), eos);

constexpr int __rel = 0x0000;		// value for relocatable labels
constexpr int __relASM = 0x0100;	// assembly address for the .RELOC block
constexpr int __relHea = 'M' + ('R' << 8);

// !!! we must start with __ID_PARAM!!!
constexpr int __id_param		= 0xFFF4; // procedure parameters
constexpr int __id_mparam		= 0xFFF5; // macro parameters
constexpr int __id_array		= 0xFFF6;
constexpr int __dta_struct		= 0xFFF7;
constexpr int __id_ext			= 0xFFF8;
constexpr int __id_smb			= 0xFFF9;
constexpr int __id_noLab		= 0xFFFA;

constexpr int __id_macro		= 0xFFFB; // >= __id_macro (line 6515)
constexpr int __id_define		= 0xFFFC;
constexpr int __id_enum		= 0xFFFD;
constexpr int __id_struct		= 0xFFFE;
constexpr int __id_proc		= 0xFFFF;

constexpr BYTE __struct_run_noLabel = lo(__id_noLab);

constexpr BYTE __array_run = lo(__id_array);
constexpr BYTE __macro_run = lo(__id_macro);
constexpr BYTE __define_run = lo(__id_define);
constexpr BYTE __enum_run = lo(__id_enum);
constexpr BYTE __struct_run = lo(__id_struct);
constexpr BYTE __proc_run = lo(__id_proc);


constexpr int __hea_dos = 0xFFFF;  // header for the block DOS
constexpr int __hea_reloc = 0x0000;  // header for the block .RELOC
constexpr int __hea_public = 0xFFED;  // header for the block symbol updates public
constexpr int __hea_external = 0xFFEE;  // header for the block symbol updates external
constexpr int __hea_address = 0xFFEF;  // header for the block address updates relocatable

constexpr char __pDef = 'D';            // Default
constexpr char __pReg = 'R';            // Registry
constexpr char __pVar = 'V';            // Variable

string __test_label = "##TB";            // label for the beginning of the #IF block
string __telse_label = "@?@?@ML?ET";      // label for the beginning of the #ELSE block
string __endt_label = "@?@?@ML?TE";      // label for the end of the #IF block (#END)

string __while_label = "##B";             // label for the beginning of the #WHILE block
string __endw_label = "@?@?@ML?E";       // label for the end of the block #WHILE (#END)

string __local_name = "L@C?L?";          // label for .LOCAL without name

vector<tMadsStackEntry> mads_stack = {
	{"@STACK_POINTER", 0x00fe},
	{"@STACK_ADDRESS", 0x0500},
	{"@PROC_VARS_ADR", 0x0600},
};

char tType[4] = { 'B','A','T','F' };					// types used internally by MADS
char relType[7] = { 'B','W','L','D','<','>','^' };		// types saved in relocatable files
vector<string> mads_param = { ".BYTE ", ".WORD ", ".LONG ", ".DWORD " };

// Forward declarations
long long calculate_value_noSPC(string& zm, string& old, int& i, const char sep, const char typ);
void search_comment_block(int& i, string &zm, string& txt);
void calculate_data(int& i, string& a, string& old, const BYTE typ);
long long calculate_value(string& a, string& old);
int5 calculate_mnemonic(int& i, string& a, string& old);
void analyze_mem(const int start, const int koniec, string& old, string& a, string& old_str, int licz, const int p_max, const bool rp);
void analyze_file(string& a, string& old_str);

long FileSize(const string& filename)
{
	return static_cast<long>(std::filesystem::file_size(filename));
}

int pos(const string& find, const string txt)
{
	const auto loc = txt.find(find);
	if (loc == string::npos)
		return -1;
	return static_cast<int>(loc);
}

int pos(const char * find, string& txt)
{
	const auto loc = txt.find(find);
	if (loc == string::npos)
		return -1;
	return static_cast<int>(loc);
}

int pos(char toFind, string& txt)
{
	const auto loc = txt.find(toFind);
	if (loc == string::npos)
		return -1;
	return static_cast<int>(loc);
}

int posInPriority(char toFind)
{
	for (int i = 0; i < sizeof(prior); ++i)
	{
		if (prior[i] == toFind)
			return i;
	}
	return -1;
}

inline void myDelete(string& txt, const int idx, const int count)
{
	txt.erase(idx, count);
}

inline void myInsert(const string& src, string& target, const int index)
{
	target.insert(index, src);
}

inline void myInsert(const char* src, string& target, const int index)
{
	target.insert(index, src);
}

long long myRound(double x)
{
	return x >= 0 ? static_cast<long long>(trunc(x + 0.5)) : static_cast<long long>(trunc(x - 0.5));
}


string copy(const string &str, const int index, const int count)
{
	return str.substr(index, count);
}

inline int length(const string &a)
{
	return static_cast<int>(a.length());
}

inline void inc(BYTE& i, int add = 1)
{
	i += add;
}

inline void inc(int &i, int add)
{
	i += add;
}

inline void inc(int& i)
{
	++i;
}

inline void inc(long long &i, long long add)
{
	i += add;
}

inline void inc(unsigned int& i, int add)
{
	i += add;
}

inline void dec(unsigned char& i, const int minus)
{
	i -= minus;
}
inline void dec(char &i, const int minus)
{
	i -= minus;
}
inline void dec(long long &i, const int minus = 1)
{
	i -= minus;
}
inline void dec(int& i, const int minus)
{
	i -= minus;
}

inline void dec(int& i)
{
	--i;
}

inline void dec(unsigned char& i)
{
	--i;
}

inline long long random(int range)
{
	return rand() % range;
}

inline void SetLength(string& x, int newSize)
{
	x.resize(newSize);
}

inline void SetLength(_strArray& par, int newSize)
{
	par.resize(newSize);
}

inline int High(const _strArray& par)
{
	return static_cast<int>(par.size())-1;
}



/**
 * \brief Skip all white-spaces and comment blocks in a string.
 * \param i ref to the index in the string. 0 based. This gets advanced to the first char that should be processed.
 * \param a ref to the string to search
 */
void skip_spaces(int &i, string &a)
{
	if (a.empty() == false)
	{
		while ((i <= a.length()) and (AllowWhiteSpaces.has(a[i])))
		{
			inc(i);
		}

		if ((i < length(a)) and ((a[i] == '/') and (a[i + 1] == '*')))
		{
			string txt;
			search_comment_block(i, a, txt);

			if (not(isComment))
				skip_spaces(i, a);
		}
	}
}

/**
 * \brief convert tabs to spaces and return new string
 * \param a string to operate on
 * \param spc tab expands to this many spaces [8]
 * \return 
 */
string Tab2Space(const string &a, const int spc = 8)
{
	int column = 0;
	stringstream result("");

	for (const char ch : a)
	{
		switch (ch)
		{
			case '\t':
			{
				const int nextTabStop = (column + spc) / spc * spc;
				while (column != nextTabStop)
				{
					result.put(' ');
					++column;
				}
				break;
			}
			case '\r':
			case '\n':
			{
				result.put(ch);
				column = 0;
				break;
			}
			default:
			{
				result.put(ch);
				++column;
			}
		}
	}

	return result.str();
}

/**
 * \brief Advance to the next char in the string, skip whitespace
 * \param i 0-based location in string
 * \param a string to work on
 */
void IncAndSkip(int &i, string &a)
{
	if (i >= a.length())
		return;
	++i;
	skip_spaces(i, a);
}

/**
 * \brief Convert string to upper case
 * \param a string to convert to upper case
 * \return new copy of the transformed string
 */
string AnsiUpperCase(const string &a)
{
	string result;
	if (a.length())
		transform(a.begin(), a.end(), std::back_inserter(result), ::toupper);
	
	return result;
}

/**
 * \brief Convert char to upper case (if case_used is false)
 * \param a char to convert 
 * \return converted character
 */
char UpCas_(const char a)
{
	if (not(case_used))
		return UpCase(a);
	else
		return a;
}

/**
 * \brief Convert from ATASCII to internal
 * \param a ATASCII value
 * \return 
 */
static BYTE ata2int(const BYTE a)
{
	const int x = a & 0x7F;
	if (x >= 0 && x <= 31)
		return a + 64;
	if (x >= 32 && x <= 95)
		return a - 32;
	return a;
}

static string IntToStr(const long long a)
{
	return to_string(a);
}

static long long StrToInt(const string &a)
{
	char* pEnd;
	if (a[0] == '$')
		return strtoll(a.c_str()+1, &pEnd, 16);

	return strtoll(a.c_str(), &pEnd, 10);
}

static long long StrToInt(const char a)
{
	string x;
	x[0] = a;
	return StrToInt(x);

}

/**
 * \brief Empty the binary buffer by writing the data to the output file
 */
void flush_dst()
{
	if (buf_i > 0)
	{
		dotObjectFile.write(reinterpret_cast<char*>(&t_buf.x[0]), buf_i);
		buf_i = 0;
	}
}

/**
 * \brief Write byte to output. Flush the buffer and handle fill before the write
 * \param a byte to write to output
 */
void put_dst(const BYTE a)
{
	if (::fill > 0)
	{
		flush_dst();

		BYTE v = obxFillValue;
		while (::fill > 0)
		{
			dotObjectFile.write(reinterpret_cast<char*>(&v), 1);
			::fill--;
		}
	}
	t_buf[buf_i] = a;
	++buf_i;
	if (buf_i >= static_cast<int>(sizeof(t_buf.x)))
		flush_dst();
}

/**
 * \brief Conversion to hexadecimal notation until 'hex_len' digits are done. If there are digits left, continue the process.
 * \param val number to convert to hex
 * \param minNr2xDigits how many digits at minimum
 * \return 
 */
string Hex(unsigned int val, short minNr2xDigits)
{
	static const char* digits = "0123456789ABCDEF";
	char twoDigits[3];
	twoDigits[2] = 0;
	string Result;
	while ((minNr2xDigits > 0) or (val != 0))
	{
		const tByte v = static_cast<tByte>(val & 0xFF);
		twoDigits[0] = digits[v >> 4];
		twoDigits[1] = digits[v & 0x0f];
		//string add = "" + digits[v >> 4] + digits[v & 0x0f];
		Result = twoDigits + Result;

		val = val >> 8;

		minNr2xDigits -= 2;
	}
	return Result;
	
}

/**
 * \brief Get a message
 * \param nr of the message to display [1 based]
 * \return string
 */
string load_mes(const int nr)
{
	return mes[nr];
}

/**
 * \brief remove the path from the full file name (ExtractFilePath)
 * \param filename 
 * \return 
 */
string GetFilePath(const string &filename)
{
	string result;
	if (!filename.empty())
	{
		int i = static_cast<int>(filename.length()) - 1;
		while (i >= 0 && filename[i] != PathDelim)
			--i;

		result = filename;
		result.resize(i+1);
	}
	return result;
}

/**
 * \brief remove the file name from the full file name (ExtractFileName)
 * \param filename 
 * \return 
 */
string GetFileName(const string &filename)
{
	string result;

	if (!filename.empty())
	{
		int i = static_cast<int>(filename.length()) - 1;
		while (i >= 0 && filename[i] != PathDelim)
			--i;
		result = filename.substr(i+1, filename.length());
		
	}
	return result;
}

string show_full_name(const string &a, const bool b, const bool c)
{
	const string pth = GetFilePath(a);
	string result = a;

	if (b)
	{
		if (pth.empty())
			result = path + result;
	}
	else
		result = GetFileName(result);
	if (c)
		result = "Source: " + result;
	return result;
}

/**
 * \brief Displays a warning, does not interrupt assembly
 * \param warningNr 0-based message nr
 * \param str_error optional error string
 */
void warning(const BYTE warningNr, const string& str_error = "" )
{
	if (!noWarning)
	{
		string txt = load_mes(warningNr);

		int lin = line;
		string nam = global_name;

		switch (warningNr)
		{
			case WARN_ILLEGAL_CHARACTER:		txt += '?'; break;

			case WARN_ACCESS_VIOLATION_AT_ADDR: txt += '$' + Hex(zpvar, 4); break;

			case WARN_UNREFERENCED_PROCEDURE:
			case WARN_UNUSED_LABEL:
			case WARN_UNSTABLE_ILLEGAL_OPCODE:
			case WARN_AMBIGUOUS_LABEL:
			case WARN_BRANCH_TOO_LONG:
			case WARN_BRANCH_OVER_PAGE_BOUNDS:	txt += str_error; break;

			case WARN_PAGE_ERROR_AT:			txt += '$' + Hex(addres, 4); break;

			case WARN_LABEL_T_IS_ONLY_FOR:
			{
				while (txt.find('\t') != string::npos)
				{
					const auto i = txt.find('\t');
					txt.erase(i, 1);
					txt.insert(i, attribute.name);
					break;
				}
				switch (attribute.attrib)
				{
					case __R: txt += "READ"; break;
					case __W: txt += "WRITE"; break;
					default: txt += "???"; break;
				}
				break;
			}

			case WARN_INFINITE_LOOP_AT_LABEL:
			{
				txt += infinite.lab;
				lin = infinite.lineNr;
				nam = infinite.name;
				break;
			}
		}

		warning_mes = show_full_name(nam, full_name, false) + " (" + IntToStr(lin) + ") WARNING: " + txt;

		if (warning_mes != warning_old)
		{
			cout << dye::light_aqua(warning_mes) << '\n';
			warning_old = warning_mes;
		}
	}
}

/**
 * \brief Save the MADS header file entries for a specific bank
 * \param bank_nr The bank that is do be dumped
 * \param title_and_type - Title and Type (first char)
 */
void saveToMAEHeaderFile(const int bank_nr, const char *title_and_type)
{
	const char active_type = title_and_type[0];
	bool okay = false;
	for (int i = static_cast<int>(t_mad.size()) - 1; i >= 0; --i)
	{
		if (t_mad[i].bank == bank_nr)
		{
			if (t_mad[i].myType == active_type)
			{
				okay = true;
				break;
			}
		}
	}

	if (okay)
	{
		dotMEAFile << '\n' << "; " << title_and_type << '\n';
		for (int i = static_cast<int>(t_mad.size()) - 1; i >= 0; --i)
		{
			if (t_mad[i].bank == bank_nr)
			{
				if (t_mad[i].myType == active_type)
				{
					dotMEAFile << t_mad[i].name << "\t=\t$" << Hex(t_mad[i].addr, 4) << '\n';
				}
			}
		}
	}
}

/**
 * \brief Add a (color) message, no duplicates.
 * \param msg Message to store
 * \param cl color code to use: 0 is default
 */
void new_message(string& msg, const BYTE cl = 0)
{
	if (msg.empty())
		return;

	msg = Tab2Space(msg);

	for (int i = static_cast<int>(::messages.size()) - 1; i >= 0; --i)
	{
		if (::messages[i].message == msg)
		{
			// Already have it
			msg.erase();
			return;
		}
	}

	const int i = static_cast<int>(::messages.size()) - 1;
	::messages[i].pass = pass;
	::messages[i].message = msg;
	::messages[i].color = cl;

	::messages.resize(i + 2);

	msg.erase();
}

/**
 * \brief Finish the assembly
 * \param err error state: 0 = without errors, 1 = only WARNING messages, 2 = error and assembly stop
 */
void DoTheEnd(const BYTE err)
{
	if (open_ok)
	{
		// Flush and close the binary output file
		flush_dst();

		const int output_size = static_cast<int>(dotObjectFile.tellp());		// Get the size of the .o
		dotObjectFile.close();

		// Finish off the following
		// - .lab file
		// - .lst file
		if (list_lab)
		{
			dotLabFile.flush();
			dotLabFile.close();
		}

		if (first_lst)
		{
			dotListFile.flush();
			dotListFile.close();

			if (!silent)
			{
				string txt = load_mes(MSG_WRITING_LISTING_FILE); // Writing listing file...
				new_message(txt, hue::DARKGRAY);
			}
		}

		if ((output_size == 0) or (err > STATUS_ONLY_WARNINGS))
		{
			// If the output was empty or there was an error then remove the output file again
			if (std::remove(filenameOBX.c_str()) == 0)
			{
				if (not silent)
				{
					string txt = load_mes(WARN_UNABLE_TO_DELETE_FILE) + filenameOBX;
					new_message(txt, hue::BROWN);
				}
			}
		}
		else
		{
			string txt = load_mes(MSG_WRITING_OBJECT_FILE + raw.use);
			if (!silent) new_message(txt, hue::DARKGRAY);
		}

		if (over && !end_string.empty())
			cout << end_string;

		if (over and not(silent))
		{
			if (err != 2)
			{
				string txt = IntToStr(line_all) + load_mes(MSG_X_LINES_ASSEMBLED) + " in " + IntToStr(pass_end) + " pass";
				if (pass > pass_max)
					txt += " (infinite loop)";

				new_message(txt, hue::BROWN);

				if (output_size > 0)
				{
					txt = IntToStr(output_size) + load_mes(MSG_X_BYTES_WRITTEN);
					new_message(txt, hue::BROWN);
				}
			}
		}
	}

	if (list_dotMEA)
	{
		for (int bank_nr = 0; bank_nr < 256; ++bank_nr)
		{
			// Check if there are any MADS header file entries for this bank
			bool ok = false;
			for (int a = static_cast<int>(t_mad.size()) - 1; a >= 0; --a)
			{
				if (t_mad[a].bank == bank_nr)
				{
					ok = true;
					break;
				}
			}

			if (ok)
			{
				if (bank > 0)
					dotMEAFile << '\n' << "lmb #" << bank_nr << "\t\t;BANK #" << bank_nr << '\n';

				saveToMAEHeaderFile(bank_nr, "CONSTANTS");	// constants
				saveToMAEHeaderFile(bank_nr, "VARIABLES");	// variables
				saveToMAEHeaderFile(bank_nr, "PROCEDURES"); // procedures
				dotMEAFile.close();
			}
		
		}
		dotMEAFile.close();
	}

	if (list_dotH)
	{
		dotHFile << '\n' << "#endif" << '\n';
		dotHFile.close();
	}

	for (int a = 0; a < static_cast<int>(::messages.size()) - 1; ++a)
	{
		if (::messages[a].color != 0) hue::set(::messages[a].color);

		if (a > 0)
			cout << '\n';
		cout << ::messages[a].message << flush;

		hue::reset();

		for (int b = static_cast<int>(::messages.size()) - 1; b >= 0; --b)
		{
			if (::messages[b].message == ::messages[a].message)
				::messages[b].pass = 0xFF;
		}
	}

	if (not(silent) and (err == 2))
		cout << '\n';

	exit(err);
}

bool TestFile(const string &filename)
{
	return std::filesystem::exists(filename);
}

/**
 * \brief Add 'val' to the global text log of the line. This is for the .lst file output
 * \param val value to add to the log
 */
void just_t(const tCardinal val)
{
	const int len = length(t);

	if (not(len > margin - 1))
	{
		if (len + 3 > margin - 3)
		{
			// If the byte list is too long just add a + to show that there is too much data
			t += " +";
			while (length(t) < margin)
			{
				t += ' ';
			}
		}
		else
		{
			// A
			t += " " + Hex(val, 2);
		}
	}
}


/**
 * \brief Output to the .lst tracing line the current bank, addreess
 * \param addr Address to output
 */
void bank_address(int addr)
{
	if ((dotRELOC.use) and (addr - rel_ofs >= 0))
		dec(addr, rel_ofs);

	if (bank > 0)
		t += Hex(bank, 2) + ",";

	t += Hex(addr, 4);           // otherwise it will not display the 64bit value
}


/**
 * \brief Save a byte 
 * \param a byte to save to the destination file
 */
void save_dst(const BYTE a)
{
	if (pass == pass_end && (opt & opt_O))
	{
		if (addres > 0xFFFF)
			warning(WARN_MEMORY_RANGE_EXCEEDED);

		if (org)
		{
			t.resize(7);

			if (hea and (opt & opt_H) and not (dotRELOC.sdx))
				t += "FFFF> ";
			int x = addres;
			int y = t_hea[hea_i];

			if (dotRELOC.use)
			{
				x -= rel_ofs;
				y -= rel_ofs;
			}

			if (hea_offset.addr >= 0)
			{
				y = hea_offset.addr + (y - x);
				x = hea_offset.addr;
			}

			int ex, ey;
			if (x <= y)
			{
				ex = x;
				ey = y;
			}
			else
			{
				ey = x;
				ex = y;
			}

			char znk;
			if (blok > 0)
			{
				// length of the relocatable block
				y = y - x + 1;
				znk = ',';
			}
			else
				znk = '-';

			if (addres >= 0)
			{
				if (ex > ey) bank_address(t_hea[hea_i - 1] + 1);
				else bank_address(x);

				if (ex <= ey && (opt & opt_H))
					t = t + znk + Hex(y, 4) + '>';
			}

			// exception when the new block has the address $FFFF - we also write two bytes of the FF FF header
			if ((hea && (opt & opt_H) && dotRELOC.sdx == false) || ((opt & opt_H) && dotRELOC.sdx == false && addres == 0xFFFF))
			{
				put_dst(0xFF);
				put_dst(0xFF);
			}

			if (opt & opt_H)
			{
				put_dst(static_cast<BYTE>(x));
				put_dst(static_cast<BYTE>(x >> 8));
				put_dst(static_cast<BYTE>(y));
				put_dst(static_cast<BYTE>(y >> 8));
			}

			org = false;
			hea = false;
		}

		just_t(a);

		if (!block_record) put_dst(a);
	}
	else
	{
		::fill = 0; // important for recording a RAW file

		if (org && raw.use)
		{
			hea = false;    // important for recording a RAW file
			org = false;    // important for recording a RAW file
		}
	}
}

/**
 * \brief 
 * \param a 
 */
void save_dstW(const int a)
{
	 save_dst(static_cast<BYTE>(a));			// lo
	 save_dst(static_cast<BYTE>(a >> 8));		// hi
}

/**
 * \brief writing a STRING, character by character
 * \param a 
 */
  void save_dstS(const string &a)
{
	const int len = static_cast<int>(a.length());

	save_dstW(len);

	for (int i = 0; i < len; ++i)
	{
		save_dst(a[i]);
	}
}

/**
 * \brief 
 * \param i 
 */
void save_nul(const int i) // only write down zeros
{
	for (int k = 0; k < i - 1; ++k)
		save_dst(0);
}

/**
 * \brief 
 * \param con 
 * \param tmp 
 */
void con_update(string &con, const string &tmp)
{
	const auto i = con.find('\t');
	if (i != string::npos)
	{
		con.erase(i, 1);
		con.insert(i, tmp);
	}
}

/**
 * \brief	displays error message no. in 'B'
			the exception is b<0 then it will display the message 'Branch...'
			for message no. 14, it will display the name of the selected 8-16 bit CPU operating mode
 * \param a 
 * \param error_nr 
 * \param str_error 
 */
void show_error(string &a, const int error_nr, string str_error = {})
{
	if (error_nr == 0)
	{
		overflow = true;
		if (pass != pass_end)
			return;
	}

	string add;
	string prv;
	string con;

	line_err = line;

	if (run_macro)
	{
		con = t_wyw[1-1].zm;
		new_message(con);
		global_name = t_wyw[wyw_idx].pl;
		line_err += t_wyw[wyw_idx].nr;
	}
	else if (not(rept_run) and not(FOX_ripit) and not(loop_used) and not(code6502))
	{
		if (line_add > 0)
			inc(line_err, line_add);
	}

	// we remove signs #0 because they are probably some weird bushes
	while (a.length() > 0 and a.find((char)0) != string::npos)
	{
		a.resize(a.find((char)0) - 1);
	}

	if (a.empty() == false and not(error_nr == MSG_CANT_OPEN_CREATE_FILE or error_nr == MSG_RESERVED_BLANK))
	{
		con = a;

		if (not t_lin.empty())
		{
			for (int i = 0; i < t_lin.size() - 1; ++i)
			{
				con += "\\" + t_lin[i];
			}

			new_message(con);
		}
	}

	con += show_full_name(global_name, full_name, false) + " (" + IntToStr(line_err + ((line_err == 0) ? 1 : 0)) + load_mes(MSG_ERROR_TO_FOLLOW);

	if ((error_nr >= MSG_VALUE_OUT_OF_RANGE and error_nr <= MSG_STRING_ERROR) && str_error.empty() == false)
		add += " " + str_error;

	if (error_nr == MSG_CANT_OPEN_CREATE_FILE)
		str_error = a; //  Cannot open or create file ' '

	if (error_nr == WARN_LABEL_X_DECLARED_TWICE or error_nr == WARN_UNDECLARED_LABEL or error_nr == WARN_UNDECLARED_MACRO)
		add += load_mes(MSG_BANK_EQU + (dotRELOC.use ? 1 : 0)) + IntToStr(bank) + ")"; // Block/bank

	if (error_nr == WARN_CANT_DECLARE_T_AS_PUBLIC)
		add += load_mes(MSG_SEGMENT_ERROR_AT);	/// ... as public

	if (error_nr == WARN_SEGMENT_X_ERROR_AT_Y)
		add += Hex(addres, 4);

	if (error_nr == ERROR_ILLEGAL_ADDR_MODE_65XX)
		add = load_mes(MSG_6502 + ((opt & opt_C) ? 1 : 0));	// if opt and 16>0 then add:='816)' else add:='02)';

	if (error_nr == WARN_MISSING_DOT_END)
	{
		if (proc)
			add = "P";
		else if (macro)
			add = "M";
	}

	if (error_nr < 0)
	{
		con += load_mes(MSG_BRANCH_OUT_OF_RANGE) + Hex(abs(error_nr), 4) + load_mes(MSG_BYTES);
	}
	else if (error_nr != MSG_RESERVED_BLANK)
		con = con + prv + load_mes(error_nr) + add;
	else
		con = con + a + add;

	if (error_nr == WARN_CANT_FIIL_HI_TO_LOW)
	{
		con_update(con, Hex(addres, 4));
	}

	con_update(con, str_error);

	status = STATUS_STOP_ON_ERROR;

	new_message(con, hue::LIGHTRED);

	// Some errors require immediate termination of assembly
	// If too many errors occur (>512), we also end the assembly
	if (AssemblyAbort.has(error_nr) || ::messages.size() > 512)
	{
		over = true;
		DoTheEnd(STATUS_STOP_ON_ERROR);
	}
}

void show_error(string& a, const int b, const char str_error)
{
	string err;
	err[0] = str_error;
	show_error(a, b, err);
}

void justify()
{
	if (pass == pass_end and !t.empty() && !FOX_ripit)
	{
		int j = static_cast<int>(t.length());
		while (j < margin)
		{
			t += '\t';
			j += 8;
		}
	}
}

/**
 * \brief	displays the message 'Undeclared label ????'
			displays the message 'Label ???? declared twice'
			displays a message 'Undeclared macro ????
 * \param old 
 * \param b 
 * \param x 
 */
void error_und(string &old, const string& b, const BYTE x)
{
	if (x == WARN_UNREFERENCED_PROCEDURE)
		warning(x, b);
	else
		show_error(old, x, b);
}

void WriteAccessFile(string &a)
{
	ofstream test(a);
	if (test.fail())
		show_error(a, MSG_CANT_OPEN_CREATE_FILE);
	test.close();
}

void NormalizePath(string& a)
{
	if (!a.empty())
	{
		for (char& i : a)
		{
			if (AllowDirectorySeparators.has(i))
				i = PathDelim;
		}
	}
}


/**
 * \brief we are looking for a file in the declared search paths
 * \brief PATH          The path from which the main assembly file is run
 * \brief GLOBAL_NAME   last used path for ICL, INS, etc. operations
 * \param a
 * \param zm
 * \return
 */
string GetFile(string a, string& zm)
{
	if (a.empty())
		show_error(zm, MSG_STRING_ERROR);

	NormalizePath(a);

	string p = GetFilePath(global_name) + a;

	if (TestFile(p))
		a = p;
	else
	{
		p = path + a;
		if (TestFile(p)) a = p;
	}

	if (TestFile(a))
		return a;

	for (int i = 0; i < t_pth.size() - 1; ++i)
	{
		// !!! the order in which T_PTH[0..] is looked does matters !!!
		p = t_pth[i];

		if (!p.empty() && p[p.length() - 1] != PathDelim)
			p += PathDelim;

		string c = p + a;
		if (TestFile(c))
			return c;
	}
	return a;
}

/**
 * \brief look for a label and return its index to the 'T_LAB' array
 * \brief if there is no such label, we return the value -1
 * \param a string to search
 * \return -1 if not found of the index into t_lab
 */
int l_lab(const string &a)
{
	const int len = static_cast<int>(a.length());

	unsigned int x = 0xffffffff;

	for (int i = 0; i < len; ++i)
	{
		x = tCRC32[static_cast<BYTE>(x) ^ static_cast<BYTE>(a[i])] ^ (x >> 8);
	}

	// OK, if the found label has a code >=__id_param, or the current value BANK=0
	// OK if the label found is from the current bank or zero bank (BNK=BANK | BNK=0)

	for (int i = static_cast<int>(t_lab.size()) - 1; i >= 0; --i)
	{
		if (t_lab[i].len == len && t_lab[i].name == x)
		{
			if (bank == 0 || t_lab[i].bank >= __id_param)
			{
				return i;
			}
			else if (set::of(bank, 0, eos).has(t_lab[i].bank))
			{
				return i;
			}
		}
	}

	return -1;
}

/**
 * \brief we remove the last string of characters after the dot from the string, the string ends with a dot
 * \param b 
 */
void cut_dot(string& b)
{
	int i = static_cast<int>(b.length())-1;
	if (!b.empty() && i > 1)
	{
		--i;
		while (i >= 0 && b[i] != '.')
			--i;
		b.resize(i+1);
	}
}

/**
 * \brief look for the label name in the T_LAB array, if the name contains a dot we cut off the name and keep looking
 * \param x what we are looking for
 * \param a input to the load_lab function passed through
 * \return 
 */
int search(string &x, string &a)
{
	string b = x + local_name;
	string toFind = b + a;
	int result = l_lab(toFind);

	while(result < 0 && b.find('.') != string::npos)
	{
		cut_dot(b);
		toFind = b + a;
		result = l_lab(toFind);
	}
	return result;
}

/**
 * \brief	we look for a label and return its index in the 'T_LAB' array
			if there is no such label, we return the value -1.
			If a macro is running and we cannot find a label, we look for it in .PROC
			If we don't find it in .PROC, then we look for it in the main program.
 * \param a 
 * \param test 
 * \return 
 */
int load_lab(string& a, const bool test)
{
	int result = -1;

	if (!ReadEnum)
	{
		if (test)
		{
			if (run_macro)
			{
				result = search(macro_nr, a);
				if (result >= 0)
					return result;
			}

			if (proc)
			{
				result = search(proc_name, a);
				if (result >= 0)
					return result;
			}

			string txt;
			result = search(txt, a);
			if (result >= 0)
				return result;
		}
		const string txt = local_name + a;
		result = l_lab(txt);

		if (result < 0)
			result = search(proc_name, a);
	}

	if (result < 0)
	{
		if (usi_idx > 0)	// test for .USE [.USING]
		{
			for (int i = 0; i <= usi_idx - 1; ++i)
			{
				if ( ((local_name.find(t_usi[i].name) == 0) || (proc_name.find(t_usi[i].name) == 0))
					|| (local_name == t_usi[i].name) || (proc_name == t_usi[i].name))
				{
					string txt = t_usi[i].myLabel + '.' + a;
					result = l_lab(txt);

					if (result >= 0)
						return result;
					else
					{
						txt = local_name + txt;
						result = l_lab(txt);

						if (result >= 0)
							return result;
					}
				}
			}
		}
	}

	return result;
}


/**
 * \brief	Save the label name, its address, etc. in the .LAB file
			additionally if required in the .C header file for cc65
			we do not save temporary, local labels in macros
 * \param label_name label to save
 * \param addr memory address
 * \param bank_nr bank nr
 * \param symbol 
 */
void save_the_label(const string &label_name, const unsigned int addr, const int bank_nr, const char symbol)
{
	if (pass == pass_end)
	{
		if (structure.use == false && !(mae_labels == false && symbol== '?') && label_name.find("::") == string::npos && label_name.find(__local_name) == string::npos)
		{
			if (list_lab)
				dotLabFile << Hex(bank_nr, 2) << '\t' << Hex(addr, 4) << '\t' << label_name << '\n';

			if (proc == false && bank_nr < 256)
			{
				if (list_dotH && bank_nr == 0)
				{
					string tmp = label_name;
					std::replace(tmp.begin(), tmp.end(), '.', '_');

					dotHFile << "#define " << name << '_' << tmp << " 0x" << Hex(addr, 4) << '\n';
				}

				if (list_dotMEA)
				{
					// Store an entry for the MADS header file
					const int i = static_cast<int>(t_mad.size()) - 1;
					t_mad[i].name = label_name;
					t_mad[i].addr = addr;
					t_mad[i].bank = static_cast<BYTE>(bank_nr);
					t_mad[i].myType = label_type;

					t_mad.resize(i + 2);
				}
			}
		}
	}
}

/**
 * \brief check whether there is a structure field declaration in T_STR
 * \param a 
 * \param id 
 * \return 
 */
int loa_str(const string &a, const int id)
{
	// !!! Be sure to look through it from the back !!!
	// Make sure to skip the last empty entry in t_str
	for (int i = static_cast<int>(t_str.size())-1-1; i >= 0; --i)
	{
		if (t_str[i].id == id && t_str[i].labelName == a)
		{
			return i;
		}
	}
	// thanks to the fact that I browse backwards, we will never find the name 
	// of the structure, the structure name has the NO number the same as the first field of the structure
	return -1;
}

/**
 * \brief read the index to a structure field with a specific number X
 * \param id 
 * \param x 
 * \return the index in t_str where .id and .no match the input
 */
int loa_str_no(int id, const int x)
{
	// !!! Be sure to look through it from the back !!!
	// Make sure to skip the last empty entry in t_str
	for (int i = static_cast<int>(t_str.size())-1-1; i >= 0; --i)
	{
		if (t_str[i].id == id && t_str[i].no == x)
			return i;		
	}

	return -1;	
}

/**
 * \brief write down information about the structure fields if they did not appear before
 * \param a 
 * \param offset 
 * \param size 
 * \param repeat 
 * \param addr 
 * \param bankNr 
 */
void save_str(string &a, int offset, int size, int repeat, const unsigned int addr, const int bankNr)
{
	int i = loa_str(a, structure.id);

	if (i < 0)
	{
		i = static_cast<int>(t_str.size()) - 1;
		t_str.resize(i + 2);
	}

	t_str[i].id = structure.id;
	t_str[i].no = structure.cnt;
	t_str[i].addr = addr;
	t_str[i].bank = bankNr;
	t_str[i].labelName = a;
	t_str[i].offset = offset;
	t_str[i].mySize = size;

	t_str[i].nrRepeat = repeat == 0 ? 1 : repeat;
}

/**
 * \brief the correct procedure for storing the label in the 'T_LAB' array 
 * \param a 
 * \param ad 
 * \param ba 
 * \param old 
 * \param symbol 
 * \param new_local 
 */
void s_lab(const string &a, const unsigned int ad, const int ba, string old, const char symbol, bool new_local = false)
{
	// check if there is no such label already
	// because if it is, do not add a new item, just correct the old one
	int x = l_lab(a);

	if (x < 0)
	{
		x = static_cast<int>(t_lab.size()) - 1;
		t_lab.resize(x + 2);
	}
	else if (symbol != '?')
	{
		if ((t_lab[x].bank !=ba) and not(struct_used.use) and not(enumeration.use))
			error_und(old, a, WARN_LABEL_X_DECLARED_TWICE);
	}

	int len = static_cast<int>(a.length());

	unsigned int tmp = 0xffffffff;  // CRC32

	int i = 0;
	while (i < len)
	{
		tmp = tCRC32[BYTE(tmp) ^ BYTE(a[i])] ^ (tmp >> 8);
		++i;
	}

	if (pass > 0)
	{
		if (symbol != '?' or mae_labels)		// if these are the MAE label style, we need to check them
		{
			if (t_lab[x].name == tmp)			// normally we only check labels with the first character <>'?'
			{
				if ((t_lab[x].bank < __id_param) and not(t_lab[x].lid))
				{
					if (t_lab[x].pass == pass)
						error_und(old, a, WARN_LABEL_X_DECLARED_TWICE);	// you cannot check the same label twice in the current run

					if (pass < pass_max - 1)
					{
						if (not(next_pass))	// check if additional mileage is needed
						{
							if (mne_used)		// some mnemonic must have been made beforehand
							{
								next_pass = t_lab[x].addr != ad;

								if (next_pass)
								{
									if (pass > 3 and (t_lab[x].lop == 0))
									{
										t_lab[x].lop = 1; // infinite loop
									}

									//	 writeln(pass,' : ',a,',', t_lab[x].lop,' | ', t_lab[x].addr);

									infinite.lab = a;
									infinite.lineNr = line;
									infinite.name = global_name;
								}
							}
						}
					}
				}
			}
		}
	}

	if ((pass == pass_end) and unused_label and (ba < __id_param) and not(structure.use) and not(run_macro) and not(t_lab[x].use))
	{
		warning(WARN_UNUSED_LABEL, a);
	}

	if (t_lab[x].lid && new_local == false)
		error_und(old, a, WARN_LABEL_X_DECLARED_TWICE);

	++t_lab[x].add;
	t_lab[x].attrib = atr;
	t_lab[x].lid = new_local;
	t_lab[x].theType = label_type;
	t_lab[x].len = len;
	t_lab[x].name = tmp;
	t_lab[x].addr = ad;
	t_lab[x].bank = ba;
	t_lab[x].block = blok;
	t_lab[x].pass = pass;
	t_lab[x].offset = org_ofset;
	if (((blok > 0) or dotRELOC.use) and (symbol != '?') )
		t_lab[x].rel = true;

	save_the_label(a, ad, ba, symbol);
}


/**
 * \brief we remember the label, address, bank 
 * \param a 
 * \param ad 
 * \param ba 
 * \param old 
 * \param new_local 
 */
void save_lab(string a, const unsigned int ad, const int ba, const string &old, bool new_local = false)
{
	if (!a.empty())
	{
		// !!! necessary test
		registerOptimisation.used = false;
		if (a == "@")
		{
			a = IntToStr(anonymous_idx) + '@';
			++anonymous_idx;
		}
		string tmp;

		if (mae_labels && a[0] != '?')
		{
			tmp = a;
			local_name = a + '.';
		}
		else if (run_macro)
			tmp = macro_nr + local_name + a;
		else if (proc)
			tmp = proc_name + local_name + a;	// !!! do not remote LOCAL_NAME !!!
		else
			tmp = local_name + a;

		s_lab(tmp, ad, ba, old, a[0], new_local);
	}
}

void save_arr(const unsigned int address, const int bankNr)
{
	t_arr[array_idx].addr = address;
	t_arr[array_idx].bank = bankNr;
	t_arr[array_idx].offset = org_ofset;

	++array_idx;

	if (array_idx >= t_arr.size())
		t_arr.resize(array_idx + 1);
}

void save_hea()
{
	t_hea[hea_i] = addres - 1 - ::fill;

	::fill = 0;

	++hea_i;

	if (hea_i >= t_hea.size())
		t_hea.resize(hea_i + 1);
}

void save_par(const string &a)
{
	int i = static_cast<int>(t_par.size()) - 1;
	t_par[i] = a;
	t_par.resize(i + 2);
}

void save_mac(const string &a)
{
	int i = static_cast<int>(t_mac.size()) - 1;
	t_mac[i] = a;
	t_mac.resize(i + 2);
}

void save_end(const BYTE a)
{
	t_end[end_idx].endCode = a;
	t_end[end_idx].addr = addres;
	t_end[end_idx].withSemicolon = false; // semicolon {
	++end_idx;

	if (end_idx >= t_end.size())
		t_end.resize(end_idx + 1);
}

void dec_end(string& zm, const BYTE a)
{
	--end_idx;

	if (t_end[end_idx].endCode != a)
	{
		switch (a)
		{
			case __endpg:  show_error(zm, ERROR_MISSING_DOT_PAGES);	break; // Missing .PAGES
			case __ends:   show_error(zm, ERROR_MISSING_DOT_STRUCT);	break; // Missing .STRUCT
			case __endm:   show_error(zm, ERROR_MISSING_DOT_MACRO);	break; // Missing .MACRO
			case __endw:   show_error(zm, ERROR_MISSING_HASH_WHILE);	break; // Missing .WHILE
			case __endt:   show_error(zm, ERROR_MISSING_HASH_IF);		break; // Missing .TEST / #IF
			case __enda:   show_error(zm, ERROR_MISSING_DOT_ARRAY);	break; // Missing .ARRAY
			case __endr:   show_error(zm, ERROR_MISSING_DOT_REPT);		break; // Missing .REPT
		}
	}
}

/**
 * \brief we write the symbol .PUBLIC
 *			if ADD = FALSE, an error will occur when the symbol is repeated
 * \param a 
 * \param zm 
 */
void save_pub(string &a, string &zm)
{
	for (int i = pub_idx - 1; i >= 0; --i)
	{
		// public symbols cannot be repeated
		if (t_pub[i].name == a)
			error_und(zm, a, WARN_LABEL_X_DECLARED_TWICE);
	}

	t_pub[pub_idx].name = a;
	++pub_idx;
	if (pub_idx >= t_pub.size())
		t_pub.resize(pub_idx + 1);
}

/**
 * \brief 
 * \param a 
 * \param idx 
 * \param b 
 * \param reloc_value 
 */
void save_rel(const int a, const int idx, const int b, relocateValue &reloc_value)
{
	if (dotRELOC.use || dotRELOC.sdx)
	{
		if (isVector)
			t_rel[rel_idx].addr = a - rel_ofs;
		else
			t_rel[rel_idx].addr = a - rel_ofs + 1;

		if (::empty == false)
			t_rel[rel_idx].idx = idx;
		else
			t_rel[rel_idx].idx = -100;

		t_rel[rel_idx].blk = b;
		t_rel[rel_idx].block = blok;

		if (dotRELOC.use)
		{
			t_rel[rel_idx].idx = size_idx;
			t_rel[rel_idx].bank = bank;
			++size_idx;

			rel_used = true;               // we allow the size to be saved to T_SIZ

			if (size_idx >= t_siz.size())
				t_siz.resize(size_idx + 1);
		}

		++rel_idx;

		if (rel_idx >= t_rel.size())
			t_rel.resize(rel_idx + 1);

		reloc = true;

		reloc_value.use = true;

		++reloc_value.count;
	}
}

/**
 * \brief 
 * \param arg 
 * \param reloc_value 
 */
void save_relAddress(const int arg, relocateValue &reloc_value)
{
	if (branch == false)
	{
		if (t_lab[arg].rel)
		{
			save_rel(addres, -1, t_lab[arg].block, reloc_value);
		}
	}
}


/**
 * \brief Check the range of the 'value' variable based on the code in 'rangeSelector'
 * B = 0 - $FF			8 Bit
 * A, V = 0 - $FFFF		16 Bit
 * E, T = 0 - $FFFFFF	24 Bit
 * F., R, L, H = 0 - $FFFFFFFF 32 Bit
 * \param txt original value as string
 * \param value to check the range on
 * \param rangeSelector B, A, V, E, T, F, R, L, H
 * \return 
 */
tCardinal wartosc(string& txt, tInt64 value, const char rangeSelector)
{
	const tCardinal result = static_cast<tCardinal>(value);

	const tInt64 i = abs(value); // ABS is necessary, otherwise it will not work properly
	long long mx;
	bool isToBig;

	switch(rangeSelector)
	{
		case 'B':
		{
			mx = 0xff;
			isToBig = i > 0xFF;
			break;
		}
		case 'A':
		case 'V':
		{
			mx = 0xffff;
			isToBig = i > 0xFFFF;
			break;
		}
		case 'E':
		case 'T':
		{
			mx = 0xffffff;
			isToBig = i > 0xFFFFFF;
			break;
		}
		default:
		{
			//   'F','R','L','H' :  x := i > $FFFFFFFF;   // !!! unreal !!! recommend for subtraction to work
			mx = 0xFFFFFFFF;
			isToBig = false;
			break;
		}
	}

	if (isToBig)
	{
		const string err = "(" + IntToStr(value) + " must be between 0 and " + IntToStr(mx) + ")";
		show_error(txt, MSG_VALUE_OUT_OF_RANGE, err);
	}

	return result;
}


/**
 * \brief Test if a is an operand
 * \param a char to test
 * \return 
 */
bool _ope(const char a)
{
	return AllowedOperands.has(a);
}


/**
 * \brief line break characters
 * \param a char to test
 * \return 
 */
bool _eol(const char a)
{
	return AllowedLineBreaks.has(a);
}

/**
 * \brief allowed characters for decimal numbers
 * \param a char to test
 * \return 
 */
bool _dec(const char a)
{
	return AllowDecimalChars.has(a);
}

/**
 * \brief allowed characters for macro parameters
 * \param a char to test
 * \return 
 */
bool _mpar(const char a)
{
	return AllowMacroChars.has(::toupper(a));
}

/**
 * \brief allowed characters for macro parameters
 * \param a char to test
 * \return
 */
bool _mpar_alpha(const char a)
{
	return AllowMacroParamChars.has(::toupper(a));
}

/**
 * \brief allowed alphabetic characters
 * \param a char to test
 * \return
 */
bool _alpha(const char a)
{
	return AllowLettersChars.has(::toupper(a));
}

/**
 * \brief acceptable characters for labels
 * \param a char to test
 * \return
 */
bool _lab(const char a)
{
	return AllowLabelChars.has(::toupper(a));
}

/**
 * \brief allowable characters for binary numbers
 * \param a char to test
 * \return
 */
bool _bin(const char a)
{
	return AllowBinaryChars.has(a);
}

/**
 * \brief allowable characters for hexadecimal numbers
 * \param a char to test
 * \return
 */
bool _hex(const char a)
{
	return AllowHexChars.has(::toupper(a));
}

/**
 * \brief first legal characters for the line
 * \param a char to test
 * \return
 */
bool _first_char(const char a)
{
	return AllowLineFirstChars.has(::toupper(a));
}

/**
 * \brief first allowable characters for directives
 * \param a char to test
 * \return
 */
bool _dir_first(const char a)
{
	return AllowDirectiveChars.has(a);
}

/**
 * \brief first legal characters for labels
 * \param a char to test
 * \return
 */
bool _lab_first(const char a)
{
	return AllowLabelFirstChars.has(::toupper(a));
}

/**
 * \brief first legal characters for labels in expressions
 * \param a char to test
 * \return
 */
bool _lab_firstEx(const char a)
{
	return AllowExpressionChars.has(::toupper(a));
}

/**
 * \brief	!!! we must calculate the code for the first 3 characters !!!
			we calculate the checksum for the 3-letter character sequence 'A'..'Z'
			the calculated checksum is an index to the HASH table
 * \param src 
 * \return 
 */
BYTE fASC(const string &src)
{
	if (src.length() >= 3)
	{
		// !!! necessarily LENGTH(A)>=3 !!! to calculate LDA.W etc.
		for (int j = 0; j < 3; ++ j)
		{
			if (_alpha(src[j]) == false)
				return 0;
		}
		const int i = src[0] - '@' + ((src[1] - '@') << 5) + ((src[2] - '@') << 10);
		return ::hash[i];
	}
	return 0;
}

/**
 * \brief	Calculate the 16-bit CRC checksum for a short character string <3..6>
			the calculated 16-bit checksum is an index to the HASH table
 * \param a 
 * \return 
 */
BYTE fCRC16(const string &a)
{
	int x = 0xffff;

	for (int i = 0; i < static_cast<int>(a.length()); ++i)
	{
		x = tCRC16[static_cast<BYTE>(x >> 8) ^ static_cast<BYTE>(a[i])] ^ (x << 8);
	}

	return ::hash[x & 0xFFFF];
}

/**  // NOLINT(clang-diagnostic-comment)
 * \brief end of line test, lines may end with characters #0, #9, ' ', ';', '//','/*','\'
 * \param i 
 * \param a 
 * \param sep 
 * \param sep2 
 * \return 
 */
bool test_char(int i, string &a, const char sep = 0, char sep2 = 0)
{
	if (a.empty())
		return false;

	if (i < a.length()-2 && a[i] == '/' && a[i+1] == '*')
	{
		string txt;
		search_comment_block(i, a, txt);
	}

	return (a[i] == 0 || a[i] == 9 || a[i] == '\\' || a[i] == ';' || a[i] == sep || a[i] == sep2) or (a[i] == '/' and a[i+1] == '/');
}

/**
 * \brief	we check whether there are no invalid characters at the end of the line
			the line may end with the characters #0,#9,';',' ','//' or comma ','
 * \param i 
 * \param a 
 * \param old 
 * \param b 
 */
void test_eol(const int i, const string &a, string &old, const char b)
{
	if ( not (AllowLineEndings.has(a[i]) || a[i] == b) and not (a[i] == '/' and (a[i + 1] == '/' || a[i + 1] == '*')))
		show_error(old, ERROR_EXTRA_CHARS_ON_LINE);
}

/**
 * \brief get directive starting with '.', '#'
 * \param i 
 * \param a 
 * \param upc 
 * \return 
 */
string get_directive(int &i, const string &a, bool upc = false)
{
	string result;
	if (!a.empty())
	{
		if (_dir_first(a[i]))
		{
			result = a[i];
			++i;
			while (i < a.length() and _alpha(a[i]))
			{
				if (upc)
					result += UpCas_(a[i]);
				else
					result += static_cast<char>(::toupper(a[i]));
				++i;
			}
		}
	}

	return result;
}

/**
 * \brief get label starting with characters 'A'..'Z','_','?','@'
			if TST = TRUE then the label must contain some characters
 * \param i 
 * \param a 
 * \param tst 
 * \return 
 */
string get_lab(int &i, string &a, const bool tst)
{
	string result;

	if (!a.empty())
	{
		if (tst) skip_spaces(i, a);

		if (_lab_first(a[i]))
		{
			while(_lab(a[i]))
			{
				result += UpCas_(a[i]);
				++i;
			}
		}
	}

	if (tst && result.empty()) show_error(a, ERROR_LABEL_NAME_REQUIRED);

	return result;	
}

/**
 * \brief	takes a string of characters, delimited by characters '' or "" 
			double '' means literal '
			double "" means literal "
			TEST = TRUE checks if the string is empty 
 * \param i 
 * \param a 
 * \param old 
 * \param test 
 * \return 
 */
string get_string(int &i, string &a, string &old, const bool test)
{
	string result;

	skip_spaces(i, a);
	if (AllowQuotes.has(a[i]) == false)
		return result;

	const char gchr = a[i];
	const int len = static_cast<int>(a.length());

	while(i < len)
	{
		++i;	// we omit the first character ' or "
		char znak = a[i];

		if (znak == gchr)
		{
			++i;

			if (a[i] == gchr)
			{
				znak = gchr;
			}
			else
			{
				if (test && result.empty())
					show_error(old, MSG_STRING_ERROR);
				return result;
			}
		}
		result += znak;
	}

	if (isComment == false)
		show_error(a, MSG_STRING_ERROR); // did not encounter a character ending ' or "

	return result;
}

/**
 * \brief	Parse a string of characters (directives), change the case to uppercase
			if TST = TRUE then the string must be non-empty
 * \param i 
 * \param a 
 * \param sep 
 * \param tst 
 * \return 
 */
string get_datUp(int &i, string &a, const char sep, const bool tst)
{
	string result;
	skip_spaces(i, a);

	if (a.empty() == false)
	{
		while(a[i] != '=' and not(test_char(i, a, ' ', sep)))
		{
			result += static_cast<char>(::toupper(a[i]));
			++i;
		}
	}

	if (tst && result.empty()) show_error(a, ERROR_UNEXPECTED_EOL);

	return result;
}

/**
 * \brief	Check whether the read string of characters represents a data type directive
			accepted directive types are .BYTE, .WORD, .LONG, .DWORD
 * \param i 
 * \param zm 
 * \param old 
 * \param tst 
 * \param err 
 * \return 0 if the type was NOT found
 */
BYTE get_type(int &i, string &zm, string &old, const bool tst, const bool err = true)
{
	skip_spaces(i, zm);

	string txt = get_directive(i, zm);

	if (txt.empty() and not(tst))
	{
		return 0;			// -1 = type not found
		// exception for .RELOC [.BYTE] [.WORD]
	}

	BYTE result = fCRC16(txt);

	if (err)
	{
		if (not(result >= __byte and result <= __dword))
			error_und(old, txt, WARN_BAD_PARAMETER_TYPE);
	}

	if (err && AllowedTypeChars.has(result) == false) error_und(old, txt, WARN_BAD_PARAMETER_TYPE);

	dec(result, __byteValue);

	return result;
}

/**
 * \brief	Check whether the read string of characters represents a data type directive for EXT
			accepted type directives are .BYTE, .WORD, .LONG, .DWORD, .PROC
 * \param i 
 * \param zm 
 * \return 
 */
BYTE get_typeExt(int &i, string &zm)
{
	skip_spaces(i, zm);

	const string txt = get_directive(i, zm);

	const BYTE result = fCRC16(txt);		// Map from .BYTE ... .PROC to a single BYTE code

	if (AllowedTypeExtChars.has(result) == false) 
		error_und(zm, txt, WARN_BAD_PARAMETER_TYPE);

	return result;
}

/**
 * \brief	Parse a string delimited by two characters 'LEFT' and 'RIGHT'
			'LEFT' and 'RIGHT' characters may be nested
			if CUT = TRUE, we remove the starting and ending brackets
 * \param i 
 * \param a 
 * \param cut 
 * \return 
 */
string bounded_string(int &i, string &a, bool cut)
{
	string result;

	if (AllowBrackets.has(a[i]) == false)
		return result;

	char left = a[i];
	char right;
	if (left == '(')
		right = ')';
	else
		right = static_cast<char>(left + 2);

	int bracket = 0;
	bool isLoop = true;
	const int len = static_cast<int>(a.length());

	while(isLoop && i < len)
	{
		char znak = a[i];

		if (znak == left)
			++bracket;
		else if (znak == right)
			--bracket;

		isLoop = bracket != 0;

		if (znak == '\\')
		{
			isLoop = false;
			break;
		}
		else
		{
			if (AllowQuotes.has(znak))
			{
				string txt = get_string(i, a, a, false);
				result += znak + txt + znak;

				if (txt.length() == 1 && txt[0] == znak)
					result += znak;
			}
			else
			{
				result += UpCas_(znak);
				++i;
			}
		}
	}

	if (isLoop && isComment == false)
	{
		switch (left)
		{
			case '[': show_error(a, ERROR_NO_MATCHING_BRACKET); break;
			case '(': show_error(a, ERROR_NEED_PARENTHESIS); break;
			case '{': show_error(a, ERROR_MISSING_CURLY_CLOSING_BRK); break;
		}
	}

	if (cut && result.empty() == false)
		result = result.substr(1, result.length() - 2);

	return result;
}

/**
 * \brief test for the presence of macro parameters, i.e. :0..9, %%0..9, :label, %%label
 * \param i 
 * \param a 
 * \return 
 */
bool test_macro_param(const int i, const string &a)
{
	return ((i < a.length() - 1) and (a[i] == ':' and _mpar(a[i + 1]))) or (a[i] == '%' and a[i + 1] == '%' and _mpar(a[i + 2]));
}

/**
 * \brief test for the presence of macro parameters, i.e. :0..9, %%0..9
 * \param i 
 * \param a 
 * \return 
 */
bool test_param(const int i, const string &a)
{
	return ((i < a.length() - 1) and (a[i] == ':' and _dec(a[i + 1]))) or (a[i] == '%' and a[i + 1] == '%' and _dec(a[i + 2]));
}

/**
 * \brief	load any characters except space, tab and 'Sep'
			if there are characters that open a string, read that string
 * \param i 
 * \param a 
 * \param sep 
 * \param space 
 * \return 
 */
string get_dat(int &i, string &a, const char sep, const bool space)
{
	string result;

	const int len = static_cast<int>(a.length());
	while (i < len)
	{
		if (a[i] == sep)
			return result;

		switch (::toupper(a[i]))
		{
			case '[':
			case '(':
			case '{':
			{
				if (isComment == false)
					result += bounded_string(i, a, false);
				else
				{
					result += a[i];
					++i;
				}
				break;
			}
			case '.': result += get_directive(i, a, true); break;
			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
			case 'G':
			case 'H':
			case 'I':
			case 'J':
			case 'K':
			case 'L':
			case 'M':
			case 'N':
			case 'O':
			case 'P':
			case 'Q':
			case 'R':
			case 'S':
			case 'T':
			case 'U':
			case 'V':
			case 'W':
			case 'X':
			case 'Y':
			case 'Z': result += get_lab(i, a, false); break;

			case '\'':
			case '"':
			{
					if (isComment == false)
					{
						char znak = a[i];
						string txt = get_string(i, a, a, false);

						result += znak + txt + znak;

						if (txt.length() == 1 && txt[0] == znak)
							result += znak;
					}
					else
					{
						result += a[i];
						++i;
					}
				break;
			}

			case '/':
			{
					switch (a[i+1])
					{
						case '/': return result;
						case '*':
						{
							string txt;
							search_comment_block(i, a, txt);
							if (isComment)
								return result;
							break;
						}

						default:
						{
							result += '/';
							++i;
						}
					}
				break;
			}

			case ';':
			case '\\': return result;

			case ' ':
			case '\t':
			{
				if (space) return result;
				result += a[i];
				++i;
				break;
			}

			default:
			{
				result += UpCas_(a[i]);
				++i;
			}
		}
	}
	return result;	
}

/**
 * \brief Parse a string of characters, omitting spaces
 * \param i index int string
 * \param a string to parse
 * \param old original string
 * \param sep 
 * \param sep2 
 * \return 
 */
string get_dat_noSPC(int &i, string &a, string &old, const char sep, const char sep2 = 0)
{
	string result;

	skip_spaces(i, a);

	const int len = static_cast<int>(a.length());

	while (i < len)
	{
		switch(a[i])
		{
			case ' ':
			case '\t':
			{
				if (sep == ' ')
					return result;
				else 
					IncAndSkip(i, a);
				break;
			}

			case '.':
			case '$':
			{
				// error if '.' and '$' are followed by "white space"
				if (AllowWhiteSpaces.has(a[i + 1])) return result;

				if (test_char(i + 1, a, ' ', 0) && isComment == false)
					show_error(old, ERROR_EXTRA_CHARS_ON_LINE);
				result += a[i];
				IncAndSkip(i, a);

				break;
			}

			case '/':
			{
					switch(a[i+1])
					{
						case '/': return result;
						case '*':
						{
							string txt;
							search_comment_block(i, a, txt);
							if (isComment) return result;
							break;
						}
						default:
						{
							result += '/';
							IncAndSkip(i, a);
						}
					}
				break;
			}

			case ';':
			case '\\': return result;

			default:
			{
				if (a[i] == sep || a[i] == sep2) return result;

				result += get_dat(i, a, sep, true);
			}
		}
	}

	return result;
}

/**
 * \brief Replace complex operators with the appropriate one-letter code
 * \param i 
 * \param old 
 * \param a 
 * \param b 
 * \param value 
 * \return 
 */
char OperExt(int &i, string &old, const char a, const char b, bool &value)
{
	char result = ' ';

	switch ((a << 8) + b)
	{
		case 15422:
		case 8509:		result = 'A'; break;       // '<>', '!='
		case 15421:		result = 'B'; break;       // '<='
		case 15933:		result = 'C'; break;       // '>='
		case 15420:		result = 'D'; break;      // '<<'
		case 15934:		result = 'E'; break;      // '>>'
		case 15677:		result = '='; break;      // '=='
		case 9766:		result = 'F'; break;      // '&&'
		case 31868:		result = 'G'; break;      // '||'
	}

	if (not(value) or (result == ' ') )
	{
		show_error(old, WARN_ILLEGAL_CHARACTER, static_cast<char>(a + b));
	}

	inc(i, 2);
	value = false;

	return result;
}

/**
 * \brief 
 * \param i 
 * \param old 
 * \param a 
 * \param value 
 * \param b 
 * \return 
 */
char OperNew(int &i, string &old, const char a, bool &value, const bool b)
{
	if (value != b)
	{
		string txt;
		txt[0] = a;
		show_error(old, WARN_ILLEGAL_CHARACTER, txt);
	}

	++i;
	value = false;

	return a;
}

/**
 * \brief Calculate the value that will modify the character or file string for '*' it is the value 128, i.e. invers
 * \param i 
 * \param a 
 * \param typ 
 * \return 
 */
int test_string(int &i, string &a, const char typ)
{
	int result = 0; // the result is a signed type, necessarily !!!
	skip_spaces(i, a);

	switch (a[i])
	{
		case '*':
		{
			result = 128;
			++i;
			break;
		}
		case '+':
		case '-':
		{
			result = static_cast<int>(calculate_value_noSPC(a, a, i, ',', typ));
			break;
		}
	}

	skip_spaces(i, a);

	return result;
}

/**
 * \brief
 * \param a
 * \param val
 * \param maximum
 */
void subrange_bounds(string& a, const int val, const int maximum)
{
	if (val < 0 or val > maximum)
		show_error(a, ERROR_CONSTANT_EXP_SUBRANGE_BOUND);
}

/**
 * \brief Write data bytes to .ARRAY depending on the set data type
 * \param war
 * \param tmp
 * \param op_
 * \param invers
 */
 void save_dta(unsigned int war, string& tmp, const char op_, const BYTE invers)
{
	int k, i;
	tByte v;

	if (aray or put_used)
	{
		if (array_used.idx > 0xFFFF)
		{
			return;
		}
		else
		{

			if (op_ == 'C' or op_ == 'D')
			{
				i = length(tmp);

				for (k = 1 - 1; k < i; ++k)
				{
					v = byte(ord(tmp[k]) + invers);
					if (op_ == 'D')
						v = ata2int(v);

					just_t(v);

					if (aray)
						t_tmp[array_used.idx] = v;
					else
						t_get[array_used.idx] = v;

					inc(array_used.idx);

					if (array_used.idx > array_used.max)
						array_used.max = array_used.idx;
				}

				return;
			}

			just_t(war);

			if (aray)
				t_tmp[array_used.idx] = cardinal(war);
			else
				t_get[array_used.idx] = byte(war);

			inc(array_used.idx);

			if (array_used.idx > array_used.max)
				array_used.max = array_used.idx;

			return;
		}
	}

	switch (op_)
	{
		case 'L':
		case 'B':
		{
			save_dst(byte(war));
			inc(addres);
			break;
		}

		case 'H':
		{
			save_dst(byte(war >> 8));
			inc(addres);
			break;
		}

		case 'M':
		{
			save_dst(byte(war >> 16));
			inc(addres);
			break;
		}

		case 'G':
		{
			save_dst(byte(war >> 24));
			inc(addres);
			break;
		}

		case 'A':
		case 'V':
		{
			save_dst(byte(war));
			save_dst(byte(war >> 8));
			inc(addres, 2);
			break;
		}

		case 'T':
		case 'E':
		{
			save_dst(byte(war));
			save_dst(byte(war >> 8));
			save_dst(byte(war >> 16));
			inc(addres, 3);
			break;
		}

		case 'F':
		{
			save_dst(byte(war));
			save_dst(byte(war >> 8));
			save_dst(byte(war >> 16));
			save_dst(byte(war >> 24));
			inc(addres, 4);
			break;
		}

		case 'R':
		{
			save_dst(byte(war >> 24));
			save_dst(byte(war >> 16));
			save_dst(byte(war >> 8));
			save_dst(byte(war));
			inc(addres, 4);
			break;
		}

		case 'C':
		case 'D':
		{
			i = length(tmp);

			for (k = 1 - 1; k < i; ++k)
			{
				v = ord(tmp[k]);

				if (op_ == 'D')
					v = ata2int(v);

				inc(v, invers);

				save_dst(v);
			}
			inc(addres, i);
			break;
		}
	}
}

/**
 * \brief Calculate the value limited by brackets '[' or '('
 * \param zm 
 * \param old 
 * \param i 
 * \return 
 */
long long calculate_value_ogr(string& zm, string& old, int& i)
{
	long long result = 0;
	skip_spaces(i, zm);

	if (AllowStringBrackets.has(zm[i]))
	{
		string txt = bounded_string(i, zm, true);

		int k = 1-1;
		result = calculate_value_noSPC(txt, old, k, 0, 'F');
	}
	return result;
}

/**
 * \brief 
 * \param i 
 * \param a 
 * \return 
 */
string get_labelEx(int& i, string& a)
{
	string result;

	if (AllowQuotes.has(a[i]))
	{
		char ch = a[i];
		string txt = get_string(i, a, a, true);

		result = ch + txt + ch;

		if (txt.length() == 1 && txt[0] == ch) result = result + ch;
	}
	else
	{
		if (_lab_firstEx(a[i]))
		{
			result += UpCas_(a[i]);
			++i;

			while (_lab(a[i]))
			{
				result += UpCas_(a[i]);
				++i;
			}
		}
	}

	return result;
}

/**
 * \brief	get label starting with 'A'..'Z','_','?','@',':'
			if there are brackets '( )' or '[ ]', we remove them
 * \param i
 * \param a
 * \param old
 * \return
 */
 string get_labEx(int& i, string& a, string& old)
{
	string result;
	skip_spaces(i, a);

	if (AllowStringBrackets.has(a[i]))
	{
		string tmp = bounded_string(i, a, true);
		int k = 1-1;
		skip_spaces(k, tmp);

		result = get_labelEx(k, tmp);
	}
	else
		result = get_labelEx(i, a);

	if (result.empty()) 
		show_error(old, ERROR_LABEL_NAME_REQUIRED);

	return result;
}

/**
 * \brief Find the index to the T_LAB array
 * \param a
 * \param old
 * \param test
 * \return
 */
 int load_label_ofset(string& a, string old, const bool test)
{
	int result;
	if (a.empty()) show_error(old, ERROR_UNEXPECTED_EOL);

	if (a[0] == ':')
	{
		int b = bank;
		bank = 0;         // we force BANK=0 to read the label from the lowest level

		a = a.substr(2-1, a.length());
		result = l_lab(a);

		bank = b;         // we restore the previous BANK value
	}
	else
		result = load_lab(a, true);

	if (test)
	{
		if (result < 0 and pass == pass_end)
			error_und(old, a, WARN_UNDECLARED_LABEL);
	}

	return result;
}

void testRange(string& old, int& i, const BYTE b)
{
	if (i < 0 || i > 0xFFFF)
	{
		show_error(old, b);
		i = 0;
	}
}

/**
 * \brief Parse [0-9]* decimal number from a string
 * \param idx Index from where to start parsing
 * \param src String from where to parse
 * \return string of parsed decimal number
 */
string read_DEC(int& idx, const string& src)
{
	string result;
	while (idx < src.length() && _dec(src[idx]))
	{
		result += src[idx];
		++idx;
	}

	return result;
}

/**
 * \brief 
 * \param idx 
 * \param src 
 * \param old 
 * \return 
 */
string read_HEX(int& idx, const string& src, string& old)
{
	string result;
	inc(idx);                                               // skip the first '$' character

	while (idx < src.length() && _hex(src[idx]))
	{
		result += static_cast<char>(::toupper(src[idx]));
		++idx;
	}

	if (!(test_param(idx, src)))
	{
		if (result.empty())
		{
			string err;
			err += src[idx];
			show_error(old, WARN_ILLEGAL_CHARACTER, err);
		}
	}

	result = '$' + result;

	return result;
}

bool macro_rept_if_test()
{
	return (!rept && if_test);
}

/**
 * \brief Formatting the listing lines before saving them to the .LST file
 * \param c 
 */
void save_lst(const char c)
{
	// if pass=pass_end then begin        // !!! will not work for OPT F+ when using .DS XXX
	// !!! must use SAVE_DST to "spit out" FILL

	if (not(loop_used) and not(FOX_ripit))
	{
		t = IntToStr(line);
		while (t.length() < 6)
			t = " " + t;
		t += ' ';
	}


	switch (c)
	{
		case 'l':
		{
			if (not(FOX_ripit))
			{
				t += "= ";
				bank_address(nul.i);
			}
			break;
		}

		case 'i':
		{
			bank_address(addres);
			break;
		}

		case 'a':
		{
			if (not(hea) and not(loop_used) and not(FOX_ripit) and
				not(structure.use) and (addres >= 0))
			{
				bank_address(addres);
			}

			if (nul.l > 0)
			{
				data_out = true;
				for (int i = 0; i < nul.l; ++i)
				{
					save_dst(nul.h[i]);
				}
			}
			break;
		}

	}   // end case

	inc(addres, nul.l);    // this must always be done regardless of the course
}

/**
 * \brief push the 'local_name' onto the stack
 */
void save_local()
{
	t_loc[local_nr].name = local_name;

	++local_nr;

	if (local_nr >= t_loc.size())
		t_loc.resize(local_nr + 1);
}

/**
 * \brief 
 * \param a Name of the construct that is closing
 */
void get_local(string &a)
{
	if (local_nr == 0)
		show_error(a, ERROR_MISSING_DOT_LOCAL);
	--local_nr;

	local_name = t_loc[local_nr].name;
}

/**
 * \brief Save the current address
 */
void new_DOS_header()
{
	BYTE old_opt = opt;
	opt = opt | opt_H;

	// we force writing the DOS header
	save_hea();

	org = true;
	opt = old_opt;
}

/**
 * \brief	mode (true)  reading the number of elements
			mode (false) checking the range for the number of elements
 * \param i 
 * \param zm 
 * \param idx 
 * \param mode 
 * \return 
 */
int read_elements(int &i, string &zm, const int idx, const bool mode)
{
	int result = 0;
	string txt;

	int cnt = 0;
	skip_spaces(i, zm);
	while (!test_char(i, zm, '.', ':'))
	{
		// reading the number of elements [] [] [] ...

		if (zm[i] == '[')
			txt = bounded_string(i, zm, false);
		else
			break;

		int ofset = static_cast<int>(calculate_value(txt, zm));

		if (mode)
		{
			// saving the number of array elements
			int k = t_arr[idx].elm.size();

			t_arr[idx].elm[k].count = ofset;
			t_arr[idx].elm.resize(k + 2);
		}
		else
		{
			// checking the number of elements, calculating the address

			subrange_bounds(zm, ofset, t_arr[idx].elm[cnt].count - 1);
			result = result + ofset * t_arr[idx].elm[cnt].mult;
		}

		++cnt;
		skip_spaces(i, zm);
	}
	return result;
}

/**
 * \brief Save the lines of the listing after assembly to a .LST file
 * \param a line of text
 */
void put_lst(const string& a)
{
	if (pass == pass_end && !a.empty())
	{
		if (run_macro)
		{
			// do not save the macro content if OPT bit5 is cleared
			if ((opt & opt_M) == 0 && !data_out)
				return;
		}

		if ((opt & opt_L) && !FOX_ripit)
		{
			if (not first_lst)
			{
				// Open the dotListFile for writing
				dotListFile.open(filenameLST, ofstream::trunc);
				if (dotListFile.fail())
				{
					cerr << "Unable to open " << filenameLST << " for writing!" << endl;
					exit(1);
				}
				first_lst = true;
			}

			if (!lst_header.empty())
			{
				dotListFile << lst_header << '\n';
				lst_header.erase();
			}

			if (!lst_string.empty())
			{
				dotListFile << lst_string << '\n';
				lst_string.erase();
			}

			dotListFile << a << '\n';

		}

		if ((opt & opt_S) and not(FOX_ripit))
			cout << a << '\n';

		if (not FOX_ripit)
			t.erase();
	}
}

/**
 * \brief additional forcing to save the listing line to the .LST file
 * \param a 
 */
void force_saving_lst(string &a)
{
	if (pass == pass_end)
	{
		save_lst(' ');
		justify();
		put_lst(t + a);
	}
}

/**
 * \brief forcing the listing line to be saved to a.LST file
 * \param a 
 */
void zapisz_lst(string& a)
{
	if (pass == pass_end)
	{
		justify();
		put_lst(t + a);
		a.erase();
	}

}

/**
 * \brief 
 * \param i 
 * \param zm 
 * \param ety 
 * \param a 
 */
void get_array(int &i, string &zm, string &ety, const int a)
{
	save_lab(ety, array_idx, __id_array, zm);
	save_lst('i');

	save_arr(a, bank);                            // here ARRAY_IDX is incremented by 1

	skip_spaces(i, zm);                            // from this point on we use ARRAY_IDX-1

	t_arr[array_idx - 1].elm.setLength(1);         // an array with the consecutive numbers of array elements

	read_elements(i, zm, array_idx - 1, true);       // determining the number of array elements

	array_used.max  = 0;

	int r = get_type(i, zm, zm, false);
	if (r == 0) r = 1;                             // default type .BYTE

	t_arr[array_idx - 1].siz = r;                  // data type .BYTE, .WORD etc.

	if (t_arr[array_idx - 1].elm.size() == 0)
	{
		// no array size
		int idx = 0xFFFF;
		t_arr[array_idx - 1].elm[0].count = idx / r;

		t_arr[array_idx - 1].elm[0].mult = r;

		t_arr[array_idx - 1].isDefined = false;
	}
	else
	{
		// specific array size
		t_arr[array_idx - 1].isDefined = true;
	}


	int _odd = t_arr[array_idx - 1].elm[0].count;        // first number of elements

	for (int k = 1; k <= t_arr[array_idx - 1].elm.size()-1; ++k)
		_odd = _odd * t_arr[array_idx - 1].elm[k].count;

	t_arr[array_idx - 1].len = _odd * r;          // total length in bytes


	_odd = 1;                                     // multiplier for subsequent columns
	for (int k = t_arr[array_idx - 1].elm.size() - 1; k >= 0; --k)
	{
		t_arr[array_idx - 1].elm[k].mult = _odd * r;

		_odd = _odd * t_arr[array_idx - 1].elm[k].count;
	}

	if (t_arr[array_idx - 1].len > 0xFFFF)
		show_error(zm, ERROR_CONSTANT_EXP_SUBRANGE_BOUND);

	array_used.typ = tType[t_arr[array_idx - 1].siz - 1];
}

/**
 * \brief saving the data values ​​created by the structure
			to maintain relocation, save via calculate_data
 * \param war 
 * \param ile 
 * \param typ 
 * \param old 
 */
void save_dtaS(string &in_war, int ile, BYTE typ, string &old)
{
	string war = in_war;
	if (!war.empty() and (war[0] == '\'' || war[0] == '"'))
	{
		// init STRUCT by STRING

		const char ch = war[0];

		ile = ile * typ;

		if (war.length() - 2 > ile)
			show_error(old, MSG_STRING_ERROR, war);

		int k = 0;
		war = get_string(k, war, old, true);

		typ = 1;

		for (k = 0; k < war.length() - 1; ++k)
		{
			string txt;
			txt += ch;
			txt += war[k];
			txt += ch;
			int j = 0;
			calculate_data(j, txt, old, typ);

			dec(ile);
		}

		war = '0';
	}

	while (ile > 0)
	{
		int i = 0;
		calculate_data(i, war, old, typ);

		war = "0";                       // if :rept occurred, generate zeros (e.g. label :cnt .word)

		dec(ile);
	}
}

/**
 * \brief 
 * \param i 
 * \param zm 
 */
void __next(int &i, string &zm)
{
	skip_spaces(i, zm);
	if (zm[i] == ',')
		IncAndSkip(i, zm);
	skip_spaces(i, zm);
}

/**
 * \brief if there is a '=' sign, we omit the spaces before and after the '=' sign
			we will remember all labels (parameters) in the dynamic array   
 * \param j 
 * \param str 
 * \param par 
 * \param mae 
 * \param sep1 
 * \param sep2 
 */
void get_parameters(int &j, string &str, _strArray &par, const bool mae, const char sep1 = '.', const char sep2 = ':')
{
	par.resize(1);

	if (str.empty()) return;

	skip_spaces(j, str);

	while (!test_char(j, str, sep1, sep2))
	{
		string txt = get_dat(j, str, ',', true);   // TRUE - ends when it encounters white space

		int i = static_cast<int>(txt.length())-1;                 // exception if it read a terminated string
		// '=' sign, e.g. 'label='
		if (i > 0 && txt[i] == '=')
		{
			txt.resize(i);
			--j;
		}

		i = j;

		skip_spaces(j, str);

		if (str[j] == '=')
		{
			// '=' new value for the label
			IncAndSkip(j, str);
			txt += '=';
			txt += get_dat(j, str, ',', true);
		}
		else
		{
			j = i;                        // if it didn't read the '=' sign
		}

		//   if txt<>'' then begin        // !!! otherwise it will not omit PROC parameters!!!
		// np. proc_label ,1
		i = static_cast<int>(par.size())-1;
		par[i] = txt;
		par.resize(i + 2);

		if (str[j] == ',' || str[j] == ' ' || str[j] == '\t')
			__next(j, str);
		else if (mae && test_char(j, str))
			break;
	}
}

/**
 * \brief 
 * \param k 
 * \param a 
 * \param txt 
 * \param num 
 */
void get_define_param(int &k, string &a, string &txt, const int num)
{
	skip_spaces(k, a);

	if (AllowStringBrackets.has(a[k]))
	{
		string tmp = bounded_string(k, a, true);

		int idx = 0;
		_strArray par;
		get_parameters(idx, tmp, par, false);

		for (idx = 0; idx <= par.size(); ++idx)
		{
			if (idx == 0)
			{
				while (pos("%%0", txt) >= 0)
				{
					const int j = pos("%%0", txt);
					myDelete(txt, j, 3);
					myInsert (IntToStr(static_cast<int>(par.size())).c_str(), txt, j);
				}

				while (pos(":0", txt) >= 0)
				{
					const int j = pos(":0", txt);
					myDelete(txt, j, 2);
					myInsert(IntToStr(static_cast<int>(par.size())).c_str(), txt, j);
				}
			}
			else
			{

				while (pos("%%" + IntToStr(idx), txt) >= 0)
				{
					const int j = pos("%%" + IntToStr(idx), txt);
					myDelete(txt, j, 3);
					myInsert(par[idx - 1].c_str(), txt, j);
				}

				while (pos(':' + IntToStr(idx), txt) >= 0)
				{
					const int j = pos(':' + IntToStr(idx), txt);
					myDelete(txt, j, 2);
					myInsert(par[idx - 1].c_str(), txt, j);
				}

			}
		}
		if (num != par.size()) 
			show_error(a, WARN_IMPROPER_NR_OF_PARAMS);
	}
	else
		show_error(a, ERROR_NEED_PARENTHESIS);
}

/**
 * \brief 
 * \param i 
 * \param arg 
 * \param a 
 * \return 
 */
string get_define(int &i, int &arg, string &a)
{
	int k = i;

	string txt = t_mac[t_lab[arg].addr + 2];

	if (t_mac[t_lab[arg].addr + 1].empty() == false)
	{
		get_define_param(k, a, txt, static_cast<int>(StrToInt(t_mac[t_lab[arg].addr + 1])));

		myDelete(a, i, k - i);
	}

	return txt;
}

/**
 * \brief	Calculate the value of the expression, taking into account arithmetic operations
			in 'J' there is a counter for STACK
			instead of dynamic arrays, I introduced static arrays and limited them
			maximum number of operations and operators in an expression up to 512
 * \param a 
 * \param old 
 * \return 
 */
long long calculate_value(string& a, string& old)
{
	int     i, j, b, x, k, v, pomoc, ofset, _hlp, len, op_idx, arg;
	int     old_reloc_value_cnt;
	string  tmp, txt;
	long long  iarg, war;
	bool    isLoop, value, old_reloc_value_use;
	char    oper, byt;

	relocateValue   reloc_value;

	_strArray stos_;
	char cash[17] = {}; // same number of elements as PRIOR
	signValueEntry stos[512] = {};
	char op[512] = {};

	long long Result;

	//const 
	//    prior:   array [0..16] of char =          // operator priority
	//        ('D','E','&','|','^','/','*','%','+',{'-',}'=','A','B','<','C','>','F','G');

	if (a.empty())
		show_error(old, ERROR_UNEXPECTED_EOL);

LOOP:
	Result = 0;

	// init dynamic array OP
	// SetLength(op,2);
	op[0] = '+';
	oper = '+';
	op_idx = 1;	

	i = 1-1;
	war = 0;
	b = 0;

	memset(&cash[0], ' ', sizeof(cash));	// we fill with spaces

	value = false;

	reloc_value.use = false;
	reloc_value.count = 0;

	j = 1;               // at the zero position (J=0) of the 'STACK' array we will add '+0'
	// !!! variables I,J are used by WHILE loops!!!

	// SetLength(stos,3);

	len = length(a);

	bool breakProcessing = false;
	while (breakProcessing == false and (i < len) and not(overflow))
	{
		switch (UpCase(a[i]))
		{
			case '#':
			{
				if (_rept_ile < 0)
					show_error(old, ERROR_HASH_ONLY_IN_REPEATED_LINES);

				if (value)
					show_error(old, ERROR_EXTRA_CHARS_ON_LINE);

				war = _rept_ile;
				value = true;
				IncAndSkip(i, a);

				break;
			}

			// read operator '<'
			case '<':
			{
				if (a[i + 1] == '<' or a[i + 1] == '=' or a[i + 1] == '>')
					oper = OperExt(i, old, '<', a[i + 1], value);
				else if (value)
					oper = OperNew(i, old, '<', value, true);
				else
					oper = OperNew(i, old, 'M', value, false);

				break;
			}

			// read operator '>'
			case '>':
			{
				if (a[i + 1] == '=' or a[i + 1] == '>')
					oper = OperExt(i, old, '>', a[i + 1], value);
				else if (value)
					oper = OperNew(i, old, '>', value, true);
				else
					oper = OperNew(i, old, 'S', value, false);
				break;
			}

			// read operator '='
			case '=':
			{
				if (a[i + 1] == '=')
					oper = OperExt(i, old, '=', '=', value);
				else if (value)
					oper = OperNew(i, old, '=', value, true);
				else
					oper = OperNew(i, old, 'X', value, false);                   // reading the bank number assigned to the label =label
				break;
			}

			// read operator '&'
			case '&':
			{
				if (a[i + 1] == '&')
					oper = OperExt(i, old, '&', '&', value);
				else
					oper = OperNew(i, old, '&', value, true);
				break;
			}

			// read operator '|'
			case '|':
			{
				if (a[i + 1] == '|')
					oper = OperExt(i, old, '|', '|', value);
				else
					oper = OperNew(i, old, '|', value, true);
				break;
			}

			// read operator '!'
			case '!':
			{
				if (a[i + 1] == '=')
					oper = OperExt(i, old, '!', '=', value);
				else
					oper = OperNew(i, old, '!', value, false);
				break;
			}

			// read operator '^'
			case '^':
			{
				if (not(value))
				{
					oper = 'H';
					inc(i);
				}
				else
					oper = OperNew(i, old, '^', value, true);
				break;
			}

			// read operator '/'
			case '/':
			{
				if (a[i + 1] == '*')
				{
					skip_spaces(i, a);
					value = false;
				}
				else
					oper = OperNew(i, old, '/', value, true);
				break;
			}

			// read operator '*'
			case '*':
			{
				if (not(value))
				{
					b = bank;
					war = addres;
					value = true;

					inc(i);

					label_type = 'V';

					if (not(branch))
					{
						if (dotRELOC.sdx)
							save_rel(addres, -1, blok, reloc_value);
						else
							save_rel(addres, -1, bank, reloc_value);
					}
				}
				else
					oper = OperNew(i, old, '*', value, true);
				break;
			}

			// read operator '~'
			case '~':
			{
				oper = OperNew(i, old, '~', value, false);
				break;
			}

			// read operator '+' '-'
			case '+':
			case '-':
			{
				oper = OperNew(i, old, a[i], value, value);
				break;
			}

			// read the decimal value or exceptionally hex 0x...
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			{
				if (value or ReadEnum)
					show_error(old, ERROR_EXTRA_CHARS_ON_LINE);

				if ((i < length(a)) and (UpCase(a[i + 1]) == 'X') and (a[i] == '0'))
				{
					// 0x...
					inc(i);
					tmp = read_HEX(i, a, old);
				}
				else 
				{
					// 0..9
					tmp = read_DEC(i, a);
				}

				war = StrToInt(tmp);

				value = true;
				break;
			}

			case '.':
			{
				// based on the TDIROP enumeration type
				// the same order in the HASH calculation program
				// !!! we calculate CRC16 for subsequent characters, we finish when the code is in the TDIROP range!!!
				k = ord(t_Dirop(_unknown));
				tmp = '.';
				inc(i);

				x = tCRC16[byte(0xffff >> 8) ^ byte('.')] ^ (0xffff << 8);

				while (_alpha(a[i]) and (i <= length(a)))
				{
					byt = UpCase(a[i]);    // !! uppercase !! because it won't work with the -c switch
					x = tCRC16[byte(x >> 8) ^ byte(byt)] ^ (x << 8);

					tmp += byt;
					inc(i);

					k = ::hash[(tCRC16[byte(x >> 8) ^ byte('.')] ^ (x << 8)) & 0xffff];

					if (AllowedDotOperators.has(k)) // DELPHI: if (k in [ord(_or)..ord(_array)])
					{
						break;
					}
				}

				skip_spaces(i, a);

				switch (t_Dirop(k))
				{

					case _r:
					{
						// .R
						if (_rept_ile < 0)
							show_error(old, ERROR_HASH_ONLY_IN_REPEATED_LINES);

						war = _rept_ile;
						if (not(loop_used) and not(FOX_ripit))
							t = " #" + Hex(cardinal(war), 2);    // save #number in LST file

						value = true;
						break;
					}

					case _lo:
					{
						// .LO (expression)
						old_reloc_value_cnt = reloc_value.count;
						old_reloc_value_use = reloc_value.use;

						war = byte(calculate_value_ogr(a, old, i));

						reloc_value.use = old_reloc_value_use;
						inc(reloc_value.count, old_reloc_value_cnt);

						value = true;

						break;
					}

					case _hi:
					{
						// .HI (expression)
						old_reloc_value_cnt = reloc_value.count;
						old_reloc_value_use = reloc_value.use;

						war = byte(calculate_value_ogr(a, old, i) >> 8);

						reloc_value.use = old_reloc_value_use;
						inc(reloc_value.count, old_reloc_value_cnt);

						value = true;

						break;
					}

					case _rnd:
					{
						// .RND
						//randomize;
						war = random(256);
						value = true;
						break;
					}

					case _asize:      // .ASIZE
					{
						war = asize;
						value = true;
						break;
					}

					case _isize:      // .ISIZE
					{
						war = isize;
						value = true;
						break;
					}

					case _get:
					case _wget:
					case _lget:
					case _dget:
					{
						// .GET, .WGET, .LGET, .DGET
						arg = 0;

						old_reloc_value_cnt = reloc_value.count;
						old_reloc_value_use = reloc_value.use;

						if (AllowStringBrackets.has(a[i]))
							arg = integer(calculate_value_ogr(a, old, i));

						subrange_bounds(old, arg, 0xFFFF);         // !!! necessary test

						reloc_value.use = old_reloc_value_use;
						inc(reloc_value.count, old_reloc_value_cnt);

						switch (t_Dirop(k))
						{
							case _get:
								war = t_get[arg];
								break;
							case _wget:
								war = Int64(t_get[arg]) + Int64(t_get[arg + 1] << 8);
								break;
							case _lget:
								war = Int64(t_get[arg]) + Int64(t_get[arg + 1] << 8) + Int64(t_get[arg + 2] << 16);
								break;
							case _dget:
								war = Int64(t_get[arg]) + Int64(t_get[arg + 1] << 8) + Int64(t_get[arg + 2] << 16) + Int64(t_get[arg + 3] << 24);
								break;
						}

						value = true;
						break;
					}

					case _or:
						oper = OperExt(k, old, '|', '|', value);      // .OR
						break;

					case _and:
						oper = OperExt(k, old, '&', '&', value);      // .AND
						break;

					case _xor:
						oper = OperNew(k, old, '^', value, true);     // .XOR
						break;

					case _not:
						oper = OperNew(k, old, '!', value, false);    // .NOT
						break;

					case _fileexists:
						{
							// .FILEEXISTS
							if (AllowStringBrackets.has(a[i]))
								txt = bounded_string(i, a, true);
							else
								txt = get_datUp(i, a, 0, false);

							k = 1 - 1;

							if (AllowQuotes.has(txt[k]))
							{
								txt = get_string(k, txt, old, true);
								txt = GetFile(txt, a);
							}

							war = TestFile(txt) ? 1 : 0;

							value = true;
							break;
						}

					case _len:
					case _filesize:
					case _sizeof:
					{
						// .LEN
						// we read the name of the label

						k = i;

						if (AllowStringBrackets.has(a[i]))
							txt = bounded_string(i, a, true);
						else
							txt = get_datUp(i, a, 0, false);

						v = 1 - 1;
						war = get_type(v, txt, old, false, false);

						if (not (byte(war) >= 1 and byte(war) <= 4))
						{
							i = k;
							txt = get_labEx(i, a, old);

							k = 1 - 1;
							skip_spaces(k, txt);

							if (AllowQuotes.has(txt[k]))
							{
								// .FILESIZE
								txt = get_string(k, txt, old, true);

								txt = GetFile(txt, a);
								if (not(TestFile(txt)))
									show_error(txt, MSG_CANT_OPEN_CREATE_FILE);

								//assignfile(fsize, txt);
								//FileMode = 0;
								//Reset(fsize, 1);
								war = FileSize(txt);
								//CloseFile(fsize);
							}
							else
							{
								arg = load_label_ofset(txt, old, true);

								if (arg >= 0)
								{
									switch (t_lab[arg].bank)
									{
										case __id_proc:
											war = t_prc[t_lab[arg].addr].len;            // block length .PROC
											break;
										case __id_array:
											war = t_arr[t_lab[arg].addr].len;            // length of the .ARRAY block
											break;
										case __id_enum:
											war = t_lab[arg].addr;                       // size of .ENUM in bytes
											break;
										case __id_struct:
											war = t_str[t_lab[arg].addr].mySize;            // length of the .STRUCT block
											break;
										case __dta_struct:
											war = t_str[t_str[t_lab[arg].addr].idx].mySize * // the size of the data defined by DTA LABEL_STRUCT
												(Int64(t_str[t_lab[arg].addr].offset) + 1);
											break;
										default:
										{
											war = t_lab[arg].lln;                        // length of the .LOCAL block

											if (var_idx > 0)
											{
												for (k = static_cast<int>(t_var.size()) - 1; k >= 0; --k)
												{
													if (t_var[k].name == txt)
													{
														war = t_var[k].mySize;
														break;
													}
												}
											}
											break;
										}
									}
								}
							}
						}

						value = true;
						break;
					}

					case _adr:
					{
						// .ADR label
						// we read the name of the label
						txt = get_labEx(i, a, old);

						arg = load_label_ofset(txt, old, true);

						war = 0;

						if (arg >= 0)
						{
							old_reloc_value_cnt = reloc_value.count;
							old_reloc_value_use = reloc_value.use;

							war = calculate_value(txt, old);

							reloc_value.use = old_reloc_value_use;
							inc(reloc_value.count, old_reloc_value_cnt);

							switch (t_lab[arg].bank)
							{
								case __id_proc:
									dec(war, t_prc[t_lab[arg].addr].offset);
									break;
								default:
									dec(war, t_lab[arg].offset);
								break;
							}
						}

						value = true;
						break;
					}

					case _def:
					{
						// .DEF label
						// we read the name of the label
						txt = get_labEx(i, a, old);
						arg = load_label_ofset(txt, old, false);

						war = arg >= 0 ? 1 : 0; // !!! doesn't work properly for .IFNDEF from Exomizer !!!

						if (exclude_proc and (arg >= 0) and (t_lab[arg].bank == __id_proc))
							war = ord(t_lab[arg].use);
						else
							if (pos('.', txt) >= 0)
							{
								// check if the label does not belong to a procedure
								while (pos('.', txt) >= 0)
								{
									cut_dot(txt);
									k = length(txt);    // remove the last dot character
									SetLength(txt, k - 1);
								}

								arg = load_label_ofset(txt, old, false);

								if (exclude_proc and (arg >= 0) and (t_lab[arg].bank == __id_proc))
									war = ord(t_lab[arg].use);
							}
						value = true;
						break;
					}

					case _zpvar:
					{
						war = zpvar;
						value = true;
						break;
					}

					case _array:
					{
						war = 0;

						if (etyArray.empty())
							show_error(old, ERROR_LABEL_NAME_REQUIRED);
						else if (value)
							get_array(i, a, etyArray, 0);
						else
						{
							get_array(i, a, etyArray, addres);
							war = t_arr[array_idx - 1].len;
						}

						etyArray.erase();
						value = true;
						break;
					}

					default:
						error_und(a, tmp, WARN_UNKNOWN_DIRECTIVE);
						break;
				}

				break;
			}

			// read the label and determine its value
			// the ':' character denotes a global label or macro parameter
			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
			case 'G':
			case 'H':
			case 'I':
			case 'J':
			case 'K':
			case 'L':
			case 'M':
			case 'N':
			case 'O':
			case 'P':
			case 'Q':
			case 'R':
			case 'S':
			case 'T':
			case 'U':
			case 'V':
			case 'W':
			case 'X':
			case 'Y':
			case 'Z':
			case '_':
			case '?':
			case '@':
			case ':':
			{
				isLoop = false;
				b = 0;
				war = 0;  //pomoc=0;

				if (a[i] == ':')
				{
					if (_lab_first(a[i + 1]))
					{
						isLoop = true;
						inc(i);
					}
					else if (_dec(a[i + 1]))
					{
						if (run_macro)
						{
							value = false;
							oper = ' ';
							//Break; // i=len+1
							breakProcessing = true;
							continue;
						}
						else
						{
							show_error(old, WARN_ILLEGAL_CHARACTER, ':');
						}
					}
					else
						show_error(old, ERROR_EXTRA_CHARS_ON_LINE);
				}
				if (i < len)
				{
					if (value)
						show_error(old, ERROR_EXTRA_CHARS_ON_LINE);

					tmp = get_lab(i, a, false);
					if (isLoop)
						tmp = ":" + tmp;

					if (tmp == "@")                    // Anonymous labels @+[1..9] (forward), @-[1..9] backward
					{
						switch (a[i])
						{
							case '-':
							{
								k = anonymous_idx - 1;
								inc(i);

								if (_dec(a[i]))
								{
									k = k - static_cast<int>(StrToInt(a[i]));
									inc(i);
								}

								test_eol(i, a, a, 0);
								tmp = IntToStr(k) + "@";
								break;
							}

							case '+':
							{
								k = anonymous_idx;
								inc(i);

								if (_dec(a[i]))
								{
									k = k + static_cast<int>(StrToInt(a[i]));
									inc(i);
								}

								test_eol(i, a, a, 0);
								tmp = IntToStr(k) + "@";
								break;
							}
						}
					}

					arg = load_label_ofset(tmp, old, true);

					if (arg >= 0)
					{
						if (t_lab[arg].lid and (t_lab[arg].add > static_cast<unsigned int>(pass << 1)) and (t_lab[arg].pass == pass_end))
						{
							if (run_macro)
								txt = macro_nr + local_name;
							else   // !!! do not remote LOKAL_NAME !!!
								if (proc)
									txt = proc_name + local_name;
								else
									txt = local_name;

							if (txt != tmp + ".")
								warning(WARN_AMBIGUOUS_LABEL, tmp);   // ambiguous labels LOCAL!!! necessarily t_lab[x].pass
						}
					}

					// if we are processing a macro, label values ??do not need to be defined
					// in other cases the error 'Undeclared label ????'

					if (arg < 0)
					{
						undeclared = true;               // an undefined label occurred
						value = false;
						oper = ' ';
						// we stop evaluating the expression
						breakProcessing = true;
						continue;
					}
					else if ((pass == pass_end) and (t_lab[arg].bank < __id_param) and t_lab[arg].sts)
						error_und(old, tmp, WARN_REFERENCED_LABEL_NOT_DEFINED);

					undeclared = t_lab[arg].sts;      // current label status

					pomoc = t_lab[arg].addr;

					if ((t_lab[arg].theType == 'P' or t_lab[arg].theType == 'V'))
						variable = true;

					t_lab[arg].use = true;

					if (attribute.attrib == __U)
					{
						attribute.attrib = t_lab[arg].attrib;
						attribute.name = tmp;
					}

					if (arg >= 0)
					{
						switch (t_lab[arg].bank)
						{

							case __id_struct:
							{
								// converting structures into DTA data, first we will check whether 'DTA_USED = true'

								if (not(dta_used))
								{
									b = t_str[pomoc].bank;
									war = t_str[pomoc].mySize;

									if (pass == pass_end)
										show_error(old, ERROR_COULD_NOT_USE_X_IN_CONTEXT, tmp);

								}
								else if (pass == 0)
								{
									// now we don't know all the structures, we have to finish
									if (a[i] != '[')
										warning(WARN_CONSTANT_EXPR_EXPECTED); // 'Constant expression expected', index is missing [idx]
								}
								else
								{
									// in the second pass we are sure that we have learned all the structures
									dta_used = false;

									struct_used.use = true;

									save_lst('i');

									// we read the offset into the 'T_STR' array
									ofset = t_lab[arg].addr;

									struct_used.idx = ofset;     // in OFSET, the index to the first field of the structure

									// number of fields of the structure
									b = t_str[ofset].offset;

									inc(ofset);
									txt.erase();

									// we read the declared number of structured data elements [?]
									arg = integer(calculate_value_ogr(a, old, i));

									testRange(old, arg, 62);

									struct_used.count = arg;

									skip_spaces(i, a);

									// we read element values ??limited by brackets ( )
									while (a[i] == '(')
									{
										tmp = bounded_string(i, a, true);

										k = 1 - 1;
										tmp = get_dat_noSPC(k, tmp, old, 0);
										k = 1 - 1;
										SetLength(stos_, 1);

										// we load elements limited by brackets, there is a counter in 'X'
										x = 0;
										while (k < length(tmp))
										{
											pomoc = loa_str_no(t_str[ofset].id, x);  // always has to find the right structure field

											v = t_str[pomoc].mySize;

											//        byt=tType[v];
											//        war=___wartosc_noSPC(tmp,old,k,',',byt);

											txt = get_dat(k, tmp, ',', false);
											save_dtaS(txt, t_str[pomoc].nrRepeat, v, old);

											inc(x);
											//        if (pass>0)
											if (x > b)
												show_error(old, WARN_IMPROPER_NR_OF_PARAMS);   // if the number of given elements is greater than in the structure

											pomoc = static_cast<int>(stos_.size())-1;
											stos_[pomoc] = txt;
											stos_.resize(pomoc + 2);

											skip_spaces(k, tmp);
											if (tmp[k] == ',')
												IncAndSkip(k, tmp);
											else
												break;
										}

										//       if (pass>0)
										if (x != b)
											show_error(old, WARN_IMPROPER_NR_OF_PARAMS);

										// decrement the element counter
										dec(arg);
										if (arg < -1)
											show_error(old, WARN_IMPROPER_NR_OF_PARAMS);

										skip_spaces(i, a);
									}

									skip_spaces(i, a);

									value = (stos_.size()-1 == b) ? true : false;   // if we have provided any initial values, then VALUE=TRUE

									if (not(dotRELOC.use or dotRELOC.sdx))
									{
										if (not(value))
											new_DOS_header();  // if we did not provide a value, we must force ORG
									}

									// !!! zeros cannot be written to a structure variable by default !!!
									// !!! because it is not always possible to save memory at the current address !!!

									// we fill the rest of the structure elements with the last values ??or zeros
									while (arg >= 0)
									{
										for (x = 0; x < b; ++x)
										{
											pomoc = loa_str_no(t_str[ofset].id, x);

											v = t_str[pomoc].mySize;   //byt=tType[v];

											_hlp = t_str[pomoc].nrRepeat * v;

											if (value)
											{
												save_dtaS(stos_[x], t_str[pomoc].nrRepeat, v, old);// new address and saving the initial value of the structure
											}
											else if (dotRELOC.use or dotRELOC.sdx)
											{
												string it = "0";
												save_dtaS(it, t_str[pomoc].nrRepeat, v, old);
											}
											else
											{
												war = 0;
												inc(addres, _hlp);              // new address without saving the initial value
											}
										}
										dec(arg);
									}

									b = bank;
									value = false;
									oper = ' ';
								}
								break;
							}

							case __id_define:
							{
								txt = get_define(i, arg, a);

								dec(i, length(tmp));

								myDelete(a, i, length(tmp));
								myInsert(txt, a, i);

								goto LOOP;
								// break; 
							}

							case __id_macro:
							{
								show_error(old, ERROR_COULD_NOT_USE_X_IN_CONTEXT, tmp);
								break;
							}

							case __id_enum:
							{
								skip_spaces(i, a);

								if (AllowStringBrackets.has(a[i]))
								{
									_hlp = usi_idx;                 // for enum_name(label1|label2|...)

									t_usi[usi_idx].location = end_idx;
									t_usi[usi_idx].myLabel = tmp;

									if (proc)                   // we will add .USE [.USING] to the list
										t_usi[usi_idx].name = proc_name;
									else
									{
										t_usi[usi_idx].name = local_name;
									}

									inc(usi_idx);
									t_usi.resize(usi_idx + 1);

									ReadEnum = true;

									war = calculate_value_ogr(a, old, i);

									ReadEnum = false;

									usi_idx = _hlp;

								}
								else
								{
									show_error(old, ERROR_COULD_NOT_USE_X_IN_CONTEXT, tmp);
								}
								break;
							}

							case __id_proc:
							{
								b = t_prc[pomoc].bank;
								war = t_prc[pomoc].addr;

								if (ExProcTest)
									t_prc[pomoc].used = true;  // the .PROC procedure must be assembled

								save_relAddress(arg, reloc_value);
								break;
							}

							case __id_ext:
							{
								if (blocked)
									error_und(old, tmp, WARN_INCOMPATIBLE_TYPES);

								if (not(branch))
								{
									if (isVector)
										_hlp = addres;
									else
										_hlp = addres + 1;

									if (dotRELOC.use)
										dec(_hlp, rel_ofs);

									t_ext[ext_idx].addr = _hlp;
									t_ext[ext_idx].bank = bank;
									t_ext[ext_idx].idx = pomoc;

									inc(ext_idx);

									if (ext_idx >= t_ext.size())
										t_ext.resize(ext_idx + 1);
								}

								ext_used.use = true;
								ext_used.idx = pomoc;
								ext_used.mySize = t_extn[pomoc].mySize;

								inc(reloc_value.count);

								break;
							}

							case __id_smb:
							{
								if (blocked)
									error_und(old, tmp, WARN_INCOMPATIBLE_TYPES);

								if (not(branch))
								{
									save_rel(addres, pomoc, 0, reloc_value);
									t_smb[pomoc].used = true;
									war = __rel;
								}
								break;
							}

							case __id_array:
							{
								b = t_arr[pomoc].bank;
								war = t_arr[pomoc].addr;

								war = war + read_elements(i, a, pomoc, false);

								save_relAddress(arg, reloc_value);

								break;
							}

							case __dta_struct:
							{
								save_relAddress(arg, reloc_value);

								b = t_str[pomoc].bank;
								war = t_str[pomoc].addr;

								ofset = integer(calculate_value_ogr(a, old, i));

								subrange_bounds(old, ofset, t_str[pomoc].offset);

								arg = t_str[t_lab[arg].addr].idx;

								ofset = ofset * t_str[arg].mySize;   // index * structure_length

								if (a[i] == '.')
								{
									inc(i);
									txt = get_lab(i, a, false);

									// we are looking in T_STR
									pomoc = loa_str(txt, t_str[arg].id);

									if (pomoc >= 0)
										inc(ofset, t_str[pomoc].offset);
									else if (pass == pass_end)
										error_und(old, txt, WARN_BAD_PARAMETER_TYPE);
								}

								inc(war, ofset);
								break;
							}

							default:
							{
								// 'Address relocation overload' error occurs
								// if we perform operations on more than one relocatable label

								save_relAddress(arg, reloc_value);

								b = t_lab[arg].bank;    // bank assigned to the label

								if (dotRELOC.use or dotRELOC.sdx)
								{
									b = t_lab[arg].block;   // block assigned to the label
									if (reloc_value.use and (reloc_value.count > 1))
										show_error(old, ERROR_ADDR_RELOCATION_OVERLOAD);
								}

								war = t_lab[arg].addr;

								while (pos('.', tmp) >= 0)
								{
									cut_dot(tmp);

									k = length(tmp);    // remove the last dot character
									SetLength(tmp, k - 1);

									arg = l_lab(tmp);

									if (arg < 0)
										arg = l_lab(local_name + tmp);

									if (arg >= 0)
										pomoc = t_lab[arg].addr;

									if ((arg >= 0) and ExProcTest)
									{
										if (t_lab[arg].bank == __id_proc)
										{
											t_prc[pomoc].used = true;   // the .PROC procedure must be assembled
											break;
										}
									}
								}

								break;
							}
						}
					}

					value = true;
				}

				// exceptionally for PASS=0 and a probable reference to the array, assume the value =0
				skip_spaces(i, a);
				if (a[i] == '[')
				{
					if (pass < pass_end)
						return Result;
					else
						error_und(old, tmp, WARN_UNDECLARED_LABEL);
				}

				break;
			}

			case '$':
			{
				// read hexadecimal value
				if (value or ReadEnum)
					show_error(old, ERROR_EXTRA_CHARS_ON_LINE);

				tmp = read_HEX(i, a, old);

				war = StrToInt(tmp);

				value = true;
				break;
			}

			// read a binary value or treat '%' as an operator
			// or as a parameter number for the macro when '%%' characters appear
			case '%':
			{
				if (a[i + 1] == '%')
				{
					value = false;
					oper = ' ';
					inc(i, 2);
				}
				else if (value)
				{
					oper = OperNew(i, old, '%', value, true);
				}
				else
				{
					inc(i);     // omit first character '%'
					//           war=get_value(i,a,'B',old);

					tmp.erase();
					while (_bin(a[i]))
					{
						tmp += a[i];
						IncAndSkip(i, a);
					}

					if (not(test_param(i, a)))
					{
						if (tmp.empty())
							show_error(old, WARN_ILLEGAL_CHARACTER, a[i]);
					}

					if (ReadEnum)
						show_error(old, ERROR_EXTRA_CHARS_ON_LINE);

					war = 0;

					//----------------------------------------------------------------------------*)
					//  implementation of converting a zero-one sequence into a decimal value     *)
					//----------------------------------------------------------------------------*)
					if (tmp.empty() == false)
					{
						k = static_cast<int>(tmp.length()) - 1;

						// remove leading zeros
						pomoc = 1 - 1;
						while (tmp[pomoc] == '0')
							inc(pomoc);

						// do the conversion
						for (ofset = k; ofset >= pomoc; --ofset)
						{
							if (tmp[ofset] == '1')
							{
								war = war + (1 << (k - ofset));
							}
						}
					}

					value = true;
				}

				break;
			}

			case '\'':
			case '"':
			{
				// read string delimited by single quotes '',""
				if (value)
					show_error(old, ERROR_EXTRA_CHARS_ON_LINE);

				k = ord(a[i]);
				tmp = get_string(i, a, old, true);

				if (length(tmp) > 2)
					show_error(old, MSG_STRING_ERROR);                // maximum 1 character, or 2 characters for 65816

				if (chr(k) == '\'')
					war = ord(tmp[1 - 1]);
				else
					war = ata2int(ord(tmp[1 - 1]));

				if (length(tmp) == 2)
				{
					if (chr(k) == '\'')
						war = (war << 8) + ord(tmp[2 - 1]);
					else
						war = (war << 8) + (ata2int(ord(tmp[2 - 1])));
				}

				if (i + 1 < a.length())
				{
					byt = a[i + 1];

					if (not(_lab_firstEx(byt) or _dec(byt) or (byt == '$' or byt == '%')))
					{
						inc(war, test_string(i, a, 'F'));
					}
				}

				value = true;

				break;
			}

			// read value between { }
			case '{':
			{
				if (value)
					show_error(old, ERROR_EXTRA_CHARS_ON_LINE);

				klamra_used = true;

				isLoop = dotRELOC.use;       // we block relocation DRELOC.USE=FALSE
				dotRELOC.use = false;

				tmp = bounded_string(i, a, true);            // test for correctness of brackets
				k = 1 - 1;
				nul = calculate_mnemonic(k, tmp, old);

				dotRELOC.use = isLoop;       // we restore the previous DRELOC.USE value

				klamra_used = false;

				war = nul.h[0];

				nul.l = 0;

				value = true;

				break;
			}

			// read value between [ ] ( )
			case '[':
			case '(':
			{
				if (value)
					show_error(old, ERROR_EXTRA_CHARS_ON_LINE);

				old_reloc_value_cnt = reloc_value.count;
				old_reloc_value_use = reloc_value.use;

				war = calculate_value_ogr(a, old, i);

				reloc_value.use = old_reloc_value_use;
				inc(reloc_value.count, old_reloc_value_cnt);

				value = true;
				break;
			}

			// if you encounter spaces, stop processing
			// put 'false' in 'value' so that it doesn't treat it as a number
			case ' ':
			case 9:
			{
				value = false;
				oper = ' ';
				// Stop processing here
				breakProcessing = true;
				continue;
				//break; 
			}

			default:
			{
				show_error(old, WARN_ILLEGAL_CHARACTER, a[i]);
			}
		}

		// If we are processing a numerical value, save it on the stack
		// this value and operator
		if (value)
		{
			op[op_idx] = '+';

			// process operators from 'OP' backwards
			for (k = op_idx; k > 0; --k)
			{
				switch (op[k])
				{
					case '-':
						war = -war;
						break;
					case 'M':
						war = byte(war);
						break;
					case 'S':
						war = byte(war >> 8);
						break;
					case 'H':
						war = byte(war >> 16);
						break;
						//     'I': war = byte(war >> 24);
					case '!':
						war = (war == 0) ? 1 : 0;
						break;
					case '~':
						war = ~war;
						break;
					case 'X':
						war = b;
						break;
				}
			}

			if (op[0] == '-')
			{
				war = -war;
				op[0] = '+';
			}

			stos[j].znak = op[0];
			stos[j].wart = war;

			int where = posInPriority(op[0]);
			cash[where] = op[0];
			// DELPHI cash[pos(op[0],prior)-1] = op[0];

			// zero index to dynamic array OP
			op_idx = 0;
			oper = ' ';

			inc(j);
			//   if (j>High(stos)) SetLength(stos,j+1);
		}
		else
		{
			op[op_idx] = oper;
			inc(op_idx);

		}
	}

	if (not(overflow))
	{
		if (oper != ' ')
			show_error(old, ERROR_UNEXPECTED_EOL);
	}

	if (reloc_value.count > 1)
		show_error(old, ERROR_ADDR_RELOCATION_OVERLOAD);

	// calculation of values according to calculation priority
	// finally add until you get the result
	len = 0;
	war = 0;

	while (cash[len] == ' ')
		inc(len);     // avoid empty operators

	if (len >= sizeof(cash))
	{
		return Result;  // no operator appeared
	}

	stos[0].znak = '+';
	stos[0].wart = 0;
	stos[j].znak = '+';
	stos[j].wart = 0;
	inc(j);

	while (j > 1)
	{
		// 'isLoop=true' if no operator occurs
		// 'isLoop=false' if the operator occurs
		i = 0;
		isLoop = true;

		while (i <= j - 2)
		{

			war = stos[i].wart;
			oper = stos[i + 1].znak;
			iarg = stos[i + 1].wart;

			// calculate according to the order of operators with 'prior'
			// operators '/','*','%' have the same priority, they are an exception

			if ((oper == cash[len]) or ((oper == '/' or oper == '*' or oper == '%') and (len >= 5 and len <= 7)))
			{
				switch (oper)
				{
					case '+':
						war = war + iarg;
						break;
					case '&':
						war = war & iarg;
						break;
					case '|':
						war = war | iarg;
						break;
					case '^':
						war = war ^ iarg;
						break;
					case '*':
						war = war * iarg;
						break;

					case '/':
					case '%':
					{
						if (iarg == 0)
						{
							overflow = true;

							if (pass == pass_end)
								show_error(old, ERROR_DIVIDE_BY_ZERO);
							else
							{
								war = 0;
							}
						}
						else
						{
							switch (oper)
							{
								case '/':
									war = war / iarg;
									break;
								case '%':
									war = war % iarg;
									break;
							}
						}
						break;
					}

					case '=':   war = (war == iarg) ? 1 : 0; break;
					case 'A':   war = (war != iarg) ? 1 : 0; break;
					case 'B':   war = (war <= iarg) ? 1 : 0; break;
					case '<':   war = (war < iarg) ? 1 : 0; break;
					case 'C':   war = (war >= iarg) ? 1 : 0; break;
					case '>':   war = (war > iarg) ? 1 : 0; break;
					case 'D':   war = war << iarg; break;
					case 'E':   war = war >> iarg; break;
					case 'F':   war = (war != 0 and iarg != 0) ? 1 : 0; break;
					case 'G':   war = (war != 0 or iarg != 0) ? 1 : 0; break;
				}

				//calculated a new value, delete the character

				stos[i].wart = war;
				stos[i + 1].znak = ' ';

				inc(i);
				isLoop = false;
			}

			inc(i);
		}

		// rewrite elements that have a non-empty character to the beginning of the STACK array
		k = 0;

		for (i = 0; i <= j - 1; ++i)
		{
			if (stos[i].znak != ' ')
			{
				stos[k] = stos[i];
				inc(k);
			}
		}

		j = k;

		if (isLoop and (len < sizeof(prior) - 1))
		{
			inc(len);
			while (cash[len] == ' ')
				inc(len);
		}
	}


	if (overflow)          // necessary condition for .MACRO
		Result = 0;
	else
		Result = war;

	return Result;

}

/**
 * \brief Parse the string, omitting spaces, then calculate its value
 * \param zm 
 * \param old 
 * \param i 
 * \param sep 
 * \param typ 
 * \return 
 */
long long calculate_value_noSPC(string &zm, string &old, int &i, const char sep, const char typ)
{
	string txt = get_dat_noSPC(i, zm, old, sep);
	const long long result = calculate_value(txt, old);
	skip_spaces(i, zm);
	wartosc(old, result, typ);
	return result;
}

/**
 * \brief	get a string of characters delimited by a comma and calculate its value
			do not load closing brackets ')'
			if TST = TRUE, check if this is the end of the expression 
 * \param i 
 * \param a 
 * \param old 
 * \param tst 
 * \return 
 */
long long get_expres(int &i, string &a, string &old, const bool tst)
{
	int k = i;
	get_dat(k, a, ')', false);

	string tmp = a;
	tmp.resize(k);

	const long long result = calculate_value_noSPC(tmp, old, i, ',', 'F');

	if (tst)
	{
		if (a[i] != ',') 
			show_error(old, ERROR_BAD_MISSING_SIN_PARAM);
		else
			++i;
	}

	return result;
}

/**
 * \brief 
 * \param prev 
 */
void _siz(const bool prev)
{
	t_siz[size_idx - (prev ? 1 : 0)].mySize = dotRELOC.mySize;
}

/**
 * \brief 
 * \param a 
 */
void _sizm(const BYTE a)
{
	if (reloc) 
		t_siz[size_idx - 1].lsb = a;
}

/**
 * \brief Return numeric values ​​for the string DTA, .BYTE, .WORD, .LONG, .DWORD
			B( -> BYTE                       
			A( -> WORD                       
			V( -> VECTOR (WORD)              
			L( -> BYTE   (bits 0..7)         
			H( -> BYTE   (bits 8..15)        
			M( -> BYTE   (bits 16..23)       
			G( -> BYTE   (bits 24..31)       
			T( -> LONG                       
			E( -> LONG                       
			F( -> DWORD                      
			R( -> DWORD  (reverse order)     
			C' -> ATASCII                    
			D' -> INTERNAL                   
			sin(centre,amp,size[,first,last])
			cos(centre,amp,size[,first,last])
			rnd(min,max)                    
 * \param i 
 * \param a 
 * \param old 
 * \param typ 
 */
void calculate_data(int &i, string &a, string &old, const BYTE typ)
{
	char op_, default_type;
	int sin_a, sin_b, sin_c, sin_d, sin_e, x, len, k;
	long long war;
	BYTE invers;
	bool value, bracket, ciag, yes;
	string tmp;

	skip_spaces(i, a);

	if (::empty || enumeration.use)
		show_error(old, ERROR_IMPROPER_SYNTAX);

	war = 0;
	invers = 0;

	value = false;
	bracket = false;
	ciag = false;

	if (!pisz)
		branch = false;

	reloc = false;

	data_out = true;

	tmp = " ";

	// ------------------ specifying the default value type ---------------------

	default_type = tType[typ - 1];
	op_= default_type;

	dotRELOC.mySize = relType[typ - 1];

	// ----------------------------------------------------------------------------
	if (!pisz && test_char(i, a))
	{
		// if data is missing, generate zeros by default
		if (defaultZero)
		{
			save_dta(cardinal(war), tmp, op_, invers);
			return;
		}
		else
			show_error(old, ERROR_UNEXPECTED_EOL);
	}

	if (structure.use)
		show_error(old, ERROR_IMPROPER_SYNTAX);

	isVector = true;

	if (!pisz) 
		dta_used = true;

	len = length(a);

	bool terminateLoop = false;

	while(i < len && terminateLoop == false)
	{
		// SIN, RND
		char upperChar = static_cast<char>(::toupper(a[i]));
		if (upperChar == 'C' || upperChar == 'R' || upperChar == 'S')
		{
			// Get the next 3 chars into a number
			x = (ord(UpCase(a[i + 1])) << 16) + (ord(UpCase(a[i + 2])) << 8) + ord(a[i + 3]);

			switch (x)
			{
				case 0x494E28: // IN(
				case 0x4F5328: // OS(
				{
					inc(i, 3);
					// we check the correctness of the brackets
					k = i;
					tmp = bounded_string(k, a, true);

					value = true;
					invers = 0;
					tmp.erase();

					inc(i);
					sin_a = integer(get_expres(i, a, old, true));
					sin_b = integer(get_expres(i, a, old, true));
					sin_c = integer(get_expres(i, a, old, false));

					if (sin_c <= 0)
					{
						show_error(old, MSG_VALUE_OUT_OF_RANGE);
						break;
					}

					sin_d = 0;
					sin_e = sin_c - 1;

					if (a[i] == ',')
					{
						inc(i);
						sin_d = integer(get_expres(i, a, old, true));
						sin_e = integer(get_expres(i, a, old, false));

						inc(i);
					}
					else inc(i);

					while (sin_d <= sin_e)
					{
						if (x == 0x4F5328)
							war = static_cast<long long>(sin_a + round(sin_b * cos(sin_d * 2 * M_PI / sin_c)));    // COS
						else
							war = static_cast<long long>(sin_a + round(sin_b * sin(sin_d * 2 * M_PI / sin_c)));   // SIN

						war = wartosc(old, war, op_);
						save_dta(cardinal(war), tmp, op_, invers);

						inc(sin_d);
					}

					skip_spaces(i, a);

					break;
				}

				case 0x4E4428: // ND(
				{
					inc(i, 3);

					// we check the correctness of the brackets
					k = i;
					tmp = bounded_string(k, a, true);

					value = true;
					invers = 0;
					tmp.erase();

					inc(i);
					sin_a = integer(get_expres(i, a, old, true));
					sin_b = integer(get_expres(i, a, old, true));
					sin_c = integer(get_expres(i, a, old, false));
					inc(i);

					sin_d = sin_b - sin_a;

					// randomize;
					for (x = 0; x < sin_c - 1; ++x)
					{
						war = sin_a + Int64(random(sin_d + 1));

						war = wartosc(old, war, op_);
						save_dta(cardinal(war), tmp, op_, invers);
					}

					skip_spaces(i, a);
					break;
				}
			}
		}

		upperChar = static_cast<char>(::toupper(a[i]));
		switch(upperChar)
		{
			case 'A':
			case 'B':
			case 'E':
			case 'F':
			case 'G':
			case 'H':
			case 'L':
			case 'M':
			case 'R':
			case 'T':
			case 'V':
			{

				k = i;

				if (AllowWhiteSpaces.has(a[i + 1]))
				{
					IncAndSkip(i, a);

					if (a[i] == '(')
						dec(i);
					else
						i = k;
				}

				if (a[i + 1] == '(')
				{

					// get string delimited by brackets ( )
					if (value) show_error(old, ERROR_EXTRA_CHARS_ON_LINE);

					op_ = UpCase(a[k]);
					inc(i);
					// we check the correctness
					k = i;
					tmp = bounded_string(k, a, true);
					bracket = true;

					// data other than 2 bytes is not relocatable in SPARTA DOS X
					// all are relocatable if we used the .RELOC directive
					if (dotRELOC.use)
					{
						//              if op_='R' then branch:=true else branch:=false;
						branch = (op_ == 'R');

						switch (op_)
						{
							case 'E':
							case 'T':   dotRELOC.mySize = relType[3-1]; break;
							case 'A':
							case 'V':   dotRELOC.mySize = relType[2-1]; break;
							case 'F':   dotRELOC.mySize = relType[4-1]; break;
							case 'L':   dotRELOC.mySize = '<'; break;
							case 'H':   dotRELOC.mySize = '>'; break;
							default:
							{
								dotRELOC.mySize = relType[1-1];
							}

							_siz(false);

						}
					}
					else
					{
						branch = (op_ == 'E' || op_ == 'F' || op_ == 'G' || op_ == 'M' || op_ == 'R' || op_ == 'T');
					}

					inc(i);             // omit the opening bracket

				}
				else
				{
					if (dotRELOC.use)
						_siz(false);

					if (value)
						show_error(old, ERROR_EXTRA_CHARS_ON_LINE);
					war = get_expres(i, a, old, false);
					value = true;

					if (pisz)
						end_string += '$' + Hex(cardinal(war), 4);
				}

				break;
			}

			case 'C':
			case 'D':
			{
				k = i;

				if (AllowWhiteSpaces.has(a[i + 1]))
				{
					IncAndSkip(i, a);

					if (AllowQuotes.has(a[i]))
						dec(i);
					else
						i = k;
				}

				if (AllowQuotes.has(a[i + 1]))
				{
					// get string delimited by single quotes '' or ''
					op_ = UpCase(a[k]);
					inc(i);
				}
				else
				{

					if (dotRELOC.use)
						_siz(false);

					if (value)
						show_error(old, ERROR_EXTRA_CHARS_ON_LINE);

					war = get_expres(i, a, old, false);
					value = true;

					if (pisz)
						end_string += '$' + Hex(cardinal(war), 4);
				}
				break;
			}

			case '\'':
			case '"':
			{
				if (value)
					show_error(old, ERROR_EXTRA_CHARS_ON_LINE);

				// if array then
				// TODO: validate this if logic
				if (op_ != 'C' && op_ != 'D')
				{
					if (a[i] == '"')
						op_ = 'D';
					else
						op_ = 'C';
				}

				tmp = get_string(i, a, old, true);
				war = 0;
				ciag = true;

				if (a[i] == '^')
				{
					tmp[length(tmp)-1] = chr(ord(tmp[length(tmp)-1]) | 0x80);

					inc(i);
				}

				invers = byte(test_string(i, a, 'B'));
				value = true;

				if (a[i] == '^')
				{
					tmp[length(tmp)-1] = chr(ord(tmp[length(tmp) - 1]) | 0x80);

					inc(i);
				}

				if (pisz)
					end_string += tmp;
				break;
			}

			case '(':
			{
				if (bracket)
					show_error(old, ERROR_USE_SQUARE_BRACKET);

				bracket = true;
				inc(i);
				break;
			}

			case ')':
			{
				if (bracket && value)
				{
					value = false;
					bracket = false;
					inc(i);

					skip_spaces(i, a);

					if (test_char(i, a) == false)
					{
						// if there is no end of line, it is the only one
						if (a[i] != ',')
							show_error(old, ERROR_USE_SQUARE_BRACKET);   // the accepted character is ','
					}
				}
				else
				{
					show_error(old, ERROR_EXTRA_CHARS_ON_LINE);
				}
				break;
			}

			case ',':
			{
				IncAndSkip(i, a);
				invers = 0;
				value = false;

				if (a[i] == ',')
				{
					show_error(old, ERROR_EXTRA_CHARS_ON_LINE);
				}
				else if (test_char(i, a))
				{
					show_error(old, ERROR_UNEXPECTED_EOL);  // if end of line error 'Unexpected end of line'
				}

				if (!bracket)
				{
					op_ = default_type;
				}
				break;
			}

			case ' ':
			case '\t':
			{
				skip_spaces(i, a);
				break;
			}

			case '/':
			{
				switch (a[i+1])
				{
					case '/': 
					{
						terminateLoop = true;
						continue;
						break;
					}
					case '*':
					{
						tmp.erase();
						search_comment_block(i, a, tmp);
						if (isComment)
						{
							terminateLoop = true;
							continue;
							break;
						}
						break;
					}
					default:
					{
						show_error(old, ERROR_EXTRA_CHARS_ON_LINE);   // value:=false; i:=length(a)+1
						break;
					}
				}
				break;
			}

			case ';':
			case '\\':
			{
				terminateLoop = true;
				continue;
				break;
			}

			default:
			{
				if (_eol(a[i]))
				{
					terminateLoop = true;
					break;
				}

				if (dotRELOC.use)
				{
					_siz(false);
				}

				if (value)
				{
					show_error(old, ERROR_EXTRA_CHARS_ON_LINE);
				}

				war = get_expres(i, a, old, false);
				value = true;

				if (pisz)
					end_string += '$' + Hex(cardinal(war), 4);
				break;
			}
		}

		if (dta_used)
		{
			if (value)
			{
				if (not(ciag))
				{
					if (op_ == 'C' or op_ == 'D')
						op_ = 'B';
				}

				if (reloc and dotRELOC.use)
				{
					dec(war, rel_ofs);

					_sizm(byte(war));    // will only save when RELOC=TRUE

					reloc = false;
				}

				if (not(pisz))
				{
					if (ext_used.use and (op_ == 'L' or op_ == 'H'))
					{
						switch (op_)
						{
							case 'L':
								t_ext[ext_idx - 1].typ = '<';
								break;
							case 'H':
							{
								t_ext[ext_idx - 1].typ = '>';
								t_ext[ext_idx - 1].lsb = byte(war);
								break;
							}
						}
					}

					war = wartosc(old, war, op_);

					if (proc)
						yes = t_prc[proc_nr - 1].used;
					else
					{
						yes = true;
					}

					if (yes)
						save_dta(cardinal(war), tmp, op_, invers);
				}
				else
					branch = true;           // if WRITE=TRUE, there is no relocation

				ciag = false;
			}
		}

		skip_spaces(i, a);
	}

	isVector = false;
	dta_used = false;
}


/**
 * \brief present the value in the form of a letter code 'Z', 'Q', 'T', 'D'
 *			letter codes symbolize the value type BYTE, WORD, LONG, DWORD
 * \param a 
 * \param old 
 * \param test 
 * \return 
 */
char value_code(long long &a, string& old, const bool test)
{
	tCardinal x = cardinal(abs(a));
	if (x >= 0 && x <= 0xFF) return 'Z';
	if (x >= 0x100 && x <= 0xFFFF) return 'Q';
	if (x >= 0x10000 && x <= 0xFFFFFF)
	{
		if (test) 
			return 'T';
		else 
			show_error(old, ERROR_ILLEGAL_ADDR_MODE_65XX);
	}
	if (!test) 
		show_error(old, MSG_VALUE_OUT_OF_RANGE);
	return 'D';
}

/**
 * \brief new register sizes for SEP, REP trace operations
			 A: t_MXinst		SEP (set bits), REP (reset bits)
			 bit 0 -> 16 bit
			 bit 1 -> 8 bit
 * \param i 
 * \param a 
 */
void reg_size(const BYTE i, const t_MXinst &a)
{
	if ((i & 0x10) != 0)
	{
		if (a == REP)
			isize = 16;
		else
			isize = 8;
	}

	if ((i & 0x20) != 0)
	{
		if (a == REP)
			asize = 16;
		else
			asize = 8;
	}	
}

/**
 * \brief we check the size of registers for the SEP, REP tracking option enabled
 *			B: BYTE	reg size = [8,16]
 *			I: BYTE	code size = [1..3]
 *			L: BYTE	LONGA | LONGI = [0,8,16]
 * \param a 
 * \param b 
 * \param i 
 * \param l 
 */
void test_reg_size(string &a, const BYTE b, const BYTE i, const BYTE l)
{
	if (b == 8)
	{
		if (i > 2 && (l >= 0 && l <= 16))
			show_error(a, ERROR_BAD_REGISTER_SIZE);
	}
	else if (i < 3 && (l >= 0 && l <= 8))
		show_error(a, ERROR_BAD_REGISTER_SIZE);
}

/**
 * \brief Check the size ID
 * \param a 
 * \param siz 
 * \param x 
 * \param pomin 
 */
void test_siz(string &a, char &siz, const char x, bool &pomin)
{
	if (siz == ' ' or siz == x) 
		siz = x;
	else 
		show_error(a, ERROR_ILLEGAL_ADDR_MODE_65XX);

	pomin = true;
}


/**
 * \brief we are looking for the value for the label named with MADS_STACK[N].NAM, if not
 *			labels with MADS_STACK have been defined, then we assign them
 *			values ​​from MADS_STACK[N].ADR
 */
tCardinal adr_label(const t_Mads n, const bool tst)
{
	tCardinal result = 0;
	string txt = mads_stack[ord(n)].name;
	int i = load_lab(txt, false);

	if (tst && i < 0)
	{
		result = mads_stack[ord(n)].addr;    // no declaration for label with TXT

		s_lab(txt, result, bank, txt, '@');
	}

	if (i >= 0) 
		result = t_lab[i].addr;   // we read the value of the label
	return result;
}

/**
 * \brief 
 */
void test_skipa()
{
	if (t_bck[1] == addres) return;

	t_bck[0] = t_bck[1];
	t_bck[1] = addres;

	skip_use = false;

	for (int i = 0; i < t_skp.size(); ++i)
	{
		if (t_skp[i].use)
		{
			t_skp[i].addr = addres;

			inc(t_skp[i].count);

			if (t_skp[i].count > 1)
				t_skp[i].use = false;
			else
				skip_use = true;
		}
	}
}

/**
 * \brief 
 * \param hlp 
 * \param res 
 */
void addResult(int5 &hlp, int5& res)
{
	for (int i = 0; i < hlp.l; ++i)
	{
		res.h[res.l + i] = hlp.h[i];
	}
	inc(res.l, hlp.l);
}

/**
 * \brief 
 * \param txt 
 * \param old 
 * \return 
 */
int5 asm_mnemo(string &txt, string &old)
{
	int i = 1-1;
	const int5 result = calculate_mnemonic(i, txt, old);

	inc(addres, result.l);

	return result;
}

/**
 * \brief 
 * \param mnemo 
 * \param zm 
 * \param zm2 
 * \param old 
 * \return 
 */
int5 moveAXY(string& mnemo, string& zm, string& zm2, string& old)
{
	string tmp;
	int5 hlp;
	BYTE r;
	int v;
	int5 result;

	mnemo[0] = 'L';
	mnemo[1] = 'D';                  // LD?
	tmp = mnemo + ' ' + zm;
	result = asm_mnemo(tmp, old);

	if (regAXY_opty and not(dotRELOC.use) and not(dotRELOC.sdx))
	{
		if (zm[0] == '#' or zm[0] == '<' or zm[0] == '>')
		{
			v = result.h[1];

			switch (mnemo[2])
			{
				case 'A':   r = 0; break;
				case 'X':   r = 1; break;
				default: r = 2; break;
			}

			if (registerOptimisation.used)
			{
				if (registerOptimisation.reg[r] == v)
				{
					result.l = 0;
				}
			}

			registerOptimisation.reg[r] = v;
		}
	}

	mnemo[0] = 'S';
	mnemo[1] = 'T';                  // ST?
	tmp = mnemo + ' ' + zm2;
	hlp = asm_mnemo(tmp, old);

	addResult(hlp, result);

	result.tmp = hlp.h[0];
	return result;
}

/**
 * \brief 
 * \param a 
 * \return 
 */
BYTE adrMode(const string &a)
{
	BYTE result = fCRC16(a);

	if (result >= 0x60 && result <= 0x7e)
		dec(result, 0x60);
	else
		result = 0;
	return result;	
}

/**
 * \brief 
 * \param ety 
 * \param old 
 * \param tst 
 */
void save_fake_label(string &ety, const string &old, const tCardinal tst)
{
	int war = load_lab(ety, false);        // we update the label value

	if (war < 0)
		s_lab(ety, tst, bank, old, ety[1-1]);
	else
		t_lab[war].addr = tst;
}


/**
 * \brief 
 * \param a 
 * \return 
 */
BYTE TypeToByte(const char a)
{
	switch (a)
	{
		case 'B':   return 1;
		case 'W':   return 2;
		case 'L':   return 3;
		case 'D':   return 4;
		default:
			return 0;
	}
}

/**
 * \brief 
 * \param a 
 * \return 
 */
BYTE ValueToType(const tInt64 a)
{

	tInt64 b = abs(a);
	if (b >= 0 && b <= 0xFF) return 1;
	if (b >= 0x100 && b <= 0xFFFF) return 2;
	if (b >= 0x10000 && b <= 0xFFFFFF) return 3;
	return 4;	
}

/**
 * \brief 
 * \param pom 
 * \param ile 
 * \param ch 
 * \return 
 */
string getByte(string &pom, const BYTE ile, const char ch)
{
	string result;
	switch(ch)
	{
		case 'W':
		{
			switch (ile)
			{
				case 2: pom[1-1] = '>'; break;
				case 1: pom[1-1] = '<'; break;
			}
			break;
		}

		case 'L':
		{
			switch (ile)
			{
				case 3: pom[1-1] = '^'; break;
				case 2: pom[1-1] = '>'; break;
				case 1: pom[1-1] = '<'; break;
			}
			break;
		}

		case 'D':
		{
			switch (ile)
			{
				case 4: result = ">>24"; break;
				case 3: pom[1-1] = '^'; break;
				case 2: pom[1-1] = '>'; break;
				case 1: pom[1-1] = '<'; break;
			}
			break;
		}
	}
	return result;	
}

/**
 * \brief the function returns a value of type INT5
 *			INT5.L  -> number of bytes
 *			INT5.H  -> machine code mnemonics, ARGUMENTS
 * \param i 
 * \param a 
 * \param old 
 * \return 
 */
int5 calculate_mnemonic(int& i, string& a, string& old)
{
	int j, m, idx, len;
	string     op_, mnemo, mnemo_tmp, zm, tmp, str, pom, add;
	tInt64    war, war_roz, help;
	tByte    code, ile, k, byt;
	char    op, siz;
	bool    test, increase, incdec, mvnmvp, pomin, opty, isvar;
	bool    branch_run, old_run_macro;
	tCardinal    tryb;
	int5    hlp;
	_strArray    par;	// 0 index

	int5 Result = {};
	Result.l = 0;

	if (a.empty())
		show_error(old, ERROR_ILLEGAL_INSTRUCTION);

	op_.erase();
	siz = ' ';
	op = ' ';

	war_roz = 0;

	mvnmvp = true;
	pomin = false;
	ext_used.use = false;

	increase = false;
	incdec = false;
	reloc = false;
	branch = false;

	mne_used = false;

	attribute.attrib = __U;

	skip_spaces(i, a);

	m = i;                  // remember the positions

	// get the name of the command, calculate its CRC16
	mnemo.erase();
	mnemo_tmp.erase();

	if (_lab_first(a[i]))
	{
		while (_lab(a[i]) or (a[i] == ':' || a[i] == '%'))
		{
			mnemo += UpCase(a[i]);
			mnemo_tmp += UpCas_(a[i]);
			inc(i);
		}
	}


	// we assemble the .PROC procedures referenced in the program
	// switch -x 'Exclude unreferenced procedures' must be enabled
	if (exclude_proc)
	{
		if (proc and (pass > 0))
		{
			if (not(t_prc[proc_nr - 1].used))
			{
				if (VerifyProc)
				{
					ExProcTest = false;
					exclude_proc = false;

					Result = calculate_mnemonic(m, a, old);     // additional line test when EXCLUDE_PROC=FALSE

					if (Result.l < __equ)
						Result.l = 0;

					ExProcTest = true;
					exclude_proc = true;

					i = m;
				}

				return Result;
			}
		}
	}


	// if we read an ARRAY array, the line begins with the character '(' or '['
	// if other than line breaks then 'Improper syntax' error
	if (aray)
	{
		Result.l = __array_run;
		i = 1-1;
		return Result;
	}

	// if a mnemonic is missing, check if it is not a digit ('Illegal instruction' error)
	// otherwise we don't process it and exit
	if (mnemo.empty())
	{
		if (_dec(a[i]) or (a[i] == '$' || a[i] == '%'))
		{
			show_error(old, ERROR_ILLEGAL_INSTRUCTION);
		}
		else
			return Result;
	}

	// exception '=' is equivalent to 'EQU'
	// for a label starting with the first character in the line
	if (mnemo == "=")
	{
		Result.l = __equ;
		return Result;
	}


	// if the ':' character appears, it means that we are combining mnemonics in the XASM style
	j = pos(':', mnemo);
	if (j >= 0)
	{
		Result.i = i;
		i = m;                      // we modify the value from address 'i'
		Result.l = __xasm;
		return Result;
	}


	// check if it is an assignment operation '=', 'EQU', 'SET', '+=', '-=', '++', '--'
	// for a label preceded by at least one space or tab

	j = i;

	skip_spaces(i, a);

	tmp.erase();
	k = 0;

	if (UpCase(a[i]) == 'E' || UpCase(a[i]) == 'S')
	{
		// EQU, SET
		tmp = get_datUp(i, a, 0, false);
		skip_spaces(i, a);
		if ((length(tmp) == 3) and not(test_char(i, a, ' ', 0)))
			k = fASC(tmp);
	}
	else
	{

		while (a[i] == '=' || a[i] == '+' || a[i] == '-')
		{
			tmp += a[i];
			if (a[i] == '=')
				break;
			IncAndSkip(i, a);
		}

		k = fCRC16(tmp);
	}

	//  if ((tmp='EQU') or (tmp='=') or (tmp='+=') or (tmp='-=') or (tmp='++') or (tmp='--') ) {
	if (k == __equ + 1 || k == (__set + 1) || k == __nill)
	{
		i = m;                      // we modify the value from address 'i'

		if (k == __set + 1)
			Result.l = __addSet;
		else
			Result.l = __addEqu;

		return Result;
	}

	i = j;
	len = length(mnemo);

	// check if there is a mnemonic extension
	if (len == 5)
	{
		if (a[i - 2] == '.')
		{
			switch (UpCase(a[i - 1]))
			{
				case 'A':
				case 'W':
				case 'Q': siz = 'Q'; break;

				case 'B':
				case 'Z': siz = 'Z'; break;

				case 'L':
				case 'T': siz = 'T'; break;
			}
		}
	}


	// look for the mnemonic in the HASH table
	k = 0;

	if (len == 3)
		k = fASC(mnemo);
	else
	{
		if (siz != ' ')
		{
			// if SIZ != ' ' it means that it is a 3-letter mnemonic with an extension
			k = fASC(mnemo);
			if (k > 0)
				SetLength(mnemo, 3); // if such a mnemonic exists, OK,
			// !!! otherwise it's probably some kind of macro!!!
		}
	}

	// mnemonic symbols have codes  <1..92>
	// mnemonic symbols 6502 		<1..56>, illegal <96..118>
	// mnemonic symbols 65816 		<57..92>

	if (not(opt & opt_C))
	{
		if (k >= 57 and k <= 92)
			k = 0;  // when 6502, you can use 65816 mnemonics as macros, etc.
	}
	else
		if (k >= 96 and k <= 118)
			k = 0;  // when 65816 you can use illegal 6502 mnemonics as macros etc.


	// in IDX we store the current assembly address, we will not use this variable for any other purpose

	if (k >= __cpbcpd and k <= __jskip)
	{

		if (addres < 0)
		{
			if (pass == pass_end)
			{
				show_error(old, WARN_NO_ORG_SPECIFIED);
			}
		}

		if (siz != ' ')
			show_error(old, ERROR_SIZE_SPECIFIER_NOT_REQ);

		if (not(k == __BckSkp or k == __phrplr))
		{
			// __BckSkp and __phrplr do not need parameters
			skip_spaces(i, a);
			SetLength(par, 1);

			while (not(test_char(i, a)))
			{  // reading macro parameters of an instruction, 'GetParameters' is not suitable for this
				idx = High(par);
				par[idx] = get_dat(i, a, ' ', true);

				SetLength(par, idx + 2);

				skip_spaces(i, a);
			}

			switch (k)     // test of the number of parameters of the macro instruction
			{
				case __inwdew:
				case __addsub:
				{
					idx = 1;
					j = 1;
					break;
				}
				case __movaxy:
				case __cpbcpd:
				{
					idx = 2;
					j = 2;
					break;
				}
				case __adwsbw:
				case __adbsbb:
				{
					idx = 2;
					j = 3;
					break;
				}
				default:
				{
					idx = 0;
					break;
				}
			}

			if (idx > 0)
			{
				if (High(par) < idx)
					show_error(old, ERROR_UNEXPECTED_EOL);
				else
					if (High(par) > j)
						show_error(old, ERROR_EXTRA_CHARS_ON_LINE);
			}

			tmp = par[0];
		}


		idx = addres;
		Result.l = 0;

		switch (k)
		{
			// service PHR, PLR
			case __phrplr:
			{
				Result.l = 5;
				if (mnemo[2 - 1] == 'H')
				{
					Result.h[0] = OP_PHA_IMP; // 0x48;
					Result.h[1] = OP_TXA_IMP; // 0x8a;
					Result.h[2] = OP_PHA_IMP; // 0x48;
					Result.h[3] = OP_TYA_IMP; // 0x98;
					Result.h[4] = OP_PHA_IMP; // 0x48;
				}
				else
				{
					Result.h[0] = OP_PLA_IMP; // 0x68;
					Result.h[1] = OP_TAY_IMP; // 0xa8;
					Result.h[2] = OP_PLA_IMP; // 0x68;
					Result.h[3] = OP_TAX_IMP; // 0xaa;
					Result.h[4] = OP_PLA_IMP; // 0x68;
				}
				break;
			}

			// service INW, INL, IND, DEW, DEL, DED
			case __inwdew:
			{
				Result.l = 0;
				ile = TypeToByte(mnemo[3 - 1]);
				mnemo[3 - 1] = 'C';

				if (mnemo[1 - 1] == 'I')
				{
					// INW, INL, IND
					str = "##INC#DEC" + IntToStr(ora_nr);

					j = load_lab(str, false);  // we read the value of the label

					if (j >= 0)
						tryb = t_lab[j].addr;
					else
					{
						tryb = 0;
					}

					j = 0;
					while (ile > 0)
					{
						//zm = mnemo + " " + tmp + "+" + IntToStr(j);
						zm = mnemo;
						zm += ' ';
						zm += tmp;
						zm += '+';
						zm += IntToStr(j);

						hlp = asm_mnemo(zm, old);
						addResult(hlp, Result);

						if (ile > 1)
						{
							zm = "BNE " + IntToStr(tryb);
							hlp = asm_mnemo(zm, old);
							addResult(hlp, Result);
						}

						inc(j);
						dec(ile);
					}

					save_fake_label(str, old, addres);

					inc(ora_nr);
				}
				else
				{
					// DEW, DEL, DED
					if (pass == pass_end)
						warning(WARN_REG_A_IS_CHANGED);// Register A is changed

					byt = 0;
					while (byt < ile - 1)
					{
						zm = "LDA " + tmp + "+" + IntToStr(byt);
						hlp = asm_mnemo(zm, old);
						addResult(hlp, Result);

						str = "##INC#DEC" + IntToStr(Int64(ora_nr) + byt);

						j = load_lab(str, false);  // we read the value of the label

						if (j >= 0)
							tryb = t_lab[j].addr;
						else
							tryb = 0;

						zm = "BNE " + IntToStr(tryb);
						hlp = asm_mnemo(zm, old);
						addResult(hlp, Result);

						inc(byt);
					}

					inc(byt);

					while (byt != 0)
					{
						if (byt != ile)
						{
							str = "##INC#DEC" + IntToStr(Int64(ora_nr) + byt - 1);
							save_fake_label(str, old, addres);
						}

						zm = "DEC " + tmp + "+" + IntToStr(Int64(byt) - 1);
						hlp = asm_mnemo(zm, old);
						addResult(hlp, Result);

						dec(byt);
					}

					inc(ora_nr, ile - 1);
				}

				break;
			}


			// service ADW, SBW
			case __adwsbw:
			{
				str = par[1];
				pom = par[2];

				test = false;
				if (pom.empty())
					pom = tmp;                        // in POM the result of the operation
				else
					test = true;

				if (tmp[1 - 1] == '#')
					show_error(old, ERROR_ILLEGAL_ADDR_MODE_65XX);

				mnemo[3 - 1] = 'C';                 // ADC, SBC

				if (mnemo[1 - 1] == 'S')
					zm = "SEC";
				else
					zm = "CLC";

				Result = asm_mnemo(zm, old);

				zm = "LDA " + tmp;

				hlp = asm_mnemo(zm, old);
				addResult(hlp, Result);

				ile = hlp.h[0];

				opty = false;
				if (ile == 0xB1)
					opty = true;      // occurs LDA(),Y

				if (not(ile == 0xA5 or ile == 0xAD or ile == 0xB5 or ile == 0xBD))
					test = true;                     // the shorter version with SCC, SCS will not pass

				if (str[1 - 1] == '#')
				{
					str[1 - 1] = '+';
					branch = true;                   // We don't relocate, we only test the value
					war = calculate_value(str, old);
					wartosc(old, war, 'A');

					str[1 - 1] = '<';
					zm = mnemo + " " + str;
					hlp = asm_mnemo(zm, old);
					addResult(hlp, Result);

					zm = "STA " + pom;
					hlp = asm_mnemo(zm, old);
					addResult(hlp, Result);

					if (not(test) and (war < 256))
					{
						macroCmd = true;       // for a valid test_skip

						if (mnemo[1 - 1] == 'S')
							zm = "SCS";
						else
						{
							zm = "SCC";
						}

						hlp = asm_mnemo(zm, old);
						addResult(hlp, Result);

						if (mnemo[1 - 1] == 'S')
							zm = "DEC " + tmp + "+1";
						else
						{
							zm = "INC " + tmp + "+1";
						}

						hlp = asm_mnemo(zm, old);
						addResult(hlp, Result);

						macroCmd = false;
					}
					else
					{
						if (opty)
						{
							zm = "iny";                     // only this way via ASM_MNEMO
							hlp = asm_mnemo(zm, old);       // otherwise there will be no relocation
							addResult(hlp, Result);

							add.erase();
						}
						else
							add = "+1";

						zm = "LDA " + tmp + add;
						hlp = asm_mnemo(zm, old);
						addResult(hlp, Result);

						str[1 - 1] = '>';
						zm = mnemo + " " + str;
						hlp = asm_mnemo(zm, old);
						addResult(hlp, Result);

						if (pos(",Y", AnsiUpperCase(pom)) == -1)
							add = "+1";

						zm = "STA " + pom + add;
						hlp = asm_mnemo(zm, old);
						addResult(hlp, Result);
					}
				}
				else
				{
					zm = mnemo + " " + str;
					hlp = asm_mnemo(zm, old);
					addResult(hlp, Result);

					byt = hlp.h[0];

					zm = "STA " + pom;
					hlp = asm_mnemo(zm, old);
					addResult(hlp, Result);

					if (opty)
					{
						zm = "iny";
						hlp = asm_mnemo(zm, old);
						addResult(hlp, Result);

						add.erase();
					}
					else
						add = "+1";

					if (byt == 0x71 or byt == 0xF1)
					{
						// $71 = ADC(),Y ; $F1 = SBC(),Y

						if (not(opty))
						{

							zm = "iny";
							hlp = asm_mnemo(zm, old);
							addResult(hlp, Result);

							if ((ile == 0xb6 or ile == 0xb9 or ile == 0xbe))
								add.erase();
						}

					}
					else
						if (not(opty and (byt == 0x79)))
							str = str + "+1";

					zm = "LDA " + tmp + add;
					hlp = asm_mnemo(zm, old);
					addResult(hlp, Result);

					zm = mnemo + " " + str;
					hlp = asm_mnemo(zm, old);
					addResult(hlp, Result);

					if (pos(",Y", AnsiUpperCase(pom)) == -1)
						add = "+1";

					zm = "STA " + pom + add;
					hlp = asm_mnemo(zm, old);
					addResult(hlp, Result);
				}

				switch (pom[length(pom)])
				{
					case '+': zm = "iny"; break;
					case '-': zm = "dey"; break;
					default:
						zm.erase();
				}

				if (zm.empty() == false)
				{
					hlp = asm_mnemo(zm, old);
					addResult(hlp, Result);
				}
				break;
			}


			// service ADB, SBB
			case __adbsbb:
			{
				str = par[1];
				pom = par[2];

				Result.l = 0;

				if (pom.empty())
				{
					pom = tmp;

					if (tmp[1 - 1] == '#')
						if (str[1 - 1] != '#')
							pom = str;
				}

				if (tmp != "@")
				{
					zm = "LDA " + tmp;
					Result = asm_mnemo(zm, old);
				}

				if (mnemo[1 - 1] == 'S')
					zm = "SUB ";
				else
					zm = "ADD ";

				zm = zm + str;
				hlp = asm_mnemo(zm, old);
				addResult(hlp, Result);

				if (pom != "@")
				{
					zm = "STA " + pom;
					hlp = asm_mnemo(zm, old);
					addResult(hlp, Result);
				}
				break;
			}

			// service ADD, SUB
			case __addsub:
			{
				inc(addres);

				mnemo[2 - 1] = mnemo[3 - 1];
				mnemo[3 - 1] = 'C';
				Result.l = 1;

				if (mnemo[1 - 1] == 'A')
					Result.h[0] = 0x18;              // code for CLC
				else
					Result.h[0] = 0x38;              // code for SEC

				zm = mnemo + " " + tmp;
				hlp = asm_mnemo(zm, old);
				addResult(hlp, Result);
				break;
			}


			// service MVA, MVX, MVY, MWA, MWX, MWY
			case __movaxy:
			{
				zm = par[1];
				op = tmp[1-1];
				Result.l = 0;

				opty = false;

				if (op == '#')
					registerOptimisation.blocked = true;   // we enable processing of macro-instructions MW?#, MV?#


				if (mnemo[2 - 1] == 'W')
				{
					// MW?
					if (op == '#')
					{
						variable = false;                 // by default VARIABLE=FALSE, which means it is not a variable
						tmp[1-1] = '+';
						branch = true;                    // We don't relocate, we only test the value
						war = calculate_value(tmp, old);
						wartosc(old, war, 'A');

						//     if (not(dotRELOC.use) and not(dotRELOC.sdx) )
						if (not(variable))           // if it is not a variable, then we test further
						{
							if (byte(war) == byte(war >> 8))
							{
								opty = true;
							}
						}

						tmp[1-1] = '<';
					}

					Result = moveAXY(mnemo, tmp, zm, old);

					test = false;

					if (not(opty))
					{
						if (op == '#')
						{
							tmp[1-1] = '>';
						}
						else                                     // exception MWA (ZP),Y ADR
							if (Result.h[0] != 0xB1)
							{
								// $B1 = LDA(ZP),Y
								if (Result.tmp == 0x91)
								{
									if (not(Result.h[0] == 0xb6 or Result.h[0] == 0xb9 or Result.h[0] == 0xbe))
										tmp = tmp + "+1";
								}
								else
								{
									tmp = tmp + "+1";
								}
							}
							else
							{
								// $C8 = INY
								if (Result.h[Result.l - 1] != 0xc8)
								{
									pom = "iny";                     // only this way via ASM_MNEMO
									hlp = asm_mnemo(pom, old);       // otherwise there will be no relocation
									addResult(hlp, Result);
								}

								test = true;
							}
					}

					if (Result.tmp == 0x91)
					{
						// $91 = STA(ZP),Y
						if ((Result.h[0] != 0xB1) and (Result.h[Result.l - 1] != 0xc8))
						{
							pom = "iny";
							hlp = asm_mnemo(pom, old);
							addResult(hlp, Result);
						}
					}
					else
						if (test)
						{
							// $96 = STX Z,Y
							if (not(Result.tmp == 0x96 or Result.tmp == 0x99))
								zm = zm + "+1";  // $99 = STA Q,Y
						}
						else
							zm = zm + "+1";
				}

				if (opty)
				{

					tmp = mnemo + " " + zm;
					hlp = asm_mnemo(tmp, old);

				}
				else
					hlp = moveAXY(mnemo, tmp, zm, old); // MV?


				addResult(hlp, Result);

				registerOptimisation.blocked = false;   // we turn off the processing of macro-instructions MWA, MVA, etc.

				break;
			}


			// service JEQ, JNE, JPL, JMI, JCC, JCS, JVC, JVS
			case __jskip:
			{

				mnemo[1 - 1] = 'B';   // we replace the pseudo-command with a mnemonic
				k = fASC(mnemo);   // we calculate the code for the mnemonic

				branch = true;    // we do not relocate
				war = calculate_value(tmp, old);

				test = false;

				war = war - 2 - addres;

				if ((war < 0) and (abs(war) - 128 > 0))
					test = true;
				if ((war > 0) and (war - 127 > 0))
					test = true;

				//  j = load_lab(tmp, false);
				//  if ((j >= 0) and (war > 0) and (t_lab[j].lop > 0) ) test=true;	// against 'infinite loop'

				if (pass == pass_end)
				{
					if (test)
					{
						if (BranchTest)
							warning(WARN_BRANCH_TOO_LONG, local_name + tmp);
					}
					else
					{
						if (((word(addres) >> 8) != (word(addres + war + 2) >> 8)))
						{
							if (BranchTest)
							{
								warning(WARN_BRANCH_OVER_PAGE_BOUNDS, local_name + tmp);
							}
						}
					}
				}

				//  if ((word(addres) shr 8 <> word(addres + war + 2) shr 8) ) test=true;
				if (not(test))
				{

					Result.l = 2;
					Result.h[0] = ord(m6502[k - 1].kod);  // mnemonic machine code in first addressing 6502
					Result.h[1] = byte(war);
				}
				else
				{
					if ((pass == pass_end) and (t_lab[j].lop == 1))
					{
						t_lab[j].lop = 2;
						if (BranchTest)
						{
							warning(WARN_BRANCH_TOO_LONG, local_name + tmp);
						}
					}

					inc(addres, 2);

					Result.l = 2;
					Result.h[0] = ord(m6502[k - 1].kod) ^ 0x20;  // mnemonic machine code in first addressing 6502
					Result.h[1] = 3;

					zm = "JMP " + tmp;
					hlp = asm_mnemo(zm, old);
					addResult(hlp, Result);
				}

				break;
			}


			// we check whether these are not pseudo jump orders
			// req, rne, rpl, rmi, rcc, rcs, rvc, rvs  -> b??  jump backwards
			// seq, sne, spl, smi, scc, scs, svc, svs  -> b??  jump forward

			case __BckSkp:
			{
				if (mnemo[1 - 1] == 'R')
				{
					if ((pass == pass_end) and skip_xsm)
						warning(WARN_REPEATING_ONLY_LAST_INST);  // Repeating only the last instruction

					if (t_bck[0] < 0)
					{
						if (pass == pass_end)
							show_error(old, ERROR_NO_INST_TO_REPEAT);              // No instruction to repeat
						war = 0;
					}
					else
					{
						war = t_bck[0];
					}
				}
				else
				{
					war = t_skp[skip_idx].addr;

					t_skp[skip_idx].use = true;

					t_skp[skip_idx].count = 0 + ord(macroCmd);

					inc(skip_idx);
					if (skip_idx >= t_skp.size())
						t_skp.resize(skip_idx + 1);
				}

				war = war - 2 - addres;

				if ((war < 0) and (abs(war) - 128 > 0))
					war = abs(war) - 128;
				if ((war > 0) and (war - 127 > 0))
					dec(war, 127); //war=war-127;

				if ((pass == pass_end) and ((word(addres) >> 8) != (word(addres + war + 2) >> 8)))
				{
					if (BranchTest)
						warning(WARN_BRANCH_OVER_PAGE_BOUNDS, local_name + tmp);
				}

				mnemo[1 - 1] = 'B';				// we replace the pseudo-command with a mnemonic
				k = fASC(mnemo);				// we calculate the code for the mnemonic

				code = m6502[k - 1].kod;		// mnemonic machine code in first addressing 6502

				if (not(xasmStyle))
				{
					skip_spaces(i, a);			// !!! must be remared, otherwise
					test_eol(i, a, old, 0);		// !!! will not work, e.g. 'ldx:dex $100', 'tya  0', 'lda:cmp:req 20'
				}

				Result.l = 2;
				Result.h[0] = code;
				Result.h[1] = byte(war);
				inc(addres, 2);

				break;
			}

			// CPB, CPW, CPL, CPD
			case __cpbcpd:
			{
				pom = par[1];

				ile = TypeToByte(mnemo[3 - 1]);

				Result.l = 0;

				str = "##CMP#" + IntToStr(ora_nr);

				j = load_lab(str, false);        // we read the value of the label

				if (j >= 0)
					tryb = t_lab[j].addr;
				else
					tryb = 0;

				test = (tmp[1 - 1] == '#');
				opty = (pom[1 - 1] == '#');

				if ((tmp == "@") and (ile != 1))
					show_error(old, ERROR_IMPROPER_SYNTAX);

				variable = false;               // by default VARIABLE=FALSE, which means it is not a variable

				if (opty)
				{
					pom[1-1] = '+';
					branch = true;                 // We don't relocate, we only test the value
					war = calculate_value(pom, old);

					pom[1-1] = '#';
				}

				isvar = variable or TestWhileOpt;

				while (ile > 0)
				{
					if (test)
						add = getByte(tmp, ile, mnemo[3 - 1]);
					else
						add = "+" + IntToStr(Int64(ile) - 1);

					if (tmp != "@")
					{
						//zm = "LDA " + tmp + add;
						zm = "LDA ";
						zm += tmp;
						zm += add;

						hlp = asm_mnemo(zm, old);
						addResult(hlp, Result);
					}

					if (opty)
						add = getByte(pom, ile, mnemo[3 - 1]);
					else
						add = "+" + IntToStr(Int64(ile) - 1);

					//zm = "CMP " + pom + add;
					zm = "CMP ";
					zm += pom;
					zm += add;

					hlp = asm_mnemo(zm, old);

					if (not(isvar) and (hlp.l == 2) and (hlp.h[0] == 0xc9) and (hlp.h[1] == 0))
						dec(addres, 2);                                   // if CMP  0 then cancel
					else
						addResult(hlp, Result);

					if (ile > 1)
					{
						zm = "BNE " + IntToStr(tryb);
						hlp = asm_mnemo(zm, old);
						addResult(hlp, Result);
					}

					dec(ile);
				}

				str = "##CMP#" + IntToStr(ora_nr);
				save_fake_label(str, old, addres);

				TestWhileOpt = true;

				inc(ora_nr);

				break;
			}

		}

		addres = idx;

		registerOptimisation.used = (k == __movaxy);

		mne_used = true;  // some mnemonic was read
		return Result;    // !!! END !!! the macro-instruction has been read and decoded
	}


	// check if this is the macro name
	if (k == 0)
	{
		idx = load_lab(mnemo_tmp, false); // look in the labels

		if (idx < 0)
		{
			if (pass == pass_end)
				error_und(old, mnemo, WARN_UNDECLARED_MACRO);
			return Result;
		}

		if (t_lab[idx].bank == __id_ext) // symbol external
		{
			if (t_extn[t_lab[idx].addr].isProcedure)
			{
				tmp = "##" + t_extn[t_lab[idx].addr].name;

				Result.i = t_lab[l_lab(tmp)].addr;
				Result.l = __proc_run;

				return Result;
			}
		}

		//    if ((t_lab[idx].bnk=__id_macro) ) { Result.l=__nill; exit }

		if (t_lab[idx].bank >= __id_macro)
		{
			Result.l = byte(t_lab[idx].bank);
			Result.i = t_lab[idx].addr;
		}
		else
			if (pass == pass_end)
				error_und(old, mnemo, WARN_UNDECLARED_MACRO);

		return Result;
	}

	dec(k);   // !!! necessarily

	// found a pseudo-order
	if (k >= __equ)
	{
		if (siz != ' ')
			show_error(old, ERROR_SIZE_SPECIFIER_NOT_REQ);
		else
		{
			Result.l = k;
			return Result;
		}
	}

	if (first_org and (opt and opt_H > 0))
		show_error(old, WARN_NO_ORG_SPECIFIED);

	// if we do not process MWA, MVA macro-instructions, etc., we block register optimization
	if (not(registerOptimisation.blocked))
	{
		registerOptimisation.used = false;

		registerOptimisation.reg[0] = -1;
		registerOptimisation.reg[1] = -1;
		registerOptimisation.reg[2] = -1;
	}

	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
	// based on the CODE machine code, it is now possible to determine what order it is //
	// no need to compare strings or replace the mnemonic with a digital value //
	// for later comparison //
	// from now on the variable K stores the instruction number!!! //
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//

	if (k < 94 and (opt & opt_C))
	{
		code = m65816[k].kod;           // machine codes 65816 are in the range <0..91>
		tryb = m65816[k].ads;           // mnemonic machine code in first addressing 65816
	}
	else if (k >= 96 and k <= 118)
	{
		code = m6502ill[k - 96].kod;        // machine codes 6502 illegal are in the range <96..118>
		tryb = m6502ill[k - 96].ads;        // mnemonic machine code in first addressing 6502

		if (pass == pass_end)
		{
			if (k == OP_ARR_IMM_ILL or k == OP_JMP_IND or k == OP_ROR_ABS or k == OP_RRA_ABS_ILL or k == OP_BVS_REL)
			{
				warning(WARN_UNSTABLE_ILLEGAL_OPCODE, mnemo);
			}
		}
	}
	else
	{
		if (k >= (sizeof(m6502) / sizeof(t_ads)))
		{
			code = 0;
			tryb = 0;
		}
		else
		{
			code = m6502[k].kod;           // machine codes 6502 are in the range <0..55>
			tryb = m6502[k].ads;           // mnemonic machine code in first addressing 6502
		}
	}


	// in .STRUCT and .ARRAY it is not possible to use CPU instructions
	// similarly, CPU instructions cannot appear in the EMPTY (.ds) block
	if (structure.use or aray or enumeration.use or ::empty)
		show_error(old, ERROR_IMPROPER_SYNTAX);

	// If the mnemonic does not require an argument, stop
	if (tryb == 0)
	{
		if (siz != ' ')
			show_error(old, ERROR_SIZE_SPECIFIER_NOT_REQ);

		if (not(xasmStyle))
		{
			skip_spaces(i, a);            // !!! must be remared, otherwise
			test_eol(i, a, a, 0);          // !!! will not work, e.g. 'ldx:dex $100', 'tya  0', 'lda:cmp:req 20'
		}

		Result.l = 1;
		Result.h[0] = code;

		mne_used = true;              // some mnemonic was read

		return Result;
	}

	// skip the spaces and go to the argument
	skip_spaces(i, a);

	if (_lab_first(a[i]))
	{
		// check if the label ends with ':'
		j = i;
		tmp = get_lab(i, a, false);

		if ((tmp.empty() == false) and (a[i] == ':'))
		{
			old_run_macro = run_macro;
			run_macro = false;
			 
			if (a[i + 1] == ':')
			{
				s_lab(tmp, addres + 1, bank, zm, tmp[1-1]);   // global label for code automodification
				inc(i);
			}
			else
			{
				save_lab(tmp, addres + 1, bank, zm);          // local label for code automodification
			}

			run_macro = old_run_macro;
			inc(i);
			skip_spaces(i, a);
		}
		else
		{
			i = j;
		}
	}

	// get argument operator to 'OP'
	// this does not apply to MVN and MVP commands (65816)
	if (not(code == 68 or code == 84))
	{
		mvnmvp = false;
		if (a[i] == '#' or a[i] == '<' or a[i] == '>' or a[i] == '^')
		{
			if (not(dotRELOC.use))
				branch = true;             // do not relocate this order
			op = a[i];
			inc(i);
		}
		else
			op = ' ';

		if ((a[i] == '@') and test_char(i + 1, a, ' ', 0))
		{
			op = '@';
			inc(i);
		}

		if (op == '@')
		{
			if (siz != ' ')
				show_error(old, ERROR_SIZE_SPECIFIER_NOT_REQ);
			else
				test_eol(i, a, a, 0);
		}
	}

	skip_spaces(i, a);

	// exceptions for which the register size is constant
	if ((opt & opt_C) and (op == '#'))
	{
		switch (code)
		{
			case 98: test_siz(a, siz, 'Q', pomin);  break; // PEA #Q
			case 254: test_siz(a, siz, 'Z', pomin); break; // COP #Z
			case 190: test_siz(a, siz, 'Z', pomin); break; // REP #Z
			case 222: test_siz(a, siz, 'Z', pomin); break; // SEP #Z
		}
	}

	zm = get_dat_noSPC(i, a, old, ';');

	if (zm.empty() == false)
	{
		if ((zm[2 - 1] == ':') and (UpCase(zm[1 - 1]) == 'A' or UpCase(zm[1 - 1]) == 'Z'))
		{
			// forcing A-BSOLUTE, Z-ERO PAGE mode in XASM style
			switch (UpCase(zm[1-1]))
			{
				case 'A': siz = 'Q'; break;
				case 'Z': siz = 'Z'; break;
			}

			zm = copy(zm, 3 - 1, length(zm));
		}
	}

	tmp.erase();

	// if there is no argument, default to the current address '*'
	// or '@' operator when ASL , ROL , LSR , ROR
	// or '@' operator for INC, DEC when CPU=65816
	if ((zm.empty()) and (op == ' '))
	{
		if ((code == 2 or code == 34 or code == 66 or code == 98) or ((opt & opt_C) and (code == 26 or code == 58)))
		{
			op = '@';
		}
		else
		{
			zm = "*";
			if (not(klamra_used))
			{
				if (pass == 0)
				{
					show_error(old, ERROR_UNEXPECTED_EOL);            // 'Default addressing mode' -67- withdrawn
				}
			}
		}
	}

	// if there is an opening bracket '(' or '[', check if it is correct
	// and make sure there are no spaces between them
	if (zm.empty() == false)
	{
		j = 1-1;

		if ((op == ' ') and (AllowStringBrackets.has(zm[1-1])))
		{

			tmp = get_dat_noSPC(j, zm, a, ',');

			if (length(tmp) == 2)
				show_error(old, ERROR_ILLEGAL_ADDR_MODE_65XX);      // there was no argument between the brackets

			// if after the bracket there is a character other than ',',' ', 0, it is ERROR

			skip_spaces(j, zm);
			test_eol(j, zm, a, ',');

			// rewrite the character following ',' to 'OP_'
			if (zm[j] == ',')
			{
				IncAndSkip(j, zm);

				if (test_param(j, zm))
				{
					Result.l = __nill;
					return Result;
				}
				else
					if (UpCase(zm[j]) == 'X' or UpCase(zm[j]) == 'Y' or UpCase(zm[j]) == 'S')
					{
						op_ += UpCase(zm[j]); //zm[j]=' ';

						if (zm[j + 1] == '+' or zm[j + 1] == '-')
						{
							if (UpCase(zm[j]) == 'S')
								show_error(old, ERROR_EXTRA_CHARS_ON_LINE);       // + - cannot be for 'S'

							incdec = true;

							switch (zm[j + 1])
							{
								case '-':
								{
									//war_roz=calculate_value('{de'+zm[j]+'}',a);
									if (UpCase(zm[j]) == 'X')
										war_roz = 0xCA;
									else
										war_roz = 0x88;   // dex, dey

									break;
								}
								case '+':
								{
									//war_roz=calculate_value('{in'+zm[j]+'}',a);
									if (UpCase(zm[j]) == 'X')
										war_roz = 0xE8;
									else
										war_roz = 0xC8;   // inx, iny
									break;
								}
							}

							test_eol(j + 2, zm, a, 0);
						}
						else
						{
							test_eol(j + 1, zm, a, 0);
						}
					}
					else
						show_error(old, ERROR_ILLEGAL_ADDR_MODE_65XX);
			}

			// remove '()' or '[]' from the string and rewrite to 'OP_'
			op_ = op_ + zm[1-1];
			zm = copy(tmp, 2-1, length(tmp) - 2);
			j = 1-1;
		}

		tmp = get_dat_noSPC(j, zm, a, ',');

		//piss

		// now if there is a character other than ',' it is ERROR
		switch (op)
		{
			case '#':
			case '<':
			case '>':
			case '@':
			case '^':
				test_eol(j, zm, a, 0);
				break;
			default:
				test_eol(j, zm, a, ',');
				break;
		}


		// rewrite the character following ',' to 'OP_' if it is 'XYS'
		// otherwise calculate the value after the decimal point
		if (zm[j] == ',')
		{
			if (mvnmvp)
			{
				//   inc(j);
				IncAndSkip(j, zm);
				str = get_dat(j, zm, ' ', true);
				war_roz = calculate_value(str, a);
				op_ = op_ + value_code(war_roz, a, true);
			}
			else
			{
				IncAndSkip(j, zm);

				if (test_param(j, zm))
				{
					Result.l = __nill;
					return Result;
				}
				else
					if (UpCase(zm[j]) == 'X' or UpCase(zm[j]) == 'Y' or UpCase(zm[j]) == 'S')
					{
						op_ += UpCase(zm[j]);

						if (SetOfPlusMinus.has(zm[j + 1]))
						{
							if (_eol(zm[j + 2]))
							{
								if (UpCase(zm[j]) == 'S')
									show_error(old, ERROR_EXTRA_CHARS_ON_LINE);      // + - cannot be for 'S'

								incdec = true;

								switch (zm[j + 1])
								{
									case '-':
									{
										//war_roz=calculate_value('{de'+zm[j]+'}',a);
										if (UpCase(zm[j]) == 'X')
											war_roz = 0xCA;
										else
											war_roz = 0x88;   // dex, dey
										break;
									}
									case '+':
									{
										//war_roz=calculate_value('{in'+zm[j]+'}',a);
										if (UpCase(zm[j]) == 'X')
											war_roz = 0xE8;
										else
											war_roz = 0xC8;   // inx, iny
										break;
									}
								}

								inc(j, 2);
							}
							else
							{
								increase = true;
								inc(j);
								war_roz = calculate_value_noSPC(zm, a, j, 0, 'F');
							}
						}
						else
						{
							inc(j);
						}
					}
					else
					{
						show_error(old, ERROR_ILLEGAL_ADDR_MODE_65XX);
					}
			}

		}
		test_eol(j, zm, a, 0);
	}


	// if it is a jump command or PEA, PEI, PER, we do not relocate (BRANCH=TRUE)
	if (((mnemo[1 - 1] == 'B') and (mnemo[2 - 1] != 'I')) or ((mnemo[1 - 1] == 'P') and (mnemo[2 - 1] == 'E') and (op_.empty()) and (op == ' ')))
	{
		branch = true;
		branch_run = true;
	}
	else
		branch_run = false;


	if (addres < 0)
	{
		if (pass == pass_end)
			show_error(old, WARN_NO_ORG_SPECIFIED);        // address<0 there was definitely no ORG
	}

	// calculate the value of the mnemonic argument, if there is no argument then 'WAR=0'

	war = 0;
	if (not((tmp.empty()) and (op == '#')))
	{
		if (op != '@')
			war = calculate_value(tmp, old);
	}

	if (increase)
		inc(war, war_roz); //war=war+war_roz;

	op_ += value_code(war, a, true);

	if (mvnmvp)
		war = war_roz + (byte(war) << 8);

	//----------------------------------------------------------------------------*)
	// exceptions apply to conditional jumps B?? (6502) ; BRA, BRL, PEA (65816)   *)
	//----------------------------------------------------------------------------*)

	if (branch_run)
	{
		op_ = "B";

		if (siz != ' ')
		{
			if (siz == 'Q')
			{
				op_ = "W";
			}
			else if (siz != 'Z')
			{
				show_error(old, ERROR_OPERAND_OVERFLOW);
			}
		}

		j = adrMode(op_);                  // we will find out which addressing mode it is

		war = war - 2 - addres;

		if (j > 0 and tryb and (maska[j-1] == 2))
		{
			idx = 128;
		}
		else
		{
			op_ = "W";

			if (siz != ' ')
			{
				if (siz != 'Q')
				{
					show_error(old, ERROR_OPERAND_OVERFLOW);
				}
			}

			dec(war);

			idx = 65536;
		}

		test = false;

		if ((war < 0) and (abs(war) - idx > 0))
		{
			war = abs(war) - idx;
			test = true;
		}

		if ((war > 0) and (war - idx + 1 > 0))
		{
			war = war - idx + 1;
			test = true;
		}

		if ((pass == pass_end) and test)
			show_error(old, integer(-war));
	}

	// based on 'OP' determine 'OP_' addressing
	switch (op)
	{
		case '#':
		case '<':
		case '>':
		case '^': op_ = "#"; break;
		case '@': op_ = "@"; break;
	}


	// find the calculated address in the 'addressing' array
	// and check whether this addressing is possible for this mnemonic
	//
	// if not found, change the argument type Z->Q

	// if 65816 then insert '~'
	if ((opt & opt_C))
		op_ = "~" + op_;

	len = length(op_);

	// if relocatable command (RELOC=TRUE) force size 'Q'
	if (reloc and not(dotRELOC.use))
	{
		if (siz == 'T')
			show_error(old, ERROR_ILLEGAL_INSTRUCTION);
		else
			if (not(op == '<' or op == '>' or op == '^'))
				siz = 'Q';
	}

	// if there is an external label (EXT_USED.USE=TRUE) force size EXT_USED.SIZ
	if (ext_used.use and (ext_used.mySize == 'B' or ext_used.mySize == 'W' or ext_used.mySize == 'L'))
	{
		if (op == '^')
			show_error(old, ERROR_ADDR_RELOCATION_OVERLOAD);

		if (op == '<' or op == '>')
		{
			if (not(dotRELOC.use) and not(dotRELOC.sdx))
				show_error(old, ERROR_IMPROPER_SYNTAX);    // for Atari-DOS there are no relocatable lo, hi

			t_ext[ext_idx - 1].typ = op;
			ext_used.mySize = 'B';

			if (op == '>')
				t_ext[ext_idx - 1].lsb = byte(war);
		}

		switch (ext_used.mySize)
		{
			case 'B': siz = 'Z'; break;
			case 'W': siz = 'Q'; break;
			case 'L': siz = 'T'; break;
		}
	}

	// check if there is a mnemonic extension
	// modify the size of the operand based on 'SIZ'
	if (siz != ' ')
	{
		switch (op_[len-1])
		{
			case 'Q':
			{
				if (siz != 'Z')
					op_[len-1] = siz;
				else
					if (pass == pass_end)
						show_error(old, ERROR_OPERAND_OVERFLOW);

				break;
			}

			case 'T':
			{
				if (not(siz == 'T'))
				{
					if (pass == pass_end)
						show_error(old, ERROR_OPERAND_OVERFLOW);
				}
				break;
			}

			case '#':
			{
				if (siz == 'T')
					show_error(old, ERROR_ILLEGAL_ADDR_MODE_65XX);
				else
					if ((siz == 'Z') and (abs(war) > 0xFF))
						show_error(old, ERROR_OPERAND_OVERFLOW);
				break;
			}

			case 'Z':
			{
				op_[len-1] = siz;
				break;
			}
		}
	}

	j = adrMode(op_);

	bool stopWhile = false;
	while (!stopWhile and ((j == 0) or ((tryb & maska[j-1]) == 0)))
	{
		switch (op_[len-1])  // change the size of the operand
		{
			case 'Z': op_[len-1] = 'Q'; break;
			case 'Q': op_[len-1] = 'T'; break;
			default:
			{
				if (pass == pass_end)
					show_error(old, ERROR_ILLEGAL_ADDR_MODE_65XX);
				else
				{
					stopWhile = true;
					continue;
				}
				break;
			}
		}
		// check whether the new operand does not conflict with the declared size
		if ((siz != ' ') and (pass == pass_end))
		{
			if (not(siz == op_[len-1]))
				show_error(old, ERROR_OPERAND_OVERFLOW);
		}

		j = adrMode(op_);  // check if there is an addressing mode for the new operand size
	}


	if ((k == 116) and (j == 4))
		code = 0x78; // exception for DOP #xx     (0x80)
	if ((k == 111) and (j == 10))
		code = 0x87; // exception for SHA abs,y   (0x9f)
	if ((k == 101) and (j == 1))
		code = 0xa3; // exception for LAX (zp,x)

	if ((k == 11) and (j == 11))
	{
		// exception for JML -> JMP
		if ((war >> 16) == (addres >> 16))
		{
			code = 0x4c;
			j = 10;
			op_ = "~Q";
		}
	}

	// writeln(mnemo,',',k,',',op_,',',j,',',hex(addres,4),' / ',hex(war,4));

	// the variable K is safe, it has not been modified up to this point

	if ((opt & opt_C) != 0)
	{
		ile = 0;
		for (int index = 7; index >= 0; --index)
		{
			if (((tryb >> 24) and maska[index] > 0))
			{
				ile = byte((8 - index) * 23);

				// if (tryb and $80000000>0 ) ile=23;
				// if (tryb and $40000000>0 ) ile=2*23;
				// if (tryb and $20000000>0 ) ile=3*23;
				// if (tryb and $10000000>0 ) ile=4*23;
				// if (tryb and $08000000>0 ) ile=5*23;
				// if (tryb and $04000000>0 ) ile=6*23;
				// if (tryb and $02000000>0 ) ile=7*23;
				// if (tryb and $01000000>0 ) ile=8*23;}
			}
		}
		inc(code, addycja_16[j + ile - 1]);  //code=byte( code+addycja_16[j+ile] )
	}
	else if ((tryb & 0x80000000) > 0)
		inc(code, addycja[j + 12 - 1]);		//code=code+addycja[j+12]
	else if ((tryb & 0x40000000) > 0)
		inc(code, addycja[j + 24 - 1]);		//code=code+addycja[j+24]
	else if ((tryb & 0x20000000) > 0)
		inc(code, addycja[j + 36 - 1]);		//code=code+addycja[j+36]
	else
		inc(code, addycja[j - 1]);			//code=code+addycja[j];

	// calculation of 'WAR' based on 'OP'
	// WAR can be any D-WORD value

	help = war;
	if (dotRELOC.use and rel_used)
		dec(help, rel_ofs);

	switch (op)
	{
		case '<': war = byte(wartosc(a, help, 'D')); break;
		case '>':
		{
			war = byte(wartosc(a, help, 'D') >> 8);
			_sizm(byte(help));
			break;
		}
		case '^':
		{
			war = byte(wartosc(a, help, 'D') >> 16);
			_sizm(byte(help));
			break;
		}
	}

	if ((longa or (longi != 0)) and macro_rept_if_test())  // LONGA | LONGI
	{
		if (not (code == 0xf4 or code == 0x02 or code == ord(REP) or code == ord(SEP)))
		{
			if (op_[len-1] == '#')
			{
				switch (mnemo[3 - 1])
				{
					case 'A':
					case 'C':
					case 'D':
					case 'P':
					case 'R':
					case 'T':   // lda, adc, sbc, and, cmp, ror, bit
					{
						if (longa > 0)
						{
							if (longa == 8)
							{
								if (siz == 'Q')
									show_error(old, ERROR_BAD_REGISTER_SIZE);
								siz = 'Z';
							}
							else
							{
								siz = 'Q';
							}
						}
						break;
					}

					//test_reg_size(a,asize,ile, longa);

					case 'X':
					case 'Y':    // ldx, ldy, cpx, cpy,
					{
						if (longi > 0)
						{
							if (longi == 8)
							{
								if (siz == 'Q')
									show_error(old, ERROR_BAD_REGISTER_SIZE);
								siz = 'Z';
							}
							else
							{
								siz = 'Q';
							}
						}
						break;
					}

				}// PEA #, COP #, REP #, SEP #
			}
			// we check the size of registers for immediate addressing mode '#'
		}
	}

	// count how many bytes the instruction and argument consist of
	ile = 1;

	if (not(mvnmvp))
	{
		if (incdec)
		{
			inc(ile);
			switch (op_[len-1])
			{
				case 'Z': war = (war & 0xff) | (byte(war_roz) << 8); break;
				case 'Q': war = (war & 0xffff) | (byte(war_roz) << 16); break;
				case 'T': war = (war & 0xffffff) | (byte(war_roz) << 24); break;
			}
		}
	}

	if (op_ == "~ZZ")
		inc(ile, 2);
	else
	{
		switch (op_[len-1])
		{
			case '#':
			{
				if (siz == ' ')
					siz = value_code(war, a, false);

				switch (siz)
				{
					case 'Z':
					{
						war = wartosc(a, war, 'B');
						inc(ile);
						break;
					}

					case 'Q':
					{
						if ((opt & opt_C) == 0)
							show_error(old, MSG_VALUE_OUT_OF_RANGE, '(' + IntToStr(war) + " must be between 0 and 255)");
						else
						{
							war = wartosc(a, war, 'A');
							inc(ile, 2);
						}
						break;
					}
				}
				break;
			}
			case 'Z':
			case 'B': inc(ile); break;

			case 'Q':
			case 'W': inc(ile, 2); break;

			case 'T': inc(ile, 3); break;
		}
	}

	// we determine the size of the relocatable argument,B,W,L,<,>
	if (dotRELOC.use and rel_used)
	{
		switch (op_[len-1])
		{
			case '#':
			{
				if ((op == '<' or op == '>' or op == '^'))
				{
					dotRELOC.mySize = op;
				}
				break;
			}
			case 'Z':
			case 'B':
			{
				dotRELOC.mySize = relType[1 - 1];
				break;
			}
			case 'Q':
			case 'W':
			{
				dotRELOC.mySize = relType[2 - 1];
				break;
			}
			case 'T':
			{
				dotRELOC.mySize = relType[3 - 1];
				break;
			}
		}

		if (not(ext_used.use))
			dec(war, rel_ofs);  // you must not modify the EXTERNAL symbol argument

		_siz(true);

		rel_used = false;
	}

	// here we perform operations to track the size of registers A,X,Y
	// modified by REP, SEP commands
	if ((opt & opt_T) and (pass == pass_end) and macro_rept_if_test())
	{
		// SEP, REP command tracking option enabled
		if (code == ord(REP) or code == ord(SEP))
		{
			reg_size(byte(war), t_MXinst(code));
		}

		//   if (mSEP ) reg_size( byte(war),true ) else
		//    if (mREP ) reg_size( byte(war),false );

		// we check the size of registers for immediate addressing mode '#'
		if (not(pomin))
		{
			if (op_[len-1] == '#')
			{
				switch (mnemo[3 - 1])
				{
					case 'A':
					case 'C':
					case 'D':
					case 'P':
					case 'R':
					case 'T':
					{
						test_reg_size(a, asize, ile, longa); // lda, adc, sbc, and, cmp, ror, bit
						break;
					}

					case 'X':
					case 'Y':
					{
						test_reg_size(a, isize, ile, longi); // ldx, ldy, cpx, cpy
						break;
					}
				}
			}
		}
	}

	// we check the label attribute

	if (pass == pass_end)
	{
		switch (attribute.attrib)
		{
			// READ
			case __R:
			{
				if (ValidWriteCode.has(code))
					warning(WARN_LABEL_T_IS_ONLY_FOR);
				break;
			}
			// WRITE
			case __W:
			{
				if (ValidReadCode.has(code))
					warning(WARN_LABEL_T_IS_ONLY_FOR);
				break;
			}
		}
	}

	// we prepare the result, CPU command + arguments

	Result.l = ile;

	Result.h[0] = code;
	Result.h[1] = byte(war);
	Result.h[2] = byte(war >> 8);
	Result.h[3] = byte(war >> 16);
	Result.h[4] = byte(war >> 24);

	if ((Result.h[0] == OP_JMP_IND) and (Result.h[1] == 0xff) and (pass == pass_end))
		warning(WARN_BUDDY_INDIRECT_JUMP);

	mne_used = true;           // some mnemonic was read

	return Result;
}

/**
 * \brief Find out if the first 3 letters of the token are a reserved word
 * \param token 
 * \return 
 */
bool reserved(const string &token)
{
	const tByte v = fASC(token);

	if (v >= 0x80 && v <= 0x9f) // PSEUDO
		return true;

	if (opt & opt_C)
		return (v >= 1 && v <= 92); // 65816
	else
		return (v >= 1 && v <= 56); // 6502
}

/**
 * \brief test whether a reserved name has been used, e.g. pseudo-instruction name 
 *			or whether the name has been used before.
 * \param ety 
 * \param zm 
 */
void reserved_word(string &ety, string &zm)
{
	if (ety[0] == '?')
		warning(WARN_ILLEGAL_CHARACTER);

	if (length(ety) == 3)
	{
		if (reserved(ety))
			error_und(zm, ety, WARN_RESERVED_WORD);
	}

	if (rept)
		show_error(zm, ERROR_MISSING_DOT_REPT);
	
	if (load_lab(ety, true) >= 0)
		error_und(zm, ety, WARN_LABEL_X_DECLARED_TWICE);        // name in use
}

/**
 * \brief execute .STRUCT (e.g. declaring a structure field using another structure)
 *			it is possible to assign fields defined by the structure to a new variable
 * \param zm 
 * \param ety 
 * \param mne 
 * \param head 
 * \param refAddress 
 */
void create_struct_variable(string &zm, string &ety, int5 &mne, const bool head, int &refAddress)
{
	int indeks, _doo, j, k, idx, hlp;
	string txt, str;

	indeks = mne.i;

	if (mne.l == __enum_run)
	{
		save_lst('a');

		save_nul(indeks);
		inc(refAddress, indeks);

		return;
	}

	// we check whether the structure does not call itself
	if (t_str[indeks].labelName + '.' == local_name)
		show_error(zm, ERROR_NO_RECURSIVE_STRUCTURES);

	_doo = refAddress - structure.addres;

	if (mne.l == __struct_run)
	{
		if (structure.use)
			save_lab(ety, _doo, bank, zm);     // new field in the structure
		else
			save_lab(ety, refAddress, bank, zm);   // definition of a structured type variable
	}

	save_lst('a');

	// we read the number of structure fields
	hlp = t_str[indeks].offset;
	inc(indeks);

	for (idx = indeks; idx < indeks + hlp - 1; ++idx)
	{
		txt = t_str[idx].labelName;

		if (mne.l == __struct_run_noLabel)
		{
			//str = ety + '.' + txt;
			str = ety;
			str += '.';
			str += txt;

			if (load_lab(ety, true) < 0)
			{
				// ETY can occur multiple times, so we test LOAD_LAB < 0
				k = 1-1;
				save_str(ety, k, 0, 1, refAddress, bank);           // we do not increase STRUCT.ADDRESS (+0)

				save_lab(ety, refAddress - structure.addres, bank, zm); // here we operate on the new STRUCT.ADDRESS value
			}
		}
		else
		{
			if (txt[0] != '.')
				txt = '.' + txt;
			str = ety + txt;
		}

		j = t_str[idx].mySize;

		if (structure.use)
		{
			k = t_str[idx].offset + _doo;                   // operation necessary because STRUCT.ADDRESS has not been increased

			save_str(str, k, j, 1, refAddress, bank);           // here the STRUCT.ADDRESS is increased

			save_lab(str, refAddress - structure.addres, bank, zm); // here we operate on the new STRUCT.ADDRESS value

			inc(structure.cnt);                          // we increase the field counter in the structure
		}
		else
		{
			save_lab(str, refAddress, bank, zm);

			if (dotRELOC.use || dotRELOC.sdx)
				save_nul(j * t_str[idx].nrRepeat);
			else
				if (head) new_DOS_header();             // we force writing the DOS header

		}

		inc(refAddress, j * t_str[idx].nrRepeat);
	}	
}

/**
 * \brief dla   DTA STRUCT_NAME [EXPRESSION]   we create structured data
 * \param i 
 * \param zm 
 * \param ety 
 * \param v 
 */
void create_struct_data(int& i, string& zm, string& ety, tByte v)
{
	int _doo, _odd, k, idx;
	tInt64 war;
	string tmp;

	struct_used.use = false;

	_odd = addres;
	war = addres;
	_doo = bank;

	k = bank;

	save_lst('a');

	if (v >= __byte)
		dec(v, __byteValue);
	else
		v = 1;

	test_skipa();

	calculate_data(i, zm, zm, v);

	// if we have created a structure, remember it accordingly
	// its details, address, creator, etc.
	if (struct_used.use)
	{
		if (ety.empty())
			show_error(zm, ERROR_LABEL_NAME_REQUIRED);

		if (pass < 2 and ety[0] == '?') 
			warning(WARN_ILLEGAL_CHARACTER);

		idx = load_lab(ety, true);

		// if the label was already declared and it is PASS=0
		// it means that we have a LABEL DECLARED TWICE error
		if (pass == 0 && idx >= 0) 
			error_und(zm, ety, WARN_LABEL_X_DECLARED_TWICE);

		_doo = __dta_struct;

		if (t_lab[idx].bank == __dta_struct)
		{
			// this declaration of structured data has already occurred
			_odd = t_lab[idx].addr;

			t_str[_odd].addr = cardinal(war);
			t_str[_odd].bank = integer(k);
		}
		else
		{
			// in TMP we create a name taking into account locality 'TMP:= ??? +ETY'
			if (run_macro)
				tmp = macro_nr + ety;
			else if (proc)
				tmp = proc_name + local_name + ety;
			else
				tmp = local_name + ety;

			// this structured data declaration has not occurred yet
			save_str(tmp, struct_used.count, 0, 1, integer(war), integer(k));

			_odd = loa_str(tmp, structure.id);

			t_str[_odd].idx = struct_used.idx;
		}
	}

	save_lab(ety, cardinal(_odd), integer(_doo), zm);     // we write the label normally outside the structure
}

/**
 * \brief 
 */
void oddaj_var()
{
	int i, y, x, k, idx;
	string txt, tmp;
	bool old_run_macro;
	int5 mne;

	if (var_idx > 0)
	{
		old_run_macro = run_macro;
		run_macro = false;
		tmp.erase();

		for (i = 0; i <= var_idx - 1; ++i)
		{
			if (t_var[i].lok == end_idx && t_var[i].excludeProc)
			{
				// we put the variables in the appropriate local area
				txt = t_var[i].name;                  // variable name .VAR

				if (t_var[i].addr >= 0)               // .VAR variables with a specific location address
					nul.i = t_var[i].addr;
				else
					nul.i = addres;                        // dynamically allocated .VAR variables

				if (nul.i < 0)
					show_error(global_name, ERROR_VAR_ADDR_OUT_OF_RANGE); // Undefined variable address

				save_lst('l');

				switch (t_var[i].theType)
				{
					case 'V':
					{
						save_lab(txt, nul.i, bank, txt);

						if (t_var[i].addr < 0)         // T_VAR[I].ADDR < 0 means that the specified value is missing
						{
							for (y = t_var[i].cnt - 1; y >= 0; --y)
							{
								save_dta(t_var[i].initialValue, tmp, tType[t_var[i].mySize - 1], 0);
							}
						}
						break;
					}

					case 'S':
					{
						y = load_lab(t_var[i].structName, false);

						mne.i = t_lab[y].addr;           // find the index to the structure
						mne.l = byte(__struct_run);

						if (t_var[i].cnt > 1)
						{

							idx = addres;
							if (t_var[i].addr >= 0)
								addres = t_var[i].addr;

							tmp = t_var[i].structName + '[' + IntToStr(t_var[i].cnt) + ']';

							k = 1-1;
							create_struct_data(k, tmp, txt, mne.l);

							addres = idx;

						}
						else if (t_var[i].inZeroPage || t_var[i].addr >= 0)
						{
							// if .ZPVAR or specific address
							x = t_var[i].addr;
							create_struct_variable(txt, txt, mne, false, x);
						}
						else
						{
							create_struct_variable(txt, txt, mne, true, addres);
						}

						break;
					}
				}

				tmp = txt;
				zapisz_lst(txt);

				t_var[i].lok = -1;
			}			
		}
		nul.i = 0;
		save_lst(' ');                          // it must be here
		run_macro = old_run_macro;
	}
}

/**
 * \brief 
 * \param a 
 */
void add_block(string &a)
{
	inc(blok);
	if (blok > 0xFF)
		show_error(a, ERROR_TOO_MANY_BLOCKS);
}

/**
 * \brief BLK UPDATE ADDRESS 
 * \param kod 
 * \param a 
 * \param h 
 */
void save_blk(const int kod, string& a, const bool h)
{
	int x, y, i, j, k;
	bool hea_fd, hea_fe, ok, tst;
	string txt = a;

	for (k = blok; k >= 0; --k)
	{

		hea_fd = false;
		ok = false;

		// search in main block

		for (y = 0; y < rel_idx; ++y)
		{
			if (t_rel[y].idx == kod)
			{
				if (t_rel[y].block == 0)
				{
					if (t_rel[y].blk == k)
					{
						if (!hea_fd)
						{
							if (h)
							{
								save_lst('a');
								save_dstW(0xfffd);
								save_dst(byte(k));
								save_dstW(0x0000);
								zapisz_lst(txt);
							}

							hea_fd = true;
							ok = true;
						}

						save_lst('a');
						save_dst(0xfd);
						save_dstW(t_rel[y].addr);

						zapisz_lst(txt);

						t_rel[y].idx = -100;      // exclude from search
					}
				}
			}
		}

		// search in relocatable blocks
		for (x = 1; x <= blok; ++x)
		{
			hea_fe = false;
			j = 0;
			tst = false;

			for (y = 0; y < rel_idx; ++y)
			{
				if (t_rel[y].idx == kod)
				{
					if (t_rel[y].block == x)
					{
						if (t_rel[y].blk == k)
						{
							if (!tst)
							{
								save_lst('a');
								tst = true;
							}

							if (!hea_fe)
							{
								if (!hea_fd && h)
								{
									save_dstW(0xfffd);
									save_dst(byte(k));
									save_dstW(0x0000);

									hea_fd = true;
								}

								save_dst(0xfe);
								save_dst(byte(x));

								hea_fe = true;
								ok = true;
							}

							j = t_rel[y].addr - j;
							if (j >= 0xfa)
							{
								for (i = 0; i <= (j / 0xfa) - 1; ++i)
								{
									save_dst(0xff);
									dec(j, 0xfa);
								}
							}

							save_dst(byte(j));
							j = t_rel[y].addr;

							t_rel[y].idx = -100;      // exclude from search
						}
					}
				}
			}

			if (tst)
				zapisz_lst(txt);
		}

		if (ok)
		{
			save_lst('a');
			save_dst(0xfc);
			zapisz_lst(txt);
		}
	}
}

/**
 * \brief read the label type public, _ODD = index to the label in the T_LAB array
 * \param txt 
 * \param zm 
 * \param _odd 
 * \return 
 */
BYTE get_pubType(string &txt, string &zm, int &_odd)
{
	tByte result = 0;

	_odd = load_lab(txt, false);

	if (_odd < 0)
		error_und(zm, txt, WARN_UNDEFINED_SYMBOL);

	// public symbols cannot be labels assigned to
	// SMB, STRUCT, PARAM, EXT, MACRO

	if (t_lab[_odd].bank >= __id_param)
	{
		result = byte(t_lab[_odd].bank);
		if (not(result == __proc_run or result == __array_run or result == __struct_run))
			error_und(zm, txt, WARN_CANT_DECLARE_T_AS_PUBLIC);
	}
	return result;
}

/**
 * \brief Parse the 8-character SMB symbol
 * \param i 
 * \param zm 
 * \return 
 */
string get_smb(int &i, string &zm)
{
	string txt = get_string(i, zm, zm, true);

	if (length(txt) > 8) 
		show_error(zm, ERROR_SMB_LABEL_TOO_LONG);        // label name too long

	while (length(txt) < 8)
	{
		txt += ' ';      // align to 8 characters
	}

	for (int x = 0; x < 8; ++x)
		txt[x] = UpCase(txt[x]);

	return txt;
}

/**
 * \brief 
 * \param i 
 * \param zm 
 */
void blk_update_new(int &i, string &zm)
{
	string str;
	int war;

	string txt = zm;
	zapisz_lst(txt);
	str.erase();

	skip_spaces(i, zm);
	txt = get_dat(i, zm, ' ', true);
	if (txt.empty())
		show_error(zm, ERROR_UNEXPECTED_EOL);

	if (test_symbols)
	{
		for (war = High(t_sym) - 1; war >= 0; --war)
		{
			if (txt == t_sym[war])
			{
				error_und(zm, txt, WARN_LABEL_X_DECLARED_TWICE);
				return;
			}
		}
	}

	war = l_lab(txt);
	if (war < 0)
		error_und(zm, txt, WARN_UNDECLARED_LABEL);

	int k = t_lab[war].block;
	if (k == 0)
		show_error(zm, ERROR_ONLY_RELOC_BLOCK);

	save_lst('a');

	// dta a($fffc),b(blk_num),a(smb_off)
	// dta c'SMB_NAME'
	save_dstW(0xfffc);
	save_dst(byte(k));
	save_dstW(t_lab[war].addr);

	zapisz_lst(str);
	save_lst('a');

	txt = get_smb(i, zm);
	for (k = 0; k < 8; ++k)
	{
		save_dst(ord(txt[k]));
	}

	zapisz_lst(txt);
	bez_lst = true;
}

/**
 * \brief 
 */
void give_sym()
{
	string txt;

	if (sym_idx > 0)
	{
		if (addres < 0)
			show_error(global_name, WARN_NO_ORG_SPECIFIED);

		test_symbols = false;

		for (int i = sym_idx - 1; i >= 0; --i)
		{
			// we prepare lines for NEW SYMBOL
			txt = "BLK UPDATE NEW " + t_sym[i] + ' ' + '\'' + t_sym[i] + '\'';

			save_lst('a');

			int k = 16;
			blk_update_new(k, txt);
		}
	}
}

/**
 * \brief 
 * \param idx 
 * \param zm 
 */
void blk_empty(const int idx, string &zm)
{
	tCardinal indeks, tst;
	int _doo;
	string txt = zm;
	save_lst('a');

	add_block(zm);

	save_dstW(0xfffe);
	save_dst(byte(blok));
	save_dst(memType | 0x80);

	save_dstW(addres);
	save_dstW(idx);

	// if the label declaration occurred before the EMPTY block
	// find them and correct their block number
	indeks = addres;
	tst = addres + idx;

	for (_doo = 0; _doo < t_lab.size(); ++_doo)
	{
		if (t_lab[_doo].addr >= indeks && t_lab[_doo].addr <= tst)
		{
			t_lab[_doo].block = blok;
		}
	}

	zapisz_lst(txt);
	bez_lst = true;
}

/**
 * \brief 
 */
void give_back_to()
{
	string zm, txt;

	if (ds_empty > 0)
	{
		dec(addres, ds_empty);
		zm = "BLK EMPTY";

		if (dotRELOC.sdx)              // dotRELOC.sdx BLOK EMPTY
			blk_empty(ds_empty, zm);
		else
		{
			// dotRELOC.use BLOK EMPTY
			save_lst('a');

			save_dstW(__hea_address);
			save_dst(byte('E'));
			save_dstW(1);                // word(addres - rel_ofs) );
			save_dstW(ds_empty);

			txt = zm;
			zapisz_lst(txt);
		}

		save_hea();
		inc(addres, ds_empty);

		ds_empty = 0;
	}
}

/**
 * \brief 
 * \param zm 
 */
void blk_update_symbol(string &zm)
{
	string txt;
	tByte k;
	int _doo;

	if (smb_idx > 0)
	{

		txt = zm;
		zapisz_lst(txt);

		for (_doo = 0; _doo < smb_idx; ++_doo)
		{
			if (t_smb[_doo].used)
			{
				save_lst('a');

				// a($fffb),c'SMB_NAME',a(blk_len)
				save_dstW(0xfffb);

				txt = t_smb[_doo].name;
				for (k = 0; k < 8; ++k)
				{
					save_dst(ord(txt[k]));
				}
				if (t_smb[_doo].weak)
					save_dst(ord(txt[8]) | 0x80);
				else
					save_dst(ord(txt[8]));

				save_dstW(0x0000);

				zapisz_lst(txt);

				save_blk(_doo, txt, false);
			}
		}

		bez_lst = true;
	}
}

/**
 * \brief 
 * \param zm 
 */
void blk_update_address(string &zm)
{
	string txt;
	int idx, i, _odd;
	tByte v;
	bool ch;

	if (!dotRELOC.use)
	{
		save_blk(-1, zm, true);      // for the Sparta DOS X block
	}
	else
	{
		txt = zm;
		zapisz_lst(txt);
		// DEBUG
#ifdef TEST_DUMP
		if (rel_idx > 0)
		{
			cout << rel_idx << endl;
			for (i = 0; i < rel_idx; ++i)
				cout << Hex(t_rel[i].addr, 4) << ',' << t_rel[i].bank << ',' << t_siz[t_rel[i].idx].mySize << ',' << t_rel[i].idx << ',' << Hex(t_siz[t_rel[i].idx].lsb, 2) << endl;
		}
#endif

		// $FFEF header, type, number of addresses, addresses
		for (i = 1; i <= sizeof(relType); ++i)
		{
			// available types B,W,L,D,<,>,^

			ch = false;
			for (idx = 0; idx < rel_idx; ++idx)
			{
				if (t_siz[t_rel[idx].idx].mySize == relType[i-1])
				{
					ch = true;
					break;
				}
			}

			if (ch)
			{
				save_lst('a');

				// A(HEADER = $FFEF)
				save_dstW(__hea_address);

				// first we will write to the buffer to find out how many there are
				// maximum we can save $FFFF addresses
				_odd = 0;

				for (idx = 0; idx < rel_idx; ++idx)
				{
					if (t_siz[t_rel[idx].idx].mySize == relType[i-1])
					{
						t_tmp[_odd] = (cardinal(t_rel[idx].addr) & 0xFFFF) | (t_siz[t_rel[idx].idx].lsb << 16);

						inc(_odd);
						testRange(zm, _odd, 13);    // be sure to make error 13 to stop immediately
					}
				}

				v = ord(relType[i-1]);  //if old_case then v:=v or $80;

				save_dst(v);                 // TYPE //+ MODE

				zapisz_lst(txt);
				save_lst('a');

				// we save information about the number of DATA_LENGTH addresses
				save_dstW(_odd);

				// now we save address information

				for (idx = 0; idx < _odd; ++idx)
				{
					// external label bank if MODE=1
					//               if old_case then save_dst( byte(t_tmp[idx] shr 16) );

					// address for relocation
					save_dstW(t_tmp[idx]);


					switch (relType[i-1])
					{
						case '>':   save_dst(byte(t_tmp[idx] >> 16)); break;
						case '^':   show_error(zm, ERROR_XOR_NOT_RELOCATABLE); break;
					}

				}

				zapisz_lst(txt);

			} //if ch then begin
		}
	}

	bez_lst = true;
	first_org = true;
}

void blk_update_external(string &zm)
{
	const static char typExt[3] = { 0, '<','>' };
	string txt;
	int idx, _doo, _odd, i;
	tByte v;
	bool ch;

	if (dotRELOC.sdx)
		return;

	if (extn_idx > 0)
	{
		txt = zm;
		zapisz_lst(txt);
		save_lst('a');

#ifdef TEST_DUMP
		cout << ext_idx << endl;
		for (i = 0; i < ext_idx; ++i)
			cout << Hex(t_ext[i].addr, 4) <<  ',' << t_extn[t_ext[i].idx].name << endl;
#endif

		for (_doo = 0; _doo < extn_idx; ++_doo)
		{
			for (i = 0; i <= 2; ++i)
			{
				ch = false;
				for (idx = 0; idx < ext_idx; ++idx)
				{
					if (t_ext[idx].idx == _doo && t_ext[idx].typ == typExt[i])
					{
						ch = true;
						break;
					}
				}

				if (ch)
				{
					// whether there are any references to external labels in the program

					save_lst('a');

					// A(HEADER = $FFEE),b(TYPE)
					save_dstW(__hea_external);

					// first we will write to the buffer to find out how many there are
					// we can save maximum $FFFF addresses and their bank numbers
					_odd = 0;

					for (idx = 0; idx < ext_idx; ++idx)
					{
						if (t_ext[idx].idx == _doo && t_ext[idx].typ == typExt[i])
						{
							t_tmp[_odd] = (t_ext[idx].addr & 0xFFFF) | (t_ext[idx].lsb << 16);

							inc(_odd);
							testRange(zm, _odd, 13);    // be sure to make error 13 to stop immediately
						}
					}

					if (typExt[i] == 0)
						v = ord(t_extn[_doo].mySize);  //if old_case then v:=v or $80;
					else
						v = ord(typExt[i]);

					save_dst(v);                    // ext_label TYPE //+ MODE

					zapisz_lst(txt);
					save_lst('a');

					// we save information about the number of addresses
					save_dstW(_odd);

					// A(EXT_LABEL_LENGTH) , C'EXT_LABEL'
					txt = t_extn[_doo].name;

					save_dstS(txt);                           // ext_label length, string

					// now we save address information
					for (idx = 0; idx < _odd; ++idx)
					{
						// external label bank
						//               if old_case then save_dst( byte(t_tmp[idx] shr 16) );

						// external label address
						save_dstW(t_tmp[idx]);

						if (typExt[i] == '>')
							save_dst(byte(t_tmp[idx] >> 24));
					}

					zapisz_lst(txt);

				} //if ch then begin
			} // for i:=0 to 2 do begin
		}

		bez_lst = true;
	}
}

/**
 * \brief 
 * \param zm 
 */
void blk_update_public(string &zm)
{
	string txt, ety, str, tmp;
	int x, idx, j, k, indeks, _odd, _doo, i, len, old_rel_idx, old_size_idx;
	int sid, sno, six;
	tByte v, sv;
	char ch, tp;
	bool test;
	tInt64 war;
	argumentSizeEntry old_sizm0, old_sizm1;

	if (dotRELOC.sdx)
		return;

	if (pub_idx > 0)
	{
		txt = zm;
		zapisz_lst(txt);
		save_lst('a');

		_odd = 0;
		_doo = -1;

		// test for the presence of public labels and test for the parameters of the __pVar procedure
		for (idx = pub_idx - 1; idx >= 0; --idx)
		{
			txt = t_pub[idx].name;

			v = get_pubType(txt, zm, _odd);

			// we add the parameters of the __pVar procedure to be made public
			// provided that we haven't made them public before
			if (v == __proc_run)
			{
				if (t_prc[t_lab[_odd].addr].myType == __pVar)
				{

					k = t_prc[t_lab[_odd].addr].nrParams;        // number of parameters
					indeks = t_prc[t_lab[_odd].addr].str;

					if (k > 0)
					{
						for (j = 0; j < k; ++j)
						{
							ety = t_par[j + indeks];

							// we omit the first character specifying the type of the parameter
							// and we read until we encounter characters that do not belong to the label name

							str.erase();
							i = 2-1;
							len = length(ety);

							while (i <= len)
							{
								if (_lab(ety[i]))
									str += ety[i];
								else
									break;
								inc(i);
							}

							_doo = l_lab(str);

							test = false;
							for (x = pub_idx - 1; x >= 0; --x)
							{
								if (t_pub[x].name == str)
								{
									test = true;
									break;
								}
							}

							if (_doo >= 0 && !test)  // we add it to T_PUB if it has not been made public before
							{
								if (t_lab[_doo].bank != __id_ext)
									save_pub(str, zm);
							}
						}
					}

					if (_doo < 0)
						error_und(zm, str, WARN_UNDECLARED_LABEL);
				}
			}
		}

		// we determine the label type PUBLIC, whether it is relocatable
		branch = false;       // we enable relocation

		old_rel_idx = rel_idx;
		old_size_idx = size_idx;
		old_sizm0 = t_siz[size_idx];
		old_sizm1 = t_siz[size_idx - 1];

		for (_odd = pub_idx - 1; _odd >= 0; --_odd)
		{
			txt = t_pub[_odd].name;
			reloc = false;

			calculate_value(txt, zm);
			t_pub[_odd].isVariable = reloc;
		}

		branch = true;        // we block relocation

		rel_idx = old_rel_idx;
		size_idx = old_size_idx;
		t_siz[size_idx]  = old_sizm0;
		t_siz[size_idx - 1] = old_sizm1;

#ifdef TEST_DUMP
		cout << pub_idx << endl;
		for (x = 0; x < pub_idx; ++x)
			cout << t_pub[x].name << ',' << t_pub[x].isVariable << endl;
#endif
		// A(HEADER = $FFED) , a(LENGTH)
		save_dstW(__hea_public);
		save_dstW(pub_idx);

		for (idx = 0; idx < pub_idx; ++idx)
		{
			txt = t_pub[idx].name;
			ety = txt;
			save_lst('a');

			v = get_pubType(txt, zm, _odd);  // V is the type, _ODD is the index to T_LAB

			switch (v)
			{
				case __proc_run:	ch = 'P'; break;		// P-ROCEDURE
				case __array_run:	ch = 'A'; break;		// A-RRAY
				case __struct_run:	ch = 'S'; break;		// S-TRUCT
				default:
				{
					if (t_pub[idx].isVariable)
						ch = 'V';							// V-ARIABLE
					else
						ch = 'C';							// C-ONSTANT
				}
			}

			tp = 'W';

			if (ch == 'C')
			{
				// type B-YTE, W-ORD, L-ONG, D-WORD
				war = t_lab[_odd].addr;

				tp = relType[ValueToType(war)-1];

				save_dst(ord(tp));     // type
			}
			else
				save_dst(byte('W')); // type W-ORD dla V-ARIABLE, P-ROCEDURE, A-RRAY, S-TRUCT

			save_dst(ord(ch));      // label_type V-ARIABLE, C-ONSTANT, P-ROCEDURE, A-RRAY, S-TRUCT

			save_dstS(txt);         // label_name     [length + atascii]

			switch (v)
			{
				case __proc_run:	k = t_prc[t_lab[_odd].addr].addr; break;	// PROC address
				case __array_run:  k = t_arr[t_lab[_odd].addr].addr; break;	// ARRAY address
				case __struct_run:	k = t_str[t_lab[_odd].addr].offset; break;	// number of fields STRUCT
				default:
					k = t_lab[_odd].addr;              // variable address
			}

			if (ch != 'C' && ch != 'S') 
				dec(k, rel_ofs);   // we do not modify the C-ONSTANT and S-TRUCT values

			for (sv = 1; sv <= TypeToByte(tp); ++sv)
			{
				// the value of a variable, constant or procedure
				save_dst(byte(k));             // size TP (B-YTE, W-ORD, L-ONG, D-WORD)
				k = k >> 8;
			}


			// S-TRUCT
			if (v == __struct_run)
			{
				sid = t_str[t_lab[_odd].addr].id;
				sno = t_str[t_lab[_odd].addr].offset;

				for (j = 0; j < sno; ++j)
				{
					six = loa_str_no(sid, j);
					txt = t_str[six].labelName;
					save_dst(byte(relType[t_str[six].mySize - 1]));
					save_dstS(txt);
					save_dstW(word(t_str[six].nrRepeat));

#ifdef TEST_DUMP
					cout << t_str[six].labelName;		// structure name
					cout << ',' << t_str[six].offset;	// offset or the number of fields of the structure
					cout << ',' << t_str[six].mySize;	// total length in bytes
					cout << ',' << Hex(t_str[six].nrRepeat, 4);
					cout << ',' << t_str[six].no) << endl;
#endif				
				}
			}
			else if (v == __array_run)
			{
				// A-RRAY
				save_dstW(t_arr[t_lab[_odd].addr].elm[0].count);  // maximum array index
				save_dst(ord(relType[t_arr[t_lab[_odd].addr].siz - 1]));   // size pol
			}
			else if (v == __proc_run)
			{
				// P-ROCEDURE
				k = t_prc[t_lab[_odd].addr].reg;
				save_dst(byte(k));             // register order
				ch = t_prc[t_lab[_odd].addr].myType;
				save_dst(ord(ch));             // procedure type ' '__pDef, 'R'__pReg, 'V'__pVar
				k = t_prc[t_lab[_odd].addr].nrParams;   // number of parameters
				indeks = t_prc[t_lab[_odd].addr].str;

				if (k > 0 && ch == __pVar)    // if __pVar, we also include the length
				{
					int loopK = k;
					for (j = 0; j < loopK; ++j)
					{
						// parameter names
						tmp = t_par[j + indeks];
						inc(k, length(tmp) + 2 - 1);
					}
				}

				save_dstW(k);                  // number of parameter data
				k = t_prc[t_lab[_odd].addr].nrParams;   // number of parameters again

				if (k > 0)                      // if there are parameters, we will save them
				{
					for (j = 0; j < k; ++j)
					{
						tmp = t_par[j + indeks];
						save_dst(ord(tmp[1-1]));
						if (ch == __pVar)
						{
							// additionally the length and name of the label if __pVar
							txt = copy(tmp, 2 - 1, length(tmp)); // we skip the first type character
							save_dstS(txt);
						}

					}
				}
			}

			zapisz_lst(ety);
		}

		bez_lst = true;
	}
}

/**
 * \brief 
 * \param i 
 * \param zm 
 *ety\param a 
 *glob\param a 
 */
 void compound_operator(int &i, string &zm, string &ety, const bool glob)
{
	string txt;

	if (ety[0] != '?')
		show_error(zm, ERROR_IMPROPER_SYNTAX); // these operations apply to temporary labels

	if (glob)
		txt = ":";
	else
		txt.erase();

	txt += ety + zm[i];

	if (if_test)
	{
		if (zm[i + 1] == '=')
		{
			myInsert(txt, zm, (i + 2));      // we modify the lines e.g. ?tmp+=3 -> ?tmp=?tmp+3
			myDelete(zm, i, 1);
		}
		else
		{
			test_eol(i + 2, zm, zm, 0);

			zm = ety + '=' + txt + '1';     // we modify the lines ?tmp++ -> ?tmp=?tmp+1
			i = length(ety);				// we modify the lines ?tmp-- -> ?tmp=?tmp-1
		}
	}
}

/**
 * \brief saving the element to the .ARRAY array
 * \param i 
 * \param zm 
 * \param idx 
 */

void get_data_array(int &i, string &zm, const int idx)
{
	string ety, str;
	int k, j, _odd;
	tByte typ;

	save_lst(' ');

	skip_spaces(i, zm);

	if (!AllowStringBrackets.has(zm[i]))
	{
		ety = '[' + IntToStr(array_used.max) + ']';

		if (zm[i] == '=')
			IncAndSkip(i, zm);

	}
	else
	{

		ety = get_dat_noSPC(i, zm, zm, '=');
		if (ety.empty())
			ety = "[0]";

		if (zm[i] != '=')
			show_error(zm, ERROR_IMPROPER_SYNTAX);
		else
			IncAndSkip(i, zm);

	}

	str = get_dat_noSPC(i, zm, zm, '\\');

	typ = t_arr[idx].siz;

	k = 1-1;

	while (true)
	{
		_odd = read_elements(k, ety, idx, false);

		array_used.idx = _odd / typ;
		array_used.typ = tType[typ - 1];

		if (!loop_used && !FOX_ripit && (length(t) + 7 < margin))
		{
			t += " [" + Hex(cardinal(_odd), 4) + ']';
		}

		j = 1 - 1;
		calculate_data(j, str, zm, typ);

		skip_spaces(k, ety);
		if ((k >= ety.length()-1) or (ety[k] != ':'))
			break;
		else
			IncAndSkip(k, ety);
	}

	if (k < length(ety))
	{
		string err;
		err += ety[k];

		show_error(zm, WARN_ILLEGAL_CHARACTER, err);
	}
}

/**
 * \brief we increase the PROC_IDX counter and therefore PROC_NR
 */
void add_proc_nr()
{
	inc(proc_idx);

	proc_nr = proc_idx;     // PROC_IDX is the first free entry in T_PRC
	// each new .PROC block must be given a new unique number

	if (proc_idx >= t_prc.size()) 
		t_prc.resize(proc_idx + 1);
}

/**
 * \brief Update the value of BANK, ADDRESS and procedure parameters of the ' ' type
 * \param ety 
 * \param zm 
 * \param a 
 */
void upd_procedure(string& ety, string& zm, const int a)
{
	string str, txt, b, tmp, add;
	int _doo, _odd, idx, i, len;
	tCardinal tst;
	bool old_bool, old_macro;

	t_prc[proc_nr].bank = bank;
	t_prc[proc_nr].addr = a;

	t_prc[proc_nr].offset = org_ofset;

	str = local_name + ety;
	save_the_label(str, a, bank, ety[1-1]);

	old_macro = run_macro;
	run_macro = false;

	old_bool = dotRELOC.use;   // so that the procedure parameters are not relocatable
	dotRELOC.use = false;

	_doo = t_prc[proc_nr].nrParams;    // number of declared parameters in the current procedure

	if (_doo > 0)
	{
		switch (t_prc[proc_nr].myType)
		{
			case __pDef:
			{
				// if the procedure had declared parameters, then
				// we will read the address for the parameters contained in @PROC_VARS_ADR
				// and update the addresses of the procedure parameters
				tst = adr_label(__PROC_VARS_ADR, true);   // @proc_vars_adr
				proc = true;                  // PROC=TRUE and PROC_NAME
				proc_name = ety + '.';          // allows you to update label addresses

				// force execution of macros @PULL 'I'
				zm = " @PULL 'I'," + IntToStr(t_prc[proc_nr].paramMemSize);

				idx = t_prc[proc_nr].str;

				for (_odd = 0; _odd < _doo; ++_odd)
				{
					// !!! this is the order of the FOR loops!!!
					txt = t_par[idx + _odd];
					str = copy(txt, 2-1, length(txt));

					// we will update the label addresses of the procedure parameters
					save_lab(str, tst, __id_param, zm);

					tst += TypeToByte(txt[0]);
				}
				break;
			}

			case __pVar:
			{
				idx = t_prc[proc_nr].str;

				for (_odd = _doo - 1; _odd >= 0; --_odd)
				{
					txt = t_par[idx + _odd];

					// we load the parameter name, we remove the first character defining the type
					// and characters that do not belong to the label name

					str.erase();
					add.erase();
					i = 2-1;
					len = length(txt);

					while (i < len)
					{
						if (_lab(txt[i]))
							str += txt[i];
						else
							break;
						++i;
					}

					while (i < len)
					{
						add += txt[i];
						++i;
					}

					b = local_name + ety + '.';

					tmp = b + str;
					i = l_lab(tmp);

					// we are looking for the closest path for the parameter
					while ((i < 0) && (pos('.', b) > 0))
					{
						cut_dot(b);

						tmp = b + str;
						i = l_lab(tmp);
					}

					t_par[idx + _odd] = txt[1-1] + tmp + add;
				}
				break;
			}
		}
	}

	dotRELOC.use = old_bool;
	run_macro = old_macro;
	save_lst(' ');
}

/**
 * \brief 
 * \param a 
 * \return 
 */
char ByteToReg(tByte &a)
{
	char result;
	switch (a & 0xc0)
	{
		case 0x40: result = 'X'; break;
		case 0x80: result = 'Y'; break;
		default:
			result = 'A';
	}

	a = byte(a << 2);
	return result;
}

/**
 * \brief
 */
 tByte RegToByte(const char a)
{
	tByte result;
	switch (a)
	{
		case 'A':   result = 0x00; break;
		case 'X':   result = 0x55; break;
		case 'Y':   result = 0xaa; break;
		default:
			result = 0xff;
	}
	return result;
}

/**
 * \brief we read and remember the types and names of parameters in the .PROC declaration
 *			depending on how the parameters are passed, the number is increased
 *			PASS_END assembly passes
 * \param ety 
 * \param nam 
 * \param zm 
 * \param i 
 */
void get_procedure(string &ety, string &nam, string &zm, int &i)
{
	int k, l, nr_param, j;
	string txt, str;
	char ch, ptype;
	tByte v;
	bool old_bool;

	bool treg[3];
	string all;				// Limit to length 4

	reserved_word(ety, zm);               // check if the name .PROC is allowed
	old_bool = run_macro;
	run_macro = false;

	save_lab(ety, proc_nr, __id_proc, zm);  // current index to T_PRC = PROC_NR

	run_macro = old_bool;

	old_bool = dotRELOC.use;            // it is necessary that the parameters of the procedure
	dotRELOC.use = false;                 // they were not relocatable

	t_prc[proc_nr].name = nam;           // the correct name of the procedure
	t_prc[proc_nr].bank = bank;          // saving the procedure bank
	t_prc[proc_nr].addr = addres;         // saving the procedure address

	t_prc[proc_nr].myType = __pDef;        // default procedure type 'D'

	t_prc[proc_nr].str = High(t_par);   // index to T_PAR, there will be procedure parameters there

	skip_spaces(i, zm);

	k = 0;
	nr_param = 0;
	str = zm[i];

	if (!test_char(i, zm, '(', 0))
		show_error(zm, ERROR_EXTRA_CHARS_ON_LINE);

	// we read declarations of parameters delimited by brackets ( )
	if (zm[i] == '(')
	{
		all.erase();                 // Here we will save the CPU register combinations

		treg[0] = false;
		treg[1] = false;
		treg[2] = false;

		proc = true;
		proc_name = ety + '.';

		// we check the correctness of the brackets, remove the starting and ending brackets
		txt = bounded_string(i, zm, true);
		skip_spaces(i, zm);
		ptype = __pDef;          // default procedure type __pDef

		// we check whether the procedure type .REG is specified
		if (zm[i] == '.')
		{
			//str:=get_datUp(i,zm,#0,false);
			str = get_directive(i, zm);
			v = fCRC16(str);

			if (not(v == __reg or v == __var))
				error_und(zm, str, WARN_UNKNOWN_DIRECTIVE);

			ptype = str[2 - 1];
			t_prc[proc_nr].myType = ptype;
		}

		if (ptype == __pVar)
		{
			if (pass_end < 3)
				pass_end = 3;     // if there are .VAR parameters, there must be at least 3 runs
		}

		// we check the presence of declarations for the MADS software stack

		if (ptype == __pDef)
		{
			adr_label(__STACK_POINTER, true);   // @stack_pointer, test for the presence of a label declaration
			adr_label(__STACK_ADDRESS, true);   // @stack_address, test for the presence of a label declaration
			adr_label(__PROC_VARS_ADR, true);   // @proc_vars_adr, test for the presence of a label declaration
		}

		j = 1 - 1;
		ch = ' ';                  // if ch=' ' then read the parameter type

		while (j < length(txt))
		{
			if (ch == ' ')
			{
				v = get_type(j, txt, zm, true);
				ch = relType[v - 1];       // parameter type 'B', 'W', 'L', 'D' with relType
			}

			// read the parameter name or !!! expression!!! (e.g. label+1)
			// parameters can be separated by a comma
			// space is used to specify a new type of parameter

			skip_spaces(j, txt);
			str = get_dat(j, txt, ',', true);    // parameter or expression
			if (str.empty())
				show_error(zm, ERROR_LABEL_NAME_REQUIRED);

			// if .REG, the parameter names are 1-letter A,X,Y or 2-letter AX, AY, etc.
			// parameter names can only be repeated once
			// only parameters of the type .BYTE, .WORD, .LONG are allowed for .REG
			if (ptype == __pReg)
			{

				if (TypeToByte(ch) != length(str))
					error_und(zm, str, WARN_INCOMPATIBLE_TYPES);

				for (l = 1 - 1; l < length(str); ++l)
				{
					v = RegToByte(UpCase(str[l])) & 3;

					if (v == 3)
					{
						string err;
						err += str[l];
						error_und(zm, err, WARN_CPU_DOES_NOT_HAVE_REGISTER);	// CPU doesn't have register ?
					}
					else if (treg[v])
						show_error(zm, ERROR_CPU_NOT_ENOUGH_REGISTERS);			// CPU doesn't have so many registers
					else
						treg[v] = true;
				}
			}

			skip_spaces(j, txt);

			// we initially remember the parameter labels as the NR_PARAM value for __pDef
			switch (ptype)
			{
				case __pDef: save_lab(str, nr_param, __id_param, zm); break;
				case __pReg: all += str; break;
			}

			str = ch + str;
			save_par(str);

			inc(k);              // we increase the counter of read procedure parameters

			inc(nr_param, TypeToByte(ch));   // we increase NR_PARAM by the data length

			if (txt[j] != ',')
				ch = ' ';
			else
				inc(j);

		}    // while

	}     // if zm[i] == '('

	if (t_prc[proc_nr].myType == __pReg)
	{
		// a maximum of 3 bytes can be transferred using registers
		if (nr_param > 3)
			show_error(zm, ERROR_CPU_NOT_ENOUGH_REGISTERS);

		// we save the order of registers
		int len = static_cast<int>(all.length());
		t_prc[proc_nr].reg = (RegToByte(len > 0 ? all[1-1] : 0) & 0xc0) | (RegToByte(len > 1 ? all[2-1] : 0) & 0x30) | (RegToByte(len > 2 ? all[3-1] : 0) & 0x0c);
	}

	t_prc[proc_nr].nrParams = k;
	t_prc[proc_nr].paramMemSize = nr_param;
	t_prc[proc_nr].used = !exclude_proc;
	dotRELOC.use = old_bool;
}

/**
 * \brief testing the completion of file assembly
 * \param zm 
 * \param do_exit 
 */
void test_exists(string& zm, const bool do_exit)
{
	if (!do_exit)
	{
		if (!run_macro && !icl_used)
		{
			if (ifelse != 0)
				show_error(zm, MSG_MISSING_ENDIF);
			else if (proc || macro)
				show_error(zm, WARN_MISSING_DOT_END);
			else if (structure.use) // !!! necessarily !!! ... or macro
				show_error(zm, ERROR_MISSING_DOT_ENDS);
			else if (enumeration.use)
				show_error(zm, WARN_REFERENCED_LABEL_NOT_DEFINED);
			else if (aray)
				show_error(zm, ERROR_MISSING_DOT_ENDA);
			else if (rept)
				show_error(zm, ERROR_MISSING_DOT_ENDR);
			else if (local_nr > 0)
				show_error(zm, ERROR_MISSING_DOT_ENDL);
			else if (pag_idx > 0)
				show_error(zm, ERROR_MISSING_DOT_ENDPG);
			else if (whi_idx > 0)
				show_error(zm, ERROR_MISSING_DOT_ENDW);
			else if (test_idx > 0)
				show_error(zm, ERROR_MISSING_DOT_ENDT);
			else if (segment > 0)
				show_error(zm, ERROR_MISSING_DOT_ENDSEG);
		}
	}
}

/**
 * \brief
 */
tByte fgetB(int &i)
{
	tByte result = t_lnk[i];
	++i;
	return result;
}

/**
 * \brief
 */
 int fgetW(int &i)
{
	int result = t_lnk[i] + (t_lnk[i + 1] << 8);
	inc(i, 2);
	return result;
}

/**
 * \brief
 */
 tCardinal fgetL(int &i)
{
	tCardinal result = t_lnk[i] + (t_lnk[i + 1] << 8) + (t_lnk[i + 2] << 16);
	inc(i, 3);
	return result;
}

/**
 * \brief
 */
 tCardinal fgetD(int& i)
{
	tCardinal result = t_lnk[i] + (t_lnk[i + 1] << 8) + (t_lnk[i + 2] << 16) + (t_lnk[i + 3] << 24);
	inc(i, 4);
	return result;
}

/**
 * \brief
 */
 string fgetS(int &i)
{
	string result;
	int x = fgetW(i);
	for (int y = x - 1; y >= 0; --y)
	{
		result += chr(fgetB(i));
	}
	return result;
}

/**
 * \brief
 */
 void flush_link()
{
	int j;

	int k = dotLINK.linkedLength;

	save_lst('a');

	if (k > 0)
	{
		for (j = 0; j <= k - 1; ++j)
		{
			save_dst(t_ins[j]);
		}
	}

	inc(addres, k);

	if (dotLINK.emptyLength > 0)
	{
		save_hea();
		org = true;
		inc(addres, dotLINK.emptyLength);
		dotLINK.emptyLength = 0;
	}
}


/**
 * \brief new assembly address for .PROC or .LOCAL
 * \param i 
 * \param zm 
 */
void get_address(int &i, string &zm)
{
	string txt;

	t_end[end_idx].restoreAddr = addres;

	// we check whether there is a new assembly address for .PROC or .LOCAL
	skip_spaces(i, zm);

	if (zm[i] == ',')
	{
		IncAndSkip(i, zm);
		txt = get_dat(i, zm, '(', true);

		block_record = true;  // we force saving the ORG if it occurred earlier
		save_dst(0);
		block_record = false;

		org_ofset = addres;

		addres = integer(calculate_value(txt, zm));

		org_ofset = addres - org_ofset;

		skip_spaces(i, zm);
	}
}

/**
 * \brief 
 */
void get_address_update()
{
	dec(addres, t_end[end_idx - 1].addr);
	inc(addres, t_end[end_idx - 1].restoreAddr);

	dec(end_idx);
}

/**
 * \brief 
 * \param k 
 * \param ety 
 * \param zm 
 * \param v 
 */
void save_externalLabel(int k, string& ety, string& zm, tByte v)
{
	string txt;
	tByte len;
	bool isProc;

	skip_spaces(k, zm);

	if (v == __proc)
	{
		// the external label declares the procedure
		proc_nr = proc_idx;                     // necessarily via ADD_PROC_NR

		if (pass == 0)
		{
			txt = "##" + ety;                       // write the label with ## characters
			get_procedure(txt, ety, zm, k);         // we read the parameters
			proc = false;                         // be sure to turn off PROC
			proc_name.erase();                       // and be sure to PROC_NAME:=''
		}

		add_proc_nr();                           // we must increase the number
		save_lab(ety, extn_idx, __id_ext, zm);
		len = 2;                                // type .WORD
		isProc = true;
	}
	else
	{
		dec(v, __byteValue);                   // normal external label
		save_lab(ety, extn_idx, __id_ext, zm);
		len = v;
		isProc = false;
	}

	t_extn[extn_idx].name = ety;              // SAVE_EXTN
	t_extn[extn_idx].mySize = relType[len - 1];   //
	t_extn[extn_idx].isProcedure = isProc;    //

	inc(extn_idx);

	if (extn_idx >= t_extn.size())
		t_extn.resize(extn_idx + 1);
}


/**
 * \brief 
 * \param zm 
 * \param i 
 * \param typ 
 */
void get_maeData(string &zm, int &i, const char typ)
{
	int _odd, _doo, idx, j, tmp;
	string txt, hlp;
	tByte v, war;
	bool yes;
	_strArray par;

	skip_spaces(i, zm);
	if (test_char(i, zm))
		show_error(zm, ERROR_UNEXPECTED_EOL);

	if (proc)
		yes = t_prc[proc_nr - 1].used;
	else
		yes = true;

	data_out = true;    // force to be shown in the LST file when executing the macro
	save_lst('a');

	SetLength(par, 1);
	get_parameters(i, zm, par, true);

	_doo = High(par);

	if (_doo > 0)
	{
		_odd = 0;
		war = 0;

		if ((typ == 'B' || typ == 'S' || typ == 'C') && _doo > 1)
		{
			txt = par[0];     // possible value that we will add to the rest of the sequence
			// provided the string has more than 1 element

			if (txt.empty())
				show_error(zm, ERROR_IMPROPER_SYNTAX);

			if (txt[1 - 1] == '+' || txt[1 - 1] == '-')
			{
				j = 1 - 1;
				war = byte(calculate_value_noSPC(txt, zm, j, 0, 'B'));
				inc(_odd);
			}
		}

		for (idx = _odd; idx < _doo; ++idx)
		{
			// proper decoding of the string
			txt = par[idx];

			if (!txt.empty())
			{
				if (AllowQuotes.has(txt[1 - 1]))
				{
					// if text string
					j = 1 - 1;
					hlp = get_string(j, txt, zm, true);
					skip_spaces(j, txt);

					if (length(txt) > j)
					{
						if (txt[j] != '*' && txt[j] != '+' && txt[j] != '-')
						{
							string err;
							err[0] = txt[j];
							show_error(zm, WARN_ILLEGAL_CHARACTER, err);
						}
					}

					v = war;
					inc(v, test_string(j, txt, 'F'));

					if (typ == 'C')
						hlp[length(hlp)] += 0x80;

					if (yes)
					{
						if (typ == 'S')
							save_dta(0, hlp, 'D', v);
						else
							save_dta(0, hlp, 'C', v);
					}

				}
				else
				{
					switch (typ)
					{
						case 'B':
						case 'H':
						case 'S':
						case 'C':
						{
							if (typ == 'H')
								txt = '$' + txt;

							j = 1 - 1;
							v = byte(calculate_value_noSPC(txt, zm, j, 0, 'B'));

							if (typ == 'S')
								v = ata2int(v);

							inc(v, war);

							if (yes)
							{
								save_dst(v);
								inc(addres);
							}
							break;
						}

						case 'W':
						case 'D':
						{
							j = 1 - 1;
							tmp = integer(calculate_value_noSPC(txt, zm, j, 0, 'A'));

							if (yes)
							{
								if (typ == 'W')
									save_dstW(tmp);
								else
								{
									save_dst(byte(tmp >> 8));   // hi
									save_dst(byte(tmp));         // lo
								}
							}

							if (yes)
								inc(addres, 2);
							break;
						}
					}
				}
			}
		}
	}
	else
		show_error(zm, ERROR_IMPROPER_SYNTAX);  // e.g. '.by .len(temp)'
}

/**
 * \brief we generate code testing the condition for .WHILE, .TEST
 * \param lar 
 * \param rar 
 * \param old 
 * \param jump 
 * \param typ
 * \param op 
 * \return 
 */
int5 asm_test(string& lar, string& rar, string& old, string& jump, const tByte typ, const tByte op)
{
	int5 hlp;
	string txt;
	int adr;
	int5 result;

	adr = addres;

	if (lar[1-1] == '#')
		show_error(old, ERROR_IMPROPER_SYNTAX);   // unrealistic argument :)

	switch (typ)
	{
		case 1:   txt = "CPB"; break;
		case 2:   txt = "CPW"; break;
		case 3:   txt = "CPL"; break;
		default:  txt = "CPD"; break;
	}

	TestWhileOpt = (op != 0 && op != 4);    // shorter code if operator '=', '<>'

	txt += " " + lar + " " + rar;
	result = asm_mnemo(txt, old);

	switch (op)
	{
		case 0:   txt = "JNE"; break;                   // <>
		case 1:   txt = "JCS"; break;                   // >=
		case 2:   txt = "JCC " + jump; break;           // <=
		case 4:   txt = "JEQ"; break;                   // =
		case 5:   txt = "JCC"; break;                   // <
		case 6:   txt = "SEQ"; break;                   // >
	}

	if (op == 2 || op == 6)
	{
		hlp = asm_mnemo(txt, old);
		addResult(hlp, result);

		if (op == 2)
			txt = "JEQ";
		else
		{
			test_skipa();
			txt = "JCS";
		}
	}

	txt += ' ' + jump;
	hlp = asm_mnemo(txt, old);
	addResult(hlp, result);

	addres = adr;                      // we restore the initial ADDRESS value

	return result;
}

/**
 * \brief Note that _r is shifted down by 1
 * \param _v 
 * \param _r 0 = .BYTE, 1 = .WORD, 2 = .LONG, 3 = .DWORD [0..3]
 * \param long_test 
 * \param _lft 
 * \param _rgt 
 */
void create_long_test(const tByte _v, const tByte _r, string &long_test, string &_lft, string &_rgt)
{
#ifdef _DEBUG
	if (_r < 0 || _r > 3)
	{
		cout << "create_long_test() has invalid param _r=" << (int)_r << endl;
		exit(1);
	}
#endif
	string txt;

	switch (_v ^ 4)
	{
		case 0:   txt = "<>"; break;
		case 1:   txt = ">="; break;
		case 2:   txt = "<="; break;
		case 4:   txt = "="; break;
		case 5:   txt = "<"; break;
		case 6:   txt = ">"; break;
	}

	if (long_test.empty() == false)
		long_test += '\\';
	long_test += ".TEST " + mads_param[_r] + _lft + txt + _rgt + R"(\ LDA:SNE#1\.ENDT\ LDA#0)";
	long_test += R"(\.IF .GET[?I-1]=41\ AND EX+?J-2+1\ STA EX+?J-2+1\ ?I--\ ELS\ STA EX+?J+1\ ?J+=2\ EIF\)";
}

/**
 * \brief generating code for the .WHILE, .TEST condition
 * \param i 
 * \param long_test 
 * \param zm 
 * \param left 
 * \param right 
 * \param v 
 * \param r 
 * \param strend 
 */
void conditional_expression(int &i, string &long_test, string &zm, string &left, string &right, tByte &v, tByte &r, const string &strend)
{
	string str, txt, _lft, _rgt, kod;
	int k, j;
	tByte _v, _r;

	if (zm.empty())
		show_error(zm, ERROR_UNEXPECTED_EOL);

	r = get_type(i, zm, zm, false);
	skip_spaces(i, zm);

	str = get_dat(i, zm, 0, false);
	k = 1-1;

	right.erase();
	left.erase();        // we read to LEFT until we encounter the operators '=', '<', '>', '!'

	if (!str.empty())
	{
		while (!_ope(str[k]) && !test_char(k, str))
		{
			if (AllowBrackets.has(str[k]))
				left += bounded_string(k, str, false);
			else
			{
				if (str[k] != ' ')
					left += str[k];
				inc(k);
			}
		}
	}

	if (r == 0 && (var_idx > 0))
	{
		// no type, if the label was .VAR, we will read the type
		for (j = static_cast<int>(t_var.size()) - 1; j >= 0; --j)
		{
			if (t_var[j].theType == 'V' && t_var[j].name == left)
			{
				r = t_var[j].mySize;
				break;
			}
		}
	}

	if (r == 0)
		r = 2-1;                         // the default type is .WORD

	skip_spaces(k, str);
	v = 0xff;

	if (!left.empty())
	{

		//----------------------------------------------------------------------------
		// 0 <>		4 =			xor 4
		// 1 >=		5 <
		// 2 <=		6 >
		//----------------------------------------------------------------------------

		switch (str[k])
		{
			// we are looking for a known combination of operators
			case 0:		v = 0x80; break;             // end of sequence
			case '=':
			{
				switch (str[k + 1])
				{
					case '=':
					{
						v = 4;       // ==
						inc(k);
						break;
					}
					default:
						v = 4;      // =
				}
				break;
			}
			case '!':
			{
				switch (str[k + 1])
				{
					case '=':   v = 0; break;        // !=
				}
				break;
			}

			case '<':
			{
				switch (str[k + 1])
				{
					case '>':   v = 0; break;        // <>
					case '=':   v = 2; break;        // <=
					default:	v = 5;               // <
				}
				break;
			}

			case '>':
			{
				switch (str[k + 1])
				{
					case '=':   v = 1; break;       // >=
					default:	v = 6;              // >
				}
				break;
			}
		}


		if (v < 0x80)
		{
			if (v < 4)
				inc(k, 2);
			else
				inc(k);

			if (_ope(str[k]))
			{
				string err;
				err = str[k];
				show_error(zm, WARN_ILLEGAL_CHARACTER, err);
			}

			v = v xor 4;

			skip_spaces(k, str);

			right = get_dat(k, str, 0, true);
		}
		else if (v == 0x80)
		{
			// for empty string default operation '<>0'
			v = 4;
			right = "#0";
		}
	}

	// V = $FF means no operator
	if (left.empty() || right.empty() || v == 0xff)
	{
		show_error(zm, ERROR_IMPROPER_SYNTAX);
		DoTheEnd(STATUS_STOP_ON_ERROR);
	}

	// if there are operators .OR, .AND, we generate a special code

	skip_spaces(k, str);
	kod.erase();

	if (!test_char(k, str))
	{
		txt = get_dat(k, str, 0, true);

		if (txt == ".OR")
			kod = "9";
		else if (txt == ".AND")
			kod = "41";
		// else
		//	show_error(zm, ERROR_IMPROPER_SYNTAX);
	}

	if (!kod.empty())
	{
		if (long_test.empty())
			long_test = R"(.LOCAL\.PUT[0]=0\?I=1\?J=0)";

		skip_spaces(k, str);
		txt = get_dat(k, str, 0, false);

		_r = 0;
		_v = 0;
		_rgt.erase();
		_lft.erase();

		j = 1 - 1;
		conditional_expression(j, long_test, txt, _lft, _rgt, _v, _r, ".PUT[?I]=" + kod + "\\ ?I++");
	}

	if (!long_test.empty())
	{
		create_long_test(v, r, long_test, left, right);
		long_test += strend;
	}
}

/**
 * \brief Convert 0-99 into binary coded decimal (tested)
 * \param nr to convert
 * \return [0..9][0..9]
 */
tByte BCD(const long long nr)
{
	return static_cast<unsigned char>((nr / 10) * 16 + (nr % 10));
}


bool str_to_float(const std::string_view& str, double& num, int *error_pos = nullptr, std::errc* ec = nullptr, std::chars_format fmt = std::chars_format::general) {
	auto ret = std::from_chars(str.data(), str.data() + str.length(), num, fmt);
	if (ec) *ec = ret.ec;
	if (error_pos)
	{
		if (ret.ec != std::errc())
		{
			*error_pos = ret.ptr - str.data();
		}
	}
	return ret.ec == std::errc();
}

double powers[99] = {
	1e-98, 1e-96, 1e-94, 1e-92, 1e-90, 1e-88, 1e-86, 1e-84, 1e-82, 1e-80,
	1e-78, 1e-76, 1e-74, 1e-72, 1e-70, 1e-68, 1e-66, 1e-64, 1e-62, 1e-60,
	1e-58, 1e-56, 1e-54, 1e-52, 1e-50, 1e-48, 1e-46, 1e-44, 1e-42, 1e-40,
	1e-38, 1e-36, 1e-34, 1e-32, 1e-30, 1e-28, 1e-26, 1e-24, 1e-22, 1e-20,
	1e-18, 1e-16, 1e-14, 1e-12, 1e-10, 1e-08, 1e-06, 1e-04, 1e-02, 1e+00,
	1e+02, 1e+04, 1e+06, 1e+08, 1e+10, 1e+12, 1e+14, 1e+16, 1e+18, 1e+20,
	1e+22, 1e+24, 1e+26, 1e+28, 1e+30, 1e+32, 1e+34, 1e+36, 1e+38, 1e+40,
	1e+42, 1e+44, 1e+46, 1e+48, 1e+50, 1e+52, 1e+54, 1e+56, 1e+58, 1e+60,
	1e+62, 1e+64, 1e+66, 1e+68, 1e+70, 1e+72, 1e+74, 1e+76, 1e+78, 1e+80,
	1e+82, 1e+84, 1e+86, 1e+88, 1e+90, 1e+92, 1e+94, 1e+96, 1e+98 };

/**
 * \brief write the real number in FP Atari format (we use the Delphi encoding properties of the real number)
 * \param a 
 * \param old 
 */
void save_fl(string &a, string &old)
{
	int i, e;
	double x;
	tInt64 n;

	if (!str_to_float(a, x, &i))
	{
		string err;
		err = a[i];
		show_error(old, WARN_ILLEGAL_CHARACTER, err);
	}
	// Make X positive
	e = 0x0E;
	if (x < 0)
	{
		e = 0x8E;
		x = -x;
	}

	// If number is too small, store 0
	if (x < 1e-99)
	{
		save_dst(0);
		save_dst(0);
		save_dst(0);
		save_dst(0);
		save_dst(0);
		save_dst(0);

		inc(addres, 6);
		return;
	}

	if (x >= 1e+98)
		show_error(old, MSG_VALUE_OUT_OF_RANGE);   // Atari FP range exceeded

	// Search correct exponent
	for (i = 0; i < 99; ++i)
	{
		if (x < powers[i])
		{
			n = myRound(x * 10000000000.0 / powers[i]);
			save_dst(e + i);
			save_dst(BCD(n / 100000000));
			save_dst(BCD((n / 1000000) % 100));
			save_dst(BCD((n / 10000) % 100));
			save_dst(BCD((n / 100) % 100));
			save_dst(BCD(n / 100));

			inc(addres, 6);             // we saved 6 bytes
			return;
		}
	}
}

/**
 * \brief memory type for Sparta DOS X blocks: M-ain, E-xtended, 0..$7f
 * \param i 
 * \param zm 
 * \return 
 */
tByte getMemType(int &i, string &zm)
{
	skip_spaces(i, zm);

	tByte Result = 0;

	if (_alpha(zm[i]))
	{
		switch (UpCase(zm[i]))
		{
			case 'M': Result = 0; break;         // M[ain]
			case 'E': Result = 2; break;         // E[xtended]
			default:
				show_error(zm, ERROR_UNEXPECTED_EOL);
		}
	}
	else
	{
		Result = byte(calculate_value_noSPC(zm, zm, i, 0, 'B'));

		if (Result > 127)
			show_error(zm, MSG_VALUE_OUT_OF_RANGE);
	}

	memType = Result;
	return Result;
}

/**
 * \brief reading a directive or pseudo-instruction in the .MACRO, .REPT block
 * \param i 
 * \param zm 
 * \return 
 */
int get_command(int &i, string &zm)
{
	string ety;
	int Result = 0;

	if (!zm.empty())
	{
		skip_spaces(i, zm);
		ety.erase();

		if (labFirstCol and (i == 1-1))
		{
			if (zm[i] == '.')
				ety = get_directive(i, zm);
			else
			{
				ety = get_datUp(i, zm, 0, false);

				if (length(ety) != 3)
					ety.erase();
				else if (!reserved(ety))
					ety.erase();
			}
		}

		if (ety.empty())
		{
			ety = get_datUp(i, zm, 0, false);
			skip_spaces(i, zm);

			if (!ety.empty())
			{
				if (ety[1-1] != '.')
				{
					if (!test_char(i, zm))
					{
						if (zm[i] == '.')
							ety = get_directive(i, zm);
						else
							ety = get_datUp(i, zm, 0, false);
					}
				}
			}
		}

		if (!ety.empty())
		{
			if (ety[1-1] == '.')
				Result = fCRC16(ety);
			else
				Result = fASC(ety);
		}
	}

	return Result;
}


/**
 * \brief reading a macro defined by the directives .MACRO, .ENDM [.MEND]
 * !!! exceptionally, the .END directive cannot end the macro definition!!!
 * \param zm 
 * \return 
 */
unsigned char dirMACRO(string &zm)
{
	save_lst(' ');                               // .MACRO

	int k = 1-1;
	tByte Result = get_command(k, zm);

	if (Result == __macro and macro)
		show_error(zm, WARN_MISSING_DOT_END);

	if (Result == __endm)
		macro = false;

	if (pass == 0 && if_test)
		save_mac(zm);   // !!! We save macros in only 1 run !!!

	zapisz_lst(zm);

	return Result;
}

/**
 * \brief substituting parameters in the .REPT block
 * \param txt 
 */
void reptLine(string& txt)
{
	string ety, tmp;
	int j, k, war;
	j = 1 - 1;
	while (j < length(txt))
	{
		if (test_param(j, txt))
		{
			k = j;
			if (txt[j] == ':')
				inc(j);
			else
			{
				inc(j, 2);       // [:par] || [%%par]
			}
			ety = read_DEC(j, txt);

			war = static_cast<int>(StrToInt(ety));

			if (war < High(reptPar))
			{
				myDelete(txt, k, j - k);
				dec(j, j - k);

				tmp = reptPar[war];
				war = static_cast<int>(calculate_value(tmp, txt));

				tmp = IntToStr(war);
				myInsert(tmp, txt, k);
				inc(j, length(tmp));
			}
		}
		else
			inc(j);
	}
}

/**
 * \brief .ENDR  -  making a .REPT loop
 * \param zm 
 * \param a 
 * \param old_str 
 * \param cnt 
 * \return 
 */
int dirENDR(string &zm, string &a, string &old_str, const int cnt)
{
	int lntmp, i, j, k, max, rile, rpt;
	string tmp, ety;
	bool old_if_test;
	_strArray tmpPar;

	int Result = 0;
	max = 0;

	if (if_test)
	{
		force_saving_lst(zm);
		rept = false;
		rept_run = true;
		rile = _rept_ile;
		lntmp = line_add;
		tmpPar = reptPar;
		line_add = t_rep[cnt].lineNr;
		old_if_test = if_test;

#ifdef TEST_DUMP
		cout << pass << ',' << "-------------------" << endl;
		for (i = t_rep[cnt].firstLineNr; i <= t_rep[cnt].lastLineNr; ++i)
			cout << t_mac[i];
		for (i = 0; i < t_rep.size(); ++i)
			cout << t_rep[i].firstLineNr << ',' << t_rep[i].lastLineNr << ',' << t_rep[i].lineNr << ' - ' << (t_rep[i].lastLineNr - t_rep[i].firstLineNr) << endl;

		exit(0);
#endif

		if (!run_macro)
		{
			tmp = "REPT";
			put_lst(show_full_name(tmp, false, true));
		}

		tmp = t_mac[t_rep[cnt].firstLineNr];                    // first line from .REPT

		i = 1-1;
		skip_spaces(i, tmp);
		ety = get_directive(i, tmp);

		if (fCRC16(ety) != __rept)
			show_error(zm, ERROR_MISSING_DOT_REPT);
		else
		{
			reptLine(tmp);
			get_parameters(i, tmp, reptPar, false, 0, 0);

			if (reptPar.empty())
				show_error(zm, ERROR_UNEXPECTED_EOL);            // Unexpected end of line

			max = static_cast<int>(calculate_value(reptPar[0], zm));           // number of loop repetitions
			if (max < 0)
				show_error(zm, MSG_VALUE_OUT_OF_RANGE);                      // !!! possible value above $ffff !!!

			reptPar[0] = IntToStr(Int64(High(reptPar)) - 1);   // number of parameters passed to .REPT
		}

		_rept_ile = 0;          // !!! necessarily at this point after reading the .REPT line

		for (j = 0; j < max; ++j)
		{
			rpt = 0;

			i = t_rep[cnt].firstLineNr + 1;                          // first line

			while (i <= t_rep[cnt].lastLineNr)
			{
				tmp = t_mac[i];
				k = 1 - 1;
				skip_spaces(k, tmp);
				ety = get_directive(k, tmp);

				if (fCRC16(ety) == __rept)
				{
					inc(rpt);
					i += dirENDR(zm, a, old_str, cnt + rpt);
				}
				else
				{
					analyze_mem(i, i + 1, zm, a, old_str, j, j + 1, true);
				}

				inc(i);
			}

		}


		rept_run = false;

		if (!run_macro && !rept)
			put_lst(show_full_name(a, full_name, true));

		line_add = lntmp;
		if_test = old_if_test;
		_rept_ile = rile;
		reptPar = tmpPar;
		Result = t_rep[cnt].lastLineNr - t_rep[cnt].firstLineNr;
	}

	return Result;
}


/**
 * \brief 
 * \param i 
 * \param zm 
 */
 void get_rept(int &i, string &zm)
{
	_strArray par;

	int j = i;
	get_parameters(i, zm, par, false, 0, 0);

	if (High(par) == 0)
		show_error(zm, ERROR_UNEXPECTED_EOL);            // Unexpected end of line

	int k =static_cast<int>(t_rep.size())-1;

	t_rep[k].firstLineNr = static_cast<int>(t_mac.size())-1;
	t_rep[k].lineNr = line;
	t_rep[rept_cnt].idx = k;
	inc(rept_cnt);

	t_rep.resize(k + 2);

	save_mac(".REPT " + copy(zm, j, length(zm)));
}

/**
 * \brief 
 */
void get_endr()
{
	dec(rept_cnt);
	t_rep[t_rep[rept_cnt].idx].lastLineNr = High(t_mac) - 1;
}

/**
 * \brief line parsing when .REPT occurs
 * \param str 
 * \return 
 */
unsigned char dirREPT(string& str)
{
	int k;
	string zm;

	unsigned char Result = 0;
	save_lst(' ');

	int i = 1 - 1;
	while (i < length(str))
	{
		zm = get_dat(i, str, '\\', false);

		k = 1 - 1;
		Result = get_command(k, zm);

		switch (Result)
		{
			case __rept: get_rept(k, zm); break;
			case __endr:
			case __dend: get_endr(); break;
		}

		//  if not(isComment) then

		if (if_test && not(Result == __rept || Result == __endr || Result == __dend))
		{
			save_mac(zm);
			zapisz_lst(zm);
		}

		if (str[i] == '\\')
			inc(i);
		else
			break;
	}

	return Result;
}

/**
 * \brief Parse the type for the variables .BYTE, .WORD, .LONG, .DWORD, .STRUCT|.ENUM
 * \param i 
 * \param zm 
 * \param typ
 * \param i_str
 * \param n_str 
 * \return 
 */
int get_vars_type(int& i, string& zm, char& typ, int& i_str, string& n_str)
{
	string txt;
	int k, x;

	skip_spaces(i, zm);

	int Result = 0;
	k = i;

	if (zm[i] == '.')
	{
		txt = get_directive(i, zm);
		Result = fCRC16(txt);

		if (Result >= __byte && Result <= __dword)
			dec(Result, __byteValue);
		else
		{
			Result = 0;     // this is not a directive specifying the data type!!! V=0!!!
			i = k;          // we restore the previous values
		}
	}
	else
	{
		txt = get_lab(i, zm, true);    // it will stop if there is no label
		x = load_lab(txt, true);
		if (x < 0)
			i = k;                       // the label has not been defined
		else
		{
			switch (t_lab[x].bank)
			{
				case __id_enum:
				{
					Result = t_lab[x].addr;
					n_str = txt;
					break;
				}

				case __id_struct:
				{
					Result = t_str[t_lab[x].addr].mySize;
					i_str = t_lab[x].addr;
					n_str = txt;

					typ = 'S';
					break;
				}

				default:
					i = k;
			}
		}
	}
	return Result;
}

/**
 * \brief .VAR .TYPE v0 [=expression], v1 [=expression] .TYPE v3 [=expression] v4
 *			.VAR v0 [=expression] .TYPE v1 [=expression] v2 .TYPE [=address]
 *			reading parameters for the .VAR directive
 * \param i 
 * \param zm 
 * \param par 
 * \param mne 
 */
void get_vars(int& i, string& zm, _strArray& par, const unsigned char mne)
{
	int idx, _doo, _odd, k, v, i_str;
	string     txt, str, n_str;
	tInt64    tst;
	char     typ;

	i_str = 0;
	n_str.erase();
	typ = 'V';
	v = get_vars_type(i, zm, typ, i_str, n_str);

	get_parameters(i, zm, par, false);
	\
	idx = 1;

	if (zm[i] == ':')
	{                // number of repetitions
		inc(i);
		txt = get_dat(i, zm, ',', true);
		idx = integer(calculate_value(txt, zm));
		testRange(zm, idx, 0);
	}

	_doo = High(par);

	if (_doo == 0)
		show_error(zm, ERROR_UNEXPECTED_EOL);            // no variables - Unexpected end of line

	skip_spaces(i, zm);

	if (v == 0)
	{
		if (zm[i] == '.')
			v = get_type(i, zm, zm, true);            // variable type
		else
		{
			if (test_char(i, zm))
				txt = par[_doo - 1];
			else
			{
				txt = get_dat(i, zm, ',', true);
				inc(_doo);
			}

			k = 1 - 1;
			v = get_vars_type(k, txt, typ, i_str, n_str);

			if (v == 0)
				error_und(zm, txt, MSG_MISSING_TYPE_LABEL);	// the type of the variable "Missing type label" was not specified
			else
			{
				par[_doo - 1] = copy(txt, k, length(txt));

				dec(_doo);                          // we succeeded, we omit the last element
			}

		}
	}

	for (_odd = 0; _odd < _doo; ++_odd)
	{
		txt = par[_odd];

		k = 1 - 1;
		str = get_lab(k, txt, true);    // we check whether the label has the correct characters

		tst = 0;
		if (txt[k] == '=')
		{              // whether the variable is initialized ('=')

			if ((mne == __zpvar) and (pass == pass_end))
				warning(WARN_UNINITIALIZED_VARIABLE);

			txt = copy(txt, k + 1, length(txt));

			tst = calculate_value(txt, zm);

			if (ValueToType(tst) > v)
				show_error(zm, MSG_VALUE_OUT_OF_RANGE);

		}
		else if (not(test_char(k, txt)))
			show_error(zm, WARN_ILLEGAL_CHARACTER, txt[k]);  // An illegal character occurred


		if (proc)                           // SAVE_VAR
			t_var[var_idx].lok = proc_lokal;      //
		else                                   //
			t_var[var_idx].lok = end_idx;        //
		//
		if (proc && (local_name.empty() == false))      //
			t_var[var_idx].name = local_name + str; //
		else                                   //
			t_var[var_idx].name = str;            //
		//
		t_var[var_idx].mySize = v;               //
		t_var[var_idx].cnt = idx;             //
		t_var[var_idx].initialValue = cardinal(tst);  //
		//
		t_var[var_idx].addr = -1;              //
		//
		t_var[var_idx].id = var_id;          //
		t_var[var_idx].theType = typ;             //
		t_var[var_idx].idx = i_str;           //
		t_var[var_idx].structName = n_str;           //

		if (proc)                                //
			t_var[var_idx].excludeProc = t_prc[proc_nr - 1].used; //
		else                                        //
			t_var[var_idx].excludeProc = true;                //

		inc(var_idx);

		if (var_idx >= t_var.size())
			t_var.resize(var_idx + 1);

	}
}

void opt_h_minus()
{
	opt = opt & static_cast<tByte>(~opt_H);
}

/**
 * \brief 
 * \param ety 
 * \param zm 
 */
void upd_structure(string& ety, string& zm)
{
	structure.cnt = 0;
	string txt = local_name + ety;      // the full name of the .STRUCT structure

	// we check whether we already remember this structure
	const int idx = loa_str(txt, structure.id);

	// did not find array offset (idx = -1)
	if (idx < 0)
	{
		save_str(txt, addres, 0, 1, addres, bank);      // add a new structure to the T_STR array
		structure.idx = loa_str(txt, structure.id);
		save_lab(ety, structure.idx, __id_struct, zm);
	}
	else
	{
		// found the offset to the array
		structure.idx = idx;

		save_lab(ety, structure.idx, __id_struct, zm);
	}

	if (pass_end < 3)
		pass_end = 3;     // if there are structures, there must be at least 3 passes
}

/**
 * \brief 
 * \param i 
 * \param zm 
 * \param txt 
 */
void search_comment_block(int &i, string &zm, string& txt)
{
	if (!zm.empty() && i < zm.length()-1)
	{
		int k = i;
		if (zm[i] == '/' && zm[i+1] == '*')
		{
			isComment = true;
			i += 2;
		}

		while (isComment && i < zm.length()-1)
		{
			if (zm[i] == '*' && zm[i+1] == '/')
			{
				i += 2;
				txt += zm.substr(k, i - k);

				isComment = false;
				break;			
			}
			++i;
		}
	}
}

/**
 * \brief 
 * \param i 
 * \param zm 
 */
void write_out(int& i, string& zm)
{
	string txt;
	bool old_empty, old_firstorg;
	int     old_addres;

	//    save_lst(' ');

	skip_spaces(i, zm);

	if (not(test_char(i, zm)))
	{
		pisz = true;
		branch = true;             // there is no relocation for .PRINT

		old_firstorg = first_org;
		first_org = false;

		old_addres = addres;
		if (addres < 0)
			addres = 0x0100;

		old_empty = ::empty;
		::empty = false;

		txt = end_string;

		end_string.erase();

		calculate_data(i, zm, zm, 4);

		pisz = false;

		::empty = old_empty;

		addres = old_addres;

		first_org = old_firstorg;

		save_lst(' ');
		justify();
		put_lst(t + zm);

		save_lst(' ');

		zm = end_string;             // !!! to place the text in the listing !!!

		end_string = txt + end_string + "\r\n";
	}
}

/**
 * \brief 
 * \param zm 
 * \param a 
 * \param old_str 
 * \param nr 
 * \param end_file 
 * \param do_exit 
 */
void analyze_lines(string& zm, string& a, string& old_str, int &nr, bool& end_file, bool& do_exit)
{
	static const tByte tora[8] = { 0x80, 0x40, 0x20, 0x10, 8, 4, 2, 1 };

	fstream g;
	ifstream f;
	//    f:   textfile;
	tByte v, r, opt_tmp;
	char ch, rodzaj, typ;
	int i, k, j, m, _odd, _doo, idx = 0, idx_get, rpt, old_ifelse, old_rept, indeks;
	int old_rept_cnt;
	bool old_loopused, old_icl_used, old_case, old_run_macro, old_isComment, yes;
	string ety, txt, str, tmp, old_macro_nr, long_test, tmpZM;
	tInt64 war;
	tCardinal tst, vtmp;

	relocateValue reloc_value;

	int5 mne;
	_strArray par, tlin;
	vector<reptEntry> old_trep;

LOOP2:

	i = 1 - 1;

	if (not(structure.use) && not(enumeration.use))
		label_type = 'V';

	overflow = false;

	mne.l = 0;
	rpt = 0;

	data_out = false;

	reloc_value.use = false;
	reloc_value.count = 0;
	rel_used = false;

	m = i;                     // we remember the initial value 'i'

	if (zm.empty())
	{
		mne_used = false;       // the mnemonic has not been read (!!! necessarily in this place!!!)

		save_lst(' ');
		put_lst(t);

		return;
	}

LOOP:
	//----------------------------------------------------------------------------
	//  we read the label from the left, from the first character in the line 	  
	// if it ends with a colon, we omit the colon                 				  
	//----------------------------------------------------------------------------
	ety = get_lab(i, zm, false);
	if ((zm[i] == ':') && test_char(i + 1, zm, ' ', 0))
		inc(i);

	skip_spaces(i, zm);

	//----------------------------------------------------------------------------
	//  or maybe there is some label preceded by white spaces and ending with ':' 
	//----------------------------------------------------------------------------
	if (ety.empty())
	{
		k = i;
		txt = get_lab(i, zm, false);

		if (txt.empty() == false)
		{
			if (((zm[i] == ':') && test_char(i + 1, zm, ' ', 0)) or structure.use or enumeration.use)
			{
				ety = txt;
				IncAndSkip(i, zm);
			}
			else
				i = k;
		}
	}


	//----------------------------------------------------------------------------
	// reading more fields of the .STRUCT structure 							  
	// if the line starts with e.g.: .BYTE the reading is carried out by __dta ...
	//----------------------------------------------------------------------------
	if (structure.use and (!ety.empty()))
	{
		i = 1 - 1;
		get_parameters(i, zm, par, false);
	}

	//----------------------------------------------------------------------------
	// labels can be CPU instructions or pseudo-instructions if labFirstCol=true 
	// this does not apply to MAE style labels, they are recognized classically   
	// this also does not apply to labels declared in .STRUCT                     
	//----------------------------------------------------------------------------
	if ((length(ety) == 3) && not(structure.use) && not(mae_labels) && labFirstCol)
	{
		if (reserved(ety))
		{
			i = m;
			ety.erase();
		}
	}

	//----------------------------------------------------------------------------
	// The first characters can only be characters that begin the label name,
	// line breaks and '.', ':', '*', '+', '-', '='
	// instructions generating the code 6502 #if #WHILE #END are allowed
	//----------------------------------------------------------------------------
	if (not(_first_char(zm[i])) && not(test_char(i, zm)))
	{
		switch (zm[i])
		{
			case '#':
			{
				tmp = get_directive(i, zm);

				mne.l = fCRC16(tmp);
				if (not(mne.l == __test || mne.l == __while || mne.l == __dend || mne.l == __telse || mne.l == __cycle))
					show_error(zm, ERROR_ILLEGAL_INSTRUCTION);

				goto JUMP_2;
			}

			case '{':
			{
				if (end_idx > 0)
				{

					if (t_end[end_idx - 1].withSemicolon)
						show_error(zm, ERROR_IMPROPER_SYNTAX);

					t_end[end_idx - 1].withSemicolon = true;
					inc(i);

					goto LOOP;

				}
				else
					show_error(zm, ERROR_IMPROPER_SYNTAX);
				break;
			}
			case '}':
			{
				if (end_idx > 0)
				{
					if (t_end[end_idx - 1].withSemicolon)
					{
						IncAndSkip(i, zm);

						if (not(test_char(i, zm)))
							show_error(zm, ERROR_IMPROPER_SYNTAX);

						mne.l = t_end[end_idx - 1].endCode;
					}
					else
						show_error(zm, ERROR_IMPROPER_SYNTAX);
				}
				break;
			}
			default:
			{
				if ((mne.l == 0) && not(aray) && not(structure.use) && not(enumeration.use))
					show_error(zm, ERROR_IMPROPER_SYNTAX);
			}
		}
	}
	//----------------------------------------------------------------------------
	//  we check whether there is a local declaration of the label					   
	// the '=' sign can replace EQU, it does not have to be preceded by "white spaces" 
	// '+=' , '-=' signs replace adding and subtracting values ??from the label 		
	// '--' , '++' characters replace decrementing and incrementing the label value    
	//----------------------------------------------------------------------------
	tmpZM = zm;

	if (zm[i] == '+' || zm[i] == '-')
	{
		if ((!ety.empty()) && (zm[i + 1] == '+' || zm[i + 1] == '-' || zm[i + 1] == '='))
			compound_operator(i, zm, ety, false);
		else
			show_error(zm, ERROR_EXTRA_CHARS_ON_LINE);
	}

	if ((zm[i] == '=') && not(aray) && not(structure.use) && not(enumeration.use))
	{
		mne.l = __addEqu;
		IncAndSkip(i, zm);                // we force the execution of __addEQU

		save_lst(' ');

		goto JUMP;
	}

	if (ifelse > 0)
		save_lst(' ');

	//----------------------------------------------------------------------------
	//  a character specifying the number of repetitions of the line ':'          
	//----------------------------------------------------------------------------
	if (macro_rept_if_test() and (zm[i] == ':'))
	{
		save_lst('a');

		inc(i);
		loop_used = true;

		txt = get_dat(i, zm, ',', true);
		_doo = integer(calculate_value(txt, zm));
		testRange(zm, _doo, 0);

		old_rept = _rept_ile;
		_rept_ile = 0;

		// test for the presence of anything
		k = i;
		txt = get_datUp(i, zm, 0, true);
		i = k;

		if ((txt.empty()) || (txt[1-1] == '*'))
			show_error(zm, ERROR_UNEXPECTED_EOL);

		if (structure.use)
		{
			rpt = _doo;
			skip_spaces(i, zm);
		}
		else
		{
			old_case = FOX_ripit;
			FOX_ripit = true;        // return the loop counter value for '#'

			indeks = line_add;
			save_lab(ety, addres, bank, zm);
			idx = High(t_mac);
			txt = get_dat(i, zm, '\\', false);
			save_mac(txt);

			line_add = line - 1;
			_odd = idx + 1;
			par = reptPar;
			SetLength(reptPar, 3);

			reptPar[0] = "1";
			reptPar[1] = "#";

			analyze_mem(idx, _odd, zm, a, old_str, 0, _doo, true);

			reptPar = par;
			loop_used = false;
			FOX_ripit = old_case;
			line_add = indeks;
			SetLength(t_mac, idx + 1);

			mne.l = __nill;
			ety.erase();
		}

		_rept_ile = old_rept;

		// FOX_ripit = false;
	}

	//----------------------------------------------------------------------------*)
	//  we check if it is a directive, only some directives can be looped:        *)
	// .PRINT, .BYTE, .WORD, .LONG, .DWORD, .GET, .PUT, .HE, .BY, .WO, .SB, .FL,  *)
	// .CB, .CBM, .DEF other directives cannot be repeated                        *)
	//----------------------------------------------------------------------------*)
	if (zm[i] == '.')
	{
		tmp = get_directive(i, zm);

		mne.l = fCRC16(tmp);

		if (not(AllDirectives.has(mne.l)))
		{
			error_und(zm, tmp, MSG_MISSING_TYPE_LABEL);
		}
		else if (loop_used)
		{
			if (not(LoopingDirective.has(mne.l)))
				show_error(zm, ERROR_CANT_REPEAT_DIRECTIVE);
		}
	}

	//----------------------------------------------------------------------------*)
	//  replacing IFT ELS EIF ELI with their equivalents .IF .ELSE .ENDIF .ELSEIF *)
	// was implemented by modifying the HASH table                                *)
	// !!! necessarily only in this place, the code will not be executed in any other place !!! *)
	//----------------------------------------------------------------------------*)
	if ((i < length(zm)) && (UpCase(zm[i]) == 'E' || UpCase(zm[i]) == 'I'))
	{
		j = i;
		txt = get_datUp(i, zm, 0, false);
		v = fASC(txt);

		if (length(txt) != 3)
			i = j;
		else if (not(v == __if || v == __else || v == __endif || v == __elseif))
			i = j;
		else
			mne.l = v;
	}


	//----------------------------------------------------------------------------*)
	//  replacing the .OR directive with the ORG pseudo-command 				  *)
	// support for .NOWARN directive                                              *)
	//----------------------------------------------------------------------------*)
	switch (mne.l)
	{
		case __or: mne.l = __org; break;
		case __nowarn:
		{
			noWarning = true;
			mne.l = 0;

			goto LOOP;
		}
	}

	if (not(skip_hlt))
		test_skipa();

	//----------------------------------------------------------------------------*)
	//  read the mnemonic                                                         *)
	//----------------------------------------------------------------------------*)

	if (ety == "@")
	{
		save_lab(ety, addres, bank, zm);
		ety.erase();
	}

	k = i;

	if ((mne.l == 0) && if_test and not(enumeration.use))
	{
		if (i < length(zm))
			mne = calculate_mnemonic(i, zm, zm);
	}

	if (mne.l == __define_run)
	{
		// .DEFINE macro at the beginning of the line

		if (t_mac[mne.i][1 - 1] == '~')           // this macro is disabled
			error_und(zm, copy(t_mac[mne.i], 2 - 1, 255), WARN_UNDECLARED_LABEL);

		txt = t_mac[mne.i + 2];

		if (t_mac[mne.i + 1].empty() == false)              // there are macro parameters
			get_define_param(i, zm, txt, static_cast<int>(StrToInt(t_mac[mne.i + 1])));

		myDelete(zm, k, i - k);
		myInsert(txt, zm, k);
		goto LOOP2;
	}


	//----------------------------------------------------------------------------*)
	//  reading labels appearing in the .ENUM block                               *)
	//----------------------------------------------------------------------------*)
	if (enumeration.use)
	{
		if ((!ety.empty()) && (mne.l == 0))
		{
			i = 1 - 1;
			get_parameters(i, zm, par, false);

			old_case = dotRELOC.use;   // label declarations in the .RELOC block cannot be relocated
			dotRELOC.use = false;

			if (High(par) > 1)
			{
				save_lst(' ');
				justify();
				put_lst(t + zm);
			}

			for (_odd = 0; _odd < High(par); ++_odd)
			{
				txt = par[_odd];

				k = 1 - 1;
				ety = get_lab(k, txt, false);   // we check whether the label has the correct characters
				if (ety.empty())
					show_error(zm, WARN_ILLEGAL_CHARACTER, txt[1-1]);   // the first character of the label is invalid

				if (txt[k] == '=')
				{
					str = copy(txt, k + 1, length(txt));

					enumeration.val = static_cast<int>(calculate_value(str, zm));
				}
				else if (not(test_char(k, txt)))
					show_error(zm, WARN_ILLEGAL_CHARACTER, txt[k]);  // An illegal character occurred

				label_type = 'C';
				branch = true;                         // there is no relocation for .ENUM labels
				save_lab(ety, enumeration.val, bank, zm);
				bez_lst = true;

				if (enumeration.val > enumeration.max)
					enumeration.max = enumeration.val;    // MAX to determine the size of the ENUM

				nul.i = integer(enumeration.val);
				save_lst('l');

				if (High(par) == 1)
					zapisz_lst(zm);
				else
					zapisz_lst(ety);

				inc(enumeration.val);
			}

			dotRELOC.use = old_case;
			nul.i = 0;
			ety.erase();
		}
	}


	//----------------------------------------------------------------------------*)
	//  additional test for labels in the .STRUCT block                                *)
	//----------------------------------------------------------------------------*)
	if (structure.use)
	{
		if ((!ety.empty()) && (mne.l == 0))
		{
			txt = par[0];
			idx = load_lab(txt, true);           // type label at the beginning

			if (idx >= 0)
			{
				if (not((t_lab[idx].bank == __id_struct) or (t_lab[idx].bank == __id_enum)))
					idx = -1;
			}

			if (idx < 0)
			{                 // or at the end of a line
				txt = par[High(par) - 1];
				idx = load_lab(txt, true);
			}
			else
			{
				for (k = 1; k < High(par); ++k)
				{
					par[k - 1] = par[k];
				}
			}

			ety = par[0];
			SetLength(par, High(par));

			if (idx < 0)
			{
				if (pass == pass_end)
					show_error(zm, ERROR_IMPROPER_SYNTAX);
			}
			else if (t_lab[idx].bank == __id_struct)
			{
				mne.l = __struct_run_noLabel;
				mne.i = t_lab[idx].addr;
			}
			else if (t_lab[idx].bank == __id_enum)
				mne.l = __byteValue + t_lab[idx].addr;
			else if (pass == pass_end)
				show_error(zm, ERROR_IMPROPER_SYNTAX);
		}
	}

JUMP_2:

	//----------------------------------------------------------------------------*)
	//  .END, #END                                                                *)
	//  overrides other .END directives?                                          *)
	//----------------------------------------------------------------------------*)
	if (macro_rept_if_test() && (mne.l == __dend))
	{
		if (end_idx == 0)
			show_error(zm, ERROR_UNREF_DIRECTIVE_DOT_END);
		else
			mne.l = t_end[end_idx - 1].endCode;
	}

	//----------------------------------------------------------------------------*)
	//  a special form of defining the value of a temporary label using characters *)
	//  '=' , '+=' , '-=' , '++' , '--'                                           *)
	//----------------------------------------------------------------------------*)
	if (mne.l == __addEqu || mne.l == __addSet)
	{
		if (!ety.empty())
			save_lab(ety, addres, bank, zm);   // label before another label

		ety = get_lab(i, zm, true);
		skip_spaces(i, zm);
		tmpZM = zm;

		if (UpCase(zm[i]) == 'E' || UpCase(zm[i]) == 'S')
			inc(i, 2);
		else if (zm[i] == '+' || zm[i] == '-')
			compound_operator(i, zm, ety, false);

		IncAndSkip(i, zm);
	}

	//----------------------------------------------------------------------------*)
	//  combining mnemonics using the ':' character, XASM style                   *)
	//----------------------------------------------------------------------------*)
	if (mne.l == __xasm)
	{
		save_lab(ety, addres, bank, zm);

		if ((pass == pass_end) && skip_use)
			warning(WARN_SKIPPING_FIRST_INSTR);    // Skipping only the first instruction

		save_lst('a');

		loop_used = true;

		old_case = case_used;            // we cannot convert to capital letters
		case_used = true;                 // we keep their original size (CASE_USED=TRUE)
		skip_hlt = true;
		xasmStyle = true;

		k = mne.i;
		ety = get_dat(k, zm, '\\', false);      // we read the arguments

		case_used = old_case;             // we restore the old CASE_USED value

		line_add = line - 1;                  // line number currently being processed
		// useful if an error occurs

		idx = High(t_mac);                  // index to a free entry in T_MAC

		while (true)
		{                // endless loop ;)
			old_case = case_used;           // here we also have to preserve the case
			case_used = true;                // therefore we force CASE_USED=TRUE

			txt = get_dat(i, zm, ':', true);      // we read the mnemonic separated by the ':'
			case_used = old_case;            // we restore the old CASE_USED value

			str = ' ' + txt + ety;               // we prepare new lines for analysis
			t_mac[idx] = str;                // we save it in T_MAC[IDX]

			test_skipa();

			_odd = idx + 1;
			analyze_mem(idx, _odd, zm, a, old_str, 0, 1, false);

			if (zm[i] == ':')
				inc(i);
			else
				break;   // we end the loop here if ':' is missing
		}


		xasmStyle = false;

		force_saving_lst(zm);

		line_add = 0;                       // be sure to reset LINE_ADD

		if (not(FOX_ripit))
			bez_lst = true;

		loop_used = false;
		skip_hlt = false;
		skip_xsm = true;                 // mnemonics were combined with ':'

		mne.l = __nill;
		ety.erase();
	}
	else if (not(mne.l == 0 || mne.l == __nill))
		skip_xsm = false;    // there was no joining of mnemonics by ':' (default)

JUMP:

	//----------------------------------------------------------------------------*)
	// if we have a macro running, save its contents to the LST file			  *)
	// blank lines are not saved, !!! works best in this form!!!				  *)
	//----------------------------------------------------------------------------*)
	if (run_macro and if_test)
	{
		if (!ety.empty())
			data_out = true;
	}

	//----------------------------------------------------------------------------*)
	// for SEGMENT blocks we check whether the code has exceeded the segment boundaries         *)
	//----------------------------------------------------------------------------*)
	if ((segment > 0) and macro_rept_if_test() and (org_ofset == 0))
	{
		if ((addres < t_seg[segment].start) or (addres > t_seg[segment].start + t_seg[segment].len))
			error_und(zm, t_seg[segment].name, WARN_SEGMENT_X_ERROR_AT_Y);
	}

	//----------------------------------------------------------------------------*)
	//  BLK (kody __blkSpa, __blkRel, __blkEmp)                                *)
	//----------------------------------------------------------------------------*)
	if (mne.l == __blk)
	{
		if (!ety.empty())
		{
			show_error(zm, ERROR_LABEL_NOT_REQUIRED);
		}
		else if (loop_used)
		{
			show_error(zm, ERROR_CANT_REPEAT_DIRECTIVE);
		}
		else
		{
			::empty = false;

			txt = get_datUp(i, zm, 0, true);

			switch (UpCase(txt[1-1]))
			{
				//BLK D[os] a
				case 'D': mne.l = __org; break;

					//BLK S[parta] a
				case 'S': mne.l = __blkSpa; break;

					//BLK R[eloc] M[ain]|E[xtended]
				case 'R': mne.l = __blkRel; break;

					//BLK E[mpty] a M[ain]|E[xtended]
				case 'E':
				{
					mne.l = __blkEmp;
					::empty = true;
					break;
				}

				//BLK N[one] a
				case 'N':
				{
					mne.l = __org;
					opt_h_minus();
					break;
				}

				//BLK U[pdate] A[ddress]
				//BLK U[pdate] E[xternal]
				//BLK U[pdate] S[ymbols]
				//BLK U[pdate] N[ew] address text
				case 'U':
				{
					if (pass == pass_end)
					{
						oddaj_var();

						give_back_to();

						txt = get_datUp(i, zm, 0, true);

						save_lst('a');

						_doo = 0;    // number of addresses to be relocated
						_odd = 0;    // number of symbols to be relocated

						for (idx = 0; idx < rel_idx; ++idx)
						{
							if (t_rel[idx].idx >= 0)
							{
								inc(_odd);
							}
							else if (t_rel[idx].idx == -1)
							{
								inc(_doo);
							}
						}

						switch (UpCase(txt[1-1]))
						{
							case 'A':
							{                       // A[ddress]
								if (not(bulkUpdatePublic.adr))
									if ((_doo > 0) or dotRELOC.use)
										blk_update_address(zm);
								bulkUpdatePublic.adr = true;
								break;
							}

							case 'E':
							{                       // E[xternal]
								if (not(bulkUpdatePublic.ext))
									blk_update_external(zm);
								bulkUpdatePublic.ext = true;
								break;
							}

							case 'P':
							{                       // P[ublic]
								if (not(bulkUpdatePublic.pub))
									blk_update_public(zm);
								bulkUpdatePublic.pub = true;
								break;
							}

							case 'S':
							{                       // S[ymbol]
								if (not(bulkUpdatePublic.symbol))
									if (_odd > 0)
										blk_update_symbol(zm);
								bulkUpdatePublic.symbol = true;
								break;
							}

							case 'N':
							{                       // N[ew]
								test_symbols = true;
								blk_update_new(i, zm);
								break;
							}

							default:
								show_error(zm, ERROR_UNEXPECTED_EOL);
						}

					}
					break;
				}

				default:
					show_error(zm, MSG_VALUE_OUT_OF_RANGE);
			}

		}
	}


	//----------------------------------------------------------------------------*)
	//  [label] .DS expression | .DS [elements0] [elements1] [...]                *)
	//  we reserve the "expression" of bytes without initializing them            *)
	//----------------------------------------------------------------------------*)
	if ((mne.l == __ds) && (macro_rept_if_test()))
	{
		if (structure.use or aray or enumeration.use)
			show_error(zm, ERROR_IMPROPER_SYNTAX);

		// after changing the assembly address of the .PROC, .LOCAL block, it is not possible to use the .DS directive

		if ((org_ofset > 0))
			show_error(zm, ERROR_ILL_INST_AT_RELOC_BLOCK);

		nul.i = integer(addres);
		save_lst('l');

		branch = true;

		skip_spaces(i, zm);

		etyArray = ety;

		_doo = integer(calculate_value_noSPC(zm, zm, i, 0, 'A'));

		if (etyArray.empty() == false)                   // there was no .ARRAY
			save_lab(ety, addres, bank, zm);          // .DS expression

		if (dotRELOC.sdx or dotRELOC.use)
		{

			if ((dotRELOC.sdx && (blok == 0)))
			{
				for (idx = 0; idx < _doo; ++idx)
				{
					save_dst(0); // when BLOK = 0 (blk sparta) it is not relocatable
				}
			}
			else
			{
				::empty = true;
				inc(ds_empty, _doo);
			}

			inc(addres, _doo);

		}
		else
		{

			data_out = true;

			txt = zm;                   // !!! only here we force writing to LST !!!
			zapisz_lst(txt);

			NoAllocVar = true;          // there is no .VAR variable allocation for .DS
			mne.l = __org;              // now we can force ORG *+

			zm = "*+" + IntToStr(_doo);
			i = 1 - 1;

			bez_lst = true;
		}

		ety.erase();
	}


	//----------------------------------------------------------------------------*)
	//  [label] .ALIGN N[,fill]                                                   *)
	//----------------------------------------------------------------------------*)
	if ((mne.l == __align) && (macro_rept_if_test()))
	{
		if (loop_used)
			show_error(zm, ERROR_CANT_REPEAT_DIRECTIVE);

		save_lst('a');

		skip_spaces(i, zm);

		if (test_char(i, zm))
			_doo = 0x100;
		else
			_doo = integer(calculate_value_noSPC(zm, zm, i, ',', 'A'));


		_odd = -1;

		if (zm[i] == ',')
		{
			IncAndSkip(i, zm);

			_odd = integer(calculate_value_noSPC(zm, zm, i, ',', 'B'));
		}


		if (_doo > 0)
			idx = (addres / _doo) * _doo;
		else
			idx = addres;

		if (idx != addres)
			inc(idx, _doo);

		if (_odd >= 0)
		{
			if (pass == pass_end)
			{
				while (idx > addres)
				{
					save_dst(byte(_odd));
					inc(addres);
				}
			}
			else
			{
				addres = idx;
			}
		}
		else
		{                     // forcing ORG

			justify();
			put_lst(t + zm);

			bez_lst = true;

			zm = "$" + Hex(idx, 4);

			NoAllocVar = true;                // there is no .VAR variable allocation for .ALIGN
			mne.l = __org;

			i = 1 - 1;
		}
	}

	//----------------------------------------------------------------------------*)
	//  NMB [addres]                                                               *)
	//  RMB [addres]                                                               *)
	//  LMB #expression [,addres]                                                  *)
	//----------------------------------------------------------------------------*)
	if (macro_rept_if_test())
	{
		ch = ' ';              // CH will contain the directive character 'N'MB, 'R'MB, 'L'MB

		switch (mne.l)
		{
			case __nmb:
			{
				inc(bank);   // we increase the MADS bank counter
				ch = 'N';
				break;
			}

			case __rmb:
			{
				bank = 0;     // We reset the MADS bank counter
				ch = 'R';
				break;
			}

			case __lmb:
			{         // we set the MADS bank counter
				skip_spaces(i, zm);
				if (zm[i] != '#')
				{
					show_error(zm, ERROR_ILLEGAL_ADDR_MODE_65XX);
				}
				else
				{
					IncAndSkip(i, zm); // other characters other than '#' are an error
				}

				txt = get_dat_noSPC(i, zm, zm, ' ');
				k = 1 - 1;
				j = integer(calculate_value_noSPC(txt, zm, k, ',', 'B'));  // bank counter value = 0..255

				bank = integer(j);

				ch = 'L';
				break;
			}

		}

		//----------------------------------------------------------------------------*)
		//  we force the execution of the @BANK_ADD macro (when OPT B+) for LMB, NMB, RMB        *)
		//----------------------------------------------------------------------------*)
		if (ch != ' ')
		{
			if (dotRELOC.use or dotRELOC.sdx)
				show_error(zm, ERROR_ILL_INST_AT_RELOC_BLOCK);
			if (first_org)
				show_error(zm, WARN_NO_ORG_SPECIFIED);

			save_lst('i');

			if (opt & opt_B)
			{
				skip_spaces(i, zm);
				if (zm[i] == ',')
					IncAndSkip(i, zm);

				str = get_dat_noSPC(i, zm, zm, ' ');
				if (str.empty() == false)
					str = ',' + str;

				force_saving_lst(zm);

				txt = "@BANK_ADD '";
				txt += ch;
				txt += "'" + str;
				i = 1 - 1;
				mne = calculate_mnemonic(i, txt, zm);
				zm = txt;
			}
		}
	}

	//----------------------------------------------------------------------------*)
	// Execute .PROC when PASS>0
	// force execution of the @CALL macro if the procedure had parameters
	// if the procedure is called from another procedure, remember
	// procedure parameters on the stack without modifying the stack pointer, *)
	// and after returning, restore them also without modifying the stack pointer           *)
	//----------------------------------------------------------------------------*)
	if (mne.l == __proc_run)
	{
		if (macro_rept_if_test())
		{
			save_lab(ety, addres, bank, zm);
			indeks = mne.i;
			idx = t_prc[indeks].nrParams;          // number of parameters
			tlin = t_lin;

			if (idx > 0)
			{
				force_saving_lst(zm);
				ety.erase();
				str.erase();

				// if a procedure of the type T_PRC[].myType = __pDef is called, which has parameters (IDX>0)
				// from the body of the currently processed procedure PROC_NR-1
				// then we will push the parameters of the current procedure onto the stack and restore them after returning

				if (proc && (t_prc[indeks].myType == __pDef))
				{
					_doo = t_prc[proc_nr - 1].paramMemSize;                     // number of bytes per parameters

					if (_doo > 0)
					{
						//str=copy(proc_name,1-1,length(proc_name)-1);  // we cut off the last dot in the name of the current procedure
						str = proc_name;
						SetLength(str, length(proc_name) - 1);

						ety = " @PUSH 'I'," + IntToStr(_doo) + " \\ ";
						str = " \\ @PULL 'J'," + IntToStr(_doo);
					}
				}

				rodzaj = t_prc[indeks].myType;  // type of procedure, method of passing parameters

				get_parameters(i, zm, par, false, 0, ':');   // dot character must accept for .LEN, .SIZEOF etc.

				// we check the number of parameters passed, if their number does not match, then
				// error 'Improper number of actual parameters' will occur
				// for procedures like __pReg, __pVar there can be fewer parameters, but not more

				_doo = High(par);

				for (_odd = _doo - 1; _odd >= 0; --_odd)
				{
					txt = par[_odd];

					if (txt.empty())
					{
						if (rodzaj == __pDef)
						{
							show_error(zm, WARN_IMPROPER_NR_OF_PARAMS);
						}
						else
						{
							txt = ";";                       // for __pReg we can omit parameters
						}
						}
					if ((txt[1-1] == '"') && (length(txt) > 2))
						txt = copy(txt, 2 - 1, length(txt) - 2);

					par[_odd] = txt;
				}

				// check the type of parameters and force the execution of the @CALL_INIT, @CALL, @CALL_END macro
				// the @CALL_INIT parameter is the number of bytes occupied by the procedure parameters

				switch (rodzaj)
				{
					case __pDef:
					{
						if (_doo != idx)
							show_error(zm, WARN_IMPROPER_NR_OF_PARAMS);
						ety += " @CALL 'I'," + IntToStr(t_prc[indeks].paramMemSize);
						break;
					}

					case __pReg:
					case __pVar:
					{
						if (_doo > idx)
							show_error(zm, WARN_IMPROPER_NR_OF_PARAMS);
						else if ((pass == pass_end) and (_doo < idx))
							warning(WARN_IMPROPER_NR_OF_PARAMS);

						ety += ' ';
					}
				}

				idx = t_prc[indeks].str;       // index to parameter names in T_PAR

				if (_doo > 0)
				{
					for (k = 0; k < _doo; ++k)
					{
						if (par[k][1 - 1] != ';')
						{
							txt = par[k];

							switch (rodzaj)
							{
								case __pDef: ety += " \\ @CALL ";
							}

							ch = ' ';
							v = byte(' ');

							if (txt == "@")
							{
								v = byte('@');
								war = 0;
								ch = 'Z';
							}
							else if (txt[1 - 1] == '#')
							{
								v = byte('#');
								branch = true;
								tmp = copy(txt, 2 - 1, length(txt));
								war = calculate_value(tmp, zm);
								ch = value_code(war, zm, true);
							}

							// type of parameter in procedure declaration (B,W,L,D)
							// the VALUE_CODE function returned the parameter type (Z,Q,T,D)
							tmpZM = copy(t_par[idx + k], 2 - 1, length(t_par[idx + k]));

							if (pass > 0)
							{
								if (ch != ' ')
								{
									switch (t_par[idx + k][1 - 1])
									{
										case 'B':
										{
											if (ch != 'Z')
												error_und(zm, tmpZM, WARN_INCOMPATIBLE_TYPES);
											break;
										}
										case 'W':
										{
											if ((ch == 'T' || ch == 'D') || (txt == "@"))
												error_und(zm, tmpZM, WARN_INCOMPATIBLE_TYPES);
											break;
										}
									}
								}
							}

							if (rodzaj != __pVar)
							{
								if (txt[1-1] == '#')
								{
									txt[1-1] = ' ';
								}
							}


							switch (rodzaj)
							{
								case __pDef:  
								{
									ety += "'";
									ety += chr(v);
									ety += "',";
									break;
								}

								case __pReg:
								{
									switch (chr(v))
									{
										case '#':
											switch (t_par[idx + k][1-1])     // by value
											{
												case 'B': {
													ety += "LD";
													ety += t_par[idx + k][2 - 1];
													ety += "#" + txt;
													break;
												}
												case 'W':
												{
													ety += "LD";
													ety += t_par[idx + k][2 - 1];
													ety += ">" + txt + "\\ LD" + t_par[idx + k][3 - 1] + "<" + txt;
													break;
												}

												case 'L': 
												{
													ety += "LD";
													ety += t_par[idx + k][2 - 1];
													ety += "^" + txt + "\\ LD" + t_par[idx + k][3 - 1] + ">" + txt + "\\ LD" + t_par[idx + k][4 - 1] + "<" + txt;
													break;
												}
											}
											break;

										case ' ':
											switch (t_par[idx + k][1-1])    // by address
											{
												case 'B': 
												{
													ety += "LD";
													ety += t_par[idx + k][2 - 1];
													ety += " " + txt;
													break;
												}
												case 'W': 
												{
													ety += "LD";
													ety += t_par[idx + k][2 - 1];
													ety += " " + txt + "+1\\ LD";
													ety += t_par[idx + k][3 - 1];
													ety += " " + txt;
													break;
												}
												case 'L':
												{
													ety += "LD";
													ety += t_par[idx + k][2 - 1];
													ety += " " + txt + "+2\\ LD";
													ety += t_par[idx + k][3 - 1];
													ety += " " + txt + "+1\\ LD";
													ety += t_par[idx + k][4 - 1];
													ety += " " + txt;
													if (dotRELOC.use)
														warning(WARN_NOT_RELOCATABLE);    // type 'L' is not relocatable
													break;
												}
											}
											break;

										case '@':
										{
											if (t_par[idx + k][2 - 1] != 'A')
											{
												ety += "TA";
												ety += t_par[idx + k][2 - 1];
											}
											break;
										}
									}
									break;
								}

								case __pVar:
								{
									switch (chr(v))
									{
										case '#':
										case ' ':
										{
											switch (t_par[idx + k][1 - 1])      // by the value '#' or the value from the address ' '
											{
												case 'B': ety += "MVA " + txt + ' ' + tmpZM; break;
												case 'W': ety += "MWA " + txt + ' ' + tmpZM; break;
												case 'L':
												{
													switch (chr(v))
													{
														case '#':
														{
															txt[1 - 1] = '(';
															txt = '#' + txt + ')';
															ety += " MWA " + txt + "&$FFFF " + tmpZM + '\\';
															ety += " MVA " + txt + ">>16 " + tmpZM + "+2\\";
															break;
														}

														case ' ':
														{
															ety += " MWA " + txt + ' ' + tmpZM + '\\';
															ety += " MVA " + txt + "+2 " + tmpZM + "+2";
															break;
														}
													}
													break;
												}
												case 'D':
												{
													switch (chr(v))
													{
														case '#':
														{
															txt[1 - 1] = '(';
															txt = '#' + txt + ')';
															ety += " MWA " + txt + "&$FFFF " + tmpZM + '\\';
															ety += " MWA " + txt + ">>16 " + tmpZM + "+2\\";
															break;
														}

														case ' ':
														{
															ety += " MWA " + txt + ' ' + tmpZM + '\\';
															ety += " MWA " + txt + "+2 " + tmpZM + "+2";
															break;
														}
													}
													break;
												}

												break;
											}
											break;
										}
										case '@':
										{
											ety += "STA " + tmpZM;
											break;
										}
									}
								}
							}

							if (rodzaj == __pDef)
							{
								ety += "'";
								ety += t_par[idx + k][1 - 1];
								ety += "',";
								if (txt[1 - 1] == '@')
									ety += '0';
								else
								{
									ety += '"' + txt + '"';
								}
							}
							else
								ety += "\\ ";
						}
					}
				}

				if (rodzaj == __pDef)
					txt = ety + " \\ @CALL 'X'," + t_prc[indeks].name + " \\ @EXIT " + IntToStr(t_prc[indeks].paramMemSize) + str;
				else
				{
					txt = ety + "JSR " + t_prc[indeks].name;
				}

				k = line_add;

				line_add = line - 1;

				_odd = High(t_mac);                      // procedure with parameters
				t_mac[_odd] = txt;                       // we force more lines to be executed
				_doo = _odd + 1;
				analyze_mem(_odd, _doo, zm, a, old_str, 0, 1, false);

				line_add = k;

				bez_lst = true;
			}
			else
			{
				txt = "JSR " + t_prc[indeks].name;         // procedure without parameters
				i = 1 - 1;
				mne = calculate_mnemonic(i, txt, zm);   // we force one line from JSR to be executed
			}

			t_lin = tlin;
			SetLength(tlin, 0);

			ety.erase();
		}
	}
	//----------------------------------------------------------------------------*)
	//  .ERROR [ERT] 'text' | .ERROR [ERT] expression                             *)
	//----------------------------------------------------------------------------*)
	if ((mne.l == __error || mne.l == __ert) && macro_rept_if_test())
	{
		if (pass == pass_end)
		{
			save_lst(' ');
			txt.erase();
			txt = get_string(i, zm, txt, true);

			if (txt.empty() == false)
			{
				if (not(run_macro))
					cout << zm << '\n';
				else
				{
					cout << old_str << '\n';
				}

				str = end_string;
				end_string.erase();

				write_out(i, zm);               // !!! the ZM variable is modified !!!

				txt = txt + end_string;
				end_string = str;
				show_error(txt, MSG_RESERVED_BLANK);
			}
			else
			{
				branch = true;             // There is no question of relocation here
				war = calculate_value_noSPC(zm, zm, i, ',', 'F');

				if (war != 0)
				{
					if (not(run_macro))
						cout << zm << '\n';
					else
						cout << old_str << '\n';

					str = end_string;
					end_string.erase();

					write_out(i, zm);             // !!! the ZM variable is modified !!!

					if (end_string.empty())
						txt = load_mes(MSG_USER_ERROR);      // User error
					else
						txt = end_string;

					end_string = str;
					show_error(txt, MSG_RESERVED_BLANK);
				}
			}
		}
		else
			mne.l = __nill;
	}

	//----------------------------------------------------------------------------*)
	//  .EXIT                                                                     *)
	//----------------------------------------------------------------------------*)
	if ((mne.l == __exit) && macro_rept_if_test())
	{
		if (not(run_macro))
			show_error(zm, ERROR_IMPROPER_SYNTAX);
		else
		{
			save_lab(ety, addres, bank, zm);
			save_lst(' ');
			force_saving_lst(zm);

			run_macro = false;
			do_exit = true;
			ety.erase();
			return;
		}
	}

	//----------------------------------------------------------------------------*)
	//  .ENDP                                                                     *)
	//----------------------------------------------------------------------------*)
	if ((mne.l == __endp) && macro_rept_if_test())
	{
		if (!ety.empty())
			show_error(zm, ERROR_LABEL_NOT_REQUIRED);
		else if (not(proc))
			show_error(zm, ERROR_MISSING_DOT_PROC);
		else
		{
			save_lst(' ');
			oddaj_var();

			get_local(zm);

			t_prc[proc_nr - 1].len = cardinal(addres) - t_prc[proc_nr - 1].addr;

			proc_nr = t_prc[proc_nr - 1].procNr;   // !!! necessarily this order, otherwise always PROC_NR=0 !!!

			get_address_update();

			proc = t_prc[proc_nr].inProcedure;
			org_ofset = t_prc[proc_nr].origOffset;
			proc_name = t_prc[proc_nr].savedProcName;
		}
	}

	//----------------------------------------------------------------------------*)
	//  label .PROC [label]                                                       *)
	//  if there are parameters limited by '()' brackets, remember them           *)
	//----------------------------------------------------------------------------*)
	if ((mne.l == __proc) && macro_rept_if_test())
	{
		if (addres < 0)
		{
			show_error(zm, WARN_NO_ORG_SPECIFIED);
		}
		else
		{
			if (ety.empty())
				ety = get_lab(i, zm, true);

			save_lst('a');
			txt = zm;
			zapisz_lst(txt);
			bez_lst = true;

			label_type = 'P';

			if (not(proc))
				ety = local_name + ety;
			else
				ety = proc_name + ety;

			save_local();
			local_name.erase();

			t_prc[proc_idx].inProcedure = proc;
			t_prc[proc_idx].procNr = proc_nr;
			t_prc[proc_idx].origOffset = org_ofset;
			t_prc[proc_idx].savedProcName = proc_name;

			proc_nr = proc_idx;     // necessarily via ADD_PROC_NR
			get_address(i, zm);     // we get the new assembly address for .PROC
			save_end(__endp);
			proc_lokal = end_idx;   // necessarily after SAVE_END, not earlier

			save_lst('a');
			proc_name.erase();

			if ((pass == pass_end) && not(exclude_proc) && not(dotRELOC.use))
			{
				if (not(t_prc[proc_nr].used))
					error_und(zm, ety, WARN_UNREFERENCED_PROCEDURE);
			}

			txt = zm;   // if TXT!=ZM then UPD_PROCEDURE modified ZM with some macro

			if (t_prc[proc_nr].name.empty())       // 'if pass==0' may be omitted if PROC is in if blocks
				get_procedure(ety, ety, zm, i);       // we remember the parameters of the procedure
			else
				upd_procedure(ety, zm, addres);       // we update the procedure addresses

			add_proc_nr();                        // new value PROC_NR, PROC_IDX

			if ((txt != zm) && proc)
			{
				save_lst(' ');
				i = 1 - 1;
				mne = calculate_mnemonic(i, zm, zm);
			}

			if (pass_end < 3)
				pass_end = 3;     // for special cases, 2 runs are not enough (JAC!)

			proc = true;
			proc_name = ety + '.';

			ety.erase();
		}
	}

	//----------------------------------------------------------------------------*)
	//  execute .STRUCT (e.g. declaring a structure field using another structure) *)
	// it is possible to assign fields defined by the structure to a new variable  *)
	//----------------------------------------------------------------------------*)
	if (mne.l == __struct_run || mne.l == __struct_run_noLabel || mne.l == __enum_run)
	{
		if (ety.empty())
		{
			show_error(zm, ERROR_LABEL_NAME_REQUIRED);
		}
		else if (macro_rept_if_test())
		{
			if ((mne.l == __enum_run) && structure.use)
			{
				mne.l = __byteValue + mne.i;                // enum type to size in bytes
			}
			else
			{
				if (High(par) < 1)
					create_struct_variable(zm, ety, mne, true, addres);
				else
				{
					for (k = 0; k < High(par); ++k)
					{
						ety = par[k];
						create_struct_variable(zm, ety, mne, true, addres);
					}
				}

				mne.l = __nill;
				ety.erase();
			}
		}
	}
	//----------------------------------------------------------------------------*)
	//----------------------------------------------------------------------------*)
	//----------------------------------------------------------------------------*)

	switch (mne.l)
	{

		//----------------------------------------------------------------------------*)
		//  execute .MACRO when PASS>0                                                *)
		//----------------------------------------------------------------------------*)
		case __macro_run:
		{
			//        if (macro ) halt {inc(macro_nr)} else
			if (if_test)
			{
				save_lab(ety, addres, bank, zm);

				if (not(FOX_ripit))
					save_lst('i');

				txt = zm;
				zapisz_lst(txt);

				indeks = mne.i;
				old_isComment = isComment;

				// we save the current macro in 'OLD_MACRO_NR'

				old_macro_nr = macro_nr;
				txt = t_mac[indeks + 1];

				// !!! the name of the LOCAL area will be repeated twice - NECESSARY !!!
				if (macro_nr.empty())
					macro_nr = macro_nr + local_name + proc_name + txt + t_mac[indeks + 3] + '.';
				else
					macro_nr = macro_nr + proc_name + txt + t_mac[indeks + 3] + '.';

				skip_spaces(i, zm);
				SetLength(par, 2);
				// the number of parameters will be entered in the par[0] position

				k = ord(t_mac[indeks + 2][1 - 1]);      // operating mode
				ch = t_mac[indeks + 2][2 - 1];          // separator

				while (not(test_char(i, zm)))
				{
					txt = get_dat(i, zm, ch, true);

					// if it is the FOX_RIPIT loop counter '#' (or .R) then we will calculate the value
					if ((txt == "#") or (txt == ".R"))
					{
						war = calculate_value(txt, zm);
						txt = IntToStr(war);
					}

					// if the passed parameter is between single quotes " ", it is a string
					if (txt.empty() == false)
						if ((txt[1-1] == '"') && (txt[length(txt)] == '"'))
							txt = copy(txt, 2 - 1, length(txt) - 2);

					_odd = High(par);

					if (k == byte('\''))
					{
						par[_odd] = txt;
						SetLength(par, _odd + 2);
					}
					else
					{
						if (txt[1-1] == '#')
						{
							par[_odd] = "'#'";
							txt = copy(txt, 2 - 1, length(txt));
						}
						else if (txt[1 - 1] == '<' || txt[1 - 1] == '>')
						{
							par[_odd] = "'#'";
							war = calculate_value(txt, zm);
							txt = IntToStr(war);
						}
						else
						{
							par[_odd] = "' '";
						}

						SetLength(par, _odd + 3);
						par[_odd + 1] = txt;
					}

					//          skip_spaces(i,zm);
					if (zm[i] == ',' || zm[i] == ' ' || zm[i] == 9)
						__next(i, zm);

					if (ch != ' ')
					{
						if (zm[i] == ch)
						{
							IncAndSkip(i, zm); // else Break;
						}
					}
				}

				// we write the number of parameters for :0
				par[0] = IntToStr(Int64(High(par)) - 1);

				// we increase the macro call number T_MAC[index+3], T_MAC[index+5]
				// !!! definitely in this place!!!

				_doo = integer(StrToInt(t_mac[indeks + 3]));
				t_mac[indeks + 3] = IntToStr(Int64(_doo) + 1);

				_doo = integer(StrToInt(t_mac[indeks + 5]));
				t_mac[indeks + 5] = IntToStr(Int64(_doo) + 1);

				// !!! protection against endless recursion!!!
				// we stop the macro execution if the number of calls exceeds 255

				if (_doo > 255)
					show_error(zm, ERROR_INFINITE_RECURSION);

				_doo = integer(StrToInt(t_mac[indeks + 4]));  // line number

				txt = t_mac[indeks];       // name of the file with the currently executing macro

				inc(wyw_idx);
				if (wyw_idx >= t_wyw.size())
					t_wyw.resize(wyw_idx + 1);

				t_wyw[wyw_idx].zm = zm;  // if an error occurs, this data will help locate it
				t_wyw[wyw_idx].pl = txt;
				t_wyw[wyw_idx].nr = _doo;

				// we remember the current index to T_MAC in _ODD
				// read the macro from T_MAC[index], substitute the parameters and write to T_MAC
				// ) we will free up space based on the previously remembered index in _ODD

				_odd = High(t_mac);

				tmp = t_mac[indeks + 1];        // macro name needed for named parameters

				while (true)
				{
					txt = t_mac[indeks + 6];

					j = 1 - 1;
					ety = get_datUp(j, txt, 0, false);  // we are loading an unknown string of uppercase letters

					if (not(fCRC16(ety) == __endm))
					{
						// to find the .ENDM, .MEND entry
						j = 1 - 1;
						while (j < length(txt))
						{
							if (test_macro_param(j, txt))
							{
								k = j;
								if (txt[j] == ':')
								{
									inc(j);
									typ = ':';                     // parameter by :
								}
								else
								{
									inc(j, 2);
									typ = '%';                     // parameter by %%
								}

								ety.erase();

								if (_dec(txt[j]))
									ety = read_DEC(j, txt);
								else
								{
									while (_mpar(txt[j]))
									{
										ety += UpCas_(txt[j]);
										inc(j);
									}
								}

								if (_mpar_alpha(ety[1-1]))
								{
									str = tmp + '.' + ety;

									while ((l_lab(str) < 0) && (!ety.empty()))
									{

										SetLength(str, length(str) - 1);
										SetLength(ety, length(ety) - 1);

										dec(j);
									}

									war = l_lab(str);

									if (war >= 0)
									{
										if (t_lab[war].bank == __id_mparam)
											war = t_lab[war].addr;
										else
											war = -1;
									}
								}
								else
								{
									war = StrToInt(ety);
								}

								if (war >= 0)
								{
									myDelete(txt, k, j - k);
									dec(j, j - k);

									if (war > Int64(High(par)) - 1)
									{
										myInsert("$FFFFFFFF", txt, k);
										inc(j, 9);   // must necessarily type $FFFFFFFF
									}
									else
									{
										myInsert(par[war], txt, k);
										inc(j, length(par[war]));
									}
								}
							}
							else
								inc(j);
						}

						save_mac(txt);
					}
					else
						break;

					inc(indeks);
				}

				str = "Macro: " + t_mac[mne.i + 1] + " [Source: " + t_mac[mne.i] + ']';

				if (not(FOX_ripit) && not(rept_run))
					put_lst(str);

				// remember in local variables

				indeks = line_add;
				line_add = 0;
				old_rept = _rept_ile;
				par = reptPar;
				old_case = rept_run;
				rept_run = false;
				old_run_macro = run_macro;
				old_ifelse = ifelse;
				old_loopused = loop_used;
				old_trep = t_rep;
				t_rep.resize(1);
				old_rept_cnt = rept_cnt;
				rept_cnt = 0;
				isComment = old_isComment;
				run_macro = true;
				loop_used = false;
				_doo = High(t_mac);

				//         for idx=_odd to _doo-1 do writeln(t_mac[idx],' | '); halt;

				analyze_mem(_odd, _doo, zm, a, old_str, idx, idx + 1, false);

				if (not(FOX_ripit))
					bez_lst = true;

				if (run_macro and (old_ifelse != ifelse))
					show_error(zm, MSG_MISSING_ENDIF);

				run_macro = old_run_macro;
				t_mac[mne.i + 5] = '0';

				loop_used = old_loopused;
				rept_run = old_case;
				ifelse = old_ifelse;
				macro_nr = old_macro_nr;
				_rept_ile = old_rept;
				reptPar = par;
				t_rep = old_trep;
				rept_cnt = old_rept_cnt;
				line_add = indeks;

				if (not(lst_off) && not(FOX_ripit) && not(rept_run))
					put_lst(show_full_name(a, full_name, true));

				dec(wyw_idx);
				SetLength(t_mac, _odd + 1);

				ety.erase();
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  label .DEFINE [label] expr                                                *)
		//  single-line macro                                                         *)
		//----------------------------------------------------------------------------*)
		case __define:
		{
			if (ety.empty())
				ety = get_lab(i, zm, true);

			if (if_test)
			{
				// .DEFINE can be changed in the same pass

				//reserved_word(ety,zm);

				k = load_lab(ety, true);
				idx = High(t_mac);
				if (k >= 0)
				{
					idx = t_lab[k].addr;// we overwrite the .DEFINE macro
				}
				else
				{
					save_lab(ety, idx, __id_define, zm); // book a new place at t_mac
					save_mac("");
					save_mac("");
					save_mac("");
				}

				skip_spaces(i, zm);
				t_mac[idx] = ety;                   // macro name
				tmp = copy(zm, i, length(zm));      //TODO: check i index
				if (High(t_lin) > 0)
					show_error(zm, ERROR_MULTILINE_ARG_NOT_SUPPORTED);

				// we check the presence of parameters: x or %%x (x from 0..9)
				for (k = 9; k >= 0; --k)
				{
					if ((pos("%%" + IntToStr(k), tmp) > 0) or (pos(":" + IntToStr(k), tmp) > 0))
						break;
				}

				if (k > 0)                          // number of parameters
					t_mac[idx + 1] = IntToStr(k);
				else
					t_mac[idx + 1].erase();

				t_mac[idx + 2] = tmp;                 // rule

				if (pass_end < 3)
					pass_end = 3;      // if there are macros, there must be at least 3 passes
			}

			save_lst(' ');

			ety.erase();
			break;
		}

		//----------------------------------------------------------------------------*)
		//  label .UNDEF [label]                                                      *)
		//----------------------------------------------------------------------------*)
		case __undef:
		{

			if (ety.empty())
				ety = get_lab(i, zm, true);

			if (if_test)
			{   // .UNDEF can be changed in the same run

				k = load_lab(ety, true);
				if ((k >= 0) && (t_lab[k].bank == __id_define))
				{
					idx = t_lab[k].addr;

					if (t_mac[idx][1-1] != '~')
						t_mac[idx] = '~' + t_mac[idx]; // mark it as disabled

				}
				else
					error_und(zm, ety, WARN_UNDECLARED_MACRO);
			}
			save_lst(' ');
			ety.erase();
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .LONGA|.LONGI ON/OFF						      *)
		//----------------------------------------------------------------------------*)
		case __longa:
		case __longi:
		{
			if (macro_rept_if_test())
			{
				if (!ety.empty())
					save_lab(ety, addres, bank, zm);

				skip_spaces(i, zm);
				str = get_lab(i, zm, false);

				if (not(opt & opt_C))
					show_error(zm, ERROR_ILLEGAL_ADDR_MODE_65XX);

				if ((str.empty()) || ((str != "ON") && (str != "OFF")))
					show_error(zm, ERROR_IMPROPER_SYNTAX);

				if ((str == "ON") || (str == "16"))
					v = 16;
				else
					v = 8;

				if (mne.l == __longa)
					longa = v;
				else
					longi = v;

				save_lst(' ');
				ety.erase();
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .A8 ; .A16 ; .AI8 ; .AI16						      *)
		//----------------------------------------------------------------------------*)
		case __a:
		case __ai:
		{
			if (macro_rept_if_test())
			{
				if (!ety.empty())
					save_lab(ety, addres, bank, zm);

				if (mne.l == __a)
					ety = ".A";
				else
					ety = ".AI";

				skip_spaces(i, zm);
				str = read_DEC(i, zm);

				if ((str == "8") or (str == "16"))
				{
					if (not(opt & opt_C))
						show_error(zm, ERROR_ILLEGAL_ADDR_MODE_65XX);

					if (str == "8")
						mne.h[0] = ord(SEP);
					else
					{
						mne.h[0] = ord(REP);
					}

					if (mne.l == __a)
						mne.h[1] = 0x20;
					else
					{
						mne.h[1] = 0x30;
					}

				}
				else
				{
					error_und(a, ety + str, WARN_UNKNOWN_DIRECTIVE);
				}

				reg_size(mne.h[1], t_MXinst(mne.h[0]));

				mne.l = 2;
				ety.erase();
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .I8 ; .I16 ; .IA8 ; .IA16						      *)
		//----------------------------------------------------------------------------*)
		case __i:
		case __ia:
		{
			if (macro_rept_if_test())
			{
				if (!ety.empty())
					save_lab(ety, addres, bank, zm);

				if (mne.l == __i)
					ety = ".I";
				else
					ety = ".IA";

				skip_spaces(i, zm);
				str = read_DEC(i, zm);

				if ((str == "8") || (str == "16"))
				{
					if (not(opt & opt_C))
						show_error(zm, ERROR_ILLEGAL_ADDR_MODE_65XX);

					if (str == "8")
						mne.h[0] = ord(SEP);
					else
					{
						mne.h[0] = ord(REP);
					}

					if (mne.l == __i)
						mne.h[1] = 0x10;
					else
					{
						mne.h[1] = 0x30;
					}
				}
				else
					error_und(a, ety + str, WARN_UNKNOWN_DIRECTIVE);

				reg_size(mne.h[1], t_MXinst(mne.h[0]));

				mne.l = 2;
				ety.erase();
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  label .MACRO [label] [par1, par2, ...]                                    *)
		//  the occurrence of the .MACRO directive means at least 3 assembly passes   *)
		//----------------------------------------------------------------------------*)
		case __macro:
		{
			if (ety.empty())
				ety = get_lab(i, zm, true);

			if ((pass == 0) && if_test)
			{
				// Konop postulated to restore macros from PROC blocks
				//           if (proc ) t_prc[proc_nr-1].used = true;  // if a macro is in .PROC, then .PROC is always in use

				reserved_word(ety, zm);
				save_lab(ety, High(t_mac), __id_macro, zm);

				str = show_full_name(a, full_name, false);
				save_mac(str);  // saving the name of the file with the macro
				//           str={local_name+}ety;                   save_mac(str);  // saving the macro name

				if (proc)
					str = proc_name + local_name + ety;
				else
					str = local_name + ety;

				save_mac(str);
				str = ety;                          // pure macro name
				skip_spaces(i, zm);

				if (AllowStringBrackets.has(zm[i]))
					tmp = bounded_string(i, zm, true);
				else
					tmp = get_dat(i, zm, 0, false);

				j = 1 - 1;
				get_parameters(j, tmp, par, true);

				ety = "'";
				ch = ',';

				j = 1 - 1;                              // parameter counter
				for (k = 0; k < High(par); ++k)
				{
					txt = par[k];
					if (AllowQuotes.has(txt[1 - 1]))
					{
						// we load and check if it is correct
						ety = txt[1 - 1];
						if (length(txt) > 2)
							ch = txt[2 - 1];
					}
					else
					{
						txt = str + '.' + txt;

						// We only record macros in the first pass, so we have to "manually"
						// check whether the next parameter is not duplicated

						if (l_lab(txt) < 0)
							save_lab(txt, j, __id_mparam, zm);   // we record the macro parameter
						else
							error_und(zm, txt, WARN_LABEL_X_DECLARED_TWICE);

						inc(j);
					}
				}

				skip_spaces(i, zm);
				if (not(test_char(i, zm)))
					show_error(zm, ERROR_EXTRA_CHARS_ON_LINE);

				str = ety + ch;
				save_mac(str); // separator and operation mode
				str = "0";
				save_mac(str); // macro call number
				str = IntToStr(line);
				save_mac(str); // line number with macro

				str = "0";
				save_mac(str); // call counter for 'infinite loop' test

				if (pass_end < 3)
					pass_end = 3;     // if there are macros, there must be at least 3 passes
			}

			macro = true;
			save_lst(' ');
			ety.erase();
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .IF [IFT] expression                                                      *)
		//----------------------------------------------------------------------------*)
		case __if:
		case __ifndef:
		case __ifdef:
		{
			save_lab(ety, addres, bank, zm);

			txt = zm;              // TXT = ZM, for later modification of TXT

			if (mne.l == __ifndef || mne.l == __ifdef)
			{
				myInsert(".DEF", txt, i);

				if (mne.l == __ifndef)
					myInsert(".NOT ", txt, i);
			}

			if_stos[ifelse].old_iftest = if_test;

			inc(ifelse);
			if (ifelse >= if_stos.size())
				if_stos.resize(ifelse + 1);

			if_stos[ifelse]._okelse = 0x7FFFFFFF;      // .ELSE blocked

			if (if_test)
			{
				save_lst(' ');
				if_stos[ifelse]._else = false;
				if_stos[ifelse]._okelse = ifelse;

				branch = true;             // There is no question of relocation here
				war = calculate_value_noSPC(txt, zm, i, 0, 'F');

				if_test = (war != 0);
			}

			if_stos[ifelse]._if_test = if_test;
			ety.erase();
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .ENDIF [EIF]                                                              *)
		//----------------------------------------------------------------------------*)
		case __endif:
		{
			if (!ety.empty())
			{
				show_error(zm, ERROR_LABEL_NOT_REQUIRED);
			}
			else if (ifelse > 0)
			{
				dec(ifelse);

				//           if_test     = if_stos[ifelse]._if_test;
				//           else_used   = if_stos[ifelse]._else;

				if_test = if_stos[ifelse].old_iftest;
			}
			else
			{
				show_error(zm, ERROR_MISSING_DOT_IF);
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .ENDM                                                                     *)
		//----------------------------------------------------------------------------*)
		case __endm:
		{
			dec_end(zm, __endm);
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .ELSE [ELS]                                                               *)
		//----------------------------------------------------------------------------*)
		case __else:
		{
			if (!ety.empty())
				show_error(zm, ERROR_LABEL_NOT_REQUIRED);
			else if (if_stos[ifelse]._okelse == ifelse)
			{
				if (if_stos[ifelse]._else)
					show_error(zm, MSG_MISSING_ENDIF);

				if (ifelse == 0)
					show_error(zm, ERROR_MISSING_DOT_IF);

				if_stos[ifelse]._else = true;
				if_test = not(if_stos[ifelse]._if_test);
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .PRINT 'string' [,string2...] [,expression1,expression2...]               *)
		//----------------------------------------------------------------------------*)
		case __print:
		{
			if (!ety.empty())
			{
				show_error(zm, ERROR_LABEL_NOT_REQUIRED);
			}
			else if (macro_rept_if_test())
			{
				if (pass == pass_end)
				{
					write_out(i, zm);
				}
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .ELSEIF [ELI] expression                                                  *)
		//----------------------------------------------------------------------------*)
		case __elseif:
		{
			if (!ety.empty())
			{
				show_error(zm, ERROR_LABEL_NOT_REQUIRED);
			}
			else if (if_stos[ifelse]._okelse == ifelse)
			{
				// !!! necessary condition

				if (if_stos[ifelse]._else)
					show_error(zm, MSG_MISSING_ENDIF);

				if (ifelse == 0)
					show_error(zm, ERROR_MISSING_DOT_IF);

				if_test = not(if_stos[ifelse]._if_test);

				if (if_test)
				{
					save_lst(' ');
					branch = true;    // There is no question of relocation here
					war = calculate_value_noSPC(zm, zm, i, 0, 'F');
					if_test = (war != 0);

					if_stos[ifelse]._if_test = if_test; // !!! be sure to save IF_TEST
				}
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  label .LOCAL [label]                                                      *)
		//----------------------------------------------------------------------------*)
		case __local:
		{
			if (macro_rept_if_test())
			{
				yes = false;

				if (ety.empty())
				{
					skip_spaces(i, zm);

					if (zm[i] == '+')
					{
						yes = true;
						inc(i);
					}

					ety = get_lab(i, zm, false);
				}

				skip_spaces(i, zm);

				// if there is no area name .LOCAL or 'address', use the default name
				if ((ety.empty()) && (zm[i] != ','))
				{
					ety = __local_name + IntToStr(lc_nr);
					inc(lc_nr);
				}

				if (yes)
				{
					tmp = local_name;
					local_name.erase();

					idx = load_lab(ety, true);

					local_name = tmp;

					if (idx < 0)
						error_und(zm, ety, WARN_UNDECLARED_LABEL);

					tmp = local_name + ety;
					idx = load_lab(tmp, true);

					t_loc[local_nr].idx = idx;
					t_loc[local_nr].addr = addres;

					save_lst('a');

					save_end(__endl);
					save_local();

					local_name = ety + '.';

					t_end[end_idx - 1].addr = 0;
					t_end[end_idx - 1].restoreAddr = 0;
				}
				else
				{
					if (pass == 0)
					{
						if ((!ety.empty()) && (ety[1-1] == '?'))
							warning(WARN_ILLEGAL_CHARACTER);
					}

					t_loc[local_nr].offset = org_ofset;
					get_address(i, zm);    // new assembly address for .LOCAL

					if (not(test_char(i, zm)))
						show_error(zm, ERROR_EXTRA_CHARS_ON_LINE);

					save_lab(ety, addres, bank, zm, true);
					save_lst('a');

					t_loc[local_nr].addr = addres;
					idx = load_lab(ety, true);   // !!! TRUE
					t_loc[local_nr].idx = idx;

					save_end(__endl);
					save_local();
					local_name = local_name + ety + '.';

					if (local_name == ".")
						local_name.erase();
				}

				ety.erase();
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .ENDL                                                                     *)
		//----------------------------------------------------------------------------*)
		case __endl:
		{
			if (!ety.empty())
				show_error(zm, ERROR_LABEL_NOT_REQUIRED);
			else if (macro_rept_if_test())
			{
				if (local_nr > 0)
				{
					save_lst(' ');

					if (not(proc))
						oddaj_var();

					get_local(zm);

					if (t_loc[local_nr].idx >= 0)
					{
						t_lab[t_loc[local_nr].idx].lln = addres - t_loc[local_nr].addr;
						t_lab[t_loc[local_nr].idx].lid = true;
					}

					org_ofset = t_loc[local_nr].offset;
					//           dec(end_idx);
					get_address_update();
				}
				else
					show_error(zm, ERROR_MISSING_DOT_LOCAL);
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .REPT expression                                                          *)
		//----------------------------------------------------------------------------*)
		case __rept:
		{
			if (if_test)
			{
				if (rept)
					show_error(zm, ERROR_MISSING_DOT_ENDR);
				else
				{
					save_lab(ety, addres, bank, zm);
					save_lst(' ');

					if (rept_cnt == 0)
						t_rep.resize(1);

					rept = true;
					get_rept(i, zm);
					ety.erase();
				}
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .ENDR                                                                     *)
		//----------------------------------------------------------------------------*)
		case __endr:
		{
			if (if_test)
			{
				if (not(rept))
					show_error(zm, ERROR_MISSING_DOT_REPT);

				//          writeln(pass,',','stop'); halt;       // !!! this shouldn't happen
				//          dirENDR(zm,a,old_str, 0);
				ety.erase();
			}
			break;
		}

		//-----------------------------------------------------------------------------
		// label .STRUCT [label]                                                        
		// the occurrence of the .STRUCT directive means at least three assembly passes
		//-----------------------------------------------------------------------------
		case __struct:
		{
			if (macro_rept_if_test())
			{
				if (structure.use)
					show_error(zm, ERROR_MISSING_DOT_ENDS);
				else
				{
					if (ety.empty())
						ety = get_lab(i, zm, true);

					if (pass == 0)
						reserved_word(ety, zm);

					skip_spaces(i, zm);
					if (not(test_char(i, zm)))
						show_error(zm, ERROR_EXTRA_CHARS_ON_LINE);

					label_type = 'C';

					structure.drelocUSE = dotRELOC.use;
					structure.drelocSDX = dotRELOC.sdx;

					dotRELOC.use = false;
					dotRELOC.sdx = false;

					upd_structure(ety, zm);

					save_local();
					local_name = local_name + ety + '.';
					structure.use = true;

					// we remember the address to return it after .ENDS
					structure.addres = addres;

					save_lst('a');
					save_end(__ends);

					ety.erase();
				}
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .ENDS                                                                     *)
		//  we save the length and number of fields of the structure in the 'T_STR' table               *)
		//----------------------------------------------------------------------------*)
		case __ends:
		{
			if (!ety.empty())
				show_error(zm, ERROR_LABEL_NOT_REQUIRED);
			else if (macro_rept_if_test())
			{
				if (not(structure.use))
					show_error(zm, ERROR_MISSING_DOT_STRUCT);
				else
				{
					save_lst(' ');
					get_local(zm);

					structure.use = false;
					t_str[structure.idx].mySize = addres - structure.addres;	// we write down the length of the structure
					t_str[structure.idx].offset = structure.cnt;				// and the number of structure fields

					addres = structure.addres;									// we return the old assembly address
					inc(structure.id);  // we increase the structure identifier (number of structures)
					structure.cnt = -1;
					dec_end(zm, __ends);

					dotRELOC.use = structure.drelocUSE;
					dotRELOC.sdx = structure.drelocSDX;
				}
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .ENUM                                                                     *)
		//----------------------------------------------------------------------------*)
		case __enum:
		{
			if (macro_rept_if_test())
			{
				if (enumeration.use)
				{
					show_error(zm, ERROR_MISSING_DOT_ENDE);
				}
				else
				{
					if (ety.empty())
						ety = get_lab(i, zm, true);

					if (pass == 0)
						reserved_word(ety, zm);

					skip_spaces(i, zm);
					if (not(test_char(i, zm)))
						show_error(zm, ERROR_EXTRA_CHARS_ON_LINE);

					label_type = 'C';

					enumeration.drelocUSE = dotRELOC.use;
					enumeration.drelocSDX = dotRELOC.sdx;

					dotRELOC.use = false;
					dotRELOC.sdx = false;

					save_lab(ety, 0, __id_enum, zm);

					save_local();
					local_name += ety + '.';

					enumeration.use = true;
					enumeration.val = 0;
					enumeration.max = 0;

					save_lst('a');

					save_end(__ende);

					ety.erase();
				}
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .ENDE                                                                     *)
		//----------------------------------------------------------------------------*)
		case __ende:
		{
			if (!ety.empty())
				show_error(zm, ERROR_LABEL_NOT_REQUIRED);
			else if (macro_rept_if_test())
			{
				if (not(enumeration.use))
				{
					show_error(zm, ERROR_MISSING_DOT_STRUCT);
				}
				else
				{
					txt = local_name;
					SetLength(txt, length(txt) - 1);
					idx = load_lab(txt, false);

					t_lab[idx].addr = ValueToType(enumeration.max);

					save_lst(' ');

					get_local(zm);

					enumeration.use = false;

					//           dec(end_idx);
					dec_end(zm, __ende);

					dotRELOC.use = enumeration.drelocUSE;
					dotRELOC.sdx = enumeration.drelocSDX;

				}
			}
			break;
		}

		//----------------------------------------------------------------------------
		//  save the value to the .ARRAY array                                        
		//----------------------------------------------------------------------------
		case __array_run:
		{
			if (!ety.empty())
				show_error(zm, ERROR_LABEL_NOT_REQUIRED);
			else
				get_data_array(i, zm, array_idx - 1);
			break;
		}


		//----------------------------------------------------------------------------
		//  .ARRAY label [elements0] [elements1] [...] type = expression              
		//  an array with defined values                                              
		//----------------------------------------------------------------------------
		case __array:
		{
			if (aray)
				show_error(zm, ERROR_MISSING_DOT_ENDA);
			else if (macro_rept_if_test())
			{
				if (ety.empty())
					ety = get_lab(i, zm, true);

				if (pass == 0)
					reserved_word(ety, zm);

				get_array(i, zm, ety, addres);
				war = 0;       // allowed maximum value of the D-WORD type
				skip_spaces(i, zm);
				if (zm[i] == '=')
				{
					IncAndSkip(i, zm);
					war = calculate_value_noSPC(zm, zm, i, 0, tType[t_arr[array_idx - 1].siz - 1]);
				}

				skip_spaces(i, zm);
				if (not(test_char(i, zm)))
					show_error(zm, ERROR_EXTRA_CHARS_ON_LINE);

				for (j = 0; j < t_arr[array_idx - 1].len; ++j)
					t_tmp[j] = cardinal(war);  // FILLCHAR fills only bytes

				aray = true;
				save_end(__enda);
				ety.erase();
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .ENDA                                                                     *)
		//----------------------------------------------------------------------------*)
		case __enda:
		{
			if (!ety.empty())
				show_error(zm, ERROR_LABEL_NOT_REQUIRED);
			else if (macro_rept_if_test())
			{
				if (not(aray))
					show_error(zm, ERROR_MISSING_DOT_ARRAY);
				else
				{
					aray = false;
					v = t_arr[array_idx - 1].siz;

					if (not t_arr[array_idx - 1].isDefined)
					{
						// no array size specified
						_doo = array_used.max;                    // array size based on the amount of data

						t_arr[array_idx - 1].elm[0].count = _doo;
						t_arr[array_idx - 1].len = _doo * v;

						if (t_arr[array_idx - 1].len - 1 > 0xFFFF)
							show_error(zm, ERROR_CONSTANT_EXP_SUBRANGE_BOUND);
					}

					if (structure.use)
						yes = false;
					else if (proc)
						yes = t_prc[proc_nr - 1].used;
					else
						yes = true;

					if (yes)
					{
						for (idx = 0; idx < (t_arr[array_idx - 1].len / v); ++idx)
						{
							save_dta(t_tmp[idx], zm, tType[v - 1], 0);
						}
					}

					t.erase();
					save_lst(' ');

					//           dec(end_idx);
					dec_end(zm, __enda);
				}
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  label EXT type                                                            *)
		//----------------------------------------------------------------------------*)
		case __ext:
		{
			if (ety.empty())
			{
				show_error(zm, ERROR_LABEL_NAME_REQUIRED);
			}
			else if (macro_rept_if_test())
			{
				if (loop_used)
				{
					show_error(zm, ERROR_CANT_REPEAT_DIRECTIVE);
				}
				else
				{

					v = get_typeExt(i, zm);

					save_externalLabel(i, ety, zm, v);

					nul.i = 0;
					save_lst('l');

					ety.erase();
				}
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .EXTRN label .PROC (par1, par2, ...) [.var]|[.reg]                        *)
		//  .EXTRN label1, label2, label3 ... type                                    *)
		//  an extended equivalent of the EXT pseudo-command                          *)
		//----------------------------------------------------------------------------*)
		case __extrn:
		{
			if (macro_rept_if_test())
			{
				//     if (loop_used ) show_error(zm,36) else {

				nul.i = 0;
				save_lst('l');
				//v = 0;

				if (ety.empty())
				{         // if .EXTRN is not preceded by a label

					while (true)
					{
						get_parameters(i, zm, par, false);
						v = get_typeExt(i, zm);

						_doo = High(par);
						if (_doo == 0)
							show_error(zm, ERROR_LABEL_NAME_REQUIRED);

						for (_odd = _doo - 1; _odd >= 0; --_odd)
						{
							txt = par[_odd];

							if (txt.empty() == false)
								save_externalLabel(i, txt, zm, v);
							else
								show_error(zm, ERROR_LABEL_NAME_REQUIRED);
						}

						skip_spaces(i, zm);
						if ((v == __proc) || (test_char(i, zm)))
							break;
					}
				}
				else
				{
					// if .EXTRN precedes a label
					v = get_typeExt(i, zm);
					save_externalLabel(i, ety, zm, v);
				}

				ety.erase();
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .VAR label1, label2, label3 ... TYPE [=address]                           *)
		//  .VAR TYPE label1, label2 ....                                             *)
		//  .ZPVAR label1, label2, label3 ... TYPE [=address]                         *)
		//  .ZPVAR TYPE label1, label2 ....                                           *)
		//----------------------------------------------------------------------------*)
		case __var:
		case __zpvar:
		{
			if (!ety.empty())
			{
				show_error(zm, ERROR_LABEL_NOT_REQUIRED);
			}
			else if ((pass > 0) && macro_rept_if_test())
			{
				if (rept_run)
				{
					show_error(zm, ERROR_CANT_REPEAT_DIRECTIVE);
				}
				else
				{
					//     nul.i=0;
					save_lst('a');
					skip_spaces(i, zm);

					if ((mne.l == __zpvar) && (zm[i] == '='))
					{
						// .ZPVAR = XX
						inc(i);
						zpvar = integer(calculate_value_noSPC(zm, zm, i, 0, 'A'));

						subrange_bounds(zm, zpvar, 0xFF);
					}
					else
					{
						if (pass_end < 3)
							pass_end = 3;    // !!! at least 3 runs needed !!!

						while (true)
						{
							get_vars(i, zm, par, mne.l);

							skip_spaces(i, zm);
							if (test_char(i, zm, 0, '='))
								break;
						}

						txt = zm;
						// check whether the variable location address '=ADDRESS' has been specified
						idx = -1;
						if (par[High(par) - 1].empty() == false)
						{
							if (par[High(par) - 1][1-1] == '=')
							{
								i = 1 - 1;
								txt = par[High(par) - 1];
							}
						}

						if (txt.empty() == false)
						{
							if (txt[i] == '=')
							{
								// new allocation address
								inc(i);
								idx = integer(calculate_value_noSPC(txt, zm, i, 0, 'A'));

								testRange(zm, idx, 87);
							}
						}

						if (mne.l == __zpvar)
						{
							if (idx >= 0)
								zpvar = idx;

							for (_doo = 0; _doo < var_idx; ++_doo)
							{
								if (t_var[_doo].id == var_id)
								{
									t_var[_doo].addr = zpvar;
									t_var[_doo].inZeroPage = true;

									if (zpvar + t_var[_doo].cnt * t_var[_doo].mySize > 256)
										show_error(zm, MSG_VALUE_OUT_OF_RANGE);
									else
									{
										for (k = 0; k < t_var[_doo].cnt * t_var[_doo].mySize; ++k)
										{
											if ((pass == pass_end) and t_zpv[zpvar])
												warning(WARN_ACCESS_VIOLATION_AT_ADDR);

											t_zpv[zpvar] = true;

											inc(zpvar);
										}
									}
								}
							}
						}
						else if (idx >= 0)
						{
							for (_doo = 0; _doo < var_idx; ++_doo)
							{
								if (t_var[_doo].id == var_id)
								{
									t_var[_doo].addr = idx;
									t_var[_doo].inZeroPage = false;
									inc(idx, t_var[_doo].cnt * t_var[_doo].mySize);
								}
							}
						}

						inc(var_id);
					}

					ety.erase();
				}
			}
			break;
		}

		// !!! requires adding support for SDX, currently no relocation support !!!
		//----------------------------------------------------------------------------*)
		//  .BY [+byte] bytes and/or ASCII                                            *)
		//  .CB [+byte] bytes and/or ASCII                                            *)
		//  .SB [+byte] bytes and/or ASCII                                            *)
		//  .WO words                                                                 *)
		//  .HE hex bytes                                                             *)
		//  .DBYTE                                                                    *)
		//----------------------------------------------------------------------------*)
		case __by:
		case __wo:
		case __he:
		case __sb:
		case __cb:
		case __dbyte:
		{
			if (structure.use)
			{
				if (structure.use)
					show_error(zm, ERROR_IMPROPER_SYNTAX);
				//else
				// show_error(zm,71);
			}
			else if (macro_rept_if_test())
			{
				save_lab(ety, addres, bank, zm);

				switch (mne.l)
				{
					case __cb: get_maeData(zm, i, 'C'); break;
					case __by: get_maeData(zm, i, 'B'); break;
					case __wo: get_maeData(zm, i, 'W'); break;
					case __he: get_maeData(zm, i, 'H'); break;
					case __sb: get_maeData(zm, i, 'S'); break;
					case __dbyte: get_maeData(zm, i, 'D'); break;
				}

				ety.erase();
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .WHILE .TYPE ARG1 OPERAND ARG2 [.AND|.OR .TYPE ARG3 OPERAND ARG4]         *)
		//----------------------------------------------------------------------------*)
		case __while:
		{
			if (macro_rept_if_test())
			{
				save_lab(ety, addres, bank, zm);
				save_lst('a');

				if (while_name.empty())
					while_name = local_name;
				while_name += IntToStr(while_nr) + '.';   // access path to WHILE

				r = 0;
				long_test.erase();
				conditional_expression(i, long_test, zm, txt, tmp, v, r, "\\EX LDA#0\\:?J/2-1 ORA#0\\.ENDL");

				save_end(__endw);

				ety = __while_label + while_name;        // start .WHILE
				save_fake_label(ety, zm, addres);
				ety = __endw_label + while_name;         // end of WHILE

				if (long_test.empty())
				{
					mne = asm_test(txt, tmp, zm, ety, r, v);
				}
				else
				{
					idx = High(t_mac);
					_odd = idx + 1;

					t_mac[idx] = long_test + "\\ JEQ " + ety;

					line_add = line - 1;

					save_lst('i');
					txt = zm;
					zapisz_lst(txt);

					opt_tmp = opt;
					opt = opt & byte(~opt_L);

					code6502 = true;

					analyze_mem(idx, _odd, zm, a, old_str, 0, 1, false);

					code6502 = false;

					line_add = 0;
					opt = opt_tmp;
					bez_lst = true;
				}

				inc(whi_idx);
				inc(while_nr);
				ety.erase();
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .ENDW                                                                     *)
		//----------------------------------------------------------------------------*)
		case __endw:
		{
			if (!ety.empty())
			{
				show_error(zm, ERROR_LABEL_NOT_REQUIRED);
			}
			else if (macro_rept_if_test())
			{
				if (whi_idx > 0)
				{
					save_lst('a');
					dec(whi_idx);

					ety = __while_label + while_name;

					war = load_lab(ety, false);    // we read the value of the label of the beginning of the loop WHILE ##B
					if (war >= 0)
					{
						tst = t_lab[war].addr;
						save_relAddress(integer(war), reloc_value);

						if (pass == pass_end)
						{
							if (t_lab[war].bank != bank)
							{
								warning(WARN_PAGE_ERROR_AT); // The .WHILE loop must be within the same bank
							}
						}
					}
					else
					{
						tst = 0;
					}

					txt = "JMP " + IntToStr(tst);
					k = 1-1;
					mne = calculate_mnemonic(k, txt, zm);

					ety = __endw_label + while_name;
					idx = addres + 3;            // E## = current address + 3 bytes (JMP)
					save_fake_label(ety, zm, idx);

					cut_dot(while_name);
					dec_end(zm, __endw);
					ety.erase();
				}
				else
					show_error(zm, ERROR_MISSING_HASH_WHILE);
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .TEST .TYPE ARG1 OPERAND ARG2 [.AND|.OR .TYPE ARG3 OPERAND ARG4]          *)
		//----------------------------------------------------------------------------*)
		case __test:
		{
			if (macro_rept_if_test())
			{
				save_lab(ety, addres, bank, zm);
				save_lst('a');

				if (test_name.empty())
					test_name = local_name;
				test_name += IntToStr(test_nr) + '.';   // access path to TEST

				long_test.erase();
				conditional_expression(i, long_test, zm, txt, tmp, v, r, "\\EX LDA0\\:?J/2-1 ORA0\\.ENDL");

				save_end(__endt);

				ety = __test_label + test_name;          // start #if __TEST_LABEL
				save_fake_label(ety, zm, addres);

				ety = __telse_label + test_name;        // we check whether #ELSE occurred
				if (load_lab(ety, false) < 0)
					ety = __endt_label + test_name;        // if there was no #ELSE, jump to #END

				if (long_test.empty())
				{
					mne = asm_test(txt, tmp, zm, ety, r, v);
				}
				else
				{
					idx = High(t_mac);
					_odd = idx + 1;

					t_mac[idx] = long_test + "\\ JEQ " + ety;

					line_add = line - 1;

					save_lst('i');
					txt = zm;
					zapisz_lst(txt);

					opt_tmp = opt;
					opt = opt & byte(~opt_L);

					code6502 = true;
					analyze_mem(idx, _odd, zm, a, old_str, 0, 1, false);
					code6502 = false;

					line_add = 0;
					opt = opt_tmp;
					bez_lst = true;
				}

				t_els[test_idx] = false;
				inc(test_idx);
				if (test_idx >= t_els.size())
					t_els.resize(test_idx + 1);

				inc(test_nr);

				ety.erase();
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .TELSE                                                                    *)
		//----------------------------------------------------------------------------*)
		case __telse:
		{
			if (!ety.empty())
			{
				show_error(zm, ERROR_LABEL_NOT_REQUIRED);
			}
			else if (macro_rept_if_test())
			{
				if (test_idx > 0)
				{
					if (t_els[test_idx - 1])
						show_error(zm, ERROR_MISSING_HASH_IF);

					txt = "JMP " + __endt_label + test_name;
					mne = asm_mnemo(txt, zm);

					t_els[test_idx - 1] = true;

					ety = __telse_label + test_name;          // address #ELSE __TELSE_LABEL
					save_fake_label(ety, zm, addres);
					dec(addres, mne.l);
					ety.erase();
				}
				else
				{
					show_error(zm, ERROR_MISSING_HASH_IF);
				}
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .ENDT                                                                     *)
		//----------------------------------------------------------------------------*)
		case __endt:
		{
			if (!ety.empty())
			{
				show_error(zm, ERROR_LABEL_NOT_REQUIRED);
			}
			else if (macro_rept_if_test())
			{
				if (test_idx > 0)
				{
					save_lst('a');
					dec(test_idx);
					ety = __test_label + test_name;

					war = load_lab(ety, false);              // address #if __TEST_LABEL
					if (war >= 0)
					{
						if (pass == pass_end)
						{
							if (t_lab[war].bank != bank)
								warning(WARN_PAGE_ERROR_AT); // TEST must be within the same bank's area
						}
					}

					ety = __endt_label + test_name;           // #END address for the #if __ENDT_LABEL block
					save_fake_label(ety, zm, addres);

					cut_dot(test_name);
					dec_end(zm, __endt);
					ety.erase();
				}
				else
				{
					show_error(zm, ERROR_MISSING_HASH_IF);
				}
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .DEF label                                                                *)
		//----------------------------------------------------------------------------*)
		case __def:
		{
			if (!ety.empty())
			{
				show_error(zm, ERROR_LABEL_NOT_REQUIRED);
			}
			else if (macro_rept_if_test())
			{
				idx = addres;
				skip_spaces(i, zm);
				label_type = 'V';

				if (zm[i] == ':')
				{
					old_case = true;           // the ':' character was used in the label name
					inc(i);
				}
				else
				{
					old_case = false;          // the ':' character was not used in the label name
				}

				ety = get_lab(i, zm, true);
				skip_spaces(i, zm);
				txt = zm;                   // we substitute ZM for TXT

				if (txt[i] == '+' || txt[i] == '-')
				{
					if (txt[i + 1] == '+' || txt[i + 1] == '-' || txt[i + 1] == '=')
					{
						compound_operator(i, txt, ety, old_case);
					}
				}

				if (txt[i] == '=')
				{
					IncAndSkip(i, txt);
					variable = false;
					idx = static_cast<int>(calculate_value_noSPC(txt, zm, i, 0, 'W'));
					if (not(variable))
						label_type = 'C';
				}
				else if (not(test_char(i, txt)))
				{
					show_error(zm, ERROR_IMPROPER_SYNTAX);
				}

				nul.i = idx;
				save_lst('l');
				data_out = true;     // force to be shown in the LST file when executing the macro
				// !!! if there is an .IFNDEF directive, I will not show it!!!

				old_run_macro = run_macro;
				run_macro = false;

				//      old_loopused  = dotRELOC.use;
				//      dotRELOC.use    = false;

				mne_used = true;

				if (old_case)
					s_lab(ety, idx, bank, zm, ety[1-1]);   // global label
				else
					save_lab(ety, idx, bank, zm);      // label in the current range

				run_macro = old_run_macro;

				ety.erase();
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .USING label [,label2...]                                                 *)
		//----------------------------------------------------------------------------*)
		case __using:
		{
			if (!ety.empty())
			{
				show_error(zm, ERROR_LABEL_NOT_REQUIRED);
			}
			else if (macro_rept_if_test())
			{
				save_lst('i');
				get_parameters(i, zm, par, false);
				if (pass > 0)
				{
					for (k = High(par) - 1; k >= 0; --k)
					{
						txt = par[k];
						_odd = load_lab(txt, false);
						if (_odd < 0)
							error_und(zm, txt, WARN_UNDECLARED_LABEL);

						t_usi[usi_idx].location = end_idx;
						t_usi[usi_idx].myLabel = txt;

						if (proc)
							t_usi[usi_idx].name = proc_name;
						else
							t_usi[usi_idx].name = local_name;

						inc(usi_idx);

						t_usi.resize(usi_idx + 1);
					}
				}
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .CBM 'text'								      *)
		//----------------------------------------------------------------------------*)
		case __cbm:
		{
			if (macro_rept_if_test())
			{
				save_lab(ety, addres, bank, zm);
				save_lst('a');
				txt = get_string(i, zm, zm, true);

				for (j = 0; j < length(txt); ++j)
				{
					if (txt[j] >= 'a' && txt[j] <= 'z')
					{
						save_dst(byte(txt[j]) - 96);
					}
					else if (txt[j] >= '[' && txt[j] <= '_')
					{
						save_dst(byte(txt[j]) - 64);
					}
					else
					{
						switch (txt[j])
						{
							case '`': save_dst(64); break;
							case '@': save_dst(0); break;
							default:
								save_dst(byte(txt[j]));
						}
					}
				}

				inc(addres, length(txt));
				ety.erase();
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .BI binary								      *)
		//----------------------------------------------------------------------------*)
		case __bi:
		{
			if (macro_rept_if_test())
			{
				save_lab(ety, addres, bank, zm);
				save_lst('a');
				skip_spaces(i, zm);

				if (test_char(i, zm))
					show_error(zm, ERROR_UNEXPECTED_EOL);

				k = 0;
				v = 0;

				while (not(test_char(i, zm)))
				{
					txt = get_dat(i, zm, ',', true);
					if (txt.empty())
						show_error(zm, MSG_VALUE_OUT_OF_RANGE);

					if (txt[length(txt) - 1] == '*')
					{
						for (j = 0; j < length(txt); ++j)
						{
							switch (txt[j])
							{
								case '0':
								{
									v = v | tora[k];
									inc(k);
									break;
								}
								case '1':
									inc(k);
									break;
								default:
									show_error(zm, MSG_VALUE_OUT_OF_RANGE);
							}

							if (k == 8)
							{
								save_dst(v);
								v = 0;
								k = 0;
							}
						}
					}
					else
					{
						for (j = 0; j < length(txt); ++j)
						{
							switch (txt[j])
							{
								case '0':
									inc(k);
									break;
								case '1':
								{
									v = v | tora[k];
									inc(k);
									break;
								}
								default:
									show_error(zm, MSG_VALUE_OUT_OF_RANGE);
							}

							if (k == 8)
							{
								save_dst(v);
								v = 0;
								k = 0;
							}

						}
					}

					if (zm[i] == ',' || zm[i] == ' ' || zm[i] == 9)
						__next(i, zm);
					else
						break;
				}

				if (k != 0)
					save_dst(v);

				ety.erase();
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .FL									      *)
		//----------------------------------------------------------------------------*)
		case __fl:
		{
			if (macro_rept_if_test())
			{
				save_lab(ety, addres, bank, zm);
				save_lst('a');
				skip_spaces(i, zm);

				if (test_char(i, zm))
					show_error(zm, ERROR_UNEXPECTED_EOL);

				while (not(test_char(i, zm)))
				{
					txt = get_dat(i, zm, ',', true);
					save_fl(txt, zm);
					if (zm[i] == ',' || zm[i] == ' ' || zm[i] == 9)
						__next(i, zm);
					else
						break;
				}

				ety.erase();
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  label EQU [=] expression                                                  *)
		//  label SMB string[8]                                                       *)
		//  label SET expression                                                      *)
		//----------------------------------------------------------------------------*)
		case __equ:
		case __smb:
		case __set:
		case __addEqu:
		case __addSet:
		{
			if (ety.empty())
			{
				show_error(zm, ERROR_LABEL_NAME_REQUIRED);
			}
			else if (macro_rept_if_test())
			{
				if (loop_used)
				{
					show_error(zm, ERROR_LABEL_NAME_REQUIRED);
				}
				else
				{
					if (dotRELOC.use)
					{
						if (mne.l == __equ || mne.l == __smb)
						{
							show_error(zm, ERROR_ILL_INST_AT_RELOC_BLOCK);
						}
					}

					label_type = 'C';
					old_case = dotRELOC.use;   // label declarations in the .RELOC block cannot be relocated
					dotRELOC.use = false;

					if (mne.l == __smb)
					{
						// SMB
						txt = get_smb(i, zm);

						// if there is a ^ sign behind the symbol name, ) we mark Weak Symbol
						if (zm[i] == '^')
						{
							t_smb[smb_idx].weak = true;
							inc(i);
						}

						// we enforce relocation for this label
						// if BLOCK>1, mark it as a relocatable label
						k = blok;
						blok = 0xFFFF;
						save_lab(ety, smb_idx, __id_smb, zm);
						blok = k;

						// we save the SMB symbol label in the T_SMB ??array
						t_smb[smb_idx].name = txt;                                 // SAVE_SMB
						inc(smb_idx);                                            //
						if (smb_idx >= t_smb.size())
							t_smb.resize(smb_idx + 1);  //

						war = __rel;
					}
					else
					{
						// EQU, addEQU
						branch = true;              // for EQU and addEQU there is no relocation

						// cannot assign label value EXTERNAL, SMB (BLOCKED = TRUE)
						blocked = true;

						// we check whether the label declaration was successful (UNDECLARED = FALSE)
						// the calculate_value procedure will modify UNDECLARED to TRUE if it occurs
						// attempt to reference an undefined label
						undeclared = false;
						variable = false;
						etyArray = ety;
						war = calculate_value_noSPC(zm, zm, i, 0, 'F');
						if (variable)
							label_type = 'V';

						blocked = false;          // !!! we must restore BLOCKED = FALSE !!!
						mne_used = true;            // a necessary test for changing label values
						_odd = load_lab(ety, true); // !!! TRUE
						if (etyArray.empty())                          // was declared by .ARRAY
						{
							t_arr[array_idx - 1].addr = static_cast<int>(war);               // update the .ARRAY address
						}
						else if ((mne.l == __set || mne.l == __addSet) && (_odd >= 0) && (pass > 0))
						{
							//s_lab(ety,cardinal(war),bank,zm,'?')      // !!! will not work for .LOCAL blocks etc. !!!

							t_lab[_odd].addr = static_cast<int>(war);	// !!! only this way and "(_ODD>=0) AND (PASS>0)" !!!
							t_lab[_odd].pass = pass;
						}
						else
						{
							save_lab(ety, cardinal(war), bank, zm);
						}
						_odd = load_lab(ety, true); // !!! TRUE

						if (_odd >= 0)
						{
							t_lab[_odd].sts = undeclared;
						}

						undeclared = false;         // !!! we must restore UNDECLARED = FALSE !!!
						data_out = true;          // force to be shown in the LST file when executing the macro
						mne_used = false;
					}

					nul.i = integer(war);
					save_lst('l');

					if (mne.l == __addEqu)
					{
						zapisz_lst(tmpZM);
						zm.erase();
					}

					dotRELOC.use = old_case;
					ety.erase();
				}
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  OPT holscmtb?f +-                                                         *)
		//----------------------------------------------------------------------------*)
		//  bit                                                                       *)
		//   0 - Header                        default = yes   'h+'                   *)
		//   1 - Object file                   default = yes   'o+                    *)
		//   2 - Listing                       default = no    'l-'                   *)
		//   3 - Screen (listing on screen)    default = no    's-'                   *)
		//   4 - CPU 8bit/16bit                default = 6502  'c-'                   *)
		//   5 - visible macro                 default = no    'm-'                   *)
		//   6 - track sep rep                 default = no    't-'                   *)
		//   7 - banked mode                   default = no    'b-'                   *)
		//----------------------------------------------------------------------------*)
		case __opt:
		{
			if (macro_rept_if_test())
			{
				txt = get_dat_noSPC(i, zm, zm, ',');   // we skip spaces

				j = 1 - 1;
				while (j < length(txt))
				{
					ch = txt[j + 1];
					opt_tmp = opt;
					v = 0;

					switch (UpCase(txt[j]))
					{
						case 'B': v = opt_B; break;
						case 'C': v = opt_C; break;
						case 'F': raw.use = (ch == '+'); break;
						case 'H': v = opt_H; break;
						case 'L': v = opt_L; break;
						case 'M': v = opt_M; break;
						case 'O': v = opt_O; break;
						case 'R': regAXY_opty = (ch == '+'); break;
						case 'S': v = opt_S; break;
						case 'T': v = opt_T; break;
						case '?': mae_labels = (ch == '+'); break;
						default:
							show_error(zm, ERROR_INVALID_OPTION);
					}

					switch (ch)
					{
						case '+': opt = opt | v; break;
						case '-': opt = opt & byte(~v); break;
						default:
							show_error(zm, ERROR_INVALID_OPTION);
					}

					if ((opt_tmp & opt_H) != (opt & opt_H))   // OPT H-
					{
						if ((opt & opt_H) == 0)
							save_hea();
					}

					inc(j, 2);
				}

				data_out = true;

				save_lst(' ');
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  ORG addres [,addres2]                                                     *)
		//  RUN addres                                                                *)
		//  INI addres                                                                *)
		//  BLK                                                                       *)
		//----------------------------------------------------------------------------*)
		case __org:
		case __run:
		case __ini:
		case __blkSpa:
		case __blkRel:
		case __blkEmp:
		{
			if (if_test)
			{
				if (loop_used or rept)
				{
					show_error(zm, ERROR_CANT_REPEAT_DIRECTIVE);
				}
				else if (dotRELOC.use)
				{
					show_error(zm, ERROR_ILL_INST_AT_RELOC_BLOCK);
				}
				else if (first_org && (!ety.empty()))
				{
					show_error(zm, WARN_NO_ORG_SPECIFIED);
				}
				else
				{
					if ((org_ofset > 0) && ((local_name.empty() == false) or proc))
						show_error(zm, ERROR_ILL_INST_AT_RELOC_BLOCK);

					if (hea_offset.addr >= 0)
						raw.old = hea_offset.addr + (addres - hea_offset.old);
					else
						raw.old = addres;

					label_type = 'V';
					data_out = true;
					save_lst('a');
					branch = true;    // Relocation is not possible here

					rel_ofs = 0;
					idx = -0xFFFF;
					hea_offset.addr = -1;
					_odd = -1;

					save_lab(ety, cardinal(addres), bank, zm);
					skip_spaces(i, zm);

					r = mne.l;    // !!! definitely V = MNE.L !!! for RUN, INI to work
					// !!! RUN, INI modify MNE.L !!!

					switch (r)
					{
						// BLK SPARTA a
						case __blkSpa:
						{
							dotRELOC.sdx = true;
							blok = 0;
							opt_tmp = opt;
							opt_h_minus();

							// a($fffa),a(str_adr),a(end_adr)
							save_dstW(0xfffa);
							idx = integer(calculate_value_noSPC(zm, zm, i, ',', 'A'));
							opt = opt_tmp;
							break;
						}

						//BLK R[eloc] M[ain]|E[xtended]
						case __blkRel:
						{
							dotRELOC.sdx = true;
							if (ds_empty > 0)
							{
								give_back_to();
								bez_lst = false;
								save_lst('a');
							}

							ds_empty = 0;
							opt_tmp = opt;
							opt_h_minus();
							v = getMemType(i, zm);
							add_block(zm);

							// a($fffe),b(blk_num),b(blk_id)
							// a(blk_off),a(blk_len)
							save_dstW(0xfffe);
							save_dst(byte(blok));
							save_dst(memType);

							if (blok == 1)
								idx = __rel;
							else
							{
								rel_ofs = addres - __rel;
								_odd = addres;
								idx = addres;
							}

							opt = opt_tmp;
							break;
						}

						//BLK E[mpty] expression M[ain]|E[xtended]
						case __blkEmp:
						{
							_odd = integer(calculate_value_noSPC(zm, zm, i, ' ', 'A'));  // !!! be sure to use the space character ' ' !!!
							// blk empty empend-tdbuf extended
							v = getMemType(i, zm);

							if (_odd > 0)
							{
								opt_tmp = opt;
								opt_h_minus();
								blk_empty(_odd, zm);
								opt = opt_tmp;
								if (ds_empty > 0)
									show_error(zm, ERROR_NO_RECURSIVE_STRUCTURES);  // either .DS or BLK EMPTY, you can't do both
							}
							idx = addres + _odd;
							break;
						}

						// ORG
						case __org:
						{
							if (dotRELOC.sdx)
								show_error(zm, ERROR_ILL_INST_AT_RELOC_BLOCK);

							if (not(NoAllocVar) && not(proc) && (local_nr == 0))
								oddaj_var();

							if (opt & opt_C)          // for 65816 we allow a 24 bit address
								typ = 'T';
							else
								typ = 'A';

							NoAllocVar = false;
							org_ofset = 0;

							if ((zm[i + 1] == ':') && (UpCase(zm[i]) == 'R'))
							{
								// org r:  (XASM)
								inc(i, 2);

								idx = integer(calculate_value_noSPC(zm, zm, i, 0, typ));

								hea_offset.addr = addres;
								hea_offset.old = idx;
								org_ofset = idx - hea_offset.addr;
							}
							else if (zm[i] == '[')
							{
								opt_tmp = opt;
								opt_h_minus();

								txt = bounded_string(i, zm, true);
								k = 1 - 1;
								if (pass > 1)
									calculate_data(k, txt, zm, 1);

								skip_spaces(i, zm);
								if (zm[i] == ',')
								{
									IncAndSkip(i, zm);
									idx = integer(calculate_value_noSPC(zm, zm, i, ',', typ));
								}

								opt = opt_tmp;
							}
							else
							{
								idx = integer(calculate_value_noSPC(zm, zm, i, ',', typ));
							}

							skip_spaces(i, zm);
							if ((i <= length(zm)) && (zm[i] == ','))
							{
								IncAndSkip(i, zm);

								hea_offset.addr = integer(calculate_value_noSPC(zm, zm, i, 0, typ));
								if (hea_offset.addr < 0)
									show_error(zm, WARN_NO_ORG_SPECIFIED);

								hea_offset.old = idx;
								org_ofset = idx - hea_offset.addr;
							}

							if (not(first_org))           // necessarily if NOT(FIRST_ORG) otherwise it will not save anything
							{
								if ((addres != idx) && raw.use)
								{
									if (raw.old < 0)
									{
										k = 0;
									}
									else if (hea_offset.addr >= 0)
									{
										k = hea_offset.addr - raw.old;
									}
									else
									{
										k = idx - raw.old;
									}

									if (k < 0)
									{
										if (pass > 0)
											show_error(zm, WARN_CANT_FIIL_HI_TO_LOW, Hex(idx, 4));
									}
									else if (not(hea))
										inc(::fill, k);  // if NOT(HEA) waits until it starts writing something to the file

									addres = idx;
								}
							}
							break;
						}

						// RUN,INI
						case __run:
						case __ini:
						{
							if (dotRELOC.sdx)
								show_error(zm, ERROR_ILL_INST_AT_RELOC_BLOCK);        // Illegal instruction at RELOC block

							if ((opt & opt_H) == 0)
								show_error(zm, ERROR_ILL_WHEN_ATARI_HEADER_DIS);  // Illegal when Atari file headers disabled

							oddaj_var();

							idx = integer(calculate_value_noSPC(zm, zm, i, 0, 'A'));
							mne.l = 2;

							mne.h[0] = byte(idx);
							mne.h[1] = byte(idx >> 8);

							idx = 0x2e0 + r - __run;

							runini.addr = addres;
							runini.used = true;

							break;
						}
					}

					if ((addres != idx) or (_odd >= 0))
					{
						save_hea();

						addres = idx;

						if (r != __blkEmp)
							org = true; // R = MNE.L
					}

					if (((opt & opt_C) && (idx >= 0) && (idx <= 0xFFFFFF)) || ((idx >= 0) && (idx <= 0xFFFF)))
						first_org = false;
					else
					{
						show_error(zm, MSG_VALUE_OUT_OF_RANGE, '(' + IntToStr(idx) + " must be between 0 and 65535)");  // !!! necessarily error(var,0) !!! for acceptance
					}
					// ORGs < 0 in initial assembly passes

					if (raw.use && (r == __run || r == __ini))
						first_org = true;

					ety.erase();
				}
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .PUT [index] = VALUE,...                                                  *)
		//----------------------------------------------------------------------------*)
		case __put:
		{
			if (macro_rept_if_test())
			{
				save_lab(ety, addres, bank, zm);
				skip_spaces(i, zm);
				array_used.max = 0;
				put_used = true;
				blocked = true;
				get_data_array(i, zm, 0);   // predefined array for .PUT
				blocked = false;
				put_used = false;
				ety.erase();
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .SAV [index] ['file',length] | ['file',ofset,length]                      *)
		//----------------------------------------------------------------------------*)
		case __sav:
		{
			//    if (!ety.empty() ) show_error(zm,38) else
			if (macro_rept_if_test())
			{
				save_lab(ety, addres, bank, zm);
				save_lst('a');
				idx_get = 0;
				skip_spaces(i, zm);

				if (AllowStringBrackets.has(zm[i]))
				{
					txt = bounded_string(i, zm, true);
					k = 1 - 1;
					idx_get = integer(calculate_value_noSPC(txt, zm, k, 0, 'A'));
				}

				txt = get_string(i, zm, zm, false);

				skip_spaces(i, zm);

				if (txt.empty() == false)
				{
					if (zm[i] == ',')
						IncAndSkip(i, zm);
					else
						show_error(zm, ERROR_UNEXPECTED_EOL);
				}

				_doo = integer(calculate_value_noSPC(zm, zm, i, ',', 'A'));

				skip_spaces(i, zm);

				if (zm[i] == ',')
				{
					IncAndSkip(i, zm);
					inc(idx_get, _doo);
					_doo = integer(calculate_value_noSPC(zm, zm, i, ',', 'A'));
				}

				_odd = idx_get + _doo;
				if (_odd > 0)
					dec(_odd);

				testRange(zm, _odd, 62); //subrange_bounds(zm,idx_get+_doo,$FFFF);

				test_eol(i, zm, zm, 0);

				if (txt.empty() == false)
				{
					txt = GetFile(txt, zm);

					if (pass == pass_end)
					{
						WriteAccessFile(txt);

						ofstream outFile;
						outFile.open(txt, ofstream::binary | ofstream::trunc);
						if (outFile.fail())
						{
							cerr << "Unable to open '" << txt << "' for writing!" << endl;
							exit(1);
						}
						outFile.write((const char*)& t_get.x[idx_get], _doo);
						outFile.close();
						/*
						AssignFile(g, txt);
						FileMode = 1;
						Rewrite(g, 1);
						blockwrite(g, t_get[idx_get], _doo);
						closefile(g);
						*/
					}
				}
				else
				{
					if ((pass == pass_end) && (_doo > 0))
					{
						for (idx = 0; idx < _doo; ++idx)
						{
							save_dst(t_get[idx_get + idx]);
						}
					}

					inc(addres, _doo);
				}

				ety.erase();
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .PAGES                                                                    *)
		//----------------------------------------------------------------------------*)
		case __pages:
		{
			if (!ety.empty())
				show_error(zm, ERROR_UNEXPECTED_EOL);
			else if (macro_rept_if_test())
			{
				if (addres < 0)
					show_error(zm, WARN_NO_ORG_SPECIFIED);
				else
				{
					save_lst('a');
					war = 1;
					txt = get_dat_noSPC(i, zm, zm, ' ');

					if (txt.empty() == false)
					{
						k = 1 - 1;
						war = calculate_value_noSPC(txt, zm, k, 0, 'A');   // the allowed range of values ??is .WORD

						k = integer(war);
						testRange(zm, k, 0);
					}

					skip_spaces(i, zm);
					if (not(test_char(i, zm)))
						show_error(zm, ERROR_EXTRA_CHARS_ON_LINE);

					t_pag[pag_idx].addr = integer((addres & 0x7FFFFF00));
					t_pag[pag_idx].cnt = integer(war) << 8;

					inc(pag_idx);
					if (pag_idx >= t_pag.size())
						t_pag.resize(pag_idx + 1);

					save_end(__endpg);
				}
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .ENDPG                                                                    *)
		//----------------------------------------------------------------------------*)
		case __endpg:
		{
			if (!ety.empty())
			{
				show_error(zm, ERROR_LABEL_NOT_REQUIRED);
			}
			else if (macro_rept_if_test())
			{
				if (pag_idx == 0)
				{
					show_error(zm, ERROR_LABEL_NOT_REQUIRED);
				}
				else
				{
					save_lst('a');
					dec(pag_idx);

					_odd = t_pag[pag_idx].addr;
					_doo = (addres - 1) & 0x7FFFFF00;

					if (pass == pass_end)
					{
						if ((_doo - _odd) >= t_pag[pag_idx].cnt)
							warning(WARN_PAGE_ERROR_AT);
					}

					dec_end(zm, __endpg);
				}
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .RELOC                                                                    *)
		//----------------------------------------------------------------------------*)
		case __reloc:
		{
			if (!ety.empty())
			{
				show_error(zm, ERROR_LABEL_NOT_REQUIRED);
			}
			else if (macro_rept_if_test())
			{
				if (not(hea and not(mne_used)))
				{
					show_error(zm, ERROR_ORG_AT_RELOC_BLOCK);
				}
				else
				{
					inc(bank);
					if (bank >= __id_param)
						show_error(zm, ERROR_TOO_MANY_BLOCKS);

					test_exists(zm, false);
					save_hea();

					if (dotRELOC.use)
					{
						rel_idx = 0;
						ext_idx = 0;
						extn_idx = 0;
						pub_idx = 0;
						smb_idx = 0;
						sym_idx = 0;
						skip_idx = 0;
						ext_used.use = false;
						rel_used = false;
						blocked = false;
						bulkUpdatePublic.adr = false;
						bulkUpdatePublic.ext = false;
						bulkUpdatePublic.pub = false;
						bulkUpdatePublic.symbol = false;
						bulkUpdatePublic.newSymbol = false;
					}

					save_lst(' ');
					k = get_type(i, zm, zm, false);

					if (k == 1)
						rel_ofs = __rel;           // $0000
					else
						rel_ofs = __relASM;        // $0100

					skip_spaces(i, zm);
					tmp = get_dat_noSPC(i, zm, zm, 0);
					if (tmp.empty() == false)
						war = calculate_value(tmp, zm);
					else
						war = 0;
					wartosc(zm, war, 'B');

					dotRELOC.use = true;
					addres = rel_ofs;

					first_org = false;
					org = true;

					save_dstW(__relHea);              // additional relocatable block header 'MR'
					save_dst(byte(war));               // user's .RELOC block code
					save_dst(byte(rel_ofs >> 8));      // information about block type .BYTE (0), .WORD (1)

					// we save label values ??for the software stack
					indeks = adr_label(__STACK_POINTER, false);
					save_dstW(indeks);  // @stack_pointer
					indeks = adr_label(__STACK_ADDRESS, false);
					save_dstW(indeks);  // @stack_address
					indeks = adr_label(__PROC_VARS_ADR, false);
					save_dstW(indeks);  // @proc_vars_adr
				}
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .LINK 'filename'                                                          *)
		//  allows you to attach relocatable code, i.e. DOS files with address $0000  *)
		//----------------------------------------------------------------------------*)
		case __link:
		{
			//    if (!ety.empty() ) show_error(zm,38) else
			if (macro_rept_if_test())
			{
				if (dotRELOC.use)
				{
					show_error(zm, ERROR_TOO_MANY_BLOCKS);
				}
				else
				{
					save_lab(ety, addres, bank, zm);
					txt = get_string(i, zm, zm, true);
					txt = GetFile(txt, zm);
					if (not(TestFile(txt)))
						show_error(txt, MSG_CANT_OPEN_CREATE_FILE);
					test_eol(i, zm, zm, 0);

					g.open(txt, std::ifstream::in | std::ifstream::binary);
					if (g.fail())
					{
						cerr << "Unable to open '" << txt << "' for reading!" << endl;
						exit(1);
					}
					//AssignFile(g, txt);
					//FileMode = 0;
					//Reset(g, 1);

					// check if we are loading a DOS file (first two bytes of the file = __HEA_DOS)
					t_lnk[0] = 0;
					t_lnk[1] = 0;

					g.read((char *)& t_lnk.x[0], 2);
					//blockread(g, t_lnk, 2);
					if ((t_lnk[0] + (t_lnk[1] << 8)) != __hea_dos)
						show_error(zm, ERROR_INCORRECT_HEADER_FOR_FILE);

					g.seekg(0, ios_base::beg);
					//Reset(g, 1);
					g.read((char*)& t_lnk.x[0], sizeof(t_lnk));
					idx = static_cast<int>(g.gcount());
					//blockread(g, t_lnk, sizeof(t_lnk), IDX); // load the entire file into T_LNK with length IDX

					// in the main loop we use the variables IDX, K, _ODD

					_odd = 0;
					dotLINK.use = false;
					dotLINK.linkedLength = 0;
					dotLINK.emptyLength = 0;

					while (_odd < idx)
					{
						j = fgetW(_odd);
						if (j == 0xFFFF)
							j = fgetW(_odd);

						switch (j)
						{
							// blk RELOC
							case __hea_reloc:
							{
								if (dotLINK.use)
									flush_link();

								k = fgetW(_odd);              // we read the length of the relocatable file, max $FFFE

								if (k == 0xffff)              // i.e. file length = 0
									k = 0;
								else
									inc(k);                     // we increase by 1

								j = fgetW(_odd);              // we read 2 bytes from the additional 'MR' header
								if (j != __relHea)
									show_error(zm, ERROR_INCORRECT_HEADER_FOR_FILE);
								//----------------------------------------------------------------------------*)
								// byte 0 - unused                                                            *)
								// byte 1 - bit 0     zero page code (0), off page zero code (1) 		      *)
								//          bit 1..7  unusable                                                *)
								//----------------------------------------------------------------------------*)
								fgetB(_odd);                 // we read 2 bytes with information about the .RELOC file
								v = fgetB(_odd);

								if ((v == 0) && (addres > 0xFF))
									show_error(zm, ERROR_ZP_RELOC_BLOCK);

								__link_stack_pointer_old = fgetW(_odd);       // @stack_pointer
								__link_stack_address_old = fgetW(_odd);       // @stack_address
								__link_proc_vars_adr_old = fgetW(_odd);       // @proc_vars_adr

								if (pass == 0)
								{
									if (dotLINK.checkedStack)
									{
										if ((__link_stack_pointer_old != __link_stack_pointer) or
											(__link_stack_address_old != __link_stack_address) or
											(__link_proc_vars_adr_old != __link_proc_vars_adr))
										{
											warning(WARN_INCOMPATIBLE_STACK_PARAMS);
										}
									}

									if (not(dotLINK.checkedStack))
									{
										__link_stack_pointer = __link_stack_pointer_old;
										__link_stack_address = __link_stack_address_old;
										__link_proc_vars_adr = __link_proc_vars_adr_old;

										txt = mads_stack[ord(__STACK_POINTER)].name;
										save_fake_label(txt, zm, __link_stack_pointer); //@stack_pointer
										txt = mads_stack[ord(__STACK_ADDRESS)].name;
										save_fake_label(txt, zm, __link_stack_address); //@stack_address
										txt = mads_stack[ord(__PROC_VARS_ADR)].name;
										save_fake_label(txt, zm, __link_proc_vars_adr); //@proc_vars_adr
									}
								}

								memcpy(t_ins.x, &t_lnk[_odd], k);	// we load the relocatable block into T_INS
								//move(t_lnk[_odd], t_ins, k);      // we load the relocatable block into T_INS
								inc(_odd, k);

								dotLINK.linkedLength = k;
								dotLINK.use = true;
								dotLINK.checkedStack = true;

								break;
							}

							// BLK UPDATE ADDRESS
							case __hea_address:
							{
								ch = chr(fgetB(_odd));
								j = fgetW(_odd);

								while (j > 0)
								{
									idx_get = fgetW(_odd);
									tst = addres;

									switch (ch)
									{
										case 'E':
										{
											dotLINK.emptyLength = idx_get;// an empty block reserving memory
											break;
										}

										case '<':
										case 'B':
										{
											inc(tst, t_ins[idx_get]);
											t_ins[idx_get] = byte(tst);
											break;
										}

										case '>':
										{
											inc(tst, t_ins[idx_get] << 8);
											v = fgetB(_odd);    // extra byte for relocation of older addresses
											inc(tst, v);
											t_ins[idx_get] = byte(tst >> 8);
											break;
										}

										case 'W':
										{
											inc(tst, t_ins[idx_get]);
											inc(tst, t_ins[idx_get + 1] << 8);

											t_ins[idx_get] = byte(tst);
											t_ins[idx_get + 1] = byte(tst >> 8);
											break;
										}

										case 'L':
										{
											inc(tst, t_ins[idx_get]);
											inc(tst, t_ins[idx_get + 1] << 8);
											inc(tst, t_ins[idx_get + 2] << 16);

											t_ins[idx_get] = byte(tst);
											t_ins[idx_get + 1] = byte(tst >> 8);
											t_ins[idx_get + 2] = byte(tst >> 16);

											break;
										}

										case 'D':
										{
											inc(tst, t_ins[idx_get]);
											inc(tst, t_ins[idx_get + 1] << 8);
											inc(tst, t_ins[idx_get + 2] << 16);
											inc(tst, t_ins[idx_get + 3] << 24);

											t_ins[idx_get] = byte(tst);
											t_ins[idx_get + 1] = byte(tst >> 8);
											t_ins[idx_get + 2] = byte(tst >> 16);
											t_ins[idx_get + 3] = byte(tst >> 24);
											break;
										}
									}

									dec(j);
								}
								break;
							}

							// BLK UPDATE EXTERNAL
							case __hea_external:
							{
								ch = chr(fgetB(_odd));
								j = fgetW(_odd);

								txt = fgetS(_odd);           // label_ext name

								idx_get = load_lab(txt, false);
								if (idx_get < 0)
								{
									war = 0;
									if (pass == pass_end)
										error_und(zm, txt, WARN_UNDEFINED_SYMBOL);
								}
								else
								{
									war = calculate_value(txt, zm);
								}

								v = ord(value_code(war, zm, true));

								// we check whether the label type is correct
								if (pass == pass_end)
								{
									switch (ch)
									{
										case 'B':
										{
											if (chr(v) != 'Z')
												error_und(zm, txt, WARN_INCOMPATIBLE_TYPES);
											break;
										}
										case 'W':
										{
											if (not(chr(v) == 'Z' or chr(v) == 'Q'))
												error_und(zm, txt, WARN_INCOMPATIBLE_TYPES);
											break;
										}
										case 'L':
										{
											if (not(chr(v) == 'Z' or chr(v) == 'Q' or chr(v) == 'T'))
												error_und(zm, txt, WARN_INCOMPATIBLE_TYPES);
											break;
										}
									}
								}

								while (j > 0)
								{
									idx_get = fgetW(_odd);
									tst = word(war);
									switch (ch)
									{
										case 'B':
										case '<':
										{
											inc(tst, t_ins[idx_get]);
											t_ins[idx_get] = byte(tst);
											break;
										}

										case '>':
										{
											inc(tst, t_ins[idx_get] << 8);

											v = fgetB(_odd);    // extra byte for relocation of older addresses

											inc(tst, v);

											t_ins[idx_get] = byte(tst >> 8);
											break;
										}

										case 'W':
										{
											inc(tst, t_ins[idx_get]);
											inc(tst, t_ins[idx_get + 1] << 8);

											t_ins[idx_get] = byte(tst);
											t_ins[idx_get + 1] = byte(tst >> 8);
											break;
										}

										case 'L':
										{
											inc(tst, t_ins[idx_get]);
											inc(tst, t_ins[idx_get + 1] << 8);
											inc(tst, t_ins[idx_get + 2] << 16);

											t_ins[idx_get] = byte(tst);
											t_ins[idx_get + 1] = byte(tst >> 8);
											t_ins[idx_get + 2] = byte(tst >> 16);
											break;
										}

										case 'D':
										{
											inc(tst, t_ins[idx_get]);
											inc(tst, t_ins[idx_get + 1] << 8);
											inc(tst, t_ins[idx_get + 2] << 16);
											inc(tst, t_ins[idx_get + 3] << 24);

											t_ins[idx_get] = byte(tst);
											t_ins[idx_get + 1] = byte(tst >> 8);
											t_ins[idx_get + 2] = byte(tst >> 16);
											t_ins[idx_get + 3] = byte(tst >> 24);
											break;
										}

									}

									dec(j);
								}

								break;
							}

							// BLK UPDATE PUBLIC
							case __hea_public:
							{
								j = fgetW(_odd);

								while (j > 0)
								{
									typ = chr(fgetB(_odd));		// TYPE			B-YTE, W-ORD, L-ONG, D-WORD
									ch = chr(fgetB(_odd));		// LABEL_TYPE   V-ARIABLE, P-ROCEDURE, C-ONSTANT

									if (dotLINK.use)
										tst = addres;
									else
										tst = 0;

									switch (ch)
									{
										case 'P':
										{
											str = fgetS(_odd);               // label_pub name
											inc(tst, fgetW(_odd));          // procedure address
											r = fgetB(_odd);                 // CPU register order
											rodzaj = chr(fgetB(_odd));       // procedure type

											if (not(rodzaj == __pDef or rodzaj == __pReg or rodzaj == __pVar))
												error_und(zm, str, WARN_INCOMPATIBLE_TYPES);

											idx_get = fgetW(_odd);           // number of parameter data

											proc_nr = proc_idx;         // necessarily via ADD_PROC_NR

											if (pass == 0)
											{
												_doo = _odd + idx_get;

												txt = "(";
												while (_odd < _doo)
												{
													v = TypeToByte(chr(fgetB(_odd)));   // number of bytes per parameter

													txt += mads_param[v-1];

													switch (rodzaj)               // parameter if necessary
													{
														case __pDef:
														{
															tmp = "par" + IntToStr(_odd);
															break;
														}

														case __pReg:
														{
															tmp.erase();
															while (v != 0)
															{
																tmp += ByteToReg(r);
																dec(v);
															}
															break;
														}

														case __pVar:
														{
															tmp = fgetS(_odd);
															break;
														}
													}

													txt += tmp + ' ';
												}

												txt += ')';

												switch (rodzaj)
												{
													case __pReg:
														txt += " .REG";
														break;
													case __pVar:
														txt += " .VAR";
														break;
												}

												i = 1 - 1;
												get_procedure(str, str, txt, i);    // we read the parameters

												proc = false;                     // be sure to turn off PROC
												proc_name.erase();                   // and necessarily PROC_NAME = ''
											}
											else
											{
												inc(_odd, idx_get);    // we necessarily increase _ODD when PASS!=0
											}

											upd_procedure(str, zm, tst);

											proc = false;           // be sure to turn off PROC
											proc_name.erase();         // and necessarily PROC_NAME = ''

											add_proc_nr();           // we must increase the procedure number

											break;
										}

										case 'A':
										{
											// we make the .ARRAY public
											str = fgetS(_odd);

											_doo = fgetW(_odd);

											inc(tst, _doo);               // new label address

											save_lab(str, array_idx, __id_array, zm);

											save_arr(tst, bank);          // here ARRAY_IDX is incremented by 1

											_doo = fgetW(_odd);

											v = TypeToByte(chr(fgetB(_odd)));

											t_arr[array_idx - 1].elm[0].count = _doo; // therefore now ARRAY_IDX-1

											t_arr[array_idx - 1].siz = v;
											t_arr[array_idx - 1].len = (_doo + 1) * v;

											break;
										}

										case 'S':
										{                        // we make .STRUCT public
											str = fgetS(_odd);
											_doo = fgetW(_odd);
											upd_structure(str, zm);    // UPD_STRUCTURE will set the STRUCT.IDX value

											save_local();
											local_name += str + '.';

											structure.addres = 0;

											while (_doo > 0)
											{
												v = TypeToByte(chr(fgetB(_odd)));
												str = fgetS(_odd);
												j = fgetW(_odd);

												save_str(str, structure.addres, v, j, addres, bank);
												save_lab(str, structure.addres, bank, zm);

												inc(structure.addres, v * j);
												inc(structure.cnt);
												dec(_doo);
											}

											t_str[structure.idx].mySize = structure.addres; // we write down the length of the structure
											t_str[structure.idx].offset = structure.cnt;   // and the number of structure fields
											inc(structure.id);  // we increase the structure identifier (number of structures)
											structure.cnt = -1;
											get_local(zm);

											break;
										}

										case 'V':
										{
											str = fgetS(_odd);
											_doo = fgetW(_odd);
											inc(tst, _doo);                  // new label address
											save_lab(str, tst, bank, zm);       // variable

											break;
										}

										case 'C':
										{
											str = fgetS(_odd);
											vtmp = 0;

											switch (typ)
											{
												case 'B':   vtmp = fgetB(_odd); break;
												case 'W':   vtmp = fgetW(_odd); break;
												case 'L':   vtmp = fgetL(_odd); break;
												case 'D':   vtmp = fgetD(_odd); break;
											}

											save_lab(str, vtmp, bank, zm);      // constant

											break;
										}

									}


									dec(j);
								}

								break;
							}

							default:
							{
								flush_link();

								k = fgetW(_odd);              // we read the length of the relocatable file
								inc(k);                      // we increase by 1

								testRange(zm, k, 62);         // maximum file length $FFFF

								save_hea();
								addres = j;
								org = true;

								dotLINK.linkedLength = k - j;

								memcpy(t_ins.x, &t_lnk[_odd], dotLINK.linkedLength);	// we load the relocatable block into T_INS
								//move(t_lnk[_odd], t_ins, dotLINK.linkedLength);      // we load the relocatable block into T_INS
								inc(_odd, dotLINK.linkedLength);
								break;
							}

						}
					}

					flush_link();

					g.close();

					ety.erase();
				}
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .PUBLIC label [,label2,label3...]                                         *)
		//----------------------------------------------------------------------------*)
		case __public:
		{
			if (!ety.empty())
			{
				show_error(zm, ERROR_LABEL_NOT_REQUIRED);
			}
			else if (macro_rept_if_test())
			{
				save_lst(' ');
				skip_spaces(i, zm);

				while (not(test_char(i, zm)))
				{
					txt = get_lab(i, zm, true);
					save_pub(txt, zm);
					_odd = load_lab(txt, false);
					if (_odd >= 0)
						t_lab[_odd].use = true;     // default label in use

					__next(i, zm);
				}

			}

			break;
		}

		//----------------------------------------------------------------------------*)
		//  .SYMBOL label                                                             *)
		//----------------------------------------------------------------------------*)
		case __symbol:
		{
			if (macro_rept_if_test())
			{
				if (not(dotRELOC.sdx))
				{
					show_error(zm, ERROR_ONLY_RELOC_BLOCK);
				}
				else
				{
					// only when SDX relocatable block
					if (ety.empty())
					{
						ety = get_lab(i, zm, true);
					}

					if (not(test_char(i, zm)))
					{
						show_error(zm, ERROR_EXTRA_CHARS_ON_LINE);
					}

					save_lst(' ');

					// we save the symbol .SYMBOL in T_SYM
					t_sym[sym_idx] = ety;                                  // SAVE_SYM
					inc(sym_idx);                                           //
					if (sym_idx > High(t_sym))
						SetLength(t_sym, sym_idx + 1); //

				}
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  INS 'filename' [+-expression] [,length[,offset]]                          *)
		//  .GET [index] 'filename' [+-expression] [,length[,offset]]                 *)
		//----------------------------------------------------------------------------*)
		case __ins:
		case __get:
		case __xget:
		{
			if (macro_rept_if_test())
			{
				if (first_org && (opt & opt_H) && (mne.l == __ins))
				{
					show_error(zm, WARN_NO_ORG_SPECIFIED);
				}
				else
				{
					data_out = true;
					idx_get = 0;
					skip_spaces(i, zm);

					if (mne.l == __get)
					{
						if (AllowStringBrackets.has(zm[i]))
						{
							txt = bounded_string(i, zm, true);
							k = 1 - 1;
							idx_get = integer(calculate_value_noSPC(txt, zm, k, 0, 'A'));
						}
					}

					txt = get_string(i, zm, zm, true);

					txt = GetFile(txt, zm);
					if (not(TestFile(txt)))
						show_error(txt, MSG_CANT_OPEN_CREATE_FILE);

					if ((GetFileName(txt) == GetFileName(a)) || (GetFileName(txt) == GetFileName(filenameASM)))
						show_error(txt, MSG_CANT_OPEN_CREATE_FILE);

					g.open(txt, std::ifstream::in | std::ifstream::binary);
					if (g.fail())
					{
						cerr << "Unable to open '" + txt + "' for reading!" << endl;
						exit(1);
					}
					//AssignFile(g, txt);
					//FileMode = 0;
					//Reset(g, 1);

					k = integer(FileSize(txt));
					_odd = 0;
					_doo = k;

					v = byte(test_string(i, zm, 'B'));

					if (zm[i] == ',')
					{
						IncAndSkip(i, zm);

						_odd = integer(calculate_value_noSPC(zm, zm, i, ',', 'F'));

						if (_odd < 0)
						{
							_doo = abs(_odd);
							_odd = k + _odd;
						}
						else
						{
							_doo = k - abs(_odd);
						}

						if (zm[i] == ',')
						{
							IncAndSkip(i, zm);

							_doo = integer(calculate_value_noSPC(zm, zm, i, ',', 'F'));
							// -           testRange(zm, _doo, 13);
							if (_doo < 0)
							{
								show_error(zm, ERROR_VALUE_OUT_OF_RANGE);
								_doo = 0;
							}

							test_eol(i, zm, zm, 0);
						}
					}
					else
					{
						test_eol(i, zm, zm, 0);
					}

					// check whether the file length has not been exceeded
					if ((abs(_odd) > k) || (abs(_odd) + abs(_doo) > k))
					{
						show_error(zm, ERROR_FILE_IS_TOO_SHORT);
					}
					k = _doo;

					// file length cannot be less than 0
					// -         testRange(zm, k, 25); //if (k>$FFFF ) show_error(zm,25);
					if (k < 0)
					{
						show_error(zm, ERROR_FILE_IS_TOO_LONG);
						k = 0;
					}

					save_lst('a');

					if (mne.l == __get || mne.l == __xget)
					{
						// .GET
						j = idx_get + _doo;
						testRange(zm, j, 62);  // subrange_bounds(zm,idx_get+_doo,$FFFF+1);

						if (_odd > 0)
						{
							g.seekg(_odd);
							//seek(g, _odd);         // omin _ODD bytes in the file
						}
						g.read((char*)&t_get.x[idx_get], _doo);
						//blockread(g, t_get[idx_get], _doo);  // read _DOO bytes from index [IDX_GET]

						if (mne.l == __xget)
						{
							for (j = 0; j < _doo; ++j)
							{
								if (t_get[idx_get + j] != 0)
								{
									t_get[idx_get + j] = t_get[idx_get + j] + v;
								}
							}
						}
						else
						{
							for (j = 0; j < _doo; ++j)
							{
								t_get[idx_get + j] = t_get[idx_get + j] + v;
							}
						}
					}
					else                               // INS
					if ((pass == pass_end) && (_doo > 0))
					{
						if (_odd > 0)
						{
							g.seekg(_odd);
							//seek(g, _odd);         // omin _ODD bytes in the file
						}

						for (j = 0; j < (_doo / 0x10000); ++j)
						{
							g.read((char*)&t_ins.x[0], 0x10000);
							//blockread(g, t_ins, 0x10000);        // read 64KB bytes
							for (idx = 0; idx <= 0xFFFF; ++idx)
							{
								save_dst(byte(t_ins[idx] + v));
							}
						}

						j = _doo % 0x10000;                  // read remaining bytes

						g.read((char*)&t_ins.x[0], j);
						//blockread(g, t_ins, j);

						for (idx = 0; idx < j; ++idx)
						{
							save_dst(byte(t_ins[idx] + v));
						}
					}

					g.close();
					//closefile(g);

					save_lab(ety, addres, bank, zm);

					if (not(mne.l == __get || mne.l == __xget))
						inc(addres, k);
					ety.erase();
				}
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  END                                                                       *)
		//----------------------------------------------------------------------------*)
		case __end:
		case __en:
		{
			if (loop_used)
			{
				show_error(zm, ERROR_CANT_REPEAT_DIRECTIVE);
			}
			else
			{
				end_file = true;
				save_lab(ety, addres, bank, zm);
				save_lst('a');
				mne.l = 0;
				ety.erase();
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  DTA [abefgtcd] 'string',expression...                                     *)
		//  or we add new fields to the .STRUCT structure                             *)
		//----------------------------------------------------------------------------*)
		case __dta:
		case __byte:
		case __word:
		case __long:
		case __dword:
		{
			if (macro_rept_if_test())
			{
				if (first_org && (opt & opt_H) && not(structure.use))
				{
					show_error(zm, WARN_NO_ORG_SPECIFIED);
				}
				else
				{
					defaultZero = (mne.l != __dta);

					if (structure.use)
					{
						// we save the structure fields
						skip_spaces(i, zm);

						if (zm[i] == ':')
						{
							// for the .byte :5 label syntax
							inc(i);
							txt = get_dat(i, zm, ',', true);
							_doo = integer(calculate_value(txt, zm));
							testRange(zm, _doo, 0);

							loop_used = true;
							rpt = _doo;
						}

						if (ety.empty())
						{
							get_parameters(i, zm, par, false, '.', 0);    // here it cannot stop reading for ':', e.g.: .BYTE :5 LABEL
						}

						if (High(par) == 0)
							show_error(zm, ERROR_IMPROPER_SYNTAX);

						skip_spaces(i, zm);
						test_eol(i, zm, zm, 0);

						for (idx = 0; idx < High(par); ++idx)
						{
							ety = par[idx];

							if ((ety.empty()) || not(_lab_first(ety[1-1])))
							{
								show_error(zm, ERROR_IMPROPER_SYNTAX);         // Structure fields must have labels
							}

							if (mne.l == __dta)
							{
								show_error(zm, ERROR_IMPROPER_SYNTAX);    // we only accept type names
							}

							j = mne.l - __byteValue;               // J = <1..4>
							k = addres - structure.addres;

							if ((ety + '.') == local_name)
							{
								show_error(zm, ERROR_NO_RECURSIVE_STRUCTURES);  // the field name must be != from the structure name
							}

							save_str(ety, k, j, rpt, addres, bank);
							save_lab(ety, k, bank, zm);

							nul.i = k;
							save_lst('l');

							if (loop_used)
							{
								j = j * rpt;
							}

							inc(addres, j);

							inc(structure.cnt);                    // we increase the counter of the structure fields
						}

					}
					else
					{
						create_struct_data(i, zm, ety, mne.l);
					}

					loop_used = false;

					ety.erase();
				}
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  ICL 'filename'                                                            *)
		//----------------------------------------------------------------------------*)
		case __icl:
		{
			if (macro_rept_if_test())
			{
				if (loop_used)
				{
					show_error(zm, ERROR_CANT_REPEAT_DIRECTIVE);
				}
				else
				{
					save_lab(ety, addres, bank, zm);
					save_lst('a');
					txt = get_string(i, zm, zm, true);
					txt = GetFile(txt, zm);
					if (not(TestFile(txt)))
					{
						string fn = txt + ".asm";
						tmp = GetFile(fn, zm);
						if (not(TestFile(tmp)))
						{
							show_error(txt, MSG_CANT_OPEN_CREATE_FILE);
						}
						else
							txt = tmp;
					}

					test_eol(i, zm, zm, 0);

					if ((txt == a) || (txt == filenameASM))
						show_error(txt, MSG_CANT_OPEN_CREATE_FILE);

					// we check whether it is a text file in the first two passes of assembly
					if (pass < 2)
					{
						f.open(txt, std::ifstream::in);
						if (f.fail())
						{
							cerr << "Unable to open '" + txt + "' for reading!" << endl;
							exit(1);
						}
						std::getline(f, tmp);
						f.close();
						//assignfile(f, txt);
						//FileMode = 0;
						//Reset(f);
						//readln(f, tmp);
						//CloseFile(f);
						//if (pos(0, tmp) > 0)
						if (tmp.find((char)0) != string::npos)
							show_error(zm, ERROR_INCORRECT_HEADER_FOR_FILE);
					}

					str = zm;
					zapisz_lst(str);

					old_icl_used = icl_used;
					icl_used = true;

					analyze_file(txt, zm);

					bez_lst = true;
					//          put_lst(show_full_name(a,full_name,true),'');
					global_name = a;

					line = nr;

					icl_used = old_icl_used;

					ety.erase();
					mne.l = 0;
				}
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .SEGDEF label, address, length, [attrib, [bank]]                          *)
		//----------------------------------------------------------------------------*)
		case __segdef:
		{
			if (macro_rept_if_test())
			{
				if (loop_used)
				{
					show_error(zm, ERROR_CANT_REPEAT_DIRECTIVE);
				}
				else if (!ety.empty())
				{
					show_error(zm, ERROR_LABEL_NOT_REQUIRED);
				}
				else
				{
					save_lst('a');
					get_parameters(i, zm, par, false);
					_doo = High(par);

					if ((_doo >= 3) && (_doo <= 5))
					{
						// at least 3 parameters required
						for (idx = 0; idx < _doo; ++idx)
						{
							if (par[idx].empty())
							{
								show_error(zm, ERROR_UNEXPECTED_EOL);
							}
						}
					}
					else
					{
						show_error(zm, ERROR_IMPROPER_SYNTAX);
					}

					k = -1;                                    // we check whether we already have this label
					for (idx = static_cast<int>(t_seg.size()) - 1; idx >= 0; --idx)
					{
						if (t_seg[idx].name == par[0])
						{
							k = idx;
							break;
						}
					}

					if (k < 0)
					{                         // first occurrence of the label
						idx = static_cast<int>(t_seg.size());	// We'll add it to the list
						t_seg.resize(idx + 2-1);
					}
					else if (t_seg[idx].pass == pass)
					{
						error_und(zm, par[0], WARN_LABEL_X_DECLARED_TWICE);
					}

					t_seg[idx].pass = pass;						// mileage number
					t_seg[idx].name = par[0];					// segment label
					tmp = par[1 - 1];							// segment starting address
					war = calculate_value(tmp, zm);				//
					t_seg[idx].start = static_cast<int>(war);	//
					t_seg[idx].addr = static_cast<int>(war);	//

					tmp = par[2 - 1];							// segment length
					war = calculate_value(tmp, zm);				//
					t_seg[idx].len = static_cast<int>(war);		//

					v = ord(__RW);								// Read/Write

					if (_doo >= 4)
					{
						if (par[3 - 1] == "RW")
							v = ord(__RW);
						else if (par[3 - 1] == "R")
							v = ord(__R);
						else if (par[3 - 1] == "W")
							v = ord(__W);
						else
							show_error(zm, ERROR_IMPROPER_SYNTAX);
					}

					t_seg[idx].attrib = t_Attrib(v);             // segment attribute

					if (_doo == 5)
					{
						tmp = par[4 - 1];							// segment bank, if missing then 0
						war = calculate_value(tmp, zm);				//
						t_seg[idx].bank = static_cast<int>(war);	//
					}
					else
						t_seg[idx].bank = 0;

					if (pass == pass_end)                    // !!! necessarily !!!
					{
						for (k = static_cast<int>(t_seg.size()) - 1; k >= 0; --k)
						{
							if (k != idx)
							{
								if (t_seg[idx].bank == t_seg[k].bank)
								{
									if ((t_seg[k].start <= t_seg[idx].start) && (t_seg[k].start + t_seg[k].len - 1 >= t_seg[idx].start))
									{
										show_error(zm, WARN_MEMORY_SEGMENTS_OVERLAP);
									}
								}
							}
						}
					}

					ety.erase();
				}
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .SEGMENT label                                                            *)
		//----------------------------------------------------------------------------*)
		case __segment:
		{
			if (macro_rept_if_test())
			{
				if (loop_used)
				{
					show_error(zm, ERROR_CANT_REPEAT_DIRECTIVE);
				}
				else if (!ety.empty())
				{
					show_error(zm, ERROR_LABEL_NOT_REQUIRED);
				}
				else
				{
					save_lst('a');
					bez_lst = true;

					txt = get_lab(i, zm, true);
					k = -1;

					for (idx = static_cast<int>(t_seg.size()) - 1; idx >= 0; --idx)
					{
						if (t_seg[idx].name == txt)
						{
							k = idx;
							break;
						}
					}

					if (k >= 0)
					{
						if (addres < 0)                          // if there was no ORG
							t_seg[segment].addr = t_seg[segment].start;
						else
						{
							t_seg[segment].addr = addres;
						}

						t_seg[segment].bank = bank;
						t_seg[segment].attrib = atr;

						txt = " org $" + Hex(t_seg[k].addr, 4);

						idx = line_add;
						line_add = line - 1;

						_odd = High(t_mac);                       // procedure with parameters
						t_mac[_odd] = txt;                        // we force more lines to be executed
						_doo = _odd + 1;
						analyze_mem(_odd, _doo, zm, a, old_str, 0, 1, false);

						line_add = idx;

						segment = k;
						bank = t_seg[k].bank;
						atr = t_seg[k].attrib;
					}
					else
					{
						error_und(zm, txt, WARN_UNDECLARED_LABEL);	// there is no such segment
					}
				}
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  .ENDSEG                                                                   *)
		//----------------------------------------------------------------------------*)
		case __endseg:
		{
			if (macro_rept_if_test())
			{
				if (loop_used)
				{
					show_error(zm, ERROR_CANT_REPEAT_DIRECTIVE);
				}
				else if (!ety.empty())
				{
					show_error(zm, ERROR_LABEL_NOT_REQUIRED);
				}
				else
				{
					save_lst('a');
					bez_lst = true;

					t_seg[segment].addr = addres;
					t_seg[segment].bank = bank;
					t_seg[segment].attrib = atr;

					txt = " org $" + Hex(t_seg[0].addr, 4);

					segment = 0;
					bank = t_seg[0].bank;
					atr = t_seg[0].attrib;

					idx = line_add;
					line_add = line - 1;

					_odd = High(t_mac);                       // procedure with parameters
					t_mac[_odd] = txt;                        // we force more lines to be executed
					_doo = _odd + 1;
					analyze_mem(_odd, _doo, zm, a, old_str, 0, 1, false);

					line_add = idx;

					if (pass == pass_end)
					{
						for (k = static_cast<int>(t_seg.size()) - 1; k >= 0; --k)
						{
							if (t_seg[k].bank == bank)
							{
								if ((t_seg[k].start < addres) and (t_seg[k].start + t_seg[k].len - 1 >= addres))
								{
									warning(WARN_MEMORY_SEGMENTS_OVERLAP);
								}
							}
						}
					}
				}
			}
			break;
		}

		//----------------------------------------------------------------------------*)
		//  #CYCLE cycle                                                              *)
		//----------------------------------------------------------------------------*)
		case __cycle:
		{
			if (macro_rept_if_test())
			{
				if (loop_used)
				{
					show_error(zm, ERROR_CANT_REPEAT_DIRECTIVE);
				}
				else
				{
					if (!ety.empty())
						save_lab(ety, addres, bank, zm);

					ety.erase();
					skip_spaces(i, zm);

					if (zm[i] == '#')
						IncAndSkip(i, zm);
					else
						show_error(zm, ERROR_CANT_REPEAT_DIRECTIVE);

					_doo = integer(calculate_value_noSPC(zm, zm, i, 0, 'A'));
					if (_doo < 2)
						show_error(zm, MSG_VALUE_OUT_OF_RANGE, '(' + IntToStr(_doo) + " must be between 2 and 65535)");

					save_lst('a');

					while (_doo > 0)
					{
						if (_doo <= 10)
							idx = _doo;
						else if (_doo % 10 < 2)
							idx = 2;
						else
							idx = _doo % 10;

						switch (idx)
						{
							case 2:
							{
								// NOP
								save_dst(0xea);
								inc(addres);
								break;
							}
							case 3:
							{
								// CMP Z
								save_dst(0xc5);
								save_dst(0);
								inc(addres, 2);
								break;
							}
							case 4:
							{
								// NOP:NOP
								save_dst(0xea);
								save_dst(0xea);
								inc(addres, 2);
								break;
							}
							case 5:
							{
								// CMP Z:NOP
								save_dst(0xc5);
								save_dst(0);
								save_dst(0xea);
								inc(addres, 3);
								break;
							}
							case 6:
							{
								// CMP (Z,X)
								save_dst(0xc1);
								save_dst(0);
								inc(addres, 2);
								break;
							}
							case 7:
							{
								// PHA:PLA
								save_dst(0x48);
								save_dst(0x68);
								inc(addres, 2);
								break;
							}
							case 8:
							{
								// CMP (Z,X):NOP
								save_dst(0xc1);
								save_dst(0);
								save_dst(0xea);
								inc(addres, 3);
								break;
							}
							case 9:
							{
								// PHA:PLA:NOP
								save_dst(0x48);
								save_dst(0x68);
								save_dst(0xea);
								inc(addres, 3);
								break;
							}
							case 10:
							{
								// PHA:PLA:CMP Z
								save_dst(0x48);
								save_dst(0x68);
								save_dst(0xc5);
								save_dst(0);
								inc(addres, 4);
								break;
							}
						}

						dec(_doo, idx);
					}

				}
			}
			break;
		}
	}


	// remembering the label if it occurred
	// and saving the decoded commands
	if (if_test)
	{
		skip_spaces(i, zm);
		txt.erase();
		search_comment_block(i, zm, txt);

		if (!ety.empty())
		{
			if (aray)
			{
				show_error(zm, ERROR_IMPROPER_SYNTAX);
			}

			label_type = 'V';          // !!! necessarily label_type = V-ariable

			if (pass == pass_end)
			{
				if (addres < 0)
				{
					warning(WARN_NO_ORG_SPECIFIED);
				}
			}

			save_lab(ety, addres, bank, zm);   // other labels unchanged

			save_lst('a');
		}

		if (mne.l < __equ)
		{
			nul = mne;

			if ((mne.l == 0) && (ety.empty()))
				save_lst(' ');
			else
				save_lst('a');

			nul.l = 0;
		}

	}

	// save the assembly result to an LST file

	if (not(loop_used) && not(FOX_ripit))
	{
		if (not(bez_lst))
		{
			zapisz_lst(zm);
		}
		bez_lst = false;
	}

	if (runini.used)
	{
		// RUN, INI will keep the current assembly address
		save_hea();
		addres = runini.addr;
		runini.used = false;
		new_DOS_header();
	}
}

/**
 * \brief download the lines and check if the line is not a comment
 * \param zm 
 * \param end_file 
 * \param ok 
 * \param _odd 
 * \param _doo 
 * \param app
 * \param appLine
 */
void get_line(string& zm, bool& end_file, bool& ok, int& _odd, int& _doo, bool& app, string& appLine)
{
	int i, j;
	string     str;

	if (end_file)
		return;

	noWarning = false;

	if (not(macro))        // for code in the REPT block */ NOP
	{
		if (isComment)
		{
			i = 1-1;
			str.erase();
			search_comment_block(i, zm, str);

			if (str.empty() == false)
			{
				save_lst(' ');
				justify();
				put_lst(t + zm);

				zm = copy(zm, i, length(zm));
			}

		}
	}

	if (not(isComment))
	{
		if (not(macro) and not(rept) and (pos('\\', zm) > 0))
		{
			// a cursory test for the presence of the '\\' character
			i = 1 - 1;
			str = get_dat(i, zm, '\\', false);                 // here we check more carefully

			SetLength(t_lin, 1);

			if (zm[i] == '\\')
			{
				save_lst(' ');
				justify();
				put_lst(t + zm);

				_odd = High(t_lin);
				i = 1 - 1;
				while (true)
				{
					str = get_dat(i, zm, '\\', false);

					j = 1 - 1;
					skip_spaces(j, str);
					if (j > length(str))
						str.erase();

					j = High(t_lin);       // SAVE_LIN
					t_lin[j] = str;        //
					SetLength(t_lin, j + 2); //

					if (zm[i] == '\\')
						inc(i);
					else
						break;
				}

				_doo = High(t_lin);

				if (str.empty())
				{
					app = true;

					appLine += t_lin[_doo - 2];

					dec(_doo, 2);
				}

				lst_off = true;
				ok = false;
				return;
			}
		}
	}

	app = false;

	i = 1 - 1;

	if (zm.empty() == false)
	{
		skip_spaces(i, zm);

		if (not((zm[i] == ';' || zm[i] == '*') || ((zm[i] == '|') && (zm[i + 1] != '|')) || isComment))
			ok = true;

		str.erase();
		search_comment_block(i, zm, str);
	}

	// !!! lines with comments /* */ were treated as empty lines!!!

	if (not(macro) && not(rept) && not(run_macro) && not(rept_run))
	{
		if ((zm.empty()) || not(ok))
		{
			save_lst(' ');

			if (zm.empty() == false)
				justify();
			else
				SetLength(t, 6);

			put_lst(t + zm);

			ok = false;
		}
	}
}

/**
 * \brief reading an ASM file line by line
 * \param a 
 * \param old_str 
 */
void analyze_file(string& a, string& old_str)
{
	ifstream f;
	string filename;
	int nr, _odd, _doo;
	bool end_file, ok, do_exit, app, _app;
	string zm, appLine, _appLine;
	tByte v;

	if (not(TestFile(a)))
		show_error(a, MSG_CANT_OPEN_CREATE_FILE);

	f.open(a, std::ifstream::in);
	if (f.fail())
	{
		cerr << "open failure: " << a << endl;
		exit(1);
	}

	_odd = 0;
	_doo = 0;

	nr = 0;                        // line number in the assembled file
	global_name = a;               // file name for 'ERROR' function
	end_file = false;              // Did it encounter an 'END' command?
	do_exit = false;               // whether EXIT occurred in the macro

	app = false;
	appLine.erase();
	_app = false;
	_appLine.erase();

	// the name of the first file will always be written to LST
	// provided that (opt and opt_L>0), i.e. OPT L+ occurred
	if (opt & opt_L)
	{
		if ((pass == pass_end) && (GetFilePath(a).empty() == false))
		{
			//   writeln(lst,show_full_name(global_name,true,true))
			//  else
			lst_string = show_full_name(global_name, full_name, true);
		}
	}

	while (f.peek() != EOF || (_doo > 0))
	{
		ok = false;
		if (_doo > 0)
		{
			zm = t_lin[_odd];
			inc(_odd);
			if (_odd >= _doo)
			{
				_doo = 0;
				lst_off = false;
				SetLength(t_lin, 0);
			}

			get_line(zm, end_file, ok, _odd, _doo, _app, _appLine);

		}
		else
		{
			//char gotOneChar = f.peek();
			//if (gotOneChar == '\r')
			//	f.get();
			std::getline(f, zm);
			//readln(f, zm);

			if (f.peek() == '\r')
				f.get();

			if (length(zm) > 0xFFFF)
			{
				// maximum line length 65535 characters
				show_error(zm, ERROR_LINE_TOO_LONG);
				DoTheEnd(STATUS_STOP_ON_ERROR);
			}

			inc(nr);
			line = nr;
			inc(line_all);

			// DEBUG
			//cout << (int)pass <<':'<< line << " " << zm << endl;
			/*
			if (pass == 1 && zm == "	org *+duch_size*4")
			{
				int a = 0;
			}
			*/
			if (pass == 3 && line == 14)
			{
				int a = 0;
			}
			
			// DEBUG

			get_line(zm, end_file, ok, _odd, _doo, app, appLine);

			if (ok && !app && appLine.empty() == false)
			{
				save_lst(' ');
				justify();
				put_lst(t + zm);

				zm = appLine + zm;
				appLine.erase();
			}
		}

		if (macro)
		{                              // saving the .MACRO block
			v = dirMACRO(zm);

			if (v == __en)
				end_file = true;  // !!! for __end+1 it will stop when 'end \lda end'
		}
		else if (rept)
		{
			// writing a .REPT block

			if (ok && !isComment)
				v = dirREPT(zm);
			else
			{
				v = 0;
			}

			if ((v == __endr || v == __dend) && (rept_cnt == 0))
				dirENDR(zm, a, old_str, 0);
			else if (v == __end + 1 || v == __en)
				end_file = true;

		}
		else if (ok && !isComment)
			analyze_lines(zm, a, old_str, nr, end_file, do_exit);

		if (end_file)
			break;
	}

	if (appLine.empty() == false)
		analyze_lines(appLine, a, old_str, nr, end_file, do_exit);

	test_skipa();
	if (skip_use)
		show_error(zm, ERROR_CANT_SKIP_OVER_THIS);             // Can't skip over this

	oddaj_var();

	if (a == filenameASM)
		give_back_to();  // !!! necessarily here, otherwise after ICL with .DS it will generate a new EMPTY block

	if (pass == pass_end)
	{
		if ((a == filenameASM))
		{
			if (dotRELOC.use || dotRELOC.sdx)
			{
				save_lst('a');
				if (!bulkUpdatePublic.adr)
				{
					zm = load_mes(MSG_BLK_UPDATE) + load_mes(MSG_ADDRESS);
					blk_update_address(zm);
				}
				save_lst('a');
				if (not(bulkUpdatePublic.ext))
				{
					zm = load_mes(MSG_BLK_UPDATE) + load_mes(MSG_EXTERNAL);
					blk_update_external(zm);
				}
				save_lst('a');
				if (not(bulkUpdatePublic.pub))
				{
					zm = load_mes(MSG_BLK_UPDATE) + load_mes(MSG_PUBLIC);
					blk_update_public(zm);
				}
				save_lst('a');
				if (not(bulkUpdatePublic.symbol))
				{
					zm = load_mes(MSG_BLK_UPDATE) + load_mes(MSG_SYMBOL);
					blk_update_symbol(zm);
				}
				save_lst('a');

				give_sym();

			}// we force BLK UPDATE at the end of the main file
		}
	}

	test_exists(zm, do_exit);
	f.close();
}

/**
 * \brief reading and analysis of rows stored in the T_MAC dynamic array
 * P_MAX specifies the number of repetitions
 * \param start 
 * \param koniec 
 * \param old 
 * \param a 
 * \param old_str
 * \param licz
 * \param p_max
 * \param rp
 */
void analyze_mem(const int start, const int koniec, string& old, string& a, string& old_str, int licz, const int p_max, const bool rp)
{
	int licznik, nr, _odd, _doo, old_line_add;
	bool end_file, ok, do_exit, old_icl_used, app, _app;
	string zm, appLine, _appLine;
	tByte v;

	do_exit = false;

	old_icl_used = icl_used;
	icl_used = true;
	old_line_add = line_add;     // !!! be sure to remember LINE_ADD !!!
	line_add = 0;            // otherwise the error line number for e.g. "lda:cmp:rq" will be wrong

	_odd = 0;
	_doo = 0;

	while (licz < p_max)
	{
		if (rp || FOX_ripit)
			_rept_ile = licz;

		licznik = start;

		nr = old_line_add;             // line number in the assembled file
		end_file = false;              // Did it encounter an 'END' command?
		do_exit = false;               // whether EXIT occurred in the macro

		app = false;
		appLine.erase();
		_app = false;
		_appLine.erase();

		while ((licznik < koniec) || (_doo > 0))
		{

			ok = false;

			if (_doo > 0)
			{

				zm = t_lin[_odd];
				inc(_odd);
				if (_odd >= _doo)
				{
					_doo = 0;
					lst_off = false;
					SetLength(t_lin, 0);
				}

				get_line(zm, end_file, ok, _odd, _doo, _app, _appLine);

			}
			else
			{
				zm = t_mac[licznik];

				reptLine(zm);       // we substitute the parameters in the .REPT block

				inc(licznik);

				inc(nr);
				line = nr;

				inc(line_all);

				get_line(zm, end_file, ok, _odd, _doo, app, appLine);

				if (ok && !app && appLine.empty() == false)
				{
					save_lst(' ');
					justify();
					put_lst(t + zm);

					zm = appLine + zm;
					appLine.erase();
				}

			}


			if (rept)
			{                   // writing a .REPT block

				if (ok && !isComment)
					v = dirREPT(zm);
				else
					v = 0;

				if ((v == __endr || v == __dend) && (rept_cnt == 0))
					dirENDR(zm, a, old_str, 0);
				else if (v == __end + 1 || v == __en)
					end_file = true;

			}
			else

				if (ok && !end_file && !isComment)
					analyze_lines(zm, a, old_str, nr, end_file, do_exit);


			if (do_exit) break;          // Macro processing ends with the .EXIT directive

		}


		if (!appLine.empty()) 
			analyze_lines(appLine, a, old_str, nr, end_file, do_exit);

		inc(licz);
	}

	test_exists(old, do_exit);

	icl_used = old_icl_used;
	line_add = old_line_add;
}

/**
 * \brief assembly of the main *.ASM file
 * \param a 
 */
void RunAssembler(string& a)
{
	int i;
	string s;

	while (pass <= pass_end)
	{
		// maximum PASS_MAX mileage
		line_all = 0;

		s.erase();
		analyze_file(a, s);

		if (list_mac)
		{
			s = " icl \"" + filenameMAC + "\"";
			analyze_file(filenameMAC, s);
		}

		t_hea[hea_i] = addres - 1;           // end of program, last entry

		inc(pass);

		// if NEXT_PASS = TRUE then increase the number of passes
		if ((pass >= pass_end) || (pass < 1))
		{
			if (next_pass)
			{
				inc(pass_end);
			}
		}


		// !!! resetting the macro call number !!!
		for (i = static_cast<int>(t_lab.size()) - 1; i >= 0; --i)
		{
			if (t_lab[i].bank == __id_macro)
				t_mac[t_lab[i].addr + 3] = '0';
		}

		label_type = 'V';

		zpvar = 0x80;

		addres = -0xFFFF;
		raw.old = -0xFFFF;

		t_seg[0].addr = -0xffff;

		hea_offset.old = 0;
		hea_offset.addr = -1;
		structure.cnt = -1;
		_rept_ile = -1;

		registerOptimisation.reg[0] = -1;
		registerOptimisation.reg[1] = -1;
		registerOptimisation.reg[2] = -1;

		for (i = 0; i < sizeof(t_zpv); ++i)
		{
			t_zpv[i] = false;
		}
		array_used.max = 0;

		hea_i = 0;
		bank = 0;
		ifelse = 0;
		blok = 0;
		rel_ofs = 0;
		org_ofset = 0;
		proc_idx = 0;
		proc_nr = 0;
		local_nr = 0;
		lc_nr = 0;
		::fill = 0;
		line_add = 0;
		structure.id = 0;
		wyw_idx = 0;
		rel_idx = 0;
		ext_idx = 0;
		extn_idx = 0;
		smb_idx = 0;
		sym_idx = 0;
		skip_idx = 0;
		pag_idx = 0;
		end_idx = 0;
		pub_idx = 0;
		usi_idx = 0;
		segment = 0;
		whi_idx = 0;
		while_nr = 0;
		ora_nr = 0;
		test_nr = 0;
		proc_lokal = 0;
		test_idx = 0;
		var_idx = 0;
		var_id = 0;
		rept_cnt = 0;
		anonymous_idx = 0;

		__link_stack_pointer = adr_label(__STACK_POINTER, false);       // @STACK_POINTER
		__link_stack_address = adr_label(__STACK_ADDRESS, false);       // @STACK_ADDRESS
		__link_proc_vars_adr = adr_label(__PROC_VARS_ADR, false);       // @PROC_VARS_ADR

		size_idx = 1;
		array_idx = 1;         // first entry reserved for .PUT

		opt = optDefault;
		atr = atrDefault;

		hea = true;
		first_org = true;
		if_test = true;
		TestWhileOpt = true;
		ExProcTest = true;

		registerOptimisation.used = false;
		registerOptimisation.blocked = false;
		mae_labels = false;
		loop_used = false;
		macro = false;
		proc = false;
		regAXY_opty = false;
		undeclared = false;
		xasmStyle = false;
		icl_used = false;
		bez_lst = false;
		::empty = false;
		enumeration.use = false;
		reloc = false;
		branch = false;
		isVector = false;
		rept = false;
		rept_run = false;
		structure.use = false;
		dta_used = false;
		code6502 = false;
		struct_used.use = false;
		aray = false;
		next_pass = false;
		mne_used = false;
		skip_use = false;
		skip_xsm = false;
		FOX_ripit = false;
		put_used = false;
		ext_used.use = false;
		dotRELOC.use = false;
		dotRELOC.sdx = false;
		rel_used = false;
		blocked = false;
		dotLINK.checkedStack = false;
		block_record = false;
		overflow = false;
		test_symbols = false;
		lst_off = false;
		noWarning = false;
		raw.use = false;
		variable = false;

		isComment = false;
		org = false;
		runini.used = false;

		local_name.erase();
		macro_nr.erase();
		while_name.erase();
		test_name.erase();

		warning_old.erase();

		SetLength(t_lin, 1);              // We can save T_LIN again

		if (binary_file.use)
		{
			addres = binary_file.addr;
			org = true;
			first_org = false;
		}

	}

	if (pass > pass_max)
		warning(WARN_INFINITE_LOOP_AT_LABEL);

	// if no error occurred and we have 16 passes, there has definitely been an endless loop
}

/**
 * \brief Display information about switches and MADS configuration
 */
void Syntax(void)
{
	hue::set(hue::WHITE);
	cout << Tab2Space(load_mes(MSG_MADS_VERSION)) << " (" << string(__DATE__) << ")" << '\n';
	hue::set(hue::DARKGRAY);
	cout << Tab2Space(load_mes(MSG_SYNTAX)) << endl;
	hue::reset();
	exit(3);
}

/**
 * \brief 
 * \param nam 
 * \param ext 
 * \return 
 */
string NewFileExt(string &nam, const string& ext)
{
	string work = nam.c_str();
	if (work.find('.') != string::npos)
	{
		cut_dot(work);
		return work + ext;
	}
	else
	{
		return work + '.' + ext;
	}
}

string NewFile(const string &a, int &i)
{
	path = "";
	string result = a;

	i += 2;	// we omit the switch character and the ':' character

	name = t.substr(i, t.length());

	if (!GetFilePath(name).empty())
		path = GetFilePath(name);

	if (!name.empty())
		result = path + GetFileName(name);

	i += static_cast<int>(name.length());

	return result;
}

void setup()
{
	t_lin.resize(1);
	t_lab.resize(1);
	::messages.resize(1);

	t_lin.resize(1);
	t_lab.resize(1);
	t_hea.resize(1);
	t_mac.resize(1);
	t_par.resize(1);
	t_loc.resize(1);
	t_prc.resize(1);
	t_pth.resize(1);
	t_wyw.resize(1);
	t_rel.resize(1);
	t_smb.resize(1);
	t_ext.resize(1);
	t_extn.resize(1);
	t_str.resize(1);
	t_pag.resize(1);
	t_end.resize(1);
	t_mad.resize(1);
	t_pub.resize(1);
	t_var.resize(1);
	t_seg.resize(2);
	t_skp.resize(1);
	t_sym.resize(1);
	t_usi.resize(1);
	t_els.resize(1);
	t_rep.resize(1);
	if_stos.resize(1);

	t_siz.resize(2);

	::messages.resize(1);

	t_arr.resize(2);	// predefined array parameters for .PUT
	t_arr[0].elm.resize(1);
	t_arr[0].addr = 0;
	t_arr[0].bank = 0;
	t_arr[0].elm[0].count = 0xFFFF + 1;
	t_arr[0].elm[0].mult = 1;
	t_arr[0].siz = 1;

	array_idx = 1;

	pass = 1;

	binary_file.use = false;
	filenameASM = "";
	status = STATUS_OK;
}


void initialize(int argc, char* argv[])
{
	for (int x = 0; x < 256; ++x)
	{
		unsigned int crc = x << 8;
		long long crc_ = crc;

		for (int y = 1; y <= 8; ++y)
		{
			crc = crc << 1;
			crc_ = crc_ >> 1;

			if ((crc & 0x00010000) > 0)
				crc = crc ^ 0x1021;
			if ((crc_ & 0x80) > 0)
				crc_ = crc_ ^ 0xedb8832000;
		}

		tCRC16[x] = int(crc);
		tCRC32[x] = (unsigned int)(crc_ >> 8);
	}

	//string src = "LDA";
	//const int i = src[0] - '@' + ((src[1] - '@') << 5) + ((src[2] - '@') << 10);

	for (int IORes = static_cast<int>(_hash.size()) - 1; IORes >= 0; --IORes)
	{
		::hash[_hash[IORes].o] = _hash[IORes].v;
	}

	// DEBUG
	/*
	for (int IORes = 0; IORes < _hash.size(); ++ IORes)
	{
		// 0-4 5-9 10-14
		char a = (_hash[IORes].o & 0x1f) + '@';
		char b = ((_hash[IORes].o >> 5) & 0x1f) + '@';
		char c = ((_hash[IORes].o >> 10) & 0x1f) + '@';

		cout << std::dec << IORes << ": " << a << b << c << ' ' << (int)_hash[IORes].o << ' ' << (int)_hash[IORes].v << ' ';
		cout << std::setfill('0') << std::setw(2)  << std::hex << (0xFF & _hash[IORes].v) << endl;
	}
	*/

	// parameter processing, variable initialization
	filenameASM.clear();

	if (argc < 2) // 1 = exe, 2 = first argument
		Syntax();

	// Parameter read
	for (int IORes = 1; IORes < argc; ++IORes)
	{
		t = argv[IORes];

		if (!t.empty())
		{
			if (t[0] != '-')
			{
				// for compatibility with Linux, Mac OS X
				if (!filenameASM.empty())
					Syntax();
				else
					filenameASM = t;
			}
			else
			{
				int _i = 2-1;
				while(_i < t.length())
				{
					switch(::toupper(t[_i]))
					{
						case 'C': case_used = true; break;
						case 'P': full_name = true; break;
						case 'S': silent = true; break;
						case 'X': exclude_proc = true; break;
						case 'U': unused_label = true; break;
						case 'V':
						{
							if (::toupper(t[_i + 1]) == 'U')
							{
								_i += 2;
								VerifyProc = true;
							}
							break;
						}
						case 'B':
						{
							if (::toupper(t[_i + 1]) == 'C')
							{
								if (t.size() != 3)
									Syntax();

								BranchTest = true;
								_i = 2;
							}
							else
							{
								if (t[_i + 1] != ':')
									Syntax();
								else
								{
									_i += 2;
									string tmp = get_dat(_i, t, ' ', true);
									binary_file.addr = static_cast<int>(calculate_value(tmp, t));
									binary_file.use = true;
								}
							}
							break;
						}
						case 'D':
						{
							if (t[_i + 1] != ':')
								Syntax();
							else
							{
								_i += 2;
								def_label = get_lab(_i, t, true);

								if (t[_i] != '=')
									nul.i = 1;
								else
								{
									++_i;
									nul.i = static_cast<int>(get_expres(_i, t, t, false));
								}
								s_lab(def_label, nul.i, bank, t, def_label[0]);
							}
							break;
						}
						case 'F':
						{
							// -f			CPU command at first column
							// -fv:value	Set raw binary fill byte to [value]
							++_i;
							if (t.length() == 2)
								labFirstCol = true;
							else
							{
								switch (::toupper(t[_i]))
								{
									case 'V':
									{
										if (t[_i + 1] != ':')
											Syntax();
										else
										{
											_i += 2;
											string tmp = get_dat(_i, t, ' ', true);
											obxFillValue = (BYTE)calculate_value(tmp, t);
										}
										break;
									}
									default:
										Syntax();
									break;
								}
							}
							break;
						}
						case 'H':
						{
							// -hc[:filename]  Header file for CC65
							// -hm[:filename]  Header file for MADS
							++_i;
							switch (::toupper(t[_i]))
							{
								case 'C':
								{
									list_dotH = true;
									if (t[_i + 1] == ':')
										filenameH = NewFile(filenameH, _i);
									break;
								}
								case 'M':
								{
									list_dotMEA = true;
									if (t[_i + 1] == ':')
										filenameHEA = NewFile(filenameHEA, _i);
									break;
								}
								default: 
									Syntax();
							}
							break;
						}
						case 'I':
						{
							// -i:path         Additional include directories
							if (t[_i + 1] != ':')
								Syntax();
							else
							{
								name = NewFile(name, _i);

								if (name.empty())
									Syntax();
								else
								{
									NormalizePath(name);
									t_pth[t_pth.size() - 1] = name;
									t_pth.resize(t_pth.size() + 1);
								}
							}

							break;
						}
						case 'L':
						{
							// -l[:filename]   Generate listing
							opt = opt | opt_L;
							if (t[_i + 1] == ':')
								filenameLST = NewFile(filenameLST, _i);
							break;
						}
						case 'M':
						{
							// -m:filename     File with macro definition
							// -ml:value       margin - left property
							++_i;
							switch (::toupper(t[_i]))
							{
								case ':':
								{
									--_i;
									list_mac = true;
									filenameMAC = NewFile(filenameMAC, _i);
									break;
								}
								case 'L':
								{
									if (t[_i + 1] != ':')
										Syntax();
									else
									{
										_i += 2;
										string tmp = get_dat(_i, t, ' ', true);
										margin = (int)calculate_value(tmp, t);

										if (margin < 32) margin = 32;
										if (margin > 127) margin = 128;
									}
									break;
								}
								default:
									Syntax();
							}
							break;
						}
						case 'O':
						{
							// -o:filename     Set object file name
							if (t[_i + 1] != ':')
								Syntax();
							else
								filenameOBX = NewFile(filenameOBX, _i);
							break;
						}
						case 'T':
						{
							// -t[:filename]   List label table
							list_lab = true;
							if (t[_i + 1] == ':')
								filenameLAB = NewFile(filenameLAB, _i);
							break;
						}
						default:
						{
							Syntax();
						}
					}
					++_i;
				}
			}
		}
	}

	if (filenameASM.empty() || GetFileName(filenameASM).empty())
		Syntax();

	NormalizePath(filenameASM);

	path = GetFilePath(filenameASM);
	if (path.empty())
	{
		path = std::filesystem::current_path().string(); // Get current working folder
		path = path + PathDelim;
	}

	NormalizePath(path);

	name = GetFileName(filenameASM);
	global_name = name;

	// we check the presence of the ASM file
	filenameASM = path + name;
	if (!TestFile(filenameASM)) 
		filenameASM = path + NewFileExt(name, "asm");

	// Set the default .lst, .obx, .lab, .mac, .h and .hea
	if (filenameLST.empty()) filenameLST = path + NewFileExt(name, "lst");
	if (filenameOBX.empty()) filenameOBX = path + NewFileExt(name, "obx");
	if (filenameLAB.empty()) filenameLAB = path + NewFileExt(name, "lab");
	if (filenameMAC.empty()) filenameMAC = path + NewFileExt(name, "mac");
	if (filenameH.empty()) filenameH = path + NewFileExt(name, "h");
	if (filenameHEA.empty()) filenameHEA = path + NewFileExt(name, "hea");

	t = load_mes(MSG_MADS_VERSION);
	// if not(silent) then new_message(t);

	if (list_lab)
	{
		dotLabFile.open(filenameLAB, ofstream::trunc);
		if (dotLabFile.fail())
		{
			cerr << "Failed to open '" << filenameLAB << "' .LAB file for output: " << strerror(errno) << endl;
			exit(1);
		}
		dotLabFile << t << '\n';
		dotLabFile << load_mes(MSG_LABEL_TABLE) << '\n';
	}

	name = GetFileName(filenameASM);
	int _i = 1-1;
	name = get_datUp(_i, name, '.', false);

	if (list_dotMEA)
	{
		dotMEAFile.open(filenameHEA, ofstream::trunc);
		if (dotMEAFile.fail())
		{
			cerr << "Failed to open '" << filenameHEA << "' .MEA file for output: " << strerror(errno) << endl;
			exit(1);
		}
	}

	if (list_dotH)
	{
		dotHFile.open(filenameH, ofstream::trunc);
		if (dotHFile.fail())
		{
			cerr << "Failed to open '" << filenameH << "' .H file for output: " << strerror(errno) << endl;
			exit(1);
		}

		dotHFile << "#ifndef _" << name << "_ASM_H_" << '\n';
		dotHFile << "#define _" << name << "_ASM_H_" << '\n\n';
	}

	dotObjectFile.open(filenameOBX, ofstream::binary | ofstream::trunc);
	if (dotObjectFile.fail())
	{
		cerr << "Failed to open '" << filenameOBX << "' file for output: " << strerror(errno) << endl;
		exit(1);
	}

	lst_header = t;            // header with information about the mads version to the LST file
}

int main(int argc, char*argv[])
{
	setup();

	// creating 'TCRC16', 'TCRC32' and 'HASH' tables, reading parameters
	initialize(argc, argv);

	if (binary_file.use)
	{
		opt = opt | opt_O;
		addres = binary_file.addr;
		org = true;
		first_org = false;
	}

	optDefault = opt;
	atrDefault = atr;

	open_ok = true;

	t.clear();

	nul.l = 0;
	nul.i = 0;

	pass = 0;

	RunAssembler(filenameASM);

	over = true;
	DoTheEnd(status);
	return status;
}
