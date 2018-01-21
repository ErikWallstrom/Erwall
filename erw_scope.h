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
#include "erw_type.h"

struct erw_Function
{
	struct erw_Type* type;
	struct erw_ASTNode* node;
	const char* name;
	int used;
};

struct erw_Variable
{
	struct erw_Type* type;
	struct erw_ASTNode* node;
	const char* name;
	int hasvalue;
	int isconst;
	int ismut;
	int used;
};

struct erw_Finalizer //Is this really a good name?
{
	struct erw_ASTNode* node;
	size_t index;
};

struct erw_Scope
{
	Vec(struct erw_Function) functions;
	Vec(struct erw_Variable) variables;
	Vec(struct erw_Type*) types;
	Vec(struct erw_Scope*) children;
	Vec(struct erw_Finalizer) finalizers;
	struct erw_Scope* parent;
	const char* funcname;
	size_t index;
	int isfunction;
};

struct erw_Scope* erw_scope_new(
	struct erw_Scope* parent, 
	const char* funcname, 
	size_t index,
	int isfunction
);
struct erw_Variable* erw_scope_getvariable(
	struct erw_Scope* self, 
	struct erw_Token* token,
	struct Str* lines
);
struct erw_Function* erw_scope_getfunction(
	struct erw_Scope* self, 
	struct erw_Token* token,
	struct Str* lines
);
struct erw_Type* erw_scope_gettype(
	struct erw_Scope* self, 
	struct erw_ASTNode* node,
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
void erw_scope_addtypedef(
	struct erw_Scope* self, 
	struct erw_ASTNode* node,
	struct Str* lines
);
void erw_scope_print(struct erw_Scope* self);
void erw_scope_dtor(struct erw_Scope* self);

#endif
