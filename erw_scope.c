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

#include "erw_scope.h"
#include "erw_error.h"
#include "str.h"
#include "log.h"

#include <stdlib.h>

static struct erw_VariableSymbol* erw_scope_findvariable(
	struct erw_Scope* self, 
	const char* name)
{
	log_assert(self, "is NULL");
	log_assert(name, "is NULL");

	struct erw_Scope* scope = self;
	while(scope)
	{
		for(size_t i = 0; i < vec_getsize(scope->variables); i++)
		{
			if(!strcmp(scope->variables[i].name, name))
			{
				return &scope->variables[i];
			}
		}

		scope = scope->parent;
		if(scope && scope->funcname)
		{
			if(strcmp(scope->funcname, self->funcname))
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

	return NULL;
}

static struct erw_FunctionSymbol* erw_scope_findfunction(
	struct erw_Scope* self, 
	const char* name)
{
	log_assert(self, "is NULL");
	log_assert(name, "is NULL");

	struct erw_Scope* scope = self;
	while(scope)
	{
		for(size_t i = 0; i < vec_getsize(scope->functions); i++)
		{
			if(!strcmp(scope->functions[i].name, name))
			{
				return &scope->functions[i];
			}
		}

		scope = scope->parent;
	}

	return NULL;
}

static struct erw_TypeSymbol* erw_scope_findtype(
	struct erw_Scope* self, 
	const char* name)
{
	log_assert(self, "is NULL");
	log_assert(name, "is NULL");

	struct erw_Scope* scope = self;
	while(scope)
	{
		for(size_t i = 0; i < vec_getsize(scope->types); i++)
		{
			if(!strcmp(scope->types[i].name, name))
			{
				return &scope->types[i];
			}
		}

		scope = scope->parent;
	}

	return NULL;
}

struct erw_Scope* erw_scope_new(struct erw_Scope* parent, const char* funcname)
{
	struct erw_Scope* self = malloc(sizeof(struct erw_Scope));
	if(!self)
	{
		log_error("malloc failed in <%s>", __func__);
	}

	self->functions = vec_ctor(struct erw_FunctionSymbol, 0);
	self->variables = vec_ctor(struct erw_VariableSymbol, 0);
	self->types = vec_ctor(struct erw_TypeSymbol, 0);
	self->children = vec_ctor(struct erw_Scope*, 0);
	self->funcname = funcname;
	self->parent = parent;

	if(parent)
	{
		vec_pushback(parent->children, self);
	}

	return self;
}

struct erw_VariableSymbol* erw_scope_getvariable(
	struct erw_Scope* self, 
	struct erw_Token* token,
	struct Str* lines)
{
	log_assert(self, "is NULL");
	log_assert(token, "is NULL");
	log_assert(lines, "is NULL");

	struct erw_VariableSymbol* ret = erw_scope_findvariable(self, token->text);
	if(!ret)
	{ 
		struct Str msg;
		str_ctor(
			&msg,
			"Undefined variable"
		);

		erw_error(
			msg.data, 
			lines[token->linenum - 1].data, 
			token->linenum, 
			token->column,
			token->column + vec_getsize(token->text) - 2
		);
		str_dtor(&msg);
	}

	return ret;
}

struct erw_FunctionSymbol* erw_scope_getfunction(
	struct erw_Scope* self, 
	struct erw_Token* token,
	struct Str* lines)
{
	log_assert(self, "is NULL");
	log_assert(token, "is NULL");
	log_assert(lines, "is NULL");

	struct erw_FunctionSymbol* ret = erw_scope_findfunction(self, token->text);
	if(!ret)
	{ 
		struct Str msg;
		str_ctor(&msg, "Undefined function");
		erw_error(
			msg.data, 
			lines[token->linenum - 1].data, 
			token->linenum, 
			token->column,
			token->column + vec_getsize(token->text) - 2
		);
		str_dtor(&msg);
	}

	return ret;
}

struct erw_TypeSymbol* erw_scope_gettype(
	struct erw_Scope* self, 
	struct erw_Token* token,
	struct Str* lines)
{
	log_assert(self, "is NULL");
	log_assert(token, "is NULL");
	log_assert(lines, "is NULL");

	struct erw_TypeSymbol* ret = erw_scope_findtype(self, token->text);
	if(!ret)
	{ 
		struct Str msg;
		str_ctor(&msg, "Undefined type");
		erw_error(
			msg.data, 
			lines[token->linenum - 1].data, 
			token->linenum, 
			token->column,
			token->column + vec_getsize(token->text) - 2
		);
		str_dtor(&msg);
	}

	return ret;
}

void erw_scope_addvariable(
	struct erw_Scope* self, 
	struct erw_ASTNode* node,
	struct Str* lines)
{
	log_assert(self, "is NULL");
	log_assert(node, "is NULL");
	log_assert(lines, "is NULL");

	struct erw_VariableSymbol* var = erw_scope_findvariable(
		self, 
		node->branches[0]->token.text
	);

	if(var)
	{
		struct Str msg;
		str_ctorfmt(
			&msg,
			"Redefinition of variable ('%s') declared at line %zu, column %zu", 
			node->branches[0]->token.text,
			var->node->branches[0]->token.linenum,
			var->node->branches[0]->token.column
		);

		erw_error(
			msg.data, 
			lines[node->branches[0]->token.linenum - 1].data, 
			node->branches[0]->token.linenum, 
			node->branches[0]->token.column,
			node->branches[0]->token.column + 
				vec_getsize(node->branches[0]->token.text) - 2
		);
		str_dtor(&msg);
	}

	struct erw_FunctionSymbol* func = erw_scope_findfunction(
		self, 
		node->branches[0]->token.text
	);

	if(func)
	{
		struct Str msg;
		str_ctorfmt(
			&msg,
			"Redefinition of variable ('%s') declared at line %zu, column %zu", 
			node->branches[0]->token.text,
			func->node->branches[0]->token.linenum,
			func->node->branches[0]->token.column
		);

		erw_error(
			msg.data, 
			lines[node->branches[0]->token.linenum - 1].data, 
			node->branches[0]->token.linenum, 
			node->branches[0]->token.column,
			node->branches[0]->token.column + 
				vec_getsize(node->branches[0]->token.text) - 2
		);
		str_dtor(&msg);
	}

	erw_scope_gettype(self, &node->branches[1]->token, lines);

	struct erw_VariableSymbol symbol;
	if(node->token.type == erw_TOKENTYPE_KEYWORD_LET)
	{
		symbol.ismut = 0;
	}
	else
	{
		symbol.ismut = 1;
	}

	if(vec_getsize(node->branches) == 3) //Not parameter
	{
		if(vec_getsize(node->branches[2]->branches))
		{
			if(!symbol.ismut)
			{
				symbol.isconst = 1;
			}
			else
			{
				symbol.isconst = 0;
			}

			symbol.hasvalue = 1;
		}
		else
		{
			symbol.isconst = 0;
			symbol.hasvalue = 0;
		}
	}
	else
	{
		symbol.isconst = 0;
		symbol.hasvalue = 1;
	}

	symbol.name = node->branches[0]->token.text;
	symbol.type = node->branches[1]->token.text;
	symbol.node = node;
	symbol.used = 0;
	vec_pushback(self->variables, symbol);
}

void erw_scope_addfunction(
	struct erw_Scope* self, 
	struct erw_ASTNode* node,
	struct Str* lines)
{
	log_assert(self, "is NULL");
	log_assert(node, "is NULL");
	log_assert(lines, "is NULL");

	struct erw_FunctionSymbol* func = erw_scope_findfunction(
		self, 
		node->branches[0]->token.text
	);

	if(func)
	{
		struct Str msg;
		str_ctorfmt(
			&msg,
			"Redefinition of function ('%s') declared at line %zu, column %zu", 
			node->branches[0]->token.text,
			func->node->branches[0]->token.linenum,
			func->node->branches[0]->token.column
		);

		erw_error(
			msg.data, 
			lines[node->branches[0]->token.linenum - 1].data, 
			node->branches[0]->token.linenum, 
			node->branches[0]->token.column,
			node->branches[0]->token.column + 
				vec_getsize(node->branches[0]->token.text) - 2
		);
		str_dtor(&msg);
	}

	struct erw_VariableSymbol* var = erw_scope_findvariable(
		self, 
		node->branches[0]->token.text
	);

	if(var)
	{
		struct Str msg;
		str_ctorfmt(
			&msg,
			"Redefinition of function ('%s') declared at line %zu, column %zu", 
			node->branches[0]->token.text,
			var->node->branches[0]->token.linenum,
			var->node->branches[0]->token.column
		);

		erw_error(
			msg.data, 
			lines[node->branches[0]->token.linenum - 1].data, 
			node->branches[0]->token.linenum, 
			node->branches[0]->token.column,
			node->branches[0]->token.column + 
				vec_getsize(node->branches[0]->token.text) - 2
		);
		str_dtor(&msg);
	}

	if(vec_getsize(node->branches[2]->branches))
	{
		erw_scope_gettype(self, &node->branches[2]->branches[0]->token, lines);
	}

	struct erw_FunctionSymbol symbol;
	symbol.name = node->branches[0]->token.text;
	if(vec_getsize(node->branches[2]->branches))
	{
		symbol.type = node->branches[2]->branches[0]->token.text;
	}
	else
	{
		symbol.type = NULL;
	}
	symbol.node = node;
	symbol.used = 0;
	vec_pushback(self->functions, symbol);
}

void erw_scope_addtype(
	struct erw_Scope* self, 
	struct erw_ASTNode* node,
	struct Str* lines)
{
	log_assert(self, "is NULL");
	log_assert(node, "is NULL");
	log_assert(lines, "is NULL");

	struct erw_TypeSymbol* type = erw_scope_findtype(
		self, 
		node->branches[0]->token.text
	);

	if(type)
	{
		struct Str msg;
		str_ctorfmt(
			&msg,
			"Redefinition of type ('%s') declared at line %zu, column %zu", 
			node->branches[0]->token.text,
			type->node->branches[0]->token.linenum,
			type->node->branches[0]->token.column
		);

		erw_error(
			msg.data, 
			lines[node->branches[0]->token.linenum - 1].data, 
			node->branches[0]->token.linenum, 
			node->branches[0]->token.column,
			node->branches[0]->token.column + 
				vec_getsize(node->branches[0]->token.text) - 2
		);
		str_dtor(&msg);
	}

	struct erw_TypeSymbol symbol;
	if(vec_getsize(node->branches) == 2)
	{
		erw_scope_gettype(self, &node->branches[1]->token, lines);
		symbol.type = node->branches[1]->token.text;
	}
	else
	{
		symbol.type = NULL; //Should this be null or same as name?
	}

	symbol.name = node->branches[0]->token.text;
	symbol.node = node;
	symbol.used = 0;
	vec_pushback(self->types, symbol);
}

void erw_scope_printinternal(struct erw_Scope* self, size_t level)
{
	for(size_t i = 0; i < level; i++)
	{
		printf("    ");
		printf("│");
	}

	printf("─ Scope Name: [%s]\n", self->funcname);

	for(size_t i = 0; i < vec_getsize(self->types); i++)
	{
		for(size_t j = 0; j < level + 1; j++)
		{
			printf("    ");
			printf("│");
		}
		printf("─ Type: %s (%s)\n", self->types[i].name, self->types[i].type);
	}

	for(size_t i = 0; i < level; i++)
	{
		printf("    ");
		printf("│");
	}

	for(size_t i = 0; i < vec_getsize(self->functions); i++)
	{
		for(size_t j = 0; j < level + 1; j++)
		{
			printf("    ");
			printf("│");
		}
		printf("─ Function: %s (%s)\n", self->functions[i].name, self->functions[i].type);
	}

	for(size_t i = 0; i < vec_getsize(self->variables); i++)
	{
		for(size_t j = 0; j < level + 1; j++)
		{
			printf("    ");
			printf("│");
		}
		printf("─ Variable: %s (%s)\n", self->variables[i].name, self->variables[i].type);
	}

	for(size_t i = 0; i < vec_getsize(self->children); i++)
	{
		erw_scope_printinternal(self->children[i], level + 1);
	}
}

void erw_scope_print(struct erw_Scope* self)
{
	log_assert(self, "is NULL");
	erw_scope_printinternal(self, 0);
}

void erw_scope_dtor(struct erw_Scope* self)
{
	log_assert(self, "is NULL");

	for(size_t i = 0; i < vec_getsize(self->children); i++)
	{
		erw_scope_dtor(self->children[i]);
	}

	vec_dtor(self->functions);
	vec_dtor(self->variables);
	vec_dtor(self->types);
	vec_dtor(self->children);
	free(self);
}

