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
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_FUNCPROT;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_FUNCDEF;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_TYPEDECLR;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_VARDECLR;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_BLOCK;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_IF;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_ELSEIF;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_ELSE;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_RETURN;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_ASSIGNMENT;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_UNEXPR;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_BINEXPR;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_FUNCCALL;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_CAST;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_DEFER;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_WHILE;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_ENUM;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_ENUMMEMBER;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_STRUCT;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_UNION;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_UNSAFE;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_REFERENCE;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_ARRAY;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_SLICE;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_LITERAL;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_TYPE;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_FUNCTYPE;
extern const struct erw_ASTNodeType* const erw_ASTNODETYPE_ACCESS;

struct erw_ASTNode
{
	union
	{
		struct
		{
			Vec(struct erw_ASTNode*) children;
		} start;

		struct
		{
			Vec(struct erw_ASTNode*) params;
			struct erw_Token* name;
			struct erw_ASTNode* type;
			int foreign;
		} funcprot;

		struct
		{
			Vec(struct erw_ASTNode*) params;
			struct erw_Token* name;
			struct erw_ASTNode* type;
			struct erw_ASTNode* block;
		} funcdef;

		struct
		{
			struct erw_Token* name;
			struct erw_ASTNode* type;
		} typedeclr;

		struct
		{
			struct erw_Token* name;
			struct erw_ASTNode* type;
			struct erw_ASTNode* value;
			int mutable;
		} vardeclr;

		struct
		{
			Vec(struct erw_ASTNode*) stmts;
		} block;

		struct
		{
			Vec(struct erw_ASTNode*) elseifs;
			struct erw_ASTNode* expr;
			struct erw_ASTNode* block;
			struct erw_ASTNode* else_;
		} if_;

		struct
		{
			struct erw_ASTNode* expr;
			struct erw_ASTNode* block;
		} elseif;

		struct
		{
			struct erw_ASTNode* block;
		} else_;

		struct
		{
			struct erw_ASTNode* expr;
		} return_;

		struct
		{
			struct erw_ASTNode* assignee;
			struct erw_ASTNode* expr;
		} assignment;

		struct
		{
			struct erw_ASTNode* expr;
			int left; //Used to distinguish reference/dereference
		} unexpr;

		struct
		{
			struct erw_ASTNode* expr1;
			struct erw_ASTNode* expr2;
		} binexpr;

		struct
		{
			Vec(struct erw_ASTNode*) args;
			struct erw_Token* name;
		} funccall;

		struct
		{
			struct erw_ASTNode* type;
			struct erw_ASTNode* expr;
		} cast;

		struct
		{
			struct erw_ASTNode* block;
		} defer;

		struct
		{
			struct erw_ASTNode* expr;
			struct erw_ASTNode* block;
		} while_;

		struct //Add type?
		{
			Vec(struct erw_ASTNode*) members;
		} enum_;

		struct
		{
			struct erw_Token* name;
			struct erw_ASTNode* value;
		} enummember;

		struct
		{
			Vec(struct erw_ASTNode*) members;
		} struct_;

		struct
		{
			Vec(struct erw_ASTNode*) members;
		} union_;

		struct
		{
			struct erw_ASTNode* block;
		} unsafe;

		struct
		{
			struct erw_ASTNode* type;
		} reference;

		struct
		{
			struct erw_ASTNode* type;
			struct erw_ASTNode* size;
		} array;

		struct
		{
			struct erw_ASTNode* type;
		} slice;

		struct
		{
			Vec(struct erw_ASTNode*) params;
			struct erw_ASTNode* type;
		} functype;

		struct
		{
			struct erw_ASTNode* expr;
			struct erw_ASTNode* index;
		} access;
	};

	const struct erw_ASTNodeType* type;
	struct erw_Token* token;
};

struct erw_ASTNode* erw_ast_new(
	const struct erw_ASTNodeType* type, 
	struct erw_Token* token
);
void erw_ast_print(struct erw_ASTNode* ast);
void erw_ast_dtor(struct erw_ASTNode* ast);

#endif
