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

#include "ast.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>

const struct ASTNodeType* const ASTNODETYPE_START = 		&(struct ASTNodeType){"Start"};
const struct ASTNodeType* const ASTNODETYPE_BLOCK = 		&(struct ASTNodeType){"Block"};
const struct ASTNodeType* const ASTNODETYPE_ARRAY = 		&(struct ASTNodeType){"Array"};
const struct ASTNodeType* const ASTNODETYPE_POINTER = 		&(struct ASTNodeType){"Pointer"};
const struct ASTNodeType* const ASTNODETYPE_TYPECAST = 		&(struct ASTNodeType){"Type Cast"};
const struct ASTNodeType* const ASTNODETYPE_FUNC_ARG = 		&(struct ASTNodeType){"Function Argument"};
const struct ASTNodeType* const ASTNODETYPE_FUNC_CALL = 	&(struct ASTNodeType){"Function Call"};
const struct ASTNodeType* const ASTNODETYPE_FUNC_ARGS = 	&(struct ASTNodeType){"Function Arguments"};
const struct ASTNodeType* const ASTNODETYPE_FUNC_RETURN = 	&(struct ASTNodeType){"Function Return"};

struct ASTNode* ast_newfromtoken(struct Token token)
{
	struct ASTNode* self = malloc(sizeof(struct ASTNode));
	if(!self)
	{
		log_error("malloc failed, in <%s>", __func__);
	}

	self->branches = vec_ctor(struct ASTNode*, 0);
	self->parent = NULL;
	self->token = token;
	self->istoken = 1;

	return self;
}

struct ASTNode* ast_newfromnodetype(const struct ASTNodeType* type)
{
	struct ASTNode* self = malloc(sizeof(struct ASTNode));
	if(!self)
	{
		log_error("malloc failed, in <%s>", __func__);
	}

	self->branches = vec_ctor(struct ASTNode*, 0);
	self->parent = NULL;
	self->descriptor = type;
	self->istoken = 0;

	return self;
}

void ast_addbranch(struct ASTNode* root, struct ASTNode* branch)
{
	log_assert(root, "is NULL");
	log_assert(branch, "is NULL");

	branch->parent = root;
	vec_pushback(root->branches, branch);
}

static void ast_printinternal(struct ASTNode* ast, size_t level)
{
	if(ast != NULL)
	{
		for(size_t i = 0; i < level; i++)
		{
			printf("    ");
			printf("│");
		}

		if(ast->istoken)
		{
			printf("─ %s (%s)\n", ast->token.type->name, ast->token.text);
		}
		else
		{
			printf("─ %s\n", ast->descriptor->name);
		}

		for(size_t i = 0; i < vec_getsize(ast->branches); i++)
		{
			ast_printinternal(ast->branches[i], level + 1);
		}
	}
}

void ast_print(struct ASTNode* ast)
{
	log_assert(ast, "is NULL");
	ast_printinternal(ast, 0);
}

inline
void ast_dtor(struct ASTNode* ast)
{
	log_assert(ast, "is NULL");
	for(size_t i = 0; i < vec_getsize(ast->branches); i++)
	{
		ast_dtor(ast->branches[i]);
	}

	vec_dtor(ast->branches);
	free(ast);
}

