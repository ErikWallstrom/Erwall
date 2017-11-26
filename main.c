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

#include "erw_semantics.h"
#include "argparser.h"
#include "ansicode.h"
#include "file.h"
#include "log.h"
#include <stdlib.h>

static Vec(struct Str) getlines(const char* source)
{
	log_assert(source, "is NULL");

	Vec(struct Str) lines = vec_ctor(struct Str, 0);
	char* newlinepos;
	while((newlinepos = strchr(source, '\n')))
	{
		struct Str line;
		size_t offset = newlinepos - source + 1;
		str_ctorfmt(&line, "%.*s", (int)(offset - 1), source);
		vec_pushback(lines, line);
		source += offset;
	}

	struct Str line;
	str_ctor(&line, "");
	vec_pushback(lines, line);
	return lines;
}

static void onargerror(void* udata)
{ 
	argparser_printhelp(udata);
	abort();
}

int main(int argc, char* argv[])
{
	struct ArgParserLongOpt options[] = { 
		{"file", "Which file to compile", 1},
		{"tokenize", "Output tokens", 0},
		{"parse", "Output abstract syntax tree", 0},
		{"generate", "Output C code", 0},
		{"compile", "Compile the C code", 0},
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
		struct ANSICode titlecolor = {
			.fg = ANSICODE_FG_GREEN, 
			.bold = 1, 
			.underline = 1
		};

		struct File file;
		file_ctor(&file, argparser.results[0].arg, FILEMODE_READ);

		Vec(struct Str) lines = getlines(file.content);
		Vec(struct erw_Token) tokens = erw_tokenize(file.content, lines);
		if(argparser.results[1].used) //--tokenize
		{ 
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
		}

		struct erw_ASTNode* ast = erw_parse(tokens, lines);
		if(argparser.results[2].used)
		{ 
			ansicode_printf(&titlecolor, "\nAbstract Syntax Tree:\n\n");
			erw_ast_print(ast);
			putchar('\n');
		}

		erw_checksemantics(ast, lines);
		file_dtor(&file);
	}
	else
	{
		log_error("Expected a file as argument");
	}

	argparser_dtor(&argparser);
}
