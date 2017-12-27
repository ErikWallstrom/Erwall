#include "erw_interpreter.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>

/*
static void erw_interpreter_loadl8(struct erw_Interpreter* self)
{
	log_assert(self, "is NULL");

	*(uint8_t*)(self->registers[self->instructions[self->ip + 1]].buffer) = 
		*(uint8_t*)(self->instructions + self->ip + 2);

	self->ip += 3;
}

static void erw_interpreter_loadl16(struct erw_Interpreter* self)
{
	log_assert(self, "is NULL");

	*(uint16_t*)(self->registers[self->instructions[self->ip + 1]].buffer) = 
		*(uint16_t*)(self->instructions + self->ip + 2);

	self->ip += 3 + 2;
}

static void erw_interpreter_loadl32(struct erw_Interpreter* self)
{
	log_assert(self, "is NULL");

	*(uint32_t*)(self->registers[self->instructions[self->ip + 1]].buffer) = 
		*(uint32_t*)(self->instructions + self->ip + 2);

	self->ip += 3 + 4;
}

static void erw_interpreter_loadl64(struct erw_Interpreter* self)
{
	log_assert(self, "is NULL");

	*(uint64_t*)(self->registers[self->instructions[self->ip + 1]].buffer) = 
		*(uint64_t*)(self->instructions + self->ip + 2);

	self->ip += 3 + 8;
}

static void erw_interpreter_add(struct erw_Interpreter* self)
{
	log_assert(self, "is NULL");

	*(uint64_t*)(self->registers[self->instructions[self->ip + 1]].buffer) = 
		*(uint64_t*)self->registers[self->instructions[self->ip + 2]].buffer + 
		*(uint64_t*)self->registers[self->instructions[self->ip + 3]].buffer;

	self->ip += 4;
}

static void erw_interpreter_sub(struct erw_Interpreter* self)
{
	log_assert(self, "is NULL");

	*(uint64_t*)(self->registers[self->instructions[self->ip + 1]].buffer) = 
		*(uint64_t*)self->registers[self->instructions[self->ip + 2]].buffer - 
		*(uint64_t*)self->registers[self->instructions[self->ip + 3]].buffer;

	self->ip += 4;
}

static void erw_interpreter_mul(struct erw_Interpreter* self)
{
	log_assert(self, "is NULL");

	*(uint64_t*)(self->registers[self->instructions[self->ip + 1]].buffer) = 
		*(uint64_t*)self->registers[self->instructions[self->ip + 2]].buffer * 
		*(uint64_t*)self->registers[self->instructions[self->ip + 3]].buffer;

	self->ip += 4;
}

static void erw_interpreter_div(struct erw_Interpreter* self)
{
	log_assert(self, "is NULL");

	*(uint64_t*)(self->registers[self->instructions[self->ip + 1]].buffer) = 
		*(uint64_t*)self->registers[self->instructions[self->ip + 2]].buffer / 
		*(uint64_t*)self->registers[self->instructions[self->ip + 3]].buffer;

	self->ip += 4;
}

static void erw_interpreter_pow(struct erw_Interpreter* self)
{
	log_assert(self, "is NULL");

	*(uint64_t*)(self->registers[self->instructions[self->ip + 1]].buffer) = 
		*(uint64_t*)self->registers[self->instructions[self->ip + 2]].buffer + 
		*(uint64_t*)self->registers[self->instructions[self->ip + 3]].buffer;

	self->ip += 4;
}

static void erw_interpreter_mod(struct erw_Interpreter* self)
{
	log_assert(self, "is NULL");

	*(uint64_t*)(self->registers[self->instructions[self->ip + 1]].buffer) = 
		*(uint64_t*)self->registers[self->instructions[self->ip + 2]].buffer % 
		*(uint64_t*)self->registers[self->instructions[self->ip + 3]].buffer;

	self->ip += 4;
}

static void erw_interpreter_mov8(struct erw_Interpreter* self)
{
	log_assert(self, "is NULL");
}

static void erw_interpreter_mov16(struct erw_Interpreter* self)
{
	log_assert(self, "is NULL");
}

static void erw_interpreter_mov32(struct erw_Interpreter* self)
{
	log_assert(self, "is NULL");
}

static void erw_interpreter_mov64(struct erw_Interpreter* self)
{
	log_assert(self, "is NULL");
}

static void erw_interpreter_push(struct erw_Interpreter* self)
{
	log_assert(self, "is NULL");

	self->sp++;
	if(self->sp >= self->stacksize)
	{
		log_error("Stack overflow");
	}
}

static void erw_interpreter_pop(struct erw_Interpreter* self)
{
	log_assert(self, "is NULL");

	if(!self->sp)
	{
		log_error("Stack underflow");
	}

	self->sp++;
}

static void erw_interpreter_call(struct erw_Interpreter* self)
{
	log_assert(self, "is NULL");

}

static void erw_interpreter_ret(struct erw_Interpreter* self)
{
	log_assert(self, "is NULL");

}

static void erw_interpreter_setrunners(struct erw_Interpreter* self)
{
	log_assert(self, "is NULL");

	self->runners[0] 	= erw_interpreter_loadl8;
	self->runners[1] 	= erw_interpreter_loadl16;
	self->runners[2] 	= erw_interpreter_loadl32;
	self->runners[3] 	= erw_interpreter_loadl64;
	self->runners[4] 	= erw_interpreter_add;
	self->runners[5] 	= erw_interpreter_sub;
	self->runners[6]	= erw_interpreter_mul;
	self->runners[7] 	= erw_interpreter_div;
	self->runners[8] 	= erw_interpreter_pow;
	self->runners[9] 	= erw_interpreter_mod;
}

static void erw_interpreter_run(struct erw_Interpreter* self)
{
	log_assert(self, "is NULL");
	while(self->ip < self->numinstructions)
	{
		self->runners[self->instructions[self->ip]](self);
	}
}

*/

struct erw_Interpreter* erw_interpreter_ctor(
	struct erw_Interpreter* self, 
	uint8_t* instructions,
	size_t numinstructions, 
	size_t stacksize)
{
	log_assert(self, "is NULL");
	log_assert(instructions, "is NULL");

	self->numinstructions = numinstructions;
	self->instructions = instructions;
	self->ip = 0;
	self->sp = 0;
	memset(self->registers, 0, sizeof(struct erw_Register) * erw_NUMREGISTERS);

	self->stack = malloc(stacksize);
	if(!self->stack)
	{
		log_error("malloc failed <%s>", __func__);
	}

	//erw_interpreter_setrunners(self);
	//erw_interpreter_run(self);

	return self;
}

void erw_interpreter_dtor(struct erw_Interpreter* self)
{
	log_assert(self, "is NULL");
	free(self->stack);
}

/* TODO: 
#include "vec.h"
int main(void)
{
	Vec(uint8_t) instructions = vec_ctor(uint8_t, 0);
	vec_pushbackwitharr(
		instructions, 
		((uint8_t[]){
			erw_INSTRUCTIONID_LOADL8, 0, 3,
			erw_INSTRUCTIONID_LOADL8, 1, 9,
			erw_INSTRUCTIONID_LOADL8, 2, 2,
			erw_INSTRUCTIONID_ADD, 0, 0, 1,
			erw_INSTRUCTIONID_MUL, 2, 0, 2,
		}),
		17
	);

	struct erw_Interpreter interpreter;
	erw_interpreter_ctor(&interpreter, instructions, vec_getsize(instructions), 1024);
	printf("Result: %lu\n", *(uint64_t*)interpreter.registers[2].buffer);
	erw_interpreter_dtor(&interpreter);
}
*/
