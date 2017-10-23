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

#include "ansicodes.h"
#include "parser.h"
#include "scope.h"
#include "file.h"
#include "log.h"

#include <stdlib.h>

Vec(char) generate(struct ASTNode* ast)
{ 
	Vec(char) ccode = vec_ctor(char, 0);
	const char header[] = { 
		"#include <inttypes.h>\n\n"

		"typedef int8_t		Int8;\n"
		"typedef int16_t 	Int16;\n"
		"typedef int32_t 	Int32;\n"
		"typedef int64_t 	Int64;\n"
		"typedef uint8_t 	UInt8;\n"
		"typedef uint16_t 	UInt16;\n"
		"typedef uint32_t 	UInt32;\n"
		"typedef uint64_t	UInt64;\n"
		"typedef float		Float32;\n" //XXX: This doesn't seem to be portable
		"typedef double		Float64;\n" //XXX: This doesn't seem to be portable
		"typedef _Bool		Bool;\n\n"
	};

	const char footer[] = { 
		"int main(int argc, char* argv[])\n"
		"{\n"
		"	\n"
		"}\n"
	};

	vec_pushbackwitharr(ccode, header, sizeof(header) - 1);
	// *Generate code from AST here*
	vec_pushbackwitharr(ccode, footer, sizeof(footer));

	return ccode;
}

void checkfunccall(struct ASTNode* callnode, struct Scope* scope);

void checkexprtype(
	struct ASTNode* type,
	struct ASTNode* exprnode, 
	struct Scope* scope
)
{
	//NOTE: Difficult as ****
	//TODO: checkfunccall
}

void checkfunccall(struct ASTNode* callnode, struct Scope* scope)
{ 
	struct ASTNode* funcnode = scope_getfunction(
		scope, 
		callnode->branches[0]->token.text
	);

	if(!funcnode)
	{
		log_error(
			"Semantic error: Attempting to call a function that does "
				"not exist (%s) at line %zu, column %zu",
			callnode->branches[0]->token.text,
			callnode->branches[0]->token.line,
			callnode->branches[0]->token.column
		);
	}

	if(vec_getsize(funcnode->branches[1]->branches) != 
		vec_getsize(callnode->branches[1]->branches))
	{
		log_error(
			"Semantic error: Attempting to call function (%s) with an "
				"incorrect number of arguments (expected %zu, got %zu)"
				" at line %zu, column %zu",
			callnode->branches[0]->token.text,
			vec_getsize(funcnode->branches[1]->branches),
			vec_getsize(callnode->branches[1]->branches),
			callnode->branches[0]->token.line,
			callnode->branches[0]->token.column
		);
	}

	for(size_t i = 0; 
		i < vec_getsize(funcnode->branches[1]->branches); i++)
	{
		checkexprtype(
			funcnode->branches[1]->branches[i]->branches[0]->branches[1],
			callnode->branches[1]->branches[i],
			scope
		);
	}
}

void checktypedeclr(struct Scope* scope, struct ASTNode* typedeclr)
{ 
	//NOTE: scope_addtype checks for existing types, no need here
	if(!scope_gettype(scope, typedeclr->branches[1]->token.text))
	{ 
		log_error(
			"Semantic error: Type '%s' does not exist "
				"(at line %zu, column %zu)",
			typedeclr->branches[1]->token.text,
			typedeclr->branches[1]->token.line,
			typedeclr->branches[1]->token.column
		);
	}
}

void createscope(struct ASTNode* blocknode, struct Scope* parent)
{ 
	struct Scope scope;
	scope_ctor(&scope, parent);

	for(size_t i = 0; i < vec_getsize(blocknode->branches); i++)
	{ 
		if(blocknode->branches[i]->istoken)
		{
			if(blocknode->branches[i]->token.type ==
				TOKENTYPE_KEYWORD_FUNC)
			{ 
				scope_addfunction(&scope, blocknode->branches[i]);
				createscope(blocknode->branches[i]->branches[3], &scope);
			}
			else if(blocknode->branches[i]->token.type ==
				TOKENTYPE_KEYWORD_TYPE)
			{
				checktypedeclr(&scope, blocknode->branches[i]);
				scope_addtype(&scope, blocknode->branches[i]);
			}
			else if(blocknode->branches[i]->token.type ==
				TOKENTYPE_KEYWORD_LET)
			{ 
				//TODO
			}
			else if(blocknode->branches[i]->token.type == 
				TOKENTYPE_KEYWORD_IF)
			{ 
				//TODO
			}
		}
		else
		{ 
			if(blocknode->branches[i]->descriptor == ASTNODETYPE_FUNC_CALL)
			{
				checkfunccall(blocknode->branches[i], &scope);
			}
		}
	}
}

void checksemantics(struct ASTNode* ast)
{ 
	struct Scope globalscope;
	scope_ctor(&globalscope, NULL);

	//Built in types
	struct Token typetoken = {.type = TOKENTYPE_KEYWORD_TYPE, .text = "type"};
	struct Token token = {.type = TOKENTYPE_TYPE, .text = "Int8"};
	struct ASTNode* node = ast_newfromtoken(typetoken);
	ast_addbranch(node, ast_newfromtoken(token));
	scope_addtype(&globalscope, node);

	token = (struct Token){.type = TOKENTYPE_TYPE, .text = "Int16"};
	node = ast_newfromtoken(typetoken);
	ast_addbranch(node, ast_newfromtoken(token));
	scope_addtype(&globalscope, node);

	token = (struct Token){.type = TOKENTYPE_TYPE, .text = "Int32"};
	node = ast_newfromtoken(typetoken);
	ast_addbranch(node, ast_newfromtoken(token));
	scope_addtype(&globalscope, node);

	token = (struct Token){.type = TOKENTYPE_TYPE, .text = "Int64"};
	node = ast_newfromtoken(typetoken);
	ast_addbranch(node, ast_newfromtoken(token));
	scope_addtype(&globalscope, node);

	token = (struct Token){.type = TOKENTYPE_TYPE, .text = "UInt8"};
	node = ast_newfromtoken(typetoken);
	ast_addbranch(node, ast_newfromtoken(token));
	scope_addtype(&globalscope, node);

	token = (struct Token){.type = TOKENTYPE_TYPE, .text = "UInt16"};
	node = ast_newfromtoken(typetoken);
	ast_addbranch(node, ast_newfromtoken(token));
	scope_addtype(&globalscope, node);

	token = (struct Token){.type = TOKENTYPE_TYPE, .text = "UInt32"};
	node = ast_newfromtoken(typetoken);
	ast_addbranch(node, ast_newfromtoken(token));
	scope_addtype(&globalscope, node);

	token = (struct Token){.type = TOKENTYPE_TYPE, .text = "UInt64"};
	node = ast_newfromtoken(typetoken);
	ast_addbranch(node, ast_newfromtoken(token));
	scope_addtype(&globalscope, node);

	token = (struct Token){.type = TOKENTYPE_TYPE, .text = "Float32"};
	node = ast_newfromtoken(typetoken);
	ast_addbranch(node, ast_newfromtoken(token));
	scope_addtype(&globalscope, node);

	token = (struct Token){.type = TOKENTYPE_TYPE, .text = "Float64"};
	node = ast_newfromtoken(typetoken);
	ast_addbranch(node, ast_newfromtoken(token));
	scope_addtype(&globalscope, node);

	token = (struct Token){.type = TOKENTYPE_TYPE, .text = "Bool"};
	node = ast_newfromtoken(typetoken);
	ast_addbranch(node, ast_newfromtoken(token));
	scope_addtype(&globalscope, node);

	for(size_t i = 0; i < vec_getsize(ast->branches); i++)
	{
		if(ast->branches[i]->token.type == TOKENTYPE_KEYWORD_FUNC)
		{
			scope_addfunction(&globalscope, ast->branches[i]);
			createscope(ast->branches[i]->branches[3], &globalscope);
		}
		else if(ast->branches[i]->token.type == TOKENTYPE_KEYWORD_TYPE)
		{
			checktypedeclr(&globalscope, ast->branches[i]);
			scope_addtype(&globalscope, ast->branches[i]);
		}
	}
}

int main(int argc, char* argv[])
{
	log_seterrorfatal(NULL, NULL);
	if(argc == 2)
	{
		struct File file;
		file_ctor(&file, argv[1], FILEMODE_READ);
		Vec(struct Token) tokens = tokenize(file.content);
		struct ASTNode* ast = parse(tokens);
		checksemantics(ast);
		Vec(char) ccode = generate(ast);

		struct ANSICode titlecolor = {
			.fg = ANSICODE_FG_GREEN, 
			.bold = 1, 
			.underline = 1
		};

		//Print out
		ansicode_printf(&titlecolor, "\nTokens:\n\n");
		for(size_t i = 0; i < vec_getsize(tokens); i++)
		{
			printf("%s: ", tokens[i].type->name);
			struct ANSICode color = {
				.fg = ANSICODE_FG_BLUE, 
				.bold = 1, 
			};
			ansicode_printf(&color, "%s\n", tokens[i].text);
		}

		ansicode_printf(&titlecolor, "\nAbstract Syntax Tree:\n\n");
		ast_print(ast);
		putchar('\n');

		ansicode_printf(&titlecolor, "C Output:\n\n");
		printf("%s", ccode);
		putchar('\n');

		//Cleanup
		vec_dtor(ccode);
		ast_dtor(ast);

		for(size_t i = 0; i < vec_getsize(tokens); i++)
		{
			vec_dtor(tokens[i].text);
		}

		vec_dtor(tokens);
		file_dtor(&file);
	}
	else
	{
		log_warning(
			"Expected one file as argument. Exiting without doing anything..."
		);
	}
}

