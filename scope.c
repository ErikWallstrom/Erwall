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

#include "scope.h"
#include "ast.h"
#include "log.h"

static struct ASTNode* scope_findvariable(struct Scope* self, const char* name)
{
	log_assert(self, "is NULL");
	log_assert(name, "is NULL");

	do {
		for(size_t i = 0; i < vec_getsize(self->variables); i++)
		{
			if(!strcmp(self->variables[i]->branches[0]->token.text, name))
			{
				return self->variables[i];
			}
		}

		self = self->parent;
	} while(self);

	return NULL;
}

static struct ASTNode* scope_findfunction(struct Scope* self, const char* name)
{
	log_assert(self, "is NULL");
	log_assert(name, "is NULL");

	do {
		for(size_t i = 0; i < vec_getsize(self->functions); i++)
		{
			if(!strcmp(self->functions[i]->branches[0]->token.text, name))
			{
				return self->functions[i];
			}
		}

		self = self->parent;
	} while(self);

	return NULL;
}

static struct ASTNode* scope_findtype(struct Scope* self, const char* name)
{
	log_assert(self, "is NULL");
	log_assert(name, "is NULL");

	do {
		for(size_t i = 0; i < vec_getsize(self->types); i++)
		{
			if(!strcmp(self->types[i]->branches[0]->token.text, name))
			{
				return self->types[i];
			}
		}

		self = self->parent;
	} while(self);

	return NULL;
}

struct Scope* scope_ctor(struct Scope* self, struct Scope* parent)
{
	log_assert(self, "is NULL");

	self->functions = vec_ctor(struct ASTNode*, 0);
	self->variables = vec_ctor(struct ASTNode* , 0);
	self->types = vec_ctor(struct ASTNode* , 0);
	self->parent = parent;

	return self;
}

struct ASTNode* scope_getvariable(struct Scope* self, struct ASTNode* name)
{ 
	log_assert(self, "is NULL");
	log_assert(name, "is NULL");

	struct ASTNode* ret = scope_findvariable(self, name->token.text);
	if(!ret)
	{ 
		log_error(
			"Semantic error: Variable '%s' (line %zu, column %zu)"
				" is undefined",
			name->token.text,
			name->token.line,
			name->token.column
		);
	}
	return ret;
}

struct ASTNode* scope_getfunction(struct Scope* self, struct ASTNode* name)
{ 
	log_assert(self, "is NULL");
	log_assert(name, "is NULL");

	struct ASTNode* ret = scope_findfunction(self, name->token.text);
	if(!ret)
	{ 
		log_error(
			"Semantic error: Function '%s' (line %zu, column %zu)"
				" is undefined",
			name->token.text,
			name->token.line,
			name->token.column
		);
	}
	return ret;
}

struct ASTNode* scope_gettype(struct Scope* self, struct ASTNode* name)
{ 
	log_assert(self, "is NULL");
	log_assert(name, "is NULL");

	struct ASTNode* ret = scope_findtype(self, name->token.text);
	if(!ret)
	{ 
		log_error(
			"Semantic error: Type '%s' (line %zu, column %zu)"
				" is undefined",
			name->token.text,
			name->token.line,
			name->token.column
		);
	}
	return ret;
}

void scope_addvariable(struct Scope* self, struct ASTNode* var)
{
	log_assert(self, "is NULL");
	log_assert(var, "is NULL");
	
	struct ASTNode* vartest = scope_findvariable(
		self,
		var->branches[0]->token.text
	);

	if(vartest)
	{
		log_error(
			"Semantic error: Variable '%s' (line %zu, column %zu)"
				" has already been declared as a variable at line "
				"%zu, column %zu",
			var->branches[0]->token.text,
			var->branches[0]->token.line,
			var->branches[0]->token.column,
			vartest->branches[0]->token.line,
			vartest->branches[0]->token.column
		);
	}

	scope_gettype(self, var->branches[1]);
	struct ASTNode* functest = scope_findfunction(
		self,
		var->branches[0]->token.text
	);

	if(functest)
	{
		log_error(
			"Semantic error: Variable '%s' (line %zu, column %zu)"
				" has already been declared as a function at line "
				"%zu, column %zu",
			var->branches[0]->token.text,
			var->branches[0]->token.line,
			var->branches[0]->token.column,
			functest->branches[0]->token.line,
			functest->branches[0]->token.column
		);
	}

	vec_pushback(self->variables, var);
}

void scope_addfunction(struct Scope* self, struct ASTNode* func)
{
	log_assert(self, "is NULL");
	log_assert(func, "is NULL");

	struct ASTNode* functest = scope_findfunction(
		self,
		func->branches[0]->token.text
	);

	if(functest)
	{
		log_error(
			"Semantic error: Function '%s' (line %zu, column %zu)"
				" has already been declared as a function at line "
				"%zu, column %zu",
			func->branches[0]->token.text,
			func->branches[0]->token.line,
			func->branches[0]->token.column,
			functest->branches[0]->token.line,
			functest->branches[0]->token.column
		);
	}

	for(size_t i = 0; i < vec_getsize(func->branches[1]->branches); i++)
		//Check arguments
	{ 
		scope_gettype(self, func->branches[1]->branches[i]->branches[1]);
	}

	if(vec_getsize(func->branches[2]->branches)) //Check return value
	{ 
		scope_gettype(self, func->branches[2]->branches[0]);
	}

	struct ASTNode* vartest = scope_findvariable(
		self,
		func->branches[0]->token.text
	);

	if(vartest)
	{
		log_error(
			"Semantic error: Function '%s' (line %zu, column %zu)"
				" has already been declared as a variable at line "
				"%zu, column %zu",
			func->branches[0]->token.text,
			func->branches[0]->token.line,
			func->branches[0]->token.column,
			vartest->branches[0]->token.line,
			vartest->branches[0]->token.column
		);
	}

	vec_pushback(self->functions, func);
}

void scope_addtype(struct Scope* self, struct ASTNode* type)
{
	log_assert(self, "is NULL");
	log_assert(type, "is NULL");

	struct ASTNode* typetest = scope_findtype(
		self,
		type->branches[0]->token.text
	);

	if(typetest)
	{
		log_error(
			"Semantic error: Type '%s' (line %zu, column %zu)"
				" has already been declared at line %zu, column %zu",
			type->branches[0]->token.text,
			type->branches[0]->token.line,
			type->branches[0]->token.column,
			typetest->branches[0]->token.line,
			typetest->branches[0]->token.column
		);
	}

	if(vec_getsize(type->branches) > 1) //Check if user defined type
	{ 
		scope_gettype(self, type->branches[1]);
	}

	vec_pushback(self->types, type);
}

void scope_dtor(struct Scope* self) 
{
	log_assert(self, "is NULL");

	vec_dtor(self->functions);
	vec_dtor(self->variables);
	vec_dtor(self->types);
}

