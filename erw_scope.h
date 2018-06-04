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
#include "erw_ast.h"

struct erw_FuncDeclr
{
	struct erw_ASTNode* node;
	struct erw_Type* type;
	int used;
};

struct erw_VarDeclr //Should this contain isconst
{
	struct erw_ASTNode* node;
	struct erw_Type* type;
	int used;
	int hasvalue;
	//int isconst;
};

struct erw_TypeDeclr
{
	struct erw_ASTNode* node;
	struct erw_Type* type;
};

struct erw_Finalizer //Is this really a good name?
{
	struct erw_ASTNode* node;
	size_t index;
};

struct erw_Scope
{
	Vec(struct erw_FuncDeclr) functions;
	Vec(struct erw_VarDeclr) variables;
	Vec(struct erw_TypeDeclr*) types;
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
struct erw_VarDeclr* erw_scope_findvar(
	struct erw_Scope* self, 
	const char* name
);
struct erw_FuncDeclr* erw_scope_findfunc(
	struct erw_Scope* self, 
	const char* name
);
struct erw_VarDeclr* erw_scope_getvar(
	struct erw_Scope* self, 
	struct erw_Token* token,
	struct Str* lines
);
struct erw_FuncDeclr* erw_scope_getfunc(
	struct erw_Scope* self, 
	struct erw_Token* token,
	struct Str* lines
);
struct erw_Type* erw_scope_gettype(
	struct erw_Scope* self, 
	struct erw_Token* token,
	struct Str* lines
);
struct erw_Type* erw_scope_createtype(
	struct erw_Scope* self, 
	struct erw_ASTNode* node,
	struct Str* lines
);
void erw_scope_addvardeclr(
	struct erw_Scope* self, 
	struct erw_ASTNode* node,
	struct Str* lines
);
void erw_scope_addfuncdeclr(
	struct erw_Scope* self, 
	struct erw_ASTNode* node,
	struct Str* lines
);
void erw_scope_addtypedeclr(
	struct erw_Scope* self, 
	struct erw_ASTNode* node,
	struct Str* lines
);
void erw_scope_print(struct erw_Scope* self, struct Str* lines);
void erw_scope_dtor(struct erw_Scope* self);

#endif
