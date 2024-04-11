#include <stdio.h>
#include <stdlib.h>


//Zero page: first 256 bytes of memory
//Second page: next 256 bytes of memory, system stack
//register: PC(program counter), SP(stack pointer)

using Byte = unsigned char;
using Word = unsigned short;

using u32 = unsigned int;

struct Mem
{
	static constexpr u32 MAX_MEM = 1024 * 64;
	Byte Data[MAX_MEM];

	void Initialize()
	{
		for(u32 ii = 0; ii < MAX_MEM; ii++) {
			Data[ii] = 0;
		}
	}
	
	//read 1 Byte from memory
	Byte& operator[]( u32 Address )
	{
		return Data[Address];
	}
};


struct CPU
{
	Word PC;	//program counter
	Word SP;	//stack pointer

	//registers
	Byte A;
	Byte X;
	Byte Y;

	//status flags
	Byte C : 1;
	Byte Z : 1;
	Byte I : 1;
	Byte D : 1;
	Byte B : 1;
	Byte V : 1;
	Byte N : 1;

	void Reset( Mem& memory )
	{
		PC = 0xFFFC;
		SP = 0x0100;
		C = Z = I = D = B = V = N = 0;
		A = X = Y = 0;

		memory.Initialize();
	}

	Byte FetchByte( u32& Cycles, Mem& memory )
	{
		Byte Data = memory[PC];
		PC++;
		Cycles--;
		return Data;
	}
	
	static constexpr Byte
		INS_LDA_IM = 0xA9;

	void Execute( u32 Cycles, Mem& memory )
	{
		while( Cycles > 0 ) {
			Byte Instruction = FetchByte( Cycles, memory );
			switch ( Instruction )
			{
			case INS_LDA_IM :
			{
				Byte value = FetchByte( Cycles, memory );
				A = value;
				Z = (A == 0);
				N = (A & 0b100000000) > 0;
			} break;
			default :
			{
				printf("instruction not handled %d", Instruction);
			} break;
			}
		}
	}	
};

int main() {
	CPU cpu;
	Mem mem;
	cpu.Reset( mem );
	mem[0xFFFC] = CPU::INS_LDA_IM;
	mem[0xFFFD] = 0x42;

	cpu.Execute( 2, mem );

	return 0;
}
