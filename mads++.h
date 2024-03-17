// mads++.h : Include file for standard system include files,
// or project specific include files.

#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

#include <charconv>
#include <cstdarg>

#include <algorithm>
#include <list>
#include <map>
#include <string>
#include <type_traits>
#include <utility>
#include <windows.h>

#include "set.h"
//#include "color.hpp"	// https://github.com/aafulei/color-console
#include "messages.h"
#include "myArray.h"
#include "opcodes.h"

using namespace std;

typedef unsigned char BYTE;
#define byte static_cast<BYTE>
#define tByte unsigned char

typedef int           integer;
typedef float         real;

#define cardinal static_cast<unsigned int>
#define tCardinal unsigned int

#define integer static_cast<int>
#define tInteger int

#define word static_cast<unsigned short>
#define tWord unsigned short

#define Int64 static_cast<long long>
#define tInt64 long long

#define ord(x) (x)
#define chr(x) (static_cast<BYTE>(x))
#define UpCase(x) ::toupper(x)

#define lo(x) (BYTE)(x & 255)


#include "hashtables.h"

//typedef unsigned char BYTE;

enum t_Dirop
{
	_unknown, _r = 1, _or, _lo, _hi, _get, _wget, _lget, _dget, _and, _xor, _not,
	_len, _adr, _def, _filesize, _sizeof, _zpvar, _rnd, _asize, _isize,
	_fileexists, _array
};

enum t_Mads
{
	__STACK_POINTER,
	__STACK_ADDRESS,
	__PROC_VARS_ADR
};

enum t_MXinst
{
	REP = 0xc2,
	SEP = 0xe2
};


enum t_Attrib {
	__U,
	__R,
	__W,
	__RW
};


typedef struct _typStrREG
{
	char& operator[](const int i) { return x[i]; }
	char x[4];
} _typStrREG;


typedef struct _typStrSMB
{
	char& operator[](const int i) { return x[i]; }
	char x[8];
} _typStrSMB;

typedef vector<string> _strArray;
typedef vector<int> _intArray;
typedef vector<bool> _bolArray;

typedef struct _bckAdr
{
	int& operator[](const int i) { return x[i]; }
	int x[2];
} _bckAdr;

typedef struct t256i
{
	int& operator[](const int i) { return x[i]; }
	int x[256];
} t256i;

typedef struct t256c
{
	unsigned int& operator[](const int i) { return x[i]; }
	unsigned int x[256];
} t256c;

typedef struct t256b
{
	bool& operator[](const int i) { return x[i]; }
	bool x[256];
} t256b;

typedef struct t256byt
{
	BYTE& operator[](const int i) { return x[i]; }
	BYTE x[256];
} t256byt;

typedef struct m4kb
{
	unsigned char& operator[](const int i) { return x[i]; }
	unsigned char x[4096];
} m4kb;

typedef struct c64kb
{
	unsigned int& operator[](const int i) { return x[i]; }
	unsigned int x[0x10000];
} c64kb;

typedef struct m64kb
{
	unsigned char& operator[](const int i) { return x[i]; }
	unsigned char x[0x10000];
} m64kb;

typedef struct labels
{
	unsigned int	name;	// label identifier (CRC32)
	unsigned int	addr;	// label value
	int				offset;	// label value before changing the assembly address
	int				len;	// label length in BYTEs <1..65535>
	int				block;	// block assigned to the label
	int				lln;	// length of the .LOCAL block
	bool			lid;	// label ID .LOCAL
	unsigned int	add;	// if there are more blocks with the same name
	int				bank;	// virtual bank counter assigned to the <0..$FF> label, for MADS purposes it is of the WORD type
	bool			sts;	// whether the label definition was successful
	bool			rel;	// or relocate
	unsigned char	pass;	// run number for a given label
	char			theType;	// label type (V-ARIABLE, P-ROCEDURE, C-ONSTANT)
	unsigned char	lop;
	bool			use;	// whether the label is used
	t_Attrib		attrib;	// attribute __R, __W, __RW
} labels;

class triggered
{
public:
	triggered()
	{
		nr = 0;
	}
	string			zm;		// listing line
	string			pl;		// the name of the file whose contents we are assembling
	int				nr;		// listing line number
};

typedef struct if_else_endif_stack_item
{
	bool			_if_test;	// IF_TEST state
	bool			_else;		// whether .ELSE occurred
	int				_okelse;	// current .IF level number
	bool			old_iftest;	// IF_TEST state before executing a new .IF
} if_else_endif_stack_item;

typedef struct relocatableLabel
{
	int				addr;		// relocatable address
	int				idx;		// index to T_SMB ​​or T_SIZ
	int				blk;		// segment number
	int				block;		// block number
	int				bank;		// bank
} relocatableLabel;

class segInfo
{
public:
	segInfo()
	{
		start = 0;
		len = 0;
		addr = 0;
		bank = 0;
		pass = 0;
		attrib = __U;
	}
	string			name;		// segment label
	int				start;		// segment starting address
	int				len;		// segment length
	int				addr;		// current assembly address of the segment
	int				bank;		// segment bank
	BYTE			pass;		// mileage number
	t_Attrib		attrib;		// attribute
};

class relocSmb
{
public:
	relocSmb()
	{
		weak = false;
		used = false;
	}
	string			name;		// 8-character SMB symbol for the SDX system
	bool			used;		// whether a symbol was referenced in the program
	bool			weak;		// whether the symbol is weak (Weak Symbol)
};

typedef struct externalLabel
{
	unsigned int	addr;		// the address where the external label appeared
	int				bank;		// bank where the external label appears
	int				idx;		// index to T_EXT
	char			typ;		// operation type ' ', '<', '>', '^'
	BYTE			lsb;
} externalLabel;

class usingLabels
{
public:
	usingLabels()
	{
		location = 0;
	}
	int				location;	// local area number
	string			name;		// name of the area in which .USING occurred
	string			myLabel;	// label
};

class pubTab
{
public:
	pubTab()
	{
		isVariable = false;
	}
	string			name;		// public label name
	bool			isVariable;	// current assembly address 1 = variable, 0 = constant
};

class messageEntry
{
public:
	messageEntry()
	{
		pass = 0;
		color = 0;
	}
	BYTE			pass;		// mileage number
	BYTE			color;		// color
	string			message;	// content of the error message
};

class localAreasEntry
{
public:
	localAreasEntry()
	{
		addr = 0;
		idx = 0;
		offset = 0;
	}
	string			name;		// block name .LOCAL
	int				addr;		// current assembly address
	int				idx;		// index to the T_LAB array
	int				offset;		// previous assembly address
};

typedef struct arrayElement
{
	int				count;		// number of elements
	int				mult;		// element multiplier on the right
} arrayElement;

class arrayEntry
{
public:
	arrayEntry()
	{
		addr = 0;
		bank = 0;
		isDefined = false;
		siz = 0;
		len = 0;
		offset = 0;
	}
	unsigned int	addr;     // array address
	int				bank;      // bank assigned to the board
	//vector<arrayElement> elm; // consecutive numbers of array elements [0..IDX]
	//conf_array<arrayElement> elm;
	myArray			elm;
	bool			isDefined;      // whether the number of elements has been specified
	BYTE			siz;         // size of the table field B-YTE, W-ORD, L-ONG, D-WORD
	int				len;      // total length of the array in BYTEs
	int				offset;
};

class variableEntry
{
public:
	variableEntry()
	{
		lok = 0;
		mySize = 0;
		excludeProc = false;
		cnt = 0;
		initialValue = 0;
		addr = 0;
		theType = 0;
		idx = 0;
		id = 0;
		inZeroPage = false;
	}
	int				lok;				// local level no
	string			name;				// variable name
	int				mySize;				// variable size
	bool			excludeProc;		// exclude procedure
	int				cnt;				// a multiple of the variable's size
	unsigned int	initialValue;		// initial value of the variable
	int				addr;				// variable address if specified
	char			theType;			// variable type V-AR, S-TRUCT, E-NUM
	int				idx;				// index to the structure creating the variable
	string			structName;			// the name of the structure creating the variable
	int				id;					// identifier of the group of labels declared by .VAR in the same block
	bool			inZeroPage;			// ZPV = TRUE for variables declared by .ZPVAR
};

class structureEntry
{
public:
	structureEntry()
	{
		id = 0;
		no = 0;
		addr = 0;
		bank = 0;
		idx = 0;
		offset = 0;
		mySize = 0;
		nrRepeat = 0;
	}
	int				id;     			// structure number (identifier)
	int				no;     			// item number in the structure (-1 if these are not structure fields)
	unsigned long	addr;    			// structure address
	int				bank;     			// bank assigned to the structure
	int				idx;     			// index to an additional structure
	int				offset;     		// offset from the beginning of the structure
	int				mySize;     		// size of data defined by structure field 1..4 (B-YTE..D-WORD)
										// or the total length of the data defined by the structure
	int				nrRepeat;			// number of SIZE type repetitions (SIZ*RPT = total field size)
	string			labelName;			// label appearing in .STRUCT
};

typedef struct reptEntry
{
	int				idx;
	int				firstLineNr;		// first line
	int				lastLineNr;			// last line  ->  t_mac[fln..lln]
	int				lineNr;				// line number
} reptEntry;

class procedureEntry
{
public:
	procedureEntry()
	{
		str = 0;
		addr = 0;
		offset = 0;
		bank = 0;
		nrParams = 0;
		paramMemSize = 0;
		myType = 0;
		reg = 0;
		used = 0;
		len = 0;
		procNr = 0;
		inProcedure = false;
		origOffset = 0;
	}
	string			name;				// procedure name .PROC
	int				str;				// index to T_MAC with parameter names
	unsigned int	addr;				// procedure address
	int				offset;
	int				bank;				// bank assigned to the procedure
	int				nrParams;			// number of procedure parameters
	int				paramMemSize;		// number of BYTEs occupied by procedure parameters
	char			myType;				// procedure type __pDef, __pReg, __pVar
	BYTE			reg;				// CPU register order
	bool			used;				// whether the procedure was used in the program or omit it during assembly
	unsigned int	len;				// procedure length in BYTEs
	int				procNr;				// proc_nr
	bool			inProcedure;		// Already in a procedure [TRUE, FALSE]
	int				origOffset;			// original offset
	string			savedProcName;		// proc_name
};

typedef struct pageEntry
{
	int				addr;				// initial memory page
	int				cnt;				// number of memory pages
} pageEntry;

typedef struct argumentSizeEntry
{
	char			mySize;
	BYTE			lsb;
} argumentSizeEntry;

typedef struct endEntry
{
	BYTE			endCode;			// block end code .END?? (which end was found)
	int				addr;
	int				restoreAddr;
	bool			withSemicolon;		// whether the { character occurred
} endEntry;

class extLabelDeclEntry
{
public:
	extLabelDeclEntry()
	{
		mySize = 0;
		isProcedure = false;
	}
	string			name;				// external label name
	char			mySize;				// declared label size from 1..4 BYTEs
	bool			isProcedure;		// whether the external label is a procedure declaration
};

class madsHeaderEntry
{
public:
	madsHeaderEntry()
	{
		addr = 0;
		bank = 0;
		myType = 0;
	}
	string			name;				// label name
	unsigned int	addr;				// address assigned to the label
	BYTE			bank;				// bank assigned to the label
	char			myType;				// label type (P-procedure, V-ariable, C-onstants)
};

// new type for CALCULATE_MNEMONIC
typedef struct int5
{
	BYTE			l;					// the number of BYTEs constituting a CPU instruction
	t256byt			h;					// command arguments
	int				i;					// offset to arrays
	BYTE			tmp;				// auxiliary BYTE
} int5;

typedef struct relocateValue
{
	bool			use;
	int				count;
} relocateValue;

typedef struct forwardEntry
{
	int				addr;				// address
	bool			use;				// whether a pseudo-jump command has been triggered
	BYTE			count;				// number of forward addresses
} forwardEntry;

typedef struct infiniteEntry
{
	string			lab;
	string			name;
	int				lineNr;
} infiniteEntry;

typedef struct runIniEntry
{
	int				addr;
	bool			used;
} runIniEntry;

typedef struct attributeEntry
{
	string			name;
	t_Attrib		attrib;
} attributeEntry;

typedef struct regOptiConfig
{
	bool			blocked;
	bool			used;
	int				reg[3];
} regOptiConfig;

typedef struct structureConfig
{
	bool			use, drelocUSE, drelocSDX;
	int				idx, id, addres, cnt;
} structureConfig;

typedef struct tStructUsed
{
	bool			use;		// whether a data structure has been created
	int				idx;		// index to the founder structure
	int				count;		// structure position counter
} tStructUsed;

typedef struct tArrayUsed
{
	int				idx;
	int				max;     // maximal index based on the entered data
	char			typ;
} tArrayUsed;

typedef struct tExtUsed
{
	bool			use;
	int				idx;
	char			mySize;
} tExtUsed;

typedef struct tDotReloc
{
	bool			use;	// whether there was a .RELOC directive
	bool			sdx;	// whether there was a pseudo-order BLK SPARTA
	char			mySize;	// label size
} tDotReloc;

typedef struct tDotLink
{
	bool			use;			// whether we link the block with the landing address $0000
	bool			checkedStack;	// did we check the stack addresses
	int				linkedLength;	// length of the linked block
	int				emptyLength;	// length of the empty block
} tDotLink;

typedef struct tBulkUpdatePublic
{
	bool			adr;     // whether it occurred BLK UPDATE ADDRESS
	bool			ext;     // whether it occurred BLK UPDATE EXTERNAL
	bool			pub;     // whether it occurred BLK UPDATE PUBLIC
	bool			symbol;			// whether it occurred BLK UPDATE SYMBOL
	bool			newSymbol;		// whether it occurred BLK UPDATE NEW SYMBOL
} tBulkUpdatePublic;

typedef struct tBinaryFile
{
	bool			use;
	int				addr;
} tBinaryFile;

typedef struct tRaw
{
	bool			use;
	int				old;
} tRaw;

typedef struct tHeaOffset
{
	int				addr;
	int				old;
} tHeaOffset;

typedef struct tEnumeration
{
	bool			use;
	bool			drelocUSE;
	bool			drelocSDX;
	int				val;
	int				max;
} tEnumeration;

typedef struct tMadsStackEntry
{
	string			name;
	unsigned int	addr;
} tMadsStackEntry;

typedef struct signValueEntry
{
	char znak;
	long long wart;
} signValueEntry;

typedef struct t_ads
{

	BYTE kod;
	tCardinal ads;
} t_ads;


constexpr int opt_H = 1;
constexpr int opt_O = 2;
constexpr int opt_L = 4;
constexpr int opt_S = 8;
constexpr int opt_C = 16;
constexpr int opt_M = 32;
constexpr int opt_T = 64;
constexpr int opt_B = 128;

constexpr BYTE STATUS_OK = 0;
constexpr BYTE STATUS_ONLY_WARNINGS = 1;
constexpr BYTE STATUS_STOP_ON_ERROR = 2;
