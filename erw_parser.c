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

#include "erw_parser.h"
#include "erw_error.h"

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

static struct erw_Token erw_parser_expect(
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
			parser->tokens[parser->current].column + 
				vec_getsize(parser->tokens[parser->current].text) - 2
		);
		str_dtor(&msg);
	}

	struct erw_Token ret = parser->tokens[parser->current];
	parser->current++;
	return ret;
}

static struct erw_ASTNode* erw_parse_type(struct erw_Parser* parser)
{ 
	struct erw_ASTNode* typenode = erw_ast_newfromtoken(
		erw_parser_expect(parser, erw_TOKENTYPE_TYPE)
	);

	/*
	while(1)
	{ 
		if(parser_check(parser, TOKENTYPE_OPERATOR_MUL))
		{ 
		}
		else if(parser_check(parser, TOKENTYPE_OPERATOR_BITAND))
		{ 
		}
		else if(parser_check(parser, TOKENTYPE_LBRACKET))
		{ 
		}
		else
		{ 
		}
	}
	*/

	return typenode;
}

static struct erw_ASTNode* erw_parse_typedeclr(struct erw_Parser* parser)
{ 
	struct erw_ASTNode* typedeclrnode = erw_ast_newfromtoken(
		erw_parser_expect(parser, erw_TOKENTYPE_KEYWORD_TYPE)
	);

	struct erw_ASTNode* newtypenode = erw_ast_newfromtoken(
		erw_parser_expect(parser, erw_TOKENTYPE_TYPE)
	);

	erw_ast_addbranch(typedeclrnode, newtypenode);
	if(erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_DECLR))
	{ 
		erw_parser_expect(parser, erw_TOKENTYPE_OPERATOR_DECLR);
		struct erw_ASTNode* typenode = erw_ast_newfromtoken(
			erw_parser_expect(parser, erw_TOKENTYPE_TYPE)
		);

		erw_ast_addbranch(typedeclrnode, typenode);
	}

	erw_parser_expect(parser, erw_TOKENTYPE_END);
	return typedeclrnode;
}

static struct erw_ASTNode* erw_parse_expression(struct erw_Parser* parser);
static struct erw_ASTNode* erw_parse_funccall(struct erw_Parser* parser);

static struct erw_ASTNode* erw_parse_factor(struct erw_Parser* parser)
{ 
	struct erw_ASTNode* factnode = NULL;
	if(erw_parser_check(parser, erw_TOKENTYPE_LITERAL_BOOL) ||
		erw_parser_check(parser, erw_TOKENTYPE_LITERAL_CHAR) ||
		erw_parser_check(parser, erw_TOKENTYPE_LITERAL_INT) ||
		erw_parser_check(parser, erw_TOKENTYPE_LITERAL_FLOAT) ||
		erw_parser_check(parser, erw_TOKENTYPE_LITERAL_STRING))
	{ 
		factnode = erw_ast_newfromtoken(parser->tokens[parser->current]);
		parser->current++;
	}
	else if(erw_parser_check(parser, erw_TOKENTYPE_IDENT))
	{ 
		if(parser->tokens[parser->current + 1].type == 
			erw_TOKENTYPE_LPAREN)
		{ 
			factnode = erw_parse_funccall(parser);
		}
		else
		{ 
			factnode = erw_ast_newfromtoken(
				erw_parser_expect(parser, erw_TOKENTYPE_IDENT)
			);
		}
	}
	else if(erw_parser_check(parser, erw_TOKENTYPE_LPAREN))
	{ 
		erw_parser_expect(parser, erw_TOKENTYPE_LPAREN);
		factnode = erw_parse_expression(parser);
		erw_parser_expect(parser, erw_TOKENTYPE_RPAREN);
	}
	else if(erw_parser_check(parser, erw_TOKENTYPE_KEYWORD_CAST))
	{ 
		factnode = erw_ast_newfromtoken(
			erw_parser_expect(parser, erw_TOKENTYPE_KEYWORD_CAST)
		);

		erw_parser_expect(parser, erw_TOKENTYPE_LPAREN);
		struct erw_ASTNode* typenode = erw_ast_newfromtoken(
			erw_parser_expect(parser, erw_TOKENTYPE_TYPE)
		);
		erw_ast_addbranch(factnode, typenode);
		erw_parser_expect(parser, erw_TOKENTYPE_COMMA);

		struct erw_ASTNode* exprnode = erw_parse_expression(parser);
		erw_ast_addbranch(factnode, exprnode);
		erw_parser_expect(parser, erw_TOKENTYPE_RPAREN);
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
			parser->tokens[parser->current].column + 
				vec_getsize(parser->tokens[parser->current].text) - 2
		);
		str_dtor(&msg);
	}

	return factnode;
}

static struct erw_ASTNode* erw_parse_exponent(struct erw_Parser* parser)
{ 
	struct erw_ASTNode* exponode = erw_parse_factor(parser);
	while(erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_POW))
	{
		struct erw_ASTNode* oldnode = exponode;
		exponode = erw_ast_newfromtoken(
			erw_parser_expect(parser, erw_TOKENTYPE_OPERATOR_POW)
		);

		erw_ast_addbranch(exponode, oldnode);
		struct erw_ASTNode* newnode = erw_parse_factor(parser);
		erw_ast_addbranch(exponode, newnode);
	}

	return exponode;
}

static struct erw_ASTNode* erw_parse_sign(struct erw_Parser* parser)
{ 
	struct erw_ASTNode* signnode;
	if(erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_SUB) ||
		erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_NOT))
	{
		signnode = erw_ast_newfromtoken(parser->tokens[parser->current]);
		parser->current++;
		erw_ast_addbranch(signnode, erw_parse_exponent(parser));
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
	while(erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_MUL) ||
		erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_DIV) ||
		erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_MOD))
	{
		struct erw_ASTNode* oldnode = termnode;
		termnode = erw_ast_newfromtoken(parser->tokens[parser->current]);
		parser->current++;

		erw_ast_addbranch(termnode, oldnode);
		struct erw_ASTNode* newnode = erw_parse_sign(parser);
		erw_ast_addbranch(termnode, newnode);
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
		pmnode = erw_ast_newfromtoken(parser->tokens[parser->current]);
		parser->current++;

		erw_ast_addbranch(pmnode, oldnode);
		struct erw_ASTNode* newnode = erw_parse_term(parser);
		erw_ast_addbranch(pmnode, newnode);
	}

	return pmnode;
}

static struct erw_ASTNode* erw_parse_comparison(struct erw_Parser* parser)
{ 
	struct erw_ASTNode* cmpnode = erw_parse_plusminus(parser);
	while(erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_LESS) ||
		erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_LESSOREQUAL) ||
		erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_GREATER) ||
		erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_GREATEROREQUAL))
	{ 
		struct erw_ASTNode* oldnode = cmpnode;
		cmpnode = erw_ast_newfromtoken(parser->tokens[parser->current]);
		parser->current++;

		erw_ast_addbranch(cmpnode, oldnode);
		struct erw_ASTNode* newnode = erw_parse_plusminus(parser);
		erw_ast_addbranch(cmpnode, newnode);
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
		eqnode = erw_ast_newfromtoken(parser->tokens[parser->current]);
		parser->current++;

		erw_ast_addbranch(eqnode, oldnode);
		struct erw_ASTNode* newnode = erw_parse_comparison(parser);
		erw_ast_addbranch(eqnode, newnode);
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
		andornode = erw_ast_newfromtoken(parser->tokens[parser->current]);
		parser->current++;

		erw_ast_addbranch(andornode, oldnode);
		struct erw_ASTNode* newnode = erw_parse_equality(parser);
		erw_ast_addbranch(andornode, newnode);
	}

	return andornode;
}
static struct erw_ASTNode* erw_parse_expression(struct erw_Parser* parser)
{
	return erw_parse_andor(parser);
}

static struct erw_ASTNode* erw_parse_funccall(struct erw_Parser* parser)
{ 
	struct erw_ASTNode* callnode = erw_ast_newfromnodetype(
		erw_ASTNODETYPE_FUNC_CALL
	);
	struct erw_ASTNode* identnode = erw_ast_newfromtoken(
		erw_parser_expect(parser, erw_TOKENTYPE_IDENT)
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
	erw_ast_addbranch(callnode, identnode);
	erw_ast_addbranch(callnode, argsnode);
	return callnode;
}

static struct erw_ASTNode* erw_parse_function(struct erw_Parser* parser);
static struct erw_ASTNode* erw_parse_block(struct erw_Parser* parser)
{ 
	struct erw_ASTNode* blocknode = erw_ast_newfromnodetype(
		erw_ASTNODETYPE_BLOCK
	);

	erw_parser_expect(parser, erw_TOKENTYPE_LCURLY);

	while(!erw_parser_check(parser, erw_TOKENTYPE_RCURLY))
	{ 
		if(erw_parser_check(parser, erw_TOKENTYPE_KEYWORD_FUNC))
		{ 
			erw_ast_addbranch(blocknode, erw_parse_function(parser));
			continue; //Don't require semicolon
		}
		else if(erw_parser_check(parser, erw_TOKENTYPE_KEYWORD_TYPE))
		{ 
			erw_ast_addbranch(blocknode, erw_parse_typedeclr(parser));
			continue; //Don't require semicolon
		}
		else if(erw_parser_check(parser, erw_TOKENTYPE_KEYWORD_LET))
		{
			struct erw_ASTNode* varnode = erw_ast_newfromtoken(
				erw_parser_expect(parser, erw_TOKENTYPE_KEYWORD_LET)
			);

			struct erw_ASTNode* identnode = erw_ast_newfromtoken(
				erw_parser_expect(parser, erw_TOKENTYPE_IDENT)
			);

			erw_parser_expect(parser, erw_TOKENTYPE_OPERATOR_DECLR);
			struct erw_ASTNode* typenode = erw_parse_type(parser);

			erw_ast_addbranch(varnode, identnode);
			erw_ast_addbranch(varnode, typenode);
			erw_ast_addbranch(blocknode, varnode);

			struct erw_ASTNode* valuenode = erw_ast_newfromnodetype(
				erw_ASTNODETYPE_VAR_VALUE
			);

			if(erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_ASSIGN))
			{ 
				erw_parser_expect(parser, erw_TOKENTYPE_OPERATOR_ASSIGN);
				erw_ast_addbranch(valuenode, erw_parse_expression(parser));
			}
			erw_ast_addbranch(varnode, valuenode);
		}
		else if(erw_parser_check(parser, erw_TOKENTYPE_KEYWORD_MUT))
		{
			struct erw_ASTNode* varnode = erw_ast_newfromtoken(
				erw_parser_expect(parser, erw_TOKENTYPE_KEYWORD_MUT)
			);

			struct erw_ASTNode* identnode = erw_ast_newfromtoken(
				erw_parser_expect(parser, erw_TOKENTYPE_IDENT)
			);

			erw_parser_expect(parser, erw_TOKENTYPE_OPERATOR_DECLR);
			struct erw_ASTNode* typenode = erw_parse_type(parser);

			erw_ast_addbranch(varnode, identnode);
			erw_ast_addbranch(varnode, typenode);
			erw_ast_addbranch(blocknode, varnode);

			struct erw_ASTNode* valuenode = erw_ast_newfromnodetype(
				erw_ASTNODETYPE_VAR_VALUE
			);

			if(erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_ASSIGN))
			{ 
				erw_parser_expect(parser, erw_TOKENTYPE_OPERATOR_ASSIGN);
				erw_ast_addbranch(valuenode, erw_parse_expression(parser));
			}
			erw_ast_addbranch(varnode, valuenode);
		}
		else if(erw_parser_check(parser, erw_TOKENTYPE_IDENT))
		{ 
			if(parser->tokens[parser->current + 1].type == 
				erw_TOKENTYPE_LPAREN)
			{ 
				erw_ast_addbranch(blocknode, erw_parse_funccall(parser));
			}
			else if(parser->tokens[parser->current + 1].type ==  
					erw_TOKENTYPE_OPERATOR_ASSIGN || 
				parser->tokens[parser->current + 1].type == 
					erw_TOKENTYPE_OPERATOR_ADDASSIGN ||
				parser->tokens[parser->current + 1].type == 
					erw_TOKENTYPE_OPERATOR_SUBASSIGN ||
				parser->tokens[parser->current + 1].type == 
					erw_TOKENTYPE_OPERATOR_MULASSIGN ||
				parser->tokens[parser->current + 1].type == 
					erw_TOKENTYPE_OPERATOR_DIVASSIGN ||
				parser->tokens[parser->current + 1].type == 
					erw_TOKENTYPE_OPERATOR_POWASSIGN ||
				parser->tokens[parser->current + 1].type == 
					erw_TOKENTYPE_OPERATOR_MODASSIGN)
			{
				struct erw_ASTNode* identnode = erw_ast_newfromtoken(
					erw_parser_expect(parser, erw_TOKENTYPE_IDENT)
				);

				struct erw_ASTNode* assignnode = erw_ast_newfromtoken(
					parser->tokens[parser->current]
				);
				parser->current++;

				struct erw_ASTNode* exprnode = erw_parse_expression(parser);
				erw_ast_addbranch(assignnode, identnode);
				erw_ast_addbranch(assignnode, exprnode);
				erw_ast_addbranch(blocknode, assignnode);
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
					parser->tokens[parser->current + 1].column + 
						vec_getsize(parser->tokens[parser->current + 1].text) 
						- 2
				);
				str_dtor(&msg);
			}
		}
		else if(erw_parser_check(parser, erw_TOKENTYPE_KEYWORD_IF))
		{ 
			struct erw_ASTNode* ifnode = erw_ast_newfromtoken(
				erw_parser_expect(parser, erw_TOKENTYPE_KEYWORD_IF)
			);
			
			erw_parser_expect(parser, erw_TOKENTYPE_LPAREN);
			erw_ast_addbranch(ifnode, erw_parse_expression(parser));
			erw_parser_expect(parser, erw_TOKENTYPE_RPAREN);
			erw_ast_addbranch(ifnode, erw_parse_block(parser));

			while(erw_parser_check(parser, erw_TOKENTYPE_KEYWORD_ELSEIF))
			{ 
				struct erw_ASTNode* elseifnode = erw_ast_newfromtoken(
					erw_parser_expect(parser, erw_TOKENTYPE_KEYWORD_ELSEIF)
				);
				
				erw_parser_expect(parser, erw_TOKENTYPE_LPAREN);
				erw_ast_addbranch(elseifnode, erw_parse_expression(parser));
				erw_parser_expect(parser, erw_TOKENTYPE_RPAREN);
				erw_ast_addbranch(elseifnode, erw_parse_block(parser));
				erw_ast_addbranch(ifnode, elseifnode);
			}

			if(erw_parser_check(parser, erw_TOKENTYPE_KEYWORD_ELSE))
			{ 
				struct erw_ASTNode* elsenode = erw_ast_newfromtoken(
					erw_parser_expect(parser, erw_TOKENTYPE_KEYWORD_ELSE)
				);

				erw_ast_addbranch(elsenode, erw_parse_block(parser));
				erw_ast_addbranch(ifnode, elsenode);
			}

			erw_ast_addbranch(blocknode, ifnode);
			continue; //Don't require semicolon
		}
		else if(erw_parser_check(parser, erw_TOKENTYPE_KEYWORD_RETURN))
		{ 
			struct erw_ASTNode* retnode = erw_ast_newfromtoken(
				erw_parser_expect(parser, erw_TOKENTYPE_KEYWORD_RETURN)
			);

			if(erw_parser_check(parser, erw_TOKENTYPE_END))
			{
				erw_parser_expect(parser, erw_TOKENTYPE_END);
			}
			else
			{
				erw_ast_addbranch(retnode, erw_parse_expression(parser));
				erw_parser_expect(parser, erw_TOKENTYPE_END);
			}

			erw_ast_addbranch(blocknode, retnode);
			break; //Don't parse any statements after return
		}
		else if(erw_parser_check(parser, erw_TOKENTYPE_FOREIGN))
		{ 
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
		}
		else if(erw_parser_check(parser, erw_TOKENTYPE_KEYWORD_DEFER))
		{
			struct erw_ASTNode* defernode = erw_ast_newfromtoken(
				erw_parser_expect(parser, erw_TOKENTYPE_KEYWORD_DEFER)
			);

			erw_ast_addbranch(defernode, erw_parse_block(parser));
			erw_ast_addbranch(blocknode, defernode);
			continue; //Don't require semicolon
		}
		else if(erw_parser_check(parser, erw_TOKENTYPE_KEYWORD_WHILE))
		{
			struct erw_ASTNode* whilenode = erw_ast_newfromtoken(
				erw_parser_expect(parser, erw_TOKENTYPE_KEYWORD_WHILE)
			);

			erw_parser_expect(parser, erw_TOKENTYPE_LPAREN);
			erw_ast_addbranch(whilenode, erw_parse_expression(parser));
			erw_parser_expect(parser, erw_TOKENTYPE_RPAREN);

			erw_ast_addbranch(whilenode, erw_parse_block(parser));
			erw_ast_addbranch(blocknode, whilenode);
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
				parser->tokens[parser->current].column + 
					vec_getsize(parser->tokens[parser->current].text) - 2
			);
			str_dtor(&msg);
		}

		erw_parser_expect(parser, erw_TOKENTYPE_END);
	}

	erw_parser_expect(parser, erw_TOKENTYPE_RCURLY);
	return blocknode;
}

static struct erw_ASTNode* erw_parse_function(struct erw_Parser* parser)
{ 
	struct erw_ASTNode* funcnode = erw_ast_newfromtoken(
		erw_parser_expect(parser, erw_TOKENTYPE_KEYWORD_FUNC)
	);

	erw_ast_addbranch(
		funcnode, 
		erw_ast_newfromtoken(
			erw_parser_expect(parser, erw_TOKENTYPE_IDENT)
		)
	);

	erw_parser_expect(parser, erw_TOKENTYPE_OPERATOR_DECLR);
	erw_parser_expect(parser, erw_TOKENTYPE_LPAREN);

	struct erw_ASTNode* argsnode = erw_ast_newfromnodetype(
		erw_ASTNODETYPE_FUNC_ARGS
	);

	while(!erw_parser_check(parser, erw_TOKENTYPE_RPAREN))
	{
		struct erw_ASTNode* argnode = NULL;
		if(erw_parser_check(parser, erw_TOKENTYPE_KEYWORD_LET))
		{
			argnode = erw_ast_newfromtoken(
				erw_parser_expect(parser, erw_TOKENTYPE_KEYWORD_LET)
			);
		}
		else if(erw_parser_check(parser, erw_TOKENTYPE_KEYWORD_MUT))
		{
			argnode = erw_ast_newfromtoken(
				erw_parser_expect(parser, erw_TOKENTYPE_KEYWORD_MUT)
			);
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
				parser->tokens[parser->current].column + 
					vec_getsize(parser->tokens[parser->current].text) - 2
			);
			str_dtor(&msg);
		}

		struct erw_ASTNode* identnode = erw_ast_newfromtoken(
			erw_parser_expect(parser, erw_TOKENTYPE_IDENT)
		);

		erw_parser_expect(parser, erw_TOKENTYPE_OPERATOR_DECLR);
		struct erw_ASTNode* typenode = erw_parse_type(parser);

		erw_ast_addbranch(argnode, identnode);
		erw_ast_addbranch(argnode, typenode);
		erw_ast_addbranch(argsnode, argnode);

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
	struct erw_ASTNode* returnnode = erw_ast_newfromnodetype(
		erw_ASTNODETYPE_FUNC_RETURN
	);

	if(erw_parser_check(parser, erw_TOKENTYPE_OPERATOR_RETURN))
	{
		erw_parser_expect(parser, erw_TOKENTYPE_OPERATOR_RETURN);
		erw_ast_addbranch(returnnode, erw_parse_type(parser));
	}

	erw_ast_addbranch(funcnode, argsnode);
	erw_ast_addbranch(funcnode, returnnode);
	erw_ast_addbranch(funcnode, erw_parse_block(parser));
	return funcnode;
}

struct erw_ASTNode* erw_parse(
	Vec(struct erw_Token) tokens, 
	Vec(struct Str) lines)
{
	struct erw_Parser parser = {
		.tokens = tokens, 
		.lines = lines,
		.current = 0
	};

	struct erw_ASTNode* root = erw_ast_newfromnodetype(erw_ASTNODETYPE_START);
	while(parser.current < vec_getsize(tokens))
	{
		if(erw_parser_check(&parser, erw_TOKENTYPE_KEYWORD_FUNC))
		{
			erw_ast_addbranch(root, erw_parse_function(&parser));
		}
		else if(erw_parser_check(&parser, erw_TOKENTYPE_KEYWORD_TYPE))
		{ 
			erw_ast_addbranch(root, erw_parse_typedeclr(&parser));
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
				tokens[parser.current].column + 
					vec_getsize(tokens[parser.current].text) - 2
			);
			str_dtor(&msg);
		}
	}

	return root;
}

