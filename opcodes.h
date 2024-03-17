#pragma once

// 6502 / 65816 opcodes
#define OP_BRK_IMP		0x00	// 0	BRK		1 -> B					Jump to the [FFFE-FFFF] vector

#define OP_PHP_IMP		0x08	// 8	PHP		.P push, SP - 1 -> SP	Push Processor Status on Stack

#define OP_BPL_REL		0x10	// 16	BPL								Branch on N = 0

#define OP_CLC_IMP		0x18	// 24	CLC		0 -> C					Clear carry flag, no argument
#define OP_BIT_BASE		0x20	// 32	BIT ZP & ABS					Test Bits in Memory with Accumulator
#define OP_PLP_IMP		0x28	// 40	PLP		.P pull, SP + 1 -> SP	Pull processor status from Stack

#define OP_BMI_REL		0x30	// 48	BMI								Branch on N = 1

#define OP_SEC_IMP		0x38	// 56	SEC		1 -> C					Set Carry Flag
#define OP_RTI_IMP		0x40	// 64	RTI								Return from interrupt
#define OP_PHA_IMP		0x48	// 72	PHA		.A push, SP - 1 -> SP	Push accumulator on stack
#define OP_BVC_REL		0x50	// 80	BVC								Branch on V = 0
#define OP_CLI_IMP		0x58	// 88	CLI		0 -> I					Clear interrupt flag, no argument

#define OP_RTS_IMP		0x60	// 96	RTS								Return from subroutine (by pulling PC from stack)
#define OP_PLA_IMP		0x68	// 104	PLA		.A pull, SP + 1 -> SP	Pull accumulator from stack

#define OP_BVS_REL		0x70	// 112	BVS								Branch on V = 0
#define OP_SEI_IMP		0x78	// 120	SEI		1 -> I					Set Interrupt Disable Status

#define OP_DEY_IMP		0x88	// 136	DEY		Y - 1 -> Y				Decrement Y
#define OP_TXA_IMP		0x8A	// 138	TXA		X -> A					Transfer X to accumulator

#define OP_BCC_REL		0x90	// 144	BCC								Branch on Carry Clear (C = 0)
#define OP_TYA_IMP		0x98	// 152	TYA		Y -> A					Transfer Y to accumulator
#define OP_TXS_IMP		0x9A	// 154	TXA		X -> SP					transfer X to stack pointer

#define OP_TAY_IMP		0xA8	// 168	TAY		A -> Y					Transfer accumulator to Y
#define OP_TAX_IMP		0xAA	// 170	TAX		A -> X					Transfer accumulator to X

#define OP_BCS_REL		0xB0	// 176	BSC								Branch on carry set
#define OP_TSX_IMP		0xBA	// 186	TSX		SP -> X					Transfer stack pointer to X

#define OP_INY_IMP		0xC8	// 200	INY		Y + 1 -> Y				Increment Y
#define OP_DEX_IMP		0xCA	// 202	DEX		X - 1 -> X				Decrement X

#define OP_BNE_REL		0xD0	// 208	BNE								Branch on Result not Zero

#define OP_INX_IMP		0xE8	// 232	INX		X + 1 -> Y				Increment X

#define OP_NOP_IMP		0xEA	// 234	NOP								No operation

#define OP_BEQ_REL		0xF0	// 240	BEQ								Branch on Z = 1
#define OP_SED_IMP		0xF8	// 248	SED		1 -> D					Set Decimal Flag

#define OP_ARR_IMM_ILL	107		// $6B	ARR # This opcode ANDs the contents of the A register with an immediate value and then RORs the result
#define OP_JMP_IND		108		// $6C	JMP indirect:		 [ADDR]->PCL, [ADDR+1]->PCH
#define OP_ROR_ABS		110		// $6E	Rotate right @ addr:	 [Addr] (->) -> [ADDR]
#define OP_RRA_ABS_ILL	111		// $6F	RRA RORs the contents of a memory location and then ADCs the result with the accumulator.

#define OP_CLV_IMP		0xB8	// 184	CLV		0 -> V	Clear overflow flag, no argument


#define OP_CLD_IMP		0xD8	// 216	CLD		0 -> D	Clear decimal mode flag, no argument