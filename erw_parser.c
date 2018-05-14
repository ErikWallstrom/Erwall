/* Copyright (C) 2017 Erik Wallström

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

#include "erw_parser.h"
#include "log.h"
#include "erw_ast.h"
#include "erw_error.h"

//TODO: !!!!!!!!!!!!!!!!!!!!!!!!!! erw_parse_funcprot
struct erw_Parser
{
	Vec(struct erw_Token) tokens;
	Vec(struct Str) lines;
	size_t current;
};

static int erw_parser_check(
	struct erw_Parser* parser,
	const struct erw_TokenType* type)
{
	if(parser->current < vec_getsize(parser->tokens))
	{
		if(parser->tokens[parser->current].type == type)
		{
			return 1;
		}
	}
	else
	{
		struct Str msg;
		str_ctorfmt(
			&msg,
			"Expected %s, reached EOF",
			type->name
		);

		erw_error(
			msg.data,
			parser->lines[parser->tokens[parser->current].linenum - 1].data,
			parser->tokens[parser->current].linenum,
			parser->tokens[parser->current].column,
			parser->tokens[parser->current].column
		);
		str_dtor(&msg);
	}

	return 0;
}

static struct erw_Token* erw_parser_expect(
	struct erw_Parser* parser,
	const struct erw_TokenType* type)
{
	if(!erw_parser_check(parser, type))
	{
		struct Str msg;
		str_ctorfmt(
			&msg,
			"Expected %s, got %s",
			type->name,
			parser->tokens[parser->current].type->name
		);

		erw_error(
			msg.data,
			parser->lines[parser->tokens[parser->current].linenum - 1].data,
			parser->tokens[parser->current].linenum,
			parser->tokens[parser->current].column,
			parser->tokens[parser->current].column 
				+ vec_getsize(parser->tokens[parser->current].text) - 2
		);
		str_dtor(&msg);
	}

	struct erw_Token* ret = &parser->tokens[parser->current];
	parser->current++;
	return ret;
}

static struct erw_ASTNode* erw_parse_expr(struct erw_Parser* parser);
static struct erw_ASTNode* erw_parse_funccall(struct erw_Parser* parser);
static struct erw_ASTNode* erw_parse_type(struct erw_Parser* parser);
static struct erw_ASTNode* erw_parse_factor(struct erw_Parser* parser)
{ 
	struct erw_ASTNode* node = NULL;
	if(erw_parser_check(parser, erw_TOKENTYPE_LITERAL_BOOL) 
		|| erw_parser_check(parser, erw_TOKENTYPE_LITERAL_CHAR) 
		|| erw_parser_check(parser, erw_TOKENTYPE_LITERAL_INT) 
		|| erw_parser_check(parser, erw_TOKENTYPE_LITERAL_FLOAT) 
		|| erw_parser_check(parser, erw_TOKENTYPE_LITERAL_STRING))
	{ 
		node = erw_ast_new(
			erw_ASTNODETYPE_LITERAL, 
			&parser->tokens[parser->current]
		);
		parser->current++;
	}
	else if(erw_parser_check(parser, erw_TOKENTYPE_IDENT))
	{ 
		if(parser->tokens[parser->current + 1].type 
			== erw_TOKENTYPE_LPAREN)
		{ 
			node = erw_parse_funccall(parser);
		}
		else if(parser->tokens[parser->current + 1].type 
			== erw_TOKENTYPE_OPERATOR_BITAND)
		{ 
			struct erw_ASTNode* identnode = erw_ast_new(
				erw_ASTNODETYPE_LITERAL, //Is this really correct?
				erw_parser_expect(parser, erw_TOKENTYPE_IDENT)
			);

			node = erw_ast_new(
				erw_ASTNODETYPE_UNEXPR, 
				erw_parser_expect(parser, erw_TOKENTYPE_OPERATOR_BITAND)
			);

			node->unexpr.expr = identnode;
			node->unexpr.left = 0;
		}
		else
		{ 
			node = erw_ast_new(
				erw_ASTNODETYPE_LITERAL, //Is this really correct?
				erw_parser_expect(parser, erw_TOKENTYPE_IDENT)
			);
		}
	}
	else if(erw_parser_check(parser, erw_TOKENTYPE_LPAREN))
	{ 
		erw_parser_expect(parser, erw_TOKENTYPE_LPAREN);
		node = erw_parse_expr(parser);
		erw_parser_expect(parser, erw_TOKENTYPE_RPAREN);
	}
	else if(erw_parser_check(parser, erw_TOKENTYPE_KEYWORD_CAST))
	{ 
		node = erw_ast_new(
			erw_ASTNODETYPE_CAST,
			erw_parser_expect(parser, erw_TOKENTYPE_KEYWORD_CAST)
		);

		erw_parser_expect(parser, erw_TOKENTYPE_LPAREN);
		node->cast.type = erw_parse_type(parser);
		erw_parser_expect(parser, erw_TOKENTYPE_COMMA);

		node->cast.expr = erw_parse_expr(parser);
		erw_parser_expect(parser, erw_TOKENTYPE_RPAREN);
	}
	else if(erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_BITAND))
	{
		node = erw_ast_new(
			erw_ASTNODETYPE_UNEXPR, 
			erw_parser_expect(parser, erw_TOKENTYPE_OPERATOR_BITAND)
		);
		node->unexpr.expr = erw_ast_new(
			erw_ASTNODETYPE_LITERAL, //Is this really correct?
			erw_parser_expect(parser, erw_TOKENTYPE_IDENT)
		);
		node->unexpr.left = 1;
	}
	else
	{ 
		struct Str msg;
		str_ctorfmt(
			&msg,
			"Unexpected %s",
			parser->tokens[parser->current].type->name
		);

		erw_error(
			msg.data, 
			parser->lines[parser->tokens[parser->current].linenum - 1].data, 
			parser->tokens[parser->current].linenum, 
			parser->tokens[parser->current].column,
			parser->tokens[parser->current].column 
				+ vec_getsize(parser->tokens[parser->current].text) - 2
		);
		str_dtor(&msg);
	}

	return node;
}

static struct erw_ASTNode* erw_parse_exponent(struct erw_Parser* parser)
{ 
	struct erw_ASTNode* exponode = erw_parse_factor(parser);
	while(erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_POW))
	{
		struct erw_ASTNode* oldnode = exponode;
		exponode = erw_ast_new(
			erw_ASTNODETYPE_BINEXPR,
			erw_parser_expect(parser, erw_TOKENTYPE_OPERATOR_POW)
		);

		exponode->binexpr.expr1 = oldnode;
		struct erw_ASTNode* newnode = erw_parse_factor(parser);
		exponode->binexpr.expr2 = newnode;
	}

	return exponode;
}

static struct erw_ASTNode* erw_parse_sign(struct erw_Parser* parser)
{ 
	struct erw_ASTNode* signnode;
	if(erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_SUB) 
		|| erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_NOT))
	{
		signnode = erw_ast_new(
			erw_ASTNODETYPE_UNEXPR,
			&parser->tokens[parser->current]
		);
		parser->current++;
		signnode->unexpr.expr = erw_parse_exponent(parser);
	}
	else
	{
		signnode = erw_parse_exponent(parser);
	}

	return signnode;
}

static struct erw_ASTNode* erw_parse_term(struct  erw_Parser* parser)
{ 
	struct erw_ASTNode* termnode = erw_parse_sign(parser);
	while(erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_MUL) 
		|| erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_DIV) 
		|| erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_MOD))
	{
		struct erw_ASTNode* oldnode = termnode;
		termnode = erw_ast_new(
			erw_ASTNODETYPE_BINEXPR,
			&parser->tokens[parser->current]
		);
		parser->current++;

		termnode->binexpr.expr1 = oldnode;
		struct erw_ASTNode* newnode = erw_parse_sign(parser);
		termnode->binexpr.expr2 = newnode;
	}

	return termnode;
}

static struct erw_ASTNode* erw_parse_plusminus(struct erw_Parser* parser)
{ 
	struct erw_ASTNode* pmnode = erw_parse_term(parser);
	while(erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_ADD) ||
		erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_SUB))
	{
		struct erw_ASTNode* oldnode = pmnode;
		pmnode = erw_ast_new(
			erw_ASTNODETYPE_BINEXPR,
			&parser->tokens[parser->current]
		);
		parser->current++;

		pmnode->binexpr.expr1 = oldnode;
		struct erw_ASTNode* newnode = erw_parse_term(parser);
		pmnode->binexpr.expr2 = newnode;
	}

	return pmnode;
}

static struct erw_ASTNode* erw_parse_comparison(struct erw_Parser* parser)
{ 
	struct erw_ASTNode* cmpnode = erw_parse_plusminus(parser);
	while(erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_LESS) 
		|| erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_LESSOREQUAL) 
		|| erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_GREATER) 
		|| erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_GREATEROREQUAL))
	{ 
		struct erw_ASTNode* oldnode = cmpnode;
		cmpnode = erw_ast_new(
			erw_ASTNODETYPE_BINEXPR,
			&parser->tokens[parser->current]
		);
		parser->current++;

		cmpnode->binexpr.expr1 = oldnode;
		struct erw_ASTNode* newnode = erw_parse_plusminus(parser);
		cmpnode->binexpr.expr2 = newnode;
	}

	return cmpnode;
}

static struct erw_ASTNode* erw_parse_equality(struct erw_Parser* parser)
{ 
	struct erw_ASTNode* eqnode = erw_parse_comparison(parser);
	while(erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_EQUAL) ||
		erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_NOTEQUAL))
	{ 
		struct erw_ASTNode* oldnode = eqnode;
		eqnode = erw_ast_new(
			erw_ASTNODETYPE_BINEXPR,
			&parser->tokens[parser->current]
		);
		parser->current++;

		eqnode->binexpr.expr1 = oldnode;
		struct erw_ASTNode* newnode = erw_parse_comparison(parser);
		eqnode->binexpr.expr2 = newnode;
	}

	return eqnode;
}

static struct erw_ASTNode* erw_parse_andor(struct erw_Parser* parser)
{ 
	struct erw_ASTNode* andornode = erw_parse_equality(parser);
	while(erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_AND) ||
		erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_OR))
	{ 
		struct erw_ASTNode* oldnode = andornode;
		andornode = erw_ast_new(
			erw_ASTNODETYPE_BINEXPR,
			&parser->tokens[parser->current]
		);
		parser->current++;

		andornode->binexpr.expr1 = oldnode;
		struct erw_ASTNode* newnode = erw_parse_equality(parser);
		andornode->binexpr.expr2 = newnode;
	}

	return andornode;
}

static struct erw_ASTNode* erw_parse_expr(struct erw_Parser* parser)
{
	return erw_parse_andor(parser);
}

static struct erw_ASTNode* erw_parse_funccall(struct erw_Parser* parser)
{
	struct erw_ASTNode* node = erw_ast_new(erw_ASTNODETYPE_FUNCCALL, NULL);
	node->funccall.name = erw_parser_expect(parser, erw_TOKENTYPE_IDENT);
	node->token = node->funccall.name;
	
	erw_parser_expect(parser, erw_TOKENTYPE_LPAREN);
	int first = 1;
	while(!erw_parser_check(parser, erw_TOKENTYPE_RPAREN))
	{
		if(first)
		{
			first = 0;
		}
		else
		{
			erw_parser_expect(parser, erw_TOKENTYPE_COMMA);
		}

		vec_pushback(node->funccall.args, erw_parse_expr(parser));
		if(!erw_parser_check(parser, erw_TOKENTYPE_COMMA))
		{
			break;
		}
	}

	erw_parser_expect(parser, erw_TOKENTYPE_RPAREN);
	return node;
}

static struct erw_ASTNode* erw_parse_type(struct erw_Parser* parser)
{ 
	struct erw_ASTNode* root = NULL;
	struct erw_ASTNode* node = NULL;
	int done = 0;
	while(!done)
	{ 
		struct erw_ASTNode* tmpnode = NULL;
		if(erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_BITAND))
		{ 
			erw_parser_expect(parser, erw_TOKENTYPE_OPERATOR_BITAND);
			tmpnode = erw_ast_new(erw_ASTNODETYPE_REFERENCE, NULL);
		}
		else if(erw_parser_check(parser, erw_TOKENTYPE_LBRACKET))
		{ 
			erw_parser_expect(parser, erw_TOKENTYPE_LBRACKET);
			if(erw_parser_check(parser, erw_TOKENTYPE_LITERAL_INT))
			{
				tmpnode = erw_ast_new(erw_ASTNODETYPE_ARRAY, NULL);
				//Parse expression?
				tmpnode->array.size = erw_ast_new(
					erw_ASTNODETYPE_LITERAL,
					erw_parser_expect(parser, erw_TOKENTYPE_LITERAL_INT)
				);
			}
			else
			{
				tmpnode = erw_ast_new(erw_ASTNODETYPE_SLICE, NULL);
			}

			erw_parser_expect(parser, erw_TOKENTYPE_RBRACKET);
		}
		else if(erw_parser_check(parser, erw_TOKENTYPE_TYPE))
		{ 
			tmpnode = erw_ast_new(
				erw_ASTNODETYPE_TYPE, 
				erw_parser_expect(parser, erw_TOKENTYPE_TYPE)
			);
			done = 1; //Break loop
		}
		else
		{
			struct Str msg;
			str_ctorfmt(
				&msg,
				"Expected '&', '[', or %s. Got %s",
				erw_TOKENTYPE_TYPE->name,
				parser->tokens[parser->current].type->name
			);

			erw_error(
				msg.data, 
				parser->lines[parser->tokens[parser->current]
					.linenum - 1].data, 
				parser->tokens[parser->current].linenum, 
				parser->tokens[parser->current].column,
				parser->tokens[parser->current].column + 
					vec_getsize(parser->tokens[parser->current].text) - 2
			);
			str_dtor(&msg);
		}

		if(!root)
		{
			node = tmpnode;
			root = node;
		}
		else
		{
			node->reference.type = tmpnode;
			node = tmpnode;
		}
	}

	return root;
}

static struct erw_ASTNode* erw_parse_vardeclr(struct erw_Parser* parser)
{
	struct erw_ASTNode* node = NULL;
	if(erw_parser_check(parser, erw_TOKENTYPE_KEYWORD_LET))
	{
		node = erw_ast_new(
			erw_ASTNODETYPE_VARDECLR,
			erw_parser_expect(parser, erw_TOKENTYPE_KEYWORD_LET)
		);
		node->vardeclr.mutable = 0;
	}
	else if(erw_parser_check(parser, erw_TOKENTYPE_KEYWORD_MUT))
	{
		node = erw_ast_new(
			erw_ASTNODETYPE_VARDECLR,
			erw_parser_expect(parser, erw_TOKENTYPE_KEYWORD_MUT)
		);
		node->vardeclr.mutable = 1;
	}
	else
	{
		struct Str msg;
		str_ctorfmt(
			&msg,
			"Expected %s or %s, got %s",
			erw_TOKENTYPE_KEYWORD_LET->name,
			erw_TOKENTYPE_KEYWORD_MUT->name,
			parser->tokens[parser->current].type->name
		);

		erw_error(
			msg.data, 
			parser->lines[parser->tokens[parser->current].linenum - 1].data, 
			parser->tokens[parser->current].linenum, 
			parser->tokens[parser->current].column,
			parser->tokens[parser->current].column 
				+ vec_getsize(parser->tokens[parser->current].text) - 2
		);
		str_dtor(&msg);
	}

	node->vardeclr.name = erw_parser_expect(parser, erw_TOKENTYPE_IDENT);
	erw_parser_expect(parser, erw_TOKENTYPE_OPERATOR_DECLR);
	node->vardeclr.type = erw_parse_type(parser);

	if(erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_ASSIGN))
	{ 
		erw_parser_expect(parser, erw_TOKENTYPE_OPERATOR_ASSIGN);
		node->vardeclr.value = erw_parse_expr(parser);
	}

	return node;
}

static void erw_parse_varlist(
	struct erw_Parser* parser, 
	Vec(struct erw_ASTNode*) varlist)
{
	erw_parser_expect(parser, erw_TOKENTYPE_LPAREN);
	int first = 1;
	while(!erw_parser_check(parser, erw_TOKENTYPE_RPAREN))
	{
		if(first)
		{
			first = 0;
		}
		else
		{
			erw_parser_expect(parser, erw_TOKENTYPE_COMMA);
		}

		vec_pushback(varlist, erw_parse_vardeclr(parser));
		if(!erw_parser_check(parser, erw_TOKENTYPE_COMMA))
		{
			break;
		}
	}

	erw_parser_expect(parser, erw_TOKENTYPE_RPAREN);
}

static struct erw_ASTNode* erw_parse_struct(struct erw_Parser* parser)
{
	struct erw_ASTNode* node = erw_ast_new(
		erw_ASTNODETYPE_STRUCT, 
		erw_parser_expect(parser, erw_TOKENTYPE_KEYWORD_STRUCT)
	);

	erw_parse_varlist(parser, node->struct_.members);
	return node;
}

static struct erw_ASTNode* erw_parse_union(struct erw_Parser* parser)
{
	struct erw_ASTNode* node = erw_ast_new(
		erw_ASTNODETYPE_UNION, 
		erw_parser_expect(parser, erw_TOKENTYPE_KEYWORD_UNION)
	);

	erw_parser_expect(parser, erw_TOKENTYPE_LPAREN);
	int first = 1;
	while(!erw_parser_check(parser, erw_TOKENTYPE_RPAREN))
	{
		if(first)
		{
			first = 0;
		}
		else
		{
			erw_parser_expect(parser, erw_TOKENTYPE_COMMA);
		}

		vec_pushback(node->union_.members, erw_parse_type(parser));
		if(!erw_parser_check(parser, erw_TOKENTYPE_COMMA))
		{
			break;
		}
	}

	erw_parser_expect(parser, erw_TOKENTYPE_RPAREN);
	return node;
}

static struct erw_ASTNode* erw_parse_enum(struct erw_Parser* parser)
{
	struct erw_ASTNode* node = erw_ast_new(
		erw_ASTNODETYPE_ENUM, 
		erw_parser_expect(parser, erw_TOKENTYPE_KEYWORD_ENUM)
	);

	erw_parser_expect(parser, erw_TOKENTYPE_LPAREN);
	int first = 1;
	while(!erw_parser_check(parser, erw_TOKENTYPE_RPAREN))
	{
		if(first)
		{
			first = 0;
		}
		else
		{
			erw_parser_expect(parser, erw_TOKENTYPE_COMMA);
		}

		struct erw_ASTNode* membernode = erw_ast_new(
			erw_ASTNODETYPE_ENUMMEMBER, 
			NULL
		);

		membernode->enummember.name = erw_parser_expect(
			parser, 
			erw_TOKENTYPE_IDENT
		);

		if(erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_ASSIGN))
		{
			erw_parser_expect(parser, erw_TOKENTYPE_OPERATOR_ASSIGN);
			//Parse expression?
			membernode->enummember.value = erw_ast_new(
				erw_ASTNODETYPE_LITERAL,
				erw_parser_expect(parser, erw_TOKENTYPE_LITERAL_INT)
			);
		}

		vec_pushback(node->enum_.members, membernode);
		if(!erw_parser_check(parser, erw_TOKENTYPE_COMMA))
		{
			break;
		}
	}

	erw_parser_expect(parser, erw_TOKENTYPE_RPAREN);
	return node;
}

static struct erw_ASTNode* erw_parse_typedeclr(struct erw_Parser* parser)
{ 
	struct erw_ASTNode* node = erw_ast_new(
		erw_ASTNODETYPE_TYPEDECLR,
		erw_parser_expect(parser, erw_TOKENTYPE_KEYWORD_TYPE)
	);

	node->typedeclr.name = erw_parser_expect(
		parser, 
		erw_TOKENTYPE_TYPE
	);

	if(erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_DECLR))
	{ 
		erw_parser_expect(parser, erw_TOKENTYPE_OPERATOR_DECLR);
		if(erw_parser_check(parser, erw_TOKENTYPE_TYPE))
		{
			node->typedeclr.type = erw_ast_new(
				erw_ASTNODETYPE_TYPE,
				erw_parser_expect(parser, erw_TOKENTYPE_TYPE)
			);
		}
		else if(erw_parser_check(parser, erw_TOKENTYPE_KEYWORD_STRUCT))
		{
			node->typedeclr.type = erw_parse_struct(parser);
		}
		else if(erw_parser_check(parser, erw_TOKENTYPE_KEYWORD_UNION))
		{
			node->typedeclr.type = erw_parse_union(parser);
		}
		else if(erw_parser_check(parser, erw_TOKENTYPE_KEYWORD_ENUM))
		{
			node->typedeclr.type = erw_parse_enum(parser);
		}
		else
		{
			struct Str msg;
			str_ctorfmt(
				&msg,
				"Expected %s, %s, %s, or %s. Got %s",
				erw_TOKENTYPE_TYPE->name,
				erw_TOKENTYPE_KEYWORD_STRUCT->name,
				erw_TOKENTYPE_KEYWORD_UNION->name,
				erw_TOKENTYPE_KEYWORD_ENUM->name,
				parser->tokens[parser->current].type->name
			);

			erw_error(
				msg.data, 
				parser->lines[parser->tokens[parser->current]
					.linenum - 1].data, 
				parser->tokens[parser->current].linenum, 
				parser->tokens[parser->current].column,
				parser->tokens[parser->current].column 
					+ vec_getsize(parser->tokens[parser->current].text) - 2
			);
			str_dtor(&msg);
		}
	}

	erw_parser_expect(parser, erw_TOKENTYPE_END);
	return node;
}

static struct erw_ASTNode* erw_parse_func(struct erw_Parser* parser);
static struct erw_ASTNode* erw_parse_block(struct erw_Parser* parser)
{
	struct erw_ASTNode* node = erw_ast_new(erw_ASTNODETYPE_BLOCK, NULL);
	erw_parser_expect(parser, erw_TOKENTYPE_LCURLY);
	while(!erw_parser_check(parser, erw_TOKENTYPE_RCURLY))
	{
		if(erw_parser_check(parser, erw_TOKENTYPE_KEYWORD_FUNC))
		{ 
			vec_pushback(node->block.stmts, erw_parse_func(parser));
			continue; //Don't require semicolon
		}
		else if(erw_parser_check(parser, erw_TOKENTYPE_KEYWORD_TYPE))
		{ 
			vec_pushback(node->block.stmts, erw_parse_typedeclr(parser));
			continue; //Don't require semicolon
		}
		else if(erw_parser_check(parser, erw_TOKENTYPE_KEYWORD_LET)
			|| erw_parser_check(parser, erw_TOKENTYPE_KEYWORD_MUT))
		{
			vec_pushback(node->block.stmts, erw_parse_vardeclr(parser));
		}
		else if(erw_parser_check(parser, erw_TOKENTYPE_IDENT))
		{ 
			if(parser->tokens[parser->current + 1].type 
				== erw_TOKENTYPE_LPAREN)
			{ 
				vec_pushback(node->block.stmts, erw_parse_funccall(parser));
			}
			else if(parser->tokens[parser->current + 1].type 
					==  erw_TOKENTYPE_OPERATOR_ASSIGN 
				|| parser->tokens[parser->current + 1].type 
					== erw_TOKENTYPE_OPERATOR_ADDASSIGN 
				|| parser->tokens[parser->current + 1].type 
					== erw_TOKENTYPE_OPERATOR_SUBASSIGN 
				|| parser->tokens[parser->current + 1].type 
					== erw_TOKENTYPE_OPERATOR_MULASSIGN 
				|| parser->tokens[parser->current + 1].type 
					== erw_TOKENTYPE_OPERATOR_DIVASSIGN 
				|| parser->tokens[parser->current + 1].type 
					== erw_TOKENTYPE_OPERATOR_POWASSIGN 
				|| parser->tokens[parser->current + 1].type 
					== erw_TOKENTYPE_OPERATOR_MODASSIGN)
			{
				//TODO: Add support for dereferencing
				struct erw_ASTNode* identnode = erw_ast_new( 
					erw_ASTNODETYPE_LITERAL,
					erw_parser_expect(parser, erw_TOKENTYPE_IDENT)
				);

				struct erw_ASTNode* assignnode = erw_ast_new(
					erw_ASTNODETYPE_ASSIGNMENT,
					&parser->tokens[parser->current]
				);
				parser->current++;

				assignnode->assignment.assignee = identnode;
				assignnode->assignment.expr = erw_parse_expr(parser);
				vec_pushback(node->block.stmts, assignnode);
			}
			else
			{
				struct Str msg;
				str_ctorfmt(
					&msg,
					"Expected '(', '=', '+=', '-=', '*=', '/=', '^=' or '%%='."
						" Got %s",
					parser->tokens[parser->current + 1].type->name
				);

				erw_error(
					msg.data, 
					parser->lines[parser->tokens[parser->current + 1]
						.linenum - 1].data, 
					parser->tokens[parser->current + 1].linenum, 
					parser->tokens[parser->current + 1].column,
					parser->tokens[parser->current + 1].column 
						+ vec_getsize(parser->tokens[parser->current + 1].text) 
						- 2
				);
				str_dtor(&msg);
			}
		}
		else if(erw_parser_check(parser, erw_TOKENTYPE_KEYWORD_IF))
		{ 
			struct erw_ASTNode* ifnode = erw_ast_new(
				erw_ASTNODETYPE_IF,
				erw_parser_expect(parser, erw_TOKENTYPE_KEYWORD_IF)
			);
			
			erw_parser_expect(parser, erw_TOKENTYPE_LPAREN);
			ifnode->if_.expr = erw_parse_expr(parser);
			erw_parser_expect(parser, erw_TOKENTYPE_RPAREN);
			ifnode->if_.block = erw_parse_block(parser);
			while(erw_parser_check(parser, erw_TOKENTYPE_KEYWORD_ELSEIF))
			{ 
				struct erw_ASTNode* elseifnode = erw_ast_new(
					erw_ASTNODETYPE_ELSEIF,
					erw_parser_expect(parser, erw_TOKENTYPE_KEYWORD_ELSEIF)
				);
				
				erw_parser_expect(parser, erw_TOKENTYPE_LPAREN);
				elseifnode->elseif.expr = erw_parse_expr(parser);
				erw_parser_expect(parser, erw_TOKENTYPE_RPAREN);
				elseifnode->elseif.block = erw_parse_block(parser);
				vec_pushback(ifnode->if_.elseifs, elseifnode);
			}

			if(erw_parser_check(parser, erw_TOKENTYPE_KEYWORD_ELSE))
			{ 
				struct erw_ASTNode* elsenode = erw_ast_new(
					erw_ASTNODETYPE_ELSE,
					erw_parser_expect(parser, erw_TOKENTYPE_KEYWORD_ELSE)
				);

				elsenode->else_.block = erw_parse_block(parser);
				ifnode->if_.else_ = elsenode;
			}

			vec_pushback(node->block.stmts, ifnode);
			continue; //Don't require semicolon
		}
		else if(erw_parser_check(parser, erw_TOKENTYPE_KEYWORD_RETURN))
		{ 
			struct erw_ASTNode* retnode = erw_ast_new(
				erw_ASTNODETYPE_RETURN,
				erw_parser_expect(parser, erw_TOKENTYPE_KEYWORD_RETURN)
			);

			if(!erw_parser_check(parser, erw_TOKENTYPE_END))
			{
				retnode->return_.expr = erw_parse_expr(parser);
			}

			erw_parser_expect(parser, erw_TOKENTYPE_END);
			vec_pushback(node->block.stmts, retnode);
			break; //Don't parse any statements after return
		}
		else if(erw_parser_check(parser, erw_TOKENTYPE_FOREIGN))
		{ 
			/*
			struct erw_ASTNode* foreign = erw_ast_newfromtoken(
				erw_parser_expect(parser, erw_TOKENTYPE_FOREIGN)
			);

			struct erw_ASTNode* argsnode = erw_ast_newfromnodetype(
				erw_ASTNODETYPE_FUNC_ARGS
			);

			erw_parser_expect(parser, erw_TOKENTYPE_LPAREN);
			while(!erw_parser_check(parser, erw_TOKENTYPE_RPAREN))
			{
				struct erw_ASTNode* argument = erw_parse_expression(parser);
				erw_ast_addbranch(argsnode, argument);

				if(erw_parser_check(parser, erw_TOKENTYPE_COMMA))
				{
					erw_parser_expect(parser, erw_TOKENTYPE_COMMA);
				}
				else
				{
					break;
				}
			}

			erw_parser_expect(parser, erw_TOKENTYPE_RPAREN);
			erw_ast_addbranch(foreign, argsnode);
			erw_ast_addbranch(blocknode, foreign);
			*/
		}
		else if(erw_parser_check(parser, erw_TOKENTYPE_KEYWORD_DEFER))
		{
			struct erw_ASTNode* defernode = erw_ast_new(
				erw_ASTNODETYPE_DEFER,
				erw_parser_expect(parser, erw_TOKENTYPE_KEYWORD_DEFER)
			);

			defernode->defer.block = erw_parse_block(parser);
			vec_pushback(node->block.stmts, defernode);
			continue; //Don't require semicolon
		}
		else if(erw_parser_check(parser, erw_TOKENTYPE_KEYWORD_WHILE))
		{
			struct erw_ASTNode* whilenode = erw_ast_new(
				erw_ASTNODETYPE_WHILE,
				erw_parser_expect(parser, erw_TOKENTYPE_KEYWORD_WHILE)
			);

			erw_parser_expect(parser, erw_TOKENTYPE_LPAREN);
			whilenode->while_.expr = erw_parse_expr(parser);
			erw_parser_expect(parser, erw_TOKENTYPE_RPAREN);
			whilenode->while_.block = erw_parse_block(parser);
			vec_pushback(node->block.stmts, whilenode);
			continue; //Don't require semicolon
		}
		else
		{ 
			struct Str msg;
			str_ctorfmt(
				&msg,
				"Unexpected %s",
				parser->tokens[parser->current].type->name
			);

			erw_error(
				msg.data, 
				parser->lines[parser->tokens[parser->current].linenum - 1].data, 
				parser->tokens[parser->current].linenum, 
				parser->tokens[parser->current].column,
				parser->tokens[parser->current].column 
					+ vec_getsize(parser->tokens[parser->current].text) - 2
			);
			str_dtor(&msg);
		}

		erw_parser_expect(parser, erw_TOKENTYPE_END);
	}

	erw_parser_expect(parser, erw_TOKENTYPE_RCURLY);
	return node;
}

static struct erw_ASTNode* erw_parse_func(struct erw_Parser* parser)
{
	struct erw_ASTNode* node = erw_ast_new(
		erw_ASTNODETYPE_FUNCDEF, 
		erw_parser_expect(parser, erw_TOKENTYPE_KEYWORD_FUNC)
	);

	node->funcdef.name = erw_parser_expect(parser, erw_TOKENTYPE_IDENT);
	erw_parser_expect(parser, erw_TOKENTYPE_OPERATOR_DECLR);
	erw_parse_varlist(parser, node->funcdef.params);
	if(erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_RETURN))
	{
		erw_parser_expect(parser, erw_TOKENTYPE_OPERATOR_RETURN);
		node->funcdef.type = erw_parse_type(parser);
	}

	node->funcdef.block = erw_parse_block(parser);
	return node;
}

struct erw_ASTNode* erw_parse(
	Vec(struct erw_Token) tokens,
	Vec(struct Str) lines)
{
	log_assert(tokens, "is NULL");
	log_assert(lines, "is NULL");

	struct erw_Parser parser = {
		.tokens = tokens,
		.lines = lines,
		.current = 0
	};

	struct erw_ASTNode* root = erw_ast_new(erw_ASTNODETYPE_START, NULL);
	while(parser.current < vec_getsize(tokens))
	{
		if(erw_parser_check(&parser, erw_TOKENTYPE_KEYWORD_FUNC))
		{
			vec_pushback(root->start.children, erw_parse_func(&parser));
		}
		else if(erw_parser_check(&parser, erw_TOKENTYPE_KEYWORD_TYPE))
		{
			vec_pushback(root->start.children, erw_parse_typedeclr(&parser));
		}
		else
		{
			struct Str msg;
			str_ctorfmt(
				&msg,
				"Unexpected %s",
				tokens[parser.current].type->name
			);

			erw_error(
				msg.data,
				lines[tokens[parser.current].linenum - 1].data,
				tokens[parser.current].linenum,
				tokens[parser.current].column,
				tokens[parser.current].column 
					+ vec_getsize(tokens[parser.current].text) - 2
			);
			str_dtor(&msg);
		}
	}

	return root;
}

