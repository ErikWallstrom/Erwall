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

#ifndef SCOPE_H
#define SCOPE_H

#include "vec.h"

struct VariableScope
{
	Vec(struct ASTNode*) variables;
	struct VariableScope* parent;
};

struct Scope
{
	struct VariableScope variablescope;
	Vec(struct ASTNode*) functions;
	Vec(struct ASTNode*) types;
	struct Scope* parent;
};

struct VariableScope* variablescope_ctor(
	struct VariableScope* self, 
	struct VariableScope* parent
);
void variablescope_dtor(struct VariableScope* self);

struct Scope* scope_ctor(struct Scope* self, struct Scope* parent);
struct ASTNode* scope_getvariable(struct Scope* self, struct ASTNode* name);
struct ASTNode* scope_getfunction(struct Scope* self, struct ASTNode* name);
struct ASTNode* scope_gettype(struct Scope* self, struct ASTNode* name);
void scope_addvariable(struct Scope* self, struct ASTNode* var);
void scope_addfunction(struct Scope* self, struct ASTNode* func);
void scope_addtype(struct Scope* self, struct ASTNode* type);
void scope_dtor(struct Scope* self);

#endif
