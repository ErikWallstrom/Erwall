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

#ifndef ERW_SCOPE_H
#define ERW_SCOPE_H

#include "erw_tokenizer.h"
#include "erw_ast.h"
#include "vec.h"

struct erw_FunctionSymbol
{
	struct erw_ASTNode* node;
	const char* name;
	const char* type;
	int used;
};

struct erw_VariableSymbol
{
	struct erw_ASTNode* node;
	const char* name;
	const char* type;
	int hasvalue;
	int isconst;
	int ismut;
	int used;
};

struct erw_TypeSymbol
{
	struct erw_ASTNode* node;
	const char* name;
	const char* type;
	int used;
};

struct erw_Scope
{
	Vec(struct erw_FunctionSymbol) functions;
	Vec(struct erw_VariableSymbol) variables;
	Vec(struct erw_TypeSymbol) types;
	Vec(struct erw_Scope*) children;
	struct erw_Scope* parent;
	const char* funcname;
};

struct erw_Scope* erw_scope_new(struct erw_Scope* parent, const char* funcname);
struct erw_VariableSymbol* erw_scope_getvariable(
	struct erw_Scope* self, 
	struct erw_Token* token,
	struct Str* lines
);
struct erw_FunctionSymbol* erw_scope_getfunction(
	struct erw_Scope* self, 
	struct erw_Token* token,
	struct Str* lines
);
struct erw_TypeSymbol* erw_scope_gettype(
	struct erw_Scope* self, 
	struct erw_Token* token,
	struct Str* lines
);
void erw_scope_addvariable(
	struct erw_Scope* self, 
	struct erw_ASTNode* node,
	struct Str* lines
);
void erw_scope_addfunction(
	struct erw_Scope* self, 
	struct erw_ASTNode* node,
	struct Str* lines
);
void erw_scope_addtype(
	struct erw_Scope* self, 
	struct erw_ASTNode* node,
	struct Str* lines
);
void erw_scope_print(struct erw_Scope* self);
void erw_scope_dtor(struct erw_Scope* self);

#endif
