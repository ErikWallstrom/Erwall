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
#include "generator.h"
#include "ansicode.h"
#include "file.h"
#include "log.h"

#include <stdlib.h>

void compile(struct Str* ccode, const char* filename)
{ 
	struct Str cfilename;
	str_ctorfmt(
		&cfilename, 
		"%.*s.c", 
		(int)(strchr(filename, '.') - filename), 
		filename
	);

	struct File cfile;
	file_ctor(&cfile, cfilename.data, FILEMODE_WRITE);
	vec_pushwitharr(cfile.content, ccode->data, ccode->len);
	file_dtor(&cfile);

	struct Str command;
	str_ctorfmt(
		&command, 
		"gcc %s -o %.*s -Wall -Wextra -Wshadow -Wstrict-prototypes"
		" -Wdouble-promotion -Wjump-misses-init -Wnull-dereference -Wrestrict"
		" -Wlogical-op -Wduplicated-branches -Wduplicated-cond -Og -g3 -lm",
		cfilename.data,
		(int)(strchr(filename, '.') - filename), 
		filename
	);

	int ret = system(command.data);
	if(ret) //NOTE: This does not seem to work
	{ 
		log_error("C Compilation failed");
	}

	remove(cfilename.data);
	str_dtor(&command);
	str_dtor(&cfilename);
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
				ansicode_printf(&color, "%s\n", tokens[i].text);
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
			ansicode_printf(&titlecolor, "\nGenerated code:\n\n");
			printf("%s\n", ccode.data);
		}

		ansicode_printf(&titlecolor, "\nCompilation Output:\n\n");
		compile(&ccode, argparser.results[0].arg);

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
		log_error("Expected a file as argument");
	}

	argparser_dtor(&argparser);
}

