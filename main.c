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
#include "semantics.h"
#include "file.h"
#include "log.h"

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

int main(int argc, char* argv[])
{
	log_seterrorfatal(NULL, NULL);
	if(argc == 2)
	{
		struct File file;
		file_ctor(&file, argv[1], FILEMODE_READ);
		Vec(struct Token) tokens = tokenize(file.content);
		struct ASTNode* ast = parse(tokens);

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

		checksemantics(ast);
		/*
		Vec(char) ccode = generate(ast);

		ansicode_printf(&titlecolor, "C Output:\n\n");
		printf("%s", ccode);
		putchar('\n');
		*/

		//Cleanup
		//TODO: Cleanup of scopes
		//vec_dtor(ccode);
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

