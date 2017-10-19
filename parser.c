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

static struct Token parser_expect(struct Parser* parser, const struct TokenType* type)
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

static struct ASTNode* parse_function(struct Parser* parser);

static struct ASTNode* parse_block(struct Parser* parser)
{ 
	struct ASTNode* root = ast_newfromnodetype(
		ASTNODETYPE_BLOCK
	);

	parser_expect(parser, TOKENTYPE_LCURLY);

	while(!parser_check(parser, TOKENTYPE_RCURLY))
	{ 
		if(parser_check(parser, TOKENTYPE_KEYWORD_FUNC))
		{ 
			ast_addbranch(root, parse_function(parser));
			continue; //Don't expect semicolon in the end
		}

		parser_expect(parser, TOKENTYPE_END);
	}

	parser_expect(parser, TOKENTYPE_RCURLY);
	return root;
}

static struct ASTNode* parse_function(struct Parser* parser)
{ 
	struct ASTNode* root = ast_newfromtoken(
		parser_expect(parser, TOKENTYPE_KEYWORD_FUNC)
	);

	ast_addbranch(
		root, 
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
		struct ASTNode* argnode = ast_newfromnodetype(
			ASTNODETYPE_FUNC_ARG
		);

		struct ASTNode* identnode = ast_newfromtoken(
			parser_expect(parser, TOKENTYPE_IDENT)
		);

		parser_expect(parser, TOKENTYPE_OPERATOR_DECLR);
		struct ASTNode* typenode = ast_newfromtoken(
			parser_expect(parser, TOKENTYPE_TYPE)
		);

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
		ast_addbranch(
			returnnode, 
			ast_newfromtoken(
				parser_expect(parser, TOKENTYPE_TYPE)
			)
		);
	}

	ast_addbranch(root, argsnode);
	ast_addbranch(root, returnnode);
	ast_addbranch(root, parse_block(parser));
	return root;
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
		else
		{
			log_error(
				"Parsing error: Unexpected %s (%s), at line %zu"
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

