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

#include "erw_ast.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>

const struct erw_ASTNodeType* const erw_ASTNODETYPE_START =
	&(struct erw_ASTNodeType){"Start"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_BLOCK = 
	&(struct erw_ASTNodeType){"Block"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_VAR_VALUE =
	&(struct erw_ASTNodeType){"Variable value"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_FUNC_ARG =
	&(struct erw_ASTNodeType){"Function Argument"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_FUNC_CALL =
	&(struct erw_ASTNodeType){"Function Call"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_FUNC_ARGS =
	&(struct erw_ASTNodeType){"Function Arguments"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_FUNC_RETURN = 
	&(struct erw_ASTNodeType){"Function Return"};

struct erw_ASTNode* erw_ast_newfromtoken(struct erw_Token token)
{
	struct erw_ASTNode* self = malloc(sizeof(struct erw_ASTNode));
	if(!self)
	{
		log_error("malloc failed, in <%s>", __func__);
	}

	self->branches = vec_ctor(struct erw_ASTNode*, 0);
	self->parent = NULL;
	self->token = token;
	self->istoken = 1;

	return self;
}

struct erw_ASTNode* erw_ast_newfromnodetype(const struct erw_ASTNodeType* type)
{
	struct erw_ASTNode* self = malloc(sizeof(struct erw_ASTNode));
	if(!self)
	{
		log_error("malloc failed, in <%s>", __func__);
	}

	self->branches = vec_ctor(struct erw_ASTNode*, 0);
	self->parent = NULL;
	self->descriptor = type;
	self->istoken = 0;

	return self;
}

void erw_ast_addbranch(struct erw_ASTNode* root, struct erw_ASTNode* branch)
{
	log_assert(root, "is NULL");
	log_assert(branch, "is NULL");

	branch->parent = root;
	vec_pushback(root->branches, branch);
}

static void erw_ast_printinternal(struct erw_ASTNode* ast, size_t level)
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
			erw_ast_printinternal(ast->branches[i], level + 1);
		}
	}
}

void erw_ast_print(struct erw_ASTNode* ast)
{
	log_assert(ast, "is NULL");
	erw_ast_printinternal(ast, 0);
}

inline
void erw_ast_dtor(struct erw_ASTNode* ast)
{
	log_assert(ast, "is NULL");
	for(size_t i = 0; i < vec_getsize(ast->branches); i++)
	{
		erw_ast_dtor(ast->branches[i]);
	}

	vec_dtor(ast->branches);
	free(ast);
}

