#ifndef SEMANTICS_H
#define SEMANTICS_H

#include "parser.h"

struct FunctionSemantics
{
	int isglobal;
	int used;
};

struct FunctionCallSemantics
{
	int isglobal;
};

struct VariableSemantics
{
	int used;
	int hasvalue;
};

void checksemantics(struct ASTNode* ast);

#endif
