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

static struct erw_Variable* erw_scope_findvariable(
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

static struct erw_Function* erw_scope_findfunction(
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

static struct erw_Type* erw_scope_findtype(
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
			if(!strcmp(scope->types[i]->typedef_.name, name))
			{
				return scope->types[i];
			}
		}

		scope = scope->parent;
	}

	return NULL;
}

struct erw_Scope* erw_scope_new(
	struct erw_Scope* parent, 
	const char* funcname, 
	size_t index,
	int isfunction)
{
	struct erw_Scope* self = malloc(sizeof(struct erw_Scope));
	if(!self)
	{
		log_error("malloc failed in <%s>", __func__);
	}

	self->functions = vec_ctor(struct erw_Function, 0);
	self->variables = vec_ctor(struct erw_Variable, 0);
	self->types = vec_ctor(struct erw_Type*, 0);
	self->children = vec_ctor(struct erw_Scope*, 0);
	self->finalizers = vec_ctor(struct erw_Finalizer, 0);
	self->parent = parent;
	self->isfunction = isfunction;
	self->index = index;
	self->funcname = funcname;

	if(parent)
	{
		vec_pushback(parent->children, self);
	}

	return self;
}

struct erw_Variable* erw_scope_getvariable(
	struct erw_Scope* self, 
	struct erw_Token* token,
	struct Str* lines)
{
	log_assert(self, "is NULL");
	log_assert(token, "is NULL");
	log_assert(lines, "is NULL");

	struct erw_Variable* ret = erw_scope_findvariable(self, token->text);
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

struct erw_Function* erw_scope_getfunction(
	struct erw_Scope* self, 
	struct erw_Token* token,
	struct Str* lines)
{
	log_assert(self, "is NULL");
	log_assert(token, "is NULL");
	log_assert(lines, "is NULL");

	struct erw_Function* ret = erw_scope_findfunction(self, token->text);
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

struct erw_Type* erw_scope_gettype(
	struct erw_Scope* self, 
	struct erw_ASTNode* node,
	struct Str* lines)
{
	log_assert(self, "is NULL");
	log_assert(node, "is NULL");
	log_assert(lines, "is NULL");

	struct erw_Type* type = erw_type_newfromastnode(node);
	struct erw_Type* basetype = erw_type_getbase(type);

	if(basetype->type == erw_TYPETYPE_TYPEDEF)
	{
		struct erw_Type* foundtype = erw_scope_findtype(
			self, 
			basetype->typedef_.name
		);

		if(!foundtype)
		{
			struct Str msg;
			str_ctor(&msg, "Undefined type");
			erw_error(
				msg.data, 
				lines[basetype->typedef_.token->linenum - 1].data, 
				basetype->typedef_.token->linenum, 
				basetype->typedef_.token->column,
				basetype->typedef_.token->column + 
					vec_getsize(basetype->typedef_.token->text) - 2
			);
			str_dtor(&msg);
		}

		basetype->typedef_.type = foundtype->typedef_.type;
	}

	return type;
}

void erw_scope_addvariable(
	struct erw_Scope* self, 
	struct erw_ASTNode* node,
	struct Str* lines)
{
	log_assert(self, "is NULL");
	log_assert(node, "is NULL");
	log_assert(lines, "is NULL");

	struct erw_Variable* var = erw_scope_findvariable(
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

	struct erw_Function* func = erw_scope_findfunction(
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

	struct erw_Variable symbol;
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

	struct erw_ASTNode* typenode = node->branches[1];

	symbol.type = erw_scope_gettype(self, typenode, lines);
	symbol.name = node->branches[0]->token.text;
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

	struct erw_Function* func = erw_scope_findfunction(
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

	struct erw_Variable* var = erw_scope_findvariable(
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

	struct erw_Function symbol;
	if(vec_getsize(node->branches[2]->branches))
	{
		struct erw_ASTNode* typenode = node->branches[2]->branches[0];
		symbol.type = erw_scope_gettype(self, typenode, lines);
	}
	else
	{
		symbol.type = NULL;
	}

	symbol.name = node->branches[0]->token.text;
	symbol.node = node;
	symbol.used = 0;
	vec_pushback(self->functions, symbol);
}

void erw_scope_addtypedef(
	struct erw_Scope* self, 
	struct erw_ASTNode* node,
	struct Str* lines)
{
	log_assert(self, "is NULL");
	log_assert(node, "is NULL");
	log_assert(lines, "is NULL");

	struct erw_Type* type1 = erw_type_newfromastnode(node->branches[0]);
	struct erw_Type* type = erw_scope_findtype(self, type1->typedef_.name);
	if(type)
	{
		struct Str msg;
		str_ctorfmt(
			&msg,
			"Redefinition of type ('%s') declared at line %zu, column %zu", 
			type1->typedef_.name,
			type->typedef_.token->linenum,
			type->typedef_.token->column
		);

		erw_error(
			msg.data, 
			lines[type1->typedef_.token->linenum - 1].data, 
			type1->typedef_.token->linenum, 
			type1->typedef_.token->column,
			type1->typedef_.token->column + 
				vec_getsize(type1->typedef_.token->text) - 2
		);
		str_dtor(&msg);
	}

	if(vec_getsize(node->branches) == 2)
	{
		log_assert(node->branches[1]->istoken, "???");
		struct erw_Type* type2 = erw_type_newfromastnode(node->branches[1]);
		if(type2->type != erw_TYPETYPE_TYPEDEF)
		{
			struct Str msg;
			str_ctor(&msg, "You cannot declare a Type to be a reference/array");
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

		type1->typedef_.type = erw_scope_gettype(
			self, 
			node->branches[1], 
			lines
		);
	}

	vec_pushback(self->types, type1);
}

void erw_scope_printinternal(struct erw_Scope* self, size_t level)
{
	for(size_t i = 0; i < level; i++)
	{
		printf("    ");
		printf("│");
	}

	if(self->isfunction)
	{
		printf(
			"─ Scope [%zu]: Function [%s]\n", 
			self->index, 
			self->funcname ?  self->funcname : "null"
		); 
	}
	else
	{
		printf("─ Scope [%zu]: \n", self->index);
	}

	for(size_t i = 0; i < vec_getsize(self->types); i++)
	{
		if(self->types[i]->typedef_.type)
		{
			int isnative = 0;
			for(size_t j = 0; j < erw_TYPEBUILTIN_COUNT; j++)
			{  
				if(self->types[i]->typedef_.name == 
					erw_type_builtins[j]->typedef_.name)
				{
					isnative = 1;
					break;
				}
			}

			if(!isnative)
			{
				for(size_t j = 0; j < level + 1; j++)
				{
					printf("    ");
					printf("│");
				}

				struct Str str = erw_type_tostring(self->types[i]->typedef_.type);
				printf("─ Type: %s (%s)\n", self->types[i]->typedef_.name, str.data);
				str_dtor(&str);
			}
		}
		else
		{
			for(size_t j = 0; j < level + 1; j++)
			{
				printf("    ");
				printf("│");
			}

			printf("─ Type: %s\n", self->types[i]->typedef_.name);
		}
	}

	for(size_t i = 0; i < vec_getsize(self->functions); i++)
	{
		for(size_t j = 0; j < level + 1; j++)
		{
			printf("    ");
			printf("│");
		}

		if(self->functions[i].type)
		{
			struct Str str = erw_type_tostring(self->functions[i].type);
			printf("─ Function: %s (%s)\n", self->functions[i].name, str.data);
		}
		else
		{
			printf("─ Function: %s\n", self->functions[i].name);
		}
	}

	for(size_t i = 0; i < vec_getsize(self->variables); i++)
	{
		for(size_t j = 0; j < level + 1; j++)
		{
			printf("    ");
			printf("│");
		}

		struct Str str = erw_type_tostring(self->variables[i].type);
		printf("─ Variable: %s (%s)\n", self->variables[i].name, str.data);
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

