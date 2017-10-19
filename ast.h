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

#ifndef AST_H
#define AST_H

#include "tokenizer.h"
#include "vec.h"

struct ASTNodeType 
{
	const char* name;
};

extern const struct ASTNodeType* const ASTNODETYPE_START;
extern const struct ASTNodeType* const ASTNODETYPE_BLOCK;
extern const struct ASTNodeType* const ASTNODETYPE_ARRAY;
extern const struct ASTNodeType* const ASTNODETYPE_POINTER;
extern const struct ASTNodeType* const ASTNODETYPE_TYPECAST;
extern const struct ASTNodeType* const ASTNODETYPE_VAR_VALUE;
//extern const struct ASTNodeType* const ASTNODETYPE_FUNC_ARG;
extern const struct ASTNodeType* const ASTNODETYPE_FUNC_CALL;
extern const struct ASTNodeType* const ASTNODETYPE_FUNC_ARGS;
extern const struct ASTNodeType* const ASTNODETYPE_FUNC_RETURN;

struct ASTNode
{
	union
	{
		struct Token token;
		const struct ASTNodeType* descriptor;
	};

	Vec(struct ASTNode*) branches;
	struct ASTNode* parent;
	int istoken;
};

struct ASTNode* ast_newfromtoken(struct Token token);
struct ASTNode* ast_newfromnodetype(const struct ASTNodeType* type);
void ast_addbranch(struct ASTNode* root, struct ASTNode* branch);
void ast_print(struct ASTNode* ast);
void ast_dtor(struct ASTNode* ast);

#endif
