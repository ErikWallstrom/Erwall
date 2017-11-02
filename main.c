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

#include "argparser.h"
#include "ansicodes.h"
#include "semantics.h"
#include "file.h"
#include "log.h"
#include "str.h"

#include <stdlib.h>

void generateblock(
	struct Str* ccode, 
	struct ASTNode* block, 
	size_t funcpos,
	const char* funcname)
{ 
	for(size_t i = 0; i < vec_getsize(block->branches); i++)
	{ 
		if(block->branches[i]->token.type == TOKENTYPE_KEYWORD_FUNC)
		{ 
			struct ASTNode* funcnode = block->branches[i];
			struct ASTNode* blocknode = funcnode->branches[3];
			struct ASTNode* retnode = funcnode->branches[2];
			struct ASTNode* namenode = funcnode->branches[0];
			struct ASTNode* argsnode = funcnode->branches[1];

			struct Str funccode;
			str_ctor(&funccode, "");

			if(vec_getsize(retnode->branches))
			{ 
				str_appendfmt(
					&funccode, 
					"\nerwall_%s ", 
					retnode->branches[0]->token.text
				);
			}
			else
			{ 
				str_append(&funccode, "\nvoid ");
			}

			str_appendfmt(
				&funccode, 
				"erwall_%s_%s(", 
				funcname,
				namenode->token.text
			); 

			int first = 1;
			for(size_t j = 0; j < vec_getsize(argsnode->branches); j++)
			{ 
				if(!first)
				{ 
					str_append(&funccode, ", ");
				}

				struct ASTNode* varnode = argsnode->branches[j];
				if(varnode->token.type == TOKENTYPE_KEYWORD_LET)
				{ 
					str_append(&funccode, "const ");
				}

				str_appendfmt(
					&funccode, 
					"erwall_%s %s", 
					varnode->branches[1]->token.text,
					varnode->branches[0]->token.text
				);
				
				first = 0;
			}

			str_append(&funccode, ")\n{\n");
			generateblock(ccode, blocknode, funcpos, namenode->token.text);
			str_append(&funccode, "\n}\n");

			str_insert(ccode, funcpos, funccode.data);
			str_dtor(&funccode);
		}
		else if(block->branches[i]->token.type == TOKENTYPE_KEYWORD_TYPE)
		{ 
			struct ASTNode* typenode = block->branches[i];
			str_appendfmt(
				ccode, 
				"typedef erwall_%s erwall_%s;\n", 
				typenode->branches[1]->token.text,
				typenode->branches[0]->token.text
			);
		}
	}
}

struct Str generate(struct ASTNode* ast)
{ 
	const char header[] = { 
		"#include <inttypes.h>\n\n"

		"typedef int8_t		erwall_Int8;\n"
		"typedef int16_t 	erwall_Int16;\n"
		"typedef int32_t 	erwall_Int32;\n"
		"typedef int64_t 	erwall_Int64;\n"
		"typedef uint8_t 	erwall_UInt8;\n"
		"typedef uint16_t 	erwall_UInt16;\n"
		"typedef uint32_t 	erwall_UInt32;\n"
		"typedef uint64_t	erwall_UInt64;\n"
		"typedef float		erwall_Float32;\n" //XXX: Not portable
		"typedef double		erwall_Float64;\n" //XXX: Not portable
		"typedef _Bool		erwall_Bool;\n\n"
	};

	const char footer[] = { 
		"\nint main(int argc, char* argv[])\n"
		"{\n"
		"	return erwall_main();\n"
		"}\n"
	};

	struct Str ccode;
	str_ctor(&ccode, header);

	for(size_t i = 0; i < vec_getsize(ast->branches); i++)
	{ 
		if(ast->branches[i]->token.type == TOKENTYPE_KEYWORD_FUNC)
		{ 
			size_t funcpos = ccode.len;

			struct ASTNode* funcnode = ast->branches[i];
			struct ASTNode* blocknode = funcnode->branches[3];
			struct ASTNode* retnode = funcnode->branches[2];
			struct ASTNode* namenode = funcnode->branches[0];
			struct ASTNode* argsnode = funcnode->branches[1];

			if(vec_getsize(retnode->branches))
			{ 
				str_appendfmt(
					&ccode, 
					"\nerwall_%s ", 
					retnode->branches[0]->token.text
				);
			}
			else
			{ 
				str_append(&ccode, "\nvoid ");
			}

			str_appendfmt(&ccode, "erwall_%s(", namenode->token.text); 
			int first = 1;
			for(size_t j = 0; j < vec_getsize(argsnode->branches); j++)
			{ 
				if(!first)
				{ 
					str_append(&ccode, ", ");
				}

				struct ASTNode* varnode = argsnode->branches[j];
				if(varnode->token.type == TOKENTYPE_KEYWORD_LET)
				{ 
					str_append(&ccode, "const ");
				}

				str_appendfmt(
					&ccode, 
					"erwall_%s %s", 
					varnode->branches[1]->token.text,
					varnode->branches[0]->token.text
				);

				first = 0;
			}

			str_append(&ccode, ")\n{\n");
			generateblock(&ccode, blocknode, funcpos, namenode->token.text);
			str_append(&ccode, "\n}\n");
		}
		else if(ast->branches[i]->token.type == TOKENTYPE_KEYWORD_TYPE)
		{ 
			struct ASTNode* typenode = ast->branches[i];
			str_appendfmt(
				&ccode, 
				"typedef erwall_%s erwall_%s;\n", 
				typenode->branches[1]->token.text,
				typenode->branches[0]->token.text
			);
		}
	}

	str_append(&ccode, footer);
	return ccode;
}

void onargerror(void* udata)
{ 
	argparser_printhelp(udata);
	abort();
}

void onerror(void* udata)
{ 
	(void)udata;
	abort();
}

int main(int argc, char* argv[])
{
	struct ArgParserLongOpt options[] = { 
		{"file", "Which file to compile", 1},
		{"tokenize", "Output tokens", 0},
		{"parse", "Output abstract syntax tree", 0},
		{"generate", "Output C code", 0},
	};

	struct ArgParser argparser;
	log_seterrorhandler(onargerror, &argparser);
	argparser_ctor(
		&argparser, 
		argc, 
		argv, 
		options, 
		sizeof options / sizeof *options
	);

	if(argparser.results[0].used)
	{
		log_seterrorhandler(onerror, NULL);
		struct ANSICode titlecolor = {
			.fg = ANSICODE_FG_GREEN, 
			.bold = 1, 
			.underline = 1
		};

		struct File file;
		file_ctor(&file, argparser.results[0].arg, FILEMODE_READ);
		Vec(struct Token) tokens = tokenize(file.content);

		if(argparser.results[1].used)
		{ 
			ansicode_printf(&titlecolor, "\nTokens:\n\n");
			for(size_t i = 0; i < vec_getsize(tokens); i++)
			{
				printf("%s: ", tokens[i].type->name);
				struct ANSICode color = {
					.fg = ANSICODE_FG_BLUE, 
					.bold = 1, 
				};
				ansicode_printf(&color, "%s\n}n", tokens[i].text);
			}
		}

		struct ASTNode* ast = parse(tokens);

		if(argparser.results[2].used)
		{ 
			ansicode_printf(&titlecolor, "\nAbstract Syntax Tree:\n\n");
			ast_print(ast);
			putchar('\n');
		}

		checksemantics(ast);
		struct Str ccode = generate(ast);

		if(argparser.results[3].used)
		{ 
			ansicode_printf(&titlecolor, "C Output:\n\n");
			printf("%s\n", ccode.data);
		}

		//Cleanup
		//TODO: Cleanup of scopes
		str_dtor(&ccode);
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

	argparser_dtor(&argparser);
}

