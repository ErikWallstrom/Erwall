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

#ifndef ERW_AST_H
#define ERW_AST_H

#include "erw_tokenizer.h"
#include "vec.h"

struct erw_ASTNodeType 
{
	const char* name;
};

extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_START;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_BLOCK;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_ARRAY;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_REFERENCE;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_VAR_VALUE;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_FUNC_CALL;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_FUNC_ARGS;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_FUNC_RETURN;

struct erw_ASTNode
{
	union
	{
		struct erw_Token token;
		const struct erw_ASTNodeType* descriptor;
	};

	Vec(struct erw_ASTNode*) branches;
	struct erw_ASTNode* parent;
	int istoken;
};

struct erw_ASTNode* erw_ast_newfromtoken(struct erw_Token token);
struct erw_ASTNode* erw_ast_newfromnodetype(const struct erw_ASTNodeType* type);
void erw_ast_addbranch(struct erw_ASTNode* root, struct erw_ASTNode* branch);
void erw_ast_print(struct erw_ASTNode* ast);
void erw_ast_dtor(struct erw_ASTNode* ast);

#endif
