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

//TODO: Free up memory allocated by erw_scope_createtype etc

#include "erw_scope.h"
#include "erw_error.h"
#include "str.h"
#include "log.h"

#include <stdlib.h>

static struct erw_VarDeclr* erw_scope_findvar(
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
			if(!strcmp(scope->variables[i].node->vardeclr.name->text, name))
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

static struct erw_FuncDeclr* erw_scope_findfunc(
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
			if(!strcmp(scope->functions[i].node->funcdef.name->text, name))
			{
				return &scope->functions[i];
			}
		}

		scope = scope->parent;
	}

	return NULL;
}

static struct erw_TypeDeclr* erw_scope_findtype(
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
			if(!strcmp(scope->types[i].type->named.name, name))
			{
				return &scope->types[i];
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

	self->functions = vec_ctor(struct erw_FuncDeclr, 0);
	self->variables = vec_ctor(struct erw_VarDeclr, 0);
	self->types = vec_ctor(struct erw_TypeDeclr, 0);
	self->children = vec_ctor(struct erw_Scope*, 0);
	self->finalizers = vec_ctor(struct erw_Finalizer, 0);
	self->index = index;
	self->parent = parent;
	self->isfunction = isfunction;
	self->funcname = funcname;

	if(parent)
	{
		vec_pushback(parent->children, self);
	}

	return self;
}

struct erw_VarDeclr* erw_scope_getvar(
	struct erw_Scope* self, 
	struct erw_Token* token,
	struct Str* lines)
{
	log_assert(self, "is NULL");
	log_assert(token, "is NULL");
	log_assert(
		token->type == erw_TOKENTYPE_IDENT, 
		"invalid type (%s)", 
		token->type->name
	);
	log_assert(lines, "is NULL");

	struct erw_VarDeclr* ret = erw_scope_findvar(self, token->text);
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

struct erw_FuncDeclr* erw_scope_getfunc(
	struct erw_Scope* self, 
	struct erw_Token* token,
	struct Str* lines)
{
	log_assert(self, "is NULL");
	log_assert(token, "is NULL");
	log_assert(
		token->type == erw_TOKENTYPE_IDENT, 
		"invalid type (%s)", 
		token->type->name
	);
	log_assert(lines, "is NULL");

	struct erw_FuncDeclr* ret = erw_scope_findfunc(self, token->text);
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
	struct erw_Token* token,
	struct Str* lines)
{
	log_assert(self, "is NULL");
	log_assert(token, "is NULL");
	log_assert(
		token->type == erw_TOKENTYPE_TYPE, 
		"invalid type (%s)", 
		token->type->name
	);
	log_assert(lines, "is NULL");

	struct erw_TypeDeclr* ret = erw_scope_findtype(self, token->text);
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

	return ret->type;
}

struct erw_Type* erw_scope_createtype(
	struct erw_Scope* self, 
	struct erw_ASTNode* node,
	struct Str* lines)
{
	log_assert(self, "is NULL");
	log_assert(node, "is NULL");

	struct erw_Type* root = NULL;
	struct erw_Type* type = NULL;
	int done = 0;
	while(!done)
	{
		struct erw_Type* tmptype = NULL;
		if(node->type == erw_ASTNODETYPE_TYPE)
		{
			/*TODO: Copy instead of reference here, so that it is easier to
			  clean up memory */
			tmptype = erw_scope_gettype(self, node->token, lines);
			tmptype->named.used = 1;
			tmptype->parent = type;
			done = 1; //Break loop
		}
		else if(node->type == erw_ASTNODETYPE_REFERENCE)
		{
			tmptype = erw_type_new(erw_TYPEINFO_REFERENCE, type);
			tmptype->reference.mutable = 0; //NOTE: Temporary
			tmptype->reference.size = sizeof(void*); //NOTE: Temporary
			tmptype->parent = type;
		}
		else if(node->type == erw_ASTNODETYPE_ARRAY)
		{
			tmptype = erw_type_new(erw_TYPEINFO_ARRAY, type);
			tmptype->array.mutable = 0; //NOTE: Temporary
			tmptype->array.elements = atol(node->array.size->token->text); 
			tmptype->parent = type;
			//NOTE: No error checking
		}
		else if(node->type == erw_ASTNODETYPE_SLICE)
		{
			tmptype = erw_type_new(erw_TYPEINFO_SLICE, type);
			tmptype->slice.mutable = 0; //NOTE: Temporary
			tmptype->slice.size = sizeof(void*); //NOTE: Temporary
			tmptype->parent = type;
		}
		else
		{
			log_assert(0, "this should not happen (%s)", node->type->name);
		}

		node = node->reference.type;
		if(!root)
		{
			type = tmptype;
			root = type;
		}
		else
		{
			if(type->info == erw_TYPEINFO_ARRAY 
				&& tmptype->info != erw_TYPEINFO_ARRAY)
			{
				type->array.size = type->array.elements * tmptype->size;
				struct erw_Type* lasttype = type->parent;
				while(lasttype && lasttype->info == erw_TYPEINFO_ARRAY)
				{
					lasttype->array.size = lasttype->array.elements 
						* lasttype->size;
					lasttype = lasttype->parent;
				}
			}

			type->reference.type = tmptype;
			type = tmptype;
		}
	}

	return root;
}

void erw_scope_addvardeclr(
	struct erw_Scope* self, 
	struct erw_ASTNode* node,
	struct Str* lines)
{
	log_assert(self, "is NULL");
	log_assert(node, "is NULL");
	log_assert(
		node->type == erw_ASTNODETYPE_VARDECLR, 
		"invalid type (%s)", 
		node->type->name
	);
	log_assert(lines, "is NULL");

	struct erw_VarDeclr* var = erw_scope_findvar(
		self, 
		node->vardeclr.name->text
	);
	if(var)
	{
		struct Str msg;
		str_ctorfmt(
			&msg,
			"Redefinition of variable ('%s') declared at line %zu, column %zu", 
			node->vardeclr.name->text,
			var->node->vardeclr.name->linenum,
			var->node->vardeclr.name->column
		);

		erw_error(
			msg.data, 
			lines[node->vardeclr.name->linenum - 1].data, 
			node->vardeclr.name->linenum, 
			node->vardeclr.name->column,
			node->vardeclr.name->column + 
				vec_getsize(node->vardeclr.name->text) - 2
		);
		str_dtor(&msg);
	}

	struct erw_FuncDeclr* func = erw_scope_findfunc(
		self, 
		node->vardeclr.name->text
	);
	if(func)
	{
		struct Str msg;
		str_ctorfmt(
			&msg,
			"Redefinition of variable ('%s') declared at line %zu, column %zu", 
			node->vardeclr.name->text,
			func->node->funcdef.name->linenum,
			func->node->funcdef.name->column
		);

		erw_error(
			msg.data, 
			lines[node->vardeclr.name->linenum - 1].data, 
			node->vardeclr.name->linenum, 
			node->vardeclr.name->column,
			node->vardeclr.name->column + 
				vec_getsize(node->vardeclr.name->text) - 2
		);
		str_dtor(&msg);
	}

	struct erw_VarDeclr symbol;
	symbol.type = erw_scope_createtype(self, node->vardeclr.type, lines);
	symbol.node = node;
	symbol.used = 0;
	vec_pushback(self->variables, symbol);
}

void erw_scope_addfuncdeclr(
	struct erw_Scope* self, 
	struct erw_ASTNode* node,
	struct Str* lines)
{
	log_assert(self, "is NULL");
	log_assert(node, "is NULL");
	log_assert(
		node->type == erw_ASTNODETYPE_FUNCDEF, 
		"invalid type (%s)", 
		node->type->name
	);
	log_assert(lines, "is NULL");

	struct erw_FuncDeclr* func = erw_scope_findfunc(
		self, 
		node->funcdef.name->text
	);
	if(func)
	{
		struct Str msg;
		str_ctorfmt(
			&msg,
			"Redefinition of function ('%s') declared at line %zu, column %zu", 
			node->funcdef.name->text,
			func->node->funcdef.name->linenum,
			func->node->funcdef.name->column
		);

		erw_error(
			msg.data, 
			lines[node->funcdef.name->linenum - 1].data, 
			node->funcdef.name->linenum, 
			node->funcdef.name->column,
			node->funcdef.name->column + 
				vec_getsize(node->funcdef.name->text) - 2
		);
		str_dtor(&msg);
	}

	struct erw_VarDeclr* var = erw_scope_findvar(
		self, 
		node->funcdef.name->text
	);
	if(var)
	{
		struct Str msg;
		str_ctorfmt(
			&msg,
			"Redefinition of function ('%s') declared at line %zu, column %zu", 
			node->funcdef.name->text,
			var->node->vardeclr.name->linenum,
			var->node->vardeclr.name->column
		);

		erw_error(
			msg.data, 
			lines[node->funcdef.name->linenum - 1].data, 
			node->funcdef.name->linenum, 
			node->funcdef.name->column,
			node->funcdef.name->column + 
				vec_getsize(node->funcdef.name->text) - 2
		);
		str_dtor(&msg);
	}

	struct erw_FuncDeclr symbol;
	if(node->funcdef.type)
	{
		symbol.type = erw_scope_createtype(self, node->funcdef.type, lines);
	}
	else
	{
		symbol.type = NULL;
	}

	symbol.node = node;
	symbol.used = 0;
	vec_pushback(self->functions, symbol);
}

void erw_scope_addtypedeclr(
	struct erw_Scope* self, 
	struct erw_ASTNode* node,
	struct Str* lines)
{
	log_assert(self, "is NULL");
	log_assert(node, "is NULL");
	log_assert(
		node->type == erw_ASTNODETYPE_TYPEDECLR, 
		"invalid type (%s)", 
		node->type->name
	);
	log_assert(lines, "is NULL");

	struct erw_TypeDeclr* type = erw_scope_findtype(
		self, 
		node->typedeclr.name->text
	);
	if(type)
	{
		struct Str msg;
		str_ctorfmt(
			&msg,
			"Redefinition of type ('%s') declared at line %zu, column %zu", 
			node->typedeclr.name->text,
			type->node->token->linenum,
			type->node->token->column
		);

		erw_error(
			msg.data, 
			lines[node->typedeclr.name->linenum - 1].data, 
			node->typedeclr.name->linenum, 
			node->typedeclr.name->column,
			node->typedeclr.name->column + 
				vec_getsize(node->typedeclr.name->text) - 2
		);
		str_dtor(&msg);
	}

	struct erw_TypeDeclr symbol;
	symbol.node = node;
	symbol.type = erw_type_new(erw_TYPEINFO_NAMED, NULL);
	symbol.type->named.name = node->typedeclr.name->text;
	symbol.type->named.used = 0;

	if(!node->typedeclr.type)
	{
		symbol.type->named.type = erw_type_new(erw_TYPEINFO_EMPTY, NULL);
		symbol.type->named.size = 0; //Should this be 1?
	}
	else if(node->typedeclr.type->type == erw_ASTNODETYPE_TYPE)
	{
		struct erw_Type* newtype = erw_scope_gettype(
			self, 
			node->typedeclr.type->token, 
			lines
		);

		symbol.type->named.type = newtype;
		symbol.type->named.size = newtype->size;
	}
	else if(node->typedeclr.type->type == erw_ASTNODETYPE_STRUCT)
	{
		struct erw_Type* newtype = erw_type_new(erw_TYPEINFO_STRUCT, NULL);
		for(size_t i = 0; 
			i < vec_getsize(node->typedeclr.type->struct_.members);
			i++)
		{
			struct erw_TypeStructMember member = {
				.type = erw_scope_createtype(
					self, 
					node->typedeclr.type->struct_.members[i]->vardeclr.type,
					lines
				),
				.name = node->typedeclr.type->struct_.members[i]->vardeclr.name
					->text
			};

			for(size_t j = 0; j < vec_getsize(newtype->struct_.members); j++)
			{
				if(!strcmp(newtype->struct_.members[j].name, member.name))
				{
					struct Str msg;
					str_ctorfmt(
						&msg,
						"Redefinition of struct member ('%s') declared at line"
							" %zu, column %zu", 
						member.name,
						node->typedeclr.type->struct_.members[j]->vardeclr.name
							->linenum,
						node->typedeclr.type->struct_.members[j]->vardeclr.name
							->column
					);

					erw_error(
						msg.data, 
						lines[node->typedeclr.type->struct_.members[i]->vardeclr
							.name->linenum - 1 ].data, 
						node->typedeclr.type->struct_.members[i]->vardeclr
							.name->linenum, 
						node->typedeclr.type->struct_.members[i]->vardeclr
							.name->column,
						node->typedeclr.type->struct_.members[i]->vardeclr
							.name->column 
							+ vec_getsize(
								node->typedeclr.type->struct_.members[i]
									->vardeclr.name->text) - 2
					);
					str_dtor(&msg);
				}
			}

			newtype->size += member.type->size;
			vec_pushback(newtype->struct_.members, member);
		}

		symbol.type->named.type = newtype;
		symbol.type->named.size = newtype->size;
	}
	else if(node->typedeclr.type->type == erw_ASTNODETYPE_UNION)
	{
		struct erw_Type* newtype = erw_type_new(erw_TYPEINFO_UNION, NULL);
		size_t largestsize = 0;
		for(size_t i = 0; 
			i < vec_getsize(node->typedeclr.type->union_.members);
			i++)
		{
			struct erw_Type* tmptype = erw_scope_createtype(
				self, 
				node->typedeclr.type->union_.members[i],
				lines
			);

			for(size_t j = 0; j < vec_getsize(newtype->union_.members); j++)
			{
				if(erw_type_compare(tmptype, newtype->union_.members[j]))
				{
					struct Str msg;
					struct Str str = erw_type_tostring(tmptype);
					str_ctorfmt(
						&msg,
						"Redefinition of struct member ('%s') declared at line"
							" %zu, column %zu", 
						str.data,
						node->typedeclr.type->union_.members[j]->token->linenum,
						node->typedeclr.type->union_.members[j]->token->column
					);

					erw_error(
						msg.data, 
						lines[node->typedeclr.type->union_.members[i]->token
							->linenum - 1].data, 
						node->typedeclr.type->union_.members[i]->token->linenum, 
						node->typedeclr.type->union_.members[i]->token->column,
						node->typedeclr.type->union_.members[i]->token->column 
							+ vec_getsize(
								node->typedeclr.type->union_.members[i]->token
									->text) - 2
					);

					str_dtor(&str);
					str_dtor(&msg);
				}
			}

			if(tmptype->size > largestsize)
			{
				largestsize = tmptype->size;
			}

			vec_pushback(newtype->union_.members, tmptype);
		}

		newtype->union_.size = largestsize;
		symbol.type->named.type = newtype;
		symbol.type->named.size = newtype->size;
	}
	else if(node->typedeclr.type->type == erw_ASTNODETYPE_ENUM)
	{
		struct erw_Type* newtype = erw_type_new(erw_TYPEINFO_ENUM, NULL);
		newtype->enum_.size = sizeof(int); //NOTE: Temporary

		size_t defaultvalue = 0;
		for(size_t i = 0; 
			i < vec_getsize(node->typedeclr.type->enum_.members);
			i++)
		{
			struct erw_TypeEnumMember member;
			member.name = node->typedeclr.type->enum_.members[i]->enummember
				.name->text;
			for(size_t j = 0; j < vec_getsize(newtype->enum_.members); j++)
			{
				if(!strcmp(newtype->enum_.members[j].name, member.name))
				{
					struct Str msg;
					str_ctorfmt(
						&msg,
						"Redefinition of enum member ('%s') declared at line"
							" %zu, column %zu", 
						member.name,
						node->typedeclr.type->enum_.members[j]->enummember.name
							->linenum,
						node->typedeclr.type->enum_.members[j]->enummember.name
							->column
					);

					erw_error(
						msg.data, 
						lines[node->typedeclr.type->enum_.members[i]->enummember
							.name->linenum - 1].data, 
						node->typedeclr.type->enum_.members[i]->enummember.name
							->linenum, 
						node->typedeclr.type->enum_.members[i]->enummember.name
							->column, 
						node->typedeclr.type->enum_.members[i]->enummember.name
							->column + vec_getsize(
								node->typedeclr.type->enum_.members[i]
									->enummember.name->text) - 2
					);

					str_dtor(&msg);
				}

				if(node->typedeclr.type->enum_.members[i]->enummember.value)
				{
					if(node->typedeclr.type->enum_.members[j]->enummember.value)
					{
						if(!strcmp(
							node->typedeclr.type->enum_.members[i]->enummember
								.value->token->text,
							node->typedeclr.type->enum_.members[j]->enummember
								.value->token->text))
						{
							struct Str msg;
							str_ctorfmt(
								&msg,
								"Enum member with same the value as '%s'"
									" declared at line %zu, column %zu", 
								node->typedeclr.type->enum_.members[i]
									->enummember.name->text,
								node->typedeclr.type->enum_.members[j]
									->enummember.name->linenum,
								node->typedeclr.type->enum_.members[j]
									->enummember .name->column
							);

							erw_error(
								msg.data, 
								lines[node->typedeclr.type->enum_.members[i]
									->enummember.name->linenum - 1].data, 
								node->typedeclr.type->enum_.members[i]
									->enummember.name->linenum, 
								node->typedeclr.type->enum_.members[i]
									->enummember.name->column, 
								node->typedeclr.type->enum_.members[i]
									->enummember.name->column + vec_getsize(
										node->typedeclr.type->enum_.members[i]
											->enummember.name->text) - 2
							);

							str_dtor(&msg);
						}
					}
				}
			}

			if(node->typedeclr.type->enum_.members[i]->enummember.value)
			{
				size_t value = atol(
					node->typedeclr.type->enum_.members[i]->enummember.value
						->token->text
				);

				if(value < defaultvalue) //Should be allowed happen?
				{
					struct Str msg;
					str_ctorfmt(
						&msg,
						"Too small value given to enum member '%s' (%zu)",
						member.name,
						value
					);

					erw_error(
						msg.data, 
						lines[node->typedeclr.type->enum_.members[i]
							->enummember.name->linenum - 1].data, 
						node->typedeclr.type->enum_.members[i]
							->enummember.name->linenum, 
						node->typedeclr.type->enum_.members[i]
							->enummember.name->column, 
						node->typedeclr.type->enum_.members[i]
							->enummember.name->column + vec_getsize(
								node->typedeclr.type->enum_.members[i]
									->enummember.name->text) - 2
					);

					str_dtor(&msg);
				}
				else
				{
					defaultvalue = value;
				}
			}

			member.value = defaultvalue;
			defaultvalue++;
			vec_pushback(newtype->enum_.members, member);
		}

		symbol.type->named.type = newtype;
		symbol.type->named.size = newtype->size;
	}
	else
	{
		log_assert(0, "this should not happen (%s)", node->type->name);
	}

	vec_pushback(self->types, symbol);
}

static void erw_scope_printinternal(
	struct erw_Scope* self, 
	size_t level, 
	struct Str* lines)
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
		for(size_t j = 0; j < level + 1; j++)
		{
			printf("    ");
			printf("│");
		}

		struct Str str = erw_type_tostring(self->types[i].type->named.type);
		printf(
			"─ Type: %s (%s)\n", 
			self->types[i].type->named.name, 
			str.data
		);

		if(self->types[i].type->named.type->info == erw_TYPEINFO_STRUCT)
		{
			for(size_t j = 0; 
				j < vec_getsize(
					self->types[i].type->named.type->struct_.members
				);
				j++)
			{
				for(size_t k = 0; k < level + 2; k++)
				{
					printf("    ");
					printf("│");
				}

				struct Str str1 = erw_type_tostring(
					self->types[i].type->named.type->struct_.members[j].type
				);
				printf(
					"─ Struct member: %s (%s)\n", 
					self->types[i].type->named.type->struct_.members[j].name,
					str1.data
				);
				str_dtor(&str1);
			}
		}
		else if(self->types[i].type->named.type->info == erw_TYPEINFO_UNION)
		{
			for(size_t j = 0; 
				j < vec_getsize(
					self->types[i].type->named.type->union_.members
				);
				j++)
			{
				for(size_t k = 0; k < level + 2; k++)
				{
					printf("    ");
					printf("│");
				}

				struct Str str1 = erw_type_tostring(
					self->types[i].type->named.type->union_.members[j]
				);

				printf("─ Union member: %s\n", str1.data);
				str_dtor(&str1);
			}
		}
		else if(self->types[i].type->named.type->info == erw_TYPEINFO_ENUM)
		{
			for(size_t j = 0; 
				j < vec_getsize(
					self->types[i].type->named.type->enum_.members
				);
				j++)
			{
				for(size_t k = 0; k < level + 2; k++)
				{
					printf("    ");
					printf("│");
				}

				
				printf(
					"─ Enum member: %s (%zu)\n", 
					self->types[i].type->named.type->enum_.members[j].name,
					self->types[i].type->named.type->enum_.members[j].value
				);
			}
		}

		str_dtor(&str);
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
			printf(
				"─ Function: %s (%s)\n", 
				self->functions[i].node->funcdef.name->text, 
				str.data
			);
			str_dtor(&str);
		}
		else
		{
			printf(
				"─ Function: %s\n", 
				self->functions[i].node->funcdef.name->text
			);
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
		printf(
			"─ Variable: %s (%s)\n", 
			self->variables[i].node->vardeclr.name->text, 
			str.data
		);
		str_dtor(&str);
	}

	for(size_t i = 0; i < vec_getsize(self->children); i++)
	{
		erw_scope_printinternal(self->children[i], level + 1, lines);
	}
}

void erw_scope_print(struct erw_Scope* self, struct Str* lines)
{
	log_assert(self, "is NULL");
	log_assert(lines, "is NULL");
	erw_scope_printinternal(self, 0, lines);
}

void erw_scope_dtor(struct erw_Scope* self)
{
	log_assert(self, "is NULL");
	for(size_t i = 0; i < vec_getsize(self->types); i++)
	{
		if(self->types[i].node) //Check if type is builtin
		{
			erw_type_dtor(self->types[i].type);
		}
	}

	vec_dtor(self->types);
	vec_dtor(self->functions);
	vec_dtor(self->variables);

	for(size_t i = 0; i < vec_getsize(self->children); i++)
	{
		erw_scope_dtor(self->children[i]);
	}

	vec_dtor(self->children);
	free(self);
}

