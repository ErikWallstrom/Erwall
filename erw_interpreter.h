/*
	Copyright (C) 2017 Erik Wallström

	This file is part of Erwall.

	Erwall is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Erwall is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Erwall.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ERW_INTERPRETER_H
#define ERW_INTERPRETER_H

#include <stdint.h>
#include <stddef.h>

#define erw_NUMREGISTERS 4
#define erw_NUMINSTRUCTIONS 29

enum erw_InstructionID
{
	erw_INSTRUCTIONID_LOADL8,
	erw_INSTRUCTIONID_LOADL16,
	erw_INSTRUCTIONID_LOADL32,
	erw_INSTRUCTIONID_LOADL64,
	erw_INSTRUCTIONID_ADD,
	erw_INSTRUCTIONID_SUB,
	erw_INSTRUCTIONID_MUL,
	erw_INSTRUCTIONID_DIV, 	
	erw_INSTRUCTIONID_POW,
	erw_INSTRUCTIONID_MOD, 	
	erw_INSTRUCTIONID_FADD,
	erw_INSTRUCTIONID_FSUB, 	
	erw_INSTRUCTIONID_FMUL,
	erw_INSTRUCTIONID_FDIV, 	
	erw_INSTRUCTIONID_FPOW,
	erw_INSTRUCTIONID_FMOD, 	
	erw_INSTRUCTIONID_MOV,
	erw_INSTRUCTIONID_PUSH,
	erw_INSTRUCTIONID_POP,
	erw_INSTRUCTIONID_CALL, 	
	erw_INSTRUCTIONID_RET, 	
	erw_INSTRUCTIONID_CMP, 	
	erw_INSTRUCTIONID_JMP,
	erw_INSTRUCTIONID_JNE, 	
	erw_INSTRUCTIONID_JGE,
	erw_INSTRUCTIONID_JLE, 	
	erw_INSTRUCTIONID_JE,
	erw_INSTRUCTIONID_JG, 	
	erw_INSTRUCTIONID_JL,
};

struct erw_Register
{
	char buffer[8];
};

struct erw_Interpreter;
typedef void(*erw_InstructionRunner)(struct erw_Interpreter*);

struct erw_Interpreter
{
	struct erw_Register registers[erw_NUMREGISTERS];
	erw_InstructionRunner runners[erw_NUMINSTRUCTIONS];
	uint8_t* instructions;
	char* stack;

	size_t numinstructions;
	size_t stacksize;
	size_t ip;
	size_t sp;
};

struct erw_Interpreter* erw_interpreter_ctor(
	struct erw_Interpreter* self, 
	uint8_t* instructions,
	size_t numinstructions,
	size_t stacksize
);

void erw_interpreter_dtor(struct erw_Interpreter* self);

#endif
