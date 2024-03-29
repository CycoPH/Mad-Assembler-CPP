#include "messages.h"

std::vector<std::string> mes = {
	/* 0 */		"Value out of range",
	/* 1 */		"Missing .ENDIF",
	/* 2 */		"Label \t declared twice",
	/* 3 */		"String error",
	/* 4 */		"Extra characters on line",
	/* 5 */		"Undeclared label \t",
	/* 6 */		"No matching bracket",
	/* 7 */		"Need parenthesis",
	/* 8 */		"Illegal character: \t",
	/* 9 */		"Reserved word \t",
	/* 10 */	"No ORG specified",
	/* 11 */	"CPU doesn't have so many registers",
	/* 12 */	"Illegal instruction ",
	/* 13 */	"Value out of range",
	/* 14 */	"Illegal addressing mode (CPU 65",
	/* 15 */	"Label name required",
	/* 16 */	"Invalid option",
	/* 17 */	"Missing .END",
	/* 18 */	"Cannot open or create file '\t'",
	/* 19 */	"Nested op-codes not supported",
	/* 20 */	"Missing '}'",
	/* 21 */	"Branch out of range by $",
	/* 22 */	" bytes",
	/* 23 */	"Unexpected end of line",
	/* 24 */	"File is too short",
	/* 25 */	"File is too long",
	/* 26 */	"Divide by zero",
	/* 27 */	"^ not relocatable",
	/* 28 */	"Missing .LOCAL",
	/* 29 */	"Missing .ENDL",
	/* 30 */	"User error",
	/* 31 */	"Operand overflow",
	/* 32 */	"Bad size specifier",
	/* 33 */	"Size specifier not required",
	/* 34 */	" ", // !!! reserved !!!  USER ERROR
	/* 35 */	"Undeclared macro \t",
	/* 36 */	"Can't repeat this directive",
	/* 37 */	"Missing .IF",
	/* 38 */	"Label not required",
	/* 39 */	"Missing .PROC",
	/* 40 */	"Improper number of actual parameters",
	/* 41 */	"Incompatible types \t",
	/* 42 */	".ENDIF expected",
	/* 43 */	"Missing .ENDR",
	/* 44 */	"SMB label too long",
	/* 45 */	"Too many blocks",
	/* 46 */	"Bad parameter type \t",
	/* 47 */	"Bad parameter number",
	/* 48 */	" lines of source assembled",
	/* 49 */	" bytes written to the object file\r\n",
	/* 50 */	"Missing type label",
	/* 51 */	"Missing .REPT",
	/* 52 */	"Bad or missing sinus parameter",
	/* 53 */	"Only RELOC block",
	/* 54 */	"Label table:",
	/* 55 */	"Missing .STRUCT",
	/* 56 */	"Missing .ENDS",
	/* 57 */	"Can not use recursive structures",
	/* 58 */	"Improper syntax",
	/* 59 */	"Missing .ARRAY",
	/* 60 */	"Missing .ENDA",
	/* 61 */	"CPU doesn't have register \t",
	/* 62 */	"Constant expression violates subrange bounds",
	/* 63 */	"Bad register size",
	/* 64 */	"Missing .PAGES",
	/* 65 */	"Missing .ENDPG",
	/* 66 */	"Infinite recursion",
	/* 67 */	"Default addressing mode",
	/* 68 */	"Unknown directive \t",
	/* 69 */	"Unreferenced procedure ",
	/* 70 */	"Page error at ",
	/* 71 */	"Illegal instruction at RELOC block",
	/* 72 */	"Unreferenced directive .END",
	/* 73 */	"Undefined symbol \t",
	/* 74 */	"Incorrect header for this file type",
	/* 75 */	"Incompatible stack parameters",
	/* 76 */	"Zero page RELOC block",
	/* 77 */	" (BANK=", // from 77 the order of occurrence is important
	/* 78 */	" (BLOK=",
	/* 79 */	"Could not use \t in this context",
	/* 80 */	") ERROR: ",
	/* 81 */	"02)",
	/* 82 */	"816)",
	/* 83 */	"ORG specified at RELOC block",
	/* 84 */	"Can't skip over this",
	/* 85 */	"Address relocation overload",
	/* 86 */	"Not relocatable",
	/* 87 */	"Variable address out of range",
	/* 88 */	"Missing #WHILE",
	/* 89 */	"Missing .ENDW",
	/* 90 */	"BLK UPDATE ",
	/* 91 */	"ADDRESS",
	/* 92 */	"EXTERNAL",
	/* 93 */	"PUBLIC",
	/* 94 */	"SYMBOL",
	/* 95 */	"Missing #IF",
	/* 96 */	"Missing .ENDT",
	/* 97 */	"Missing .MACRO",
	/* 98 */	"Skipping only the first instruction",
	/* 99 */	"Repeating only the last instruction",
	/* 100 */	"Only SDX RELOC block",
	/* 101 */	"Line too long",
	/* 102 */	"Constant expression expected",
	/* 103 */	"Can not declare label \t as public",
	/* 104 */	"Segment \t error at $",
	/* 105 */	"Writing listing file...",
	/* 106 */	"Writing object file...",
	/* 107 */	"Use square brackets instead",
	/* 108 */	"Can't fill from higher ($\t) to lower memory location ($\t)",
	/* 109 */	"Access violations at address ",
	/* 110 */	"No instruction to repeat",
	/* 111 */	"Illegal when Atari file headers disabled",
	/* 112 */	"The referenced label \t has not previously been defined properly",
	/* 113 */	"Missing .ENDSEG",
	/* 114 */	"Uninitialized variable",
	/* 115 */	"Unused label ",
	/* 116 */	"'#' is allowed only in repeated lines",
	/* 117 */	"Memory segments overlap",
	/* 118 */	"Label \t is only for ",
	/* 119 */	"Infinite loop by label ",
	/* 120 */	"Unstable illegal code ",
	/* 121 */	"Ambiguous label ",
	/* 122 */	"Missing .ENDE",
	/* 123 */	"Multi-line argument is not supported",
	/* 124 */	"Buggy indirect jump",
	/* 125 */	"Branch too long, to long branch was used ",
	/* 126 */	"Branch across page boundary ",
	/* 127 */	"Register A is changed",
	/* 128 */	"Memory range has been exceeded",
	/* 129 */	"Syntax: mads source [options]\r\n"
				"-b:address\tGenerate binary file at specified address <address>\r\n"
				"-bc\t\tActivate branch condition test\r\n"
				"-c\t\tActivate case sensitivity for labels\r\n"
				"-d:label=value\tDefine a label and set it to <value>\r\n"
				"-f\t\tAllow mnemonics at the first column of a line\r\n"
				"-fv:value\tSet raw binary fill BYTE to <value>\r\n"
				"-hc[:filename]\tGenerate \".h\" header file for CA65\r\n"
				"-hm[:filename]\tGenerate \".hea\" header file for MADS\r\n"
				"-i:path\t\tUse additional include directory, can be specified multiple times\r\n"
				"-l[:filename]\tGenerate \".lst\" listing file\r\n"
				"-m:filename\tInclude macro definitions from file\r\n"
				"-ml:value\tSet left margin for listing to <value>\r\n"
				"-o:filename\tSet object file name\r\n"
				"-p\t\tDisplay fully qualified file names in listing and error messages\r\n"
				"-s\t\tSuppress info messages\r\n"
				"-t[:filename]\tGenerate \".lab\" labels file\r\n"
				"-u\t\tDisplay warnings for unused labels\r\n"
				"-vu\t\tVerify code inside unreferenced procedures\r\n"
				"-x\t\tExclude unreferenced procedures from code generation",
	/* 130 */   "",


	/* 131 */	"mads 2.1.7", // version};
	/* 132 */	"Unable to delete file "
};
