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

#include "parser.h"
#include "log.h"

struct Parser
{
	Vec(struct Token) tokens;
	size_t current;
};

static int parser_check(struct Parser* parser, const struct TokenType* type)
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
		log_error("Parsing error: Expected %s, reached EOF", type->name);
	}

	return 0;
}

static 
struct Token parser_expect(struct Parser* parser, const struct TokenType* type)
{
	if(!parser_check(parser, type))
	{
		log_error(
			"Parsing error: Expected %s, got %s (%s), at line %zu"
				", column %zu", 
			type->name,
			parser->tokens[parser->current].type->name,
			parser->tokens[parser->current].text,
			parser->tokens[parser->current].line,
			parser->tokens[parser->current].column
		);
	}

	struct Token ret = parser->tokens[parser->current];
	parser->current++;
	return ret;
}

static struct ASTNode* parse_type(struct Parser* parser)
{ 
	struct ASTNode* typenode = ast_newfromtoken(
		parser_expect(parser, TOKENTYPE_TYPE)
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

static struct ASTNode* parse_expression(struct Parser* parser);

static struct ASTNode* parse_funccall(struct Parser* parser)
{ 
	struct ASTNode* callnode = ast_newfromnodetype(ASTNODETYPE_FUNC_CALL);
	struct ASTNode* identnode = ast_newfromtoken(
		parser_expect(parser, TOKENTYPE_IDENT)
	);

	struct ASTNode* argsnode = ast_newfromnodetype(
		ASTNODETYPE_FUNC_ARGS
	);

	parser_expect(parser, TOKENTYPE_LPAREN);
	while(!parser_check(parser, TOKENTYPE_RPAREN))
	{
		struct ASTNode* argument = parse_expression(parser);
		ast_addbranch(argsnode, argument);

		if(parser_check(parser, TOKENTYPE_COMMA))
		{
			parser_expect(parser, TOKENTYPE_COMMA);
		}
		else
		{
			break;
		}
	}

	parser_expect(parser, TOKENTYPE_RPAREN);
	ast_addbranch(callnode, identnode);
	ast_addbranch(callnode, argsnode);
	return callnode;
}

static struct ASTNode* parse_factor(struct Parser* parser)
{ 
	struct ASTNode* factnode = NULL;
	if(parser_check(parser, TOKENTYPE_LITERAL_BOOL) ||
		parser_check(parser, TOKENTYPE_LITERAL_CHAR) ||
		parser_check(parser, TOKENTYPE_LITERAL_INT) ||
		parser_check(parser, TOKENTYPE_LITERAL_FLOAT) ||
		parser_check(parser, TOKENTYPE_LITERAL_STRING))
	{ 
		factnode = ast_newfromtoken(parser->tokens[parser->current]);
		parser->current++;
	}
	else if(parser_check(parser, TOKENTYPE_TYPE))
	{ 
		factnode = ast_newfromnodetype(ASTNODETYPE_TYPECAST);
		ast_addbranch(factnode, parse_type(parser));
		parser_expect(parser, TOKENTYPE_LPAREN);
		ast_addbranch(factnode, parse_expression(parser));
		parser_expect(parser, TOKENTYPE_RPAREN);
	}
	else if(parser_check(parser, TOKENTYPE_IDENT))
	{ 
		if(parser->tokens[parser->current + 1].type == 
			TOKENTYPE_LPAREN)
		{ 
			factnode = parse_funccall(parser);
		}
		else
		{ 
			factnode = ast_newfromtoken(
				parser_expect(parser, TOKENTYPE_IDENT)
			);
		}
	}
	else if(parser_check(parser, TOKENTYPE_LPAREN))
	{ 
		parser_expect(parser, TOKENTYPE_LPAREN);
		factnode = parse_expression(parser);
		parser_expect(parser, TOKENTYPE_RPAREN);
	}
	else
	{ 
		log_error(
			"Parsing error: Unexpected %s ('%s'), at line %zu"
				", column %zu", 
			parser->tokens[parser->current].type->name,
			parser->tokens[parser->current].text,
			parser->tokens[parser->current].line,
			parser->tokens[parser->current].column
		);
	}

	return factnode;
}

static struct ASTNode* parse_exponent(struct Parser* parser)
{ 
	struct ASTNode* exponode = parse_factor(parser);
	while(parser_check(parser, TOKENTYPE_OPERATOR_POW))
	{
		struct ASTNode* oldnode = exponode;
		exponode = ast_newfromtoken(
			parser_expect(parser, TOKENTYPE_OPERATOR_POW)
		);

		ast_addbranch(exponode, oldnode);
		struct ASTNode* newnode = parse_factor(parser);
		ast_addbranch(exponode, newnode);
	}

	return exponode;
}

static struct ASTNode* parse_sign(struct Parser* parser)
{ 
	struct ASTNode* signnode;
	if(parser_check(parser, TOKENTYPE_OPERATOR_SUB) ||
		parser_check(parser, TOKENTYPE_OPERATOR_NOT))
	{
		signnode = ast_newfromtoken(parser->tokens[parser->current]);
		parser->current++;
		ast_addbranch(signnode, parse_exponent(parser));
	}
	else
	{
		signnode = parse_exponent(parser);
	}

	return signnode;
}

static struct ASTNode* parse_term(struct Parser* parser)
{ 
	struct ASTNode* termnode = parse_sign(parser);
	while(parser_check(parser, TOKENTYPE_OPERATOR_MUL) ||
		parser_check(parser, TOKENTYPE_OPERATOR_DIV) ||
		parser_check(parser, TOKENTYPE_OPERATOR_MOD))
	{
		struct ASTNode* oldnode = termnode;
		termnode = ast_newfromtoken(parser->tokens[parser->current]);
		parser->current++;

		ast_addbranch(termnode, oldnode);
		struct ASTNode* newnode = parse_sign(parser);
		ast_addbranch(termnode, newnode);
	}

	return termnode;
}

static struct ASTNode* parse_plusminus(struct Parser* parser)
{ 
	struct ASTNode* pmnode = parse_term(parser);
	while(parser_check(parser, TOKENTYPE_OPERATOR_ADD) ||
		parser_check(parser, TOKENTYPE_OPERATOR_SUB))
	{
		struct ASTNode* oldnode = pmnode;
		pmnode = ast_newfromtoken(parser->tokens[parser->current]);
		parser->current++;

		ast_addbranch(pmnode, oldnode);
		struct ASTNode* newnode = parse_term(parser);
		ast_addbranch(pmnode, newnode);
	}

	return pmnode;
}

static struct ASTNode* parse_comparison(struct Parser* parser)
{ 
	struct ASTNode* cmpnode = parse_plusminus(parser);
	while(parser_check(parser, TOKENTYPE_OPERATOR_LESS) ||
		parser_check(parser, TOKENTYPE_OPERATOR_LESSOREQUAL) ||
		parser_check(parser, TOKENTYPE_OPERATOR_GREATER) ||
		parser_check(parser, TOKENTYPE_OPERATOR_GREATEROREQUAL))
	{ 
		struct ASTNode* oldnode = cmpnode;
		cmpnode = ast_newfromtoken(parser->tokens[parser->current]);
		parser->current++;

		ast_addbranch(cmpnode, oldnode);
		struct ASTNode* newnode = parse_plusminus(parser);
		ast_addbranch(cmpnode, newnode);
	}

	return cmpnode;
}

static struct ASTNode* parse_equality(struct Parser* parser)
{ 
	struct ASTNode* eqnode = parse_comparison(parser);
	while(parser_check(parser, TOKENTYPE_OPERATOR_EQUAL) ||
		parser_check(parser, TOKENTYPE_OPERATOR_NOTEQUAL))
	{ 
		struct ASTNode* oldnode = eqnode;
		eqnode = ast_newfromtoken(parser->tokens[parser->current]);
		parser->current++;

		ast_addbranch(eqnode, oldnode);
		struct ASTNode* newnode = parse_comparison(parser);
		ast_addbranch(eqnode, newnode);
	}

	return eqnode;
}

static struct ASTNode* parse_andor(struct Parser* parser)
{ 
	struct ASTNode* andornode = parse_equality(parser);
	while(parser_check(parser, TOKENTYPE_OPERATOR_AND) ||
		parser_check(parser, TOKENTYPE_OPERATOR_OR))
	{ 
		struct ASTNode* oldnode = andornode;
		andornode = ast_newfromtoken(parser->tokens[parser->current]);
		parser->current++;

		ast_addbranch(andornode, oldnode);
		struct ASTNode* newnode = parse_equality(parser);
		ast_addbranch(andornode, newnode);
	}

	return andornode;
}

static struct ASTNode* parse_expression(struct Parser* parser)
{
	return parse_andor(parser);
}

static struct ASTNode* parse_typedeclr(struct Parser* parser)
{ 
	struct ASTNode* typedeclrnode = ast_newfromtoken(
		parser_expect(parser, TOKENTYPE_KEYWORD_TYPE)
	);

	struct ASTNode* newtypenode = ast_newfromtoken(
		parser_expect(parser, TOKENTYPE_TYPE)
	);

	parser_expect(parser, TOKENTYPE_OPERATOR_DECLR);
	struct ASTNode* typenode = ast_newfromtoken(
		parser_expect(parser, TOKENTYPE_TYPE)
	);

	ast_addbranch(typedeclrnode, newtypenode);
	ast_addbranch(typedeclrnode, typenode);
	parser_expect(parser, TOKENTYPE_END);

	return typedeclrnode;
}

static struct ASTNode* parse_function(struct Parser* parser);

static struct ASTNode* parse_block(struct Parser* parser)
{ 
	struct ASTNode* blocknode = ast_newfromnodetype(
		ASTNODETYPE_BLOCK
	);

	parser_expect(parser, TOKENTYPE_LCURLY);

	while(!parser_check(parser, TOKENTYPE_RCURLY))
	{ 
		if(parser_check(parser, TOKENTYPE_KEYWORD_FUNC))
		{ 
			ast_addbranch(blocknode, parse_function(parser));
			continue; //Don't require semicolon
		}
		else if(parser_check(parser, TOKENTYPE_KEYWORD_TYPE))
		{ 
			ast_addbranch(blocknode, parse_typedeclr(parser));
			continue; //Don't require semicolon
		}
		else if(parser_check(parser, TOKENTYPE_KEYWORD_LET))
		{
			struct ASTNode* varnode = ast_newfromtoken(
				parser_expect(parser, TOKENTYPE_KEYWORD_LET)
			);

			struct ASTNode* identnode = ast_newfromtoken(
				parser_expect(parser, TOKENTYPE_IDENT)
			);

			parser_expect(parser, TOKENTYPE_OPERATOR_DECLR);
			struct ASTNode* typenode = parse_type(parser);

			ast_addbranch(varnode, identnode);
			ast_addbranch(varnode, typenode);
			ast_addbranch(blocknode, varnode);

			struct ASTNode* valuenode = ast_newfromnodetype(
				ASTNODETYPE_VAR_VALUE
			);

			if(parser_check(parser, TOKENTYPE_OPERATOR_ASSIGN))
			{ 
				parser_expect(parser, TOKENTYPE_OPERATOR_ASSIGN);
				ast_addbranch(valuenode, parse_expression(parser));
			}
			ast_addbranch(varnode, valuenode);
		}
		else if(parser_check(parser, TOKENTYPE_KEYWORD_MUT))
		{
			struct ASTNode* varnode = ast_newfromtoken(
				parser_expect(parser, TOKENTYPE_KEYWORD_MUT)
			);

			struct ASTNode* identnode = ast_newfromtoken(
				parser_expect(parser, TOKENTYPE_IDENT)
			);

			parser_expect(parser, TOKENTYPE_OPERATOR_DECLR);
			struct ASTNode* typenode = parse_type(parser);

			ast_addbranch(varnode, identnode);
			ast_addbranch(varnode, typenode);
			ast_addbranch(blocknode, varnode);

			struct ASTNode* valuenode = ast_newfromnodetype(
				ASTNODETYPE_VAR_VALUE
			);

			if(parser_check(parser, TOKENTYPE_OPERATOR_ASSIGN))
			{ 
				parser_expect(parser, TOKENTYPE_OPERATOR_ASSIGN);
				ast_addbranch(varnode, parse_expression(parser));
			}
			ast_addbranch(varnode, valuenode);
		}
		else if(parser_check(parser, TOKENTYPE_IDENT))
		{ 
			if(parser->tokens[parser->current + 1].type == 
				TOKENTYPE_LPAREN)
			{ 
				ast_addbranch(blocknode, parse_funccall(parser));
			}
		}
		else if(parser_check(parser, TOKENTYPE_KEYWORD_IF))
		{ 
			struct ASTNode* ifnode = ast_newfromtoken(
				parser_expect(parser, TOKENTYPE_KEYWORD_IF)
			);
			
			parser_expect(parser, TOKENTYPE_LPAREN);
			ast_addbranch(ifnode, parse_expression(parser));
			parser_expect(parser, TOKENTYPE_RPAREN);
			ast_addbranch(ifnode, parse_block(parser));

			while(parser_check(parser, TOKENTYPE_KEYWORD_ELSEIF))
			{ 
				struct ASTNode* elseifnode = ast_newfromtoken(
					parser_expect(parser, TOKENTYPE_KEYWORD_ELSEIF)
				);
				
				parser_expect(parser, TOKENTYPE_LPAREN);
				ast_addbranch(elseifnode, parse_expression(parser));
				parser_expect(parser, TOKENTYPE_RPAREN);
				ast_addbranch(elseifnode, parse_block(parser));
				ast_addbranch(ifnode, elseifnode);
			}

			if(parser_check(parser, TOKENTYPE_KEYWORD_ELSE))
			{ 
				struct ASTNode* elsenode = ast_newfromtoken(
					parser_expect(parser, TOKENTYPE_KEYWORD_ELSE)
				);
				ast_addbranch(elsenode, parse_block(parser));
				ast_addbranch(ifnode, elsenode);
			}

			ast_addbranch(blocknode, ifnode);
			continue; //Don't require semicolon
		}
		else if(parser_check(parser, TOKENTYPE_KEYWORD_RETURN))
		{ 
			struct ASTNode* retnode = ast_newfromtoken(
				parser_expect(parser, TOKENTYPE_KEYWORD_RETURN)
			);

			ast_addbranch(retnode, parse_expression(parser));
			ast_addbranch(blocknode, retnode);
		}
		else if(parser_check(parser, TOKENTYPE_FOREIGN))
		{ 
			struct ASTNode* foreign = ast_newfromtoken(
				parser_expect(parser, TOKENTYPE_FOREIGN)
			);

			struct ASTNode* argsnode = ast_newfromnodetype(
				ASTNODETYPE_FUNC_ARGS
			);

			parser_expect(parser, TOKENTYPE_LPAREN);
			while(!parser_check(parser, TOKENTYPE_RPAREN))
			{
				struct ASTNode* argument = parse_expression(parser);
				ast_addbranch(argsnode, argument);

				if(parser_check(parser, TOKENTYPE_COMMA))
				{
					parser_expect(parser, TOKENTYPE_COMMA);
				}
				else
				{
					break;
				}
			}

			parser_expect(parser, TOKENTYPE_RPAREN);
			ast_addbranch(foreign, argsnode);
			ast_addbranch(blocknode, foreign);
		}
		else
		{ 
			log_error(
				"Parsing error: Unexpected %s ('%s'), at line %zu"
					", column %zu", 
				parser->tokens[parser->current].type->name,
				parser->tokens[parser->current].text,
				parser->tokens[parser->current].line,
				parser->tokens[parser->current].column
			);
		}

		parser_expect(parser, TOKENTYPE_END);
	}

	parser_expect(parser, TOKENTYPE_RCURLY);
	return blocknode;
}

static struct ASTNode* parse_function(struct Parser* parser)
{ 
	struct ASTNode* funcnode = ast_newfromtoken(
		parser_expect(parser, TOKENTYPE_KEYWORD_FUNC)
	);

	ast_addbranch(
		funcnode, 
		ast_newfromtoken(
			parser_expect(parser, TOKENTYPE_IDENT)
		)
	);

	parser_expect(parser, TOKENTYPE_OPERATOR_DECLR);
	parser_expect(parser, TOKENTYPE_LPAREN);

	struct ASTNode* argsnode = ast_newfromnodetype(
		ASTNODETYPE_FUNC_ARGS
	);

	while(!parser_check(parser, TOKENTYPE_RPAREN))
	{
		struct ASTNode* argnode = NULL;
		if(parser_check(parser, TOKENTYPE_KEYWORD_LET))
		{
			argnode = ast_newfromtoken(
				parser_expect(parser, TOKENTYPE_KEYWORD_LET)
			);
		}
		else if(parser_check(parser, TOKENTYPE_KEYWORD_MUT))
		{
			argnode = ast_newfromtoken(
				parser_expect(parser, TOKENTYPE_KEYWORD_MUT)
			);
		}
		else
		{
			log_error(
				"Parsing error: Expected %s or %s, got %s (%s), at line %zu"
					", column %zu", 
				TOKENTYPE_KEYWORD_LET->name,
				TOKENTYPE_KEYWORD_MUT->name,
				parser->tokens[parser->current].type->name,
				parser->tokens[parser->current].text,
				parser->tokens[parser->current].line,
				parser->tokens[parser->current].column
			);
		}

		struct ASTNode* identnode = ast_newfromtoken(
			parser_expect(parser, TOKENTYPE_IDENT)
		);

		parser_expect(parser, TOKENTYPE_OPERATOR_DECLR);
		struct ASTNode* typenode = parse_type(parser);

		ast_addbranch(argnode, identnode);
		ast_addbranch(argnode, typenode);
		ast_addbranch(argsnode, argnode);

		if(parser_check(parser, TOKENTYPE_COMMA))
		{
			parser_expect(parser, TOKENTYPE_COMMA);
		}
		else
		{
			break;
		}
	}

	parser_expect(parser, TOKENTYPE_RPAREN);
	struct ASTNode* returnnode = ast_newfromnodetype(
		ASTNODETYPE_FUNC_RETURN
	);

	if(parser_check(parser, TOKENTYPE_OPERATOR_RETURN))
	{
		parser_expect(parser, TOKENTYPE_OPERATOR_RETURN);
		ast_addbranch(returnnode, parse_type(parser));
	}

	ast_addbranch(funcnode, argsnode);
	ast_addbranch(funcnode, returnnode);
	ast_addbranch(funcnode, parse_block(parser));
	return funcnode;
}

struct ASTNode* parse(Vec(struct Token) tokens)
{
	struct Parser parser = {
		.tokens = tokens, 
		.current = 0
	};

	struct ASTNode* root = ast_newfromnodetype(ASTNODETYPE_START);
	while(parser.current < vec_getsize(tokens))
	{
		if(parser_check(&parser, TOKENTYPE_KEYWORD_FUNC))
		{
			ast_addbranch(root, parse_function(&parser));
		}
		else if(parser_check(&parser, TOKENTYPE_KEYWORD_TYPE))
		{ 
			ast_addbranch(root, parse_typedeclr(&parser));
		}
		else
		{
			log_error(
				"Parsing error: Unexpected %s ('%s'), at line %zu"
					", column %zu", 
				tokens[parser.current].type->name,
				tokens[parser.current].text,
				tokens[parser.current].line,
				tokens[parser.current].column
			);
		}
	}

	return root;
}

