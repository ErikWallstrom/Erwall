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

#include <inttypes.h>
#include <stdlib.h>
#include <time.h>

static uint64_t getperformancefreq(void)
{
	return 1000000000; //10^-9 (nano)
}

static uint64_t getperformancecount(void)
{
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);

	uint64_t ticks = ts.tv_sec;
	ticks *= getperformancefreq();
	ticks += ts.tv_nsec;

	return ticks;
}

static void compile(struct Str* code, const char* filename)
{ 
	log_assert(code, "is NULL");
	log_assert(filename, "is NULL");

	struct Str cfilename;
	str_ctorfmt(
		&cfilename, 
		"%.*s.c", 
		(int)(strchr(filename, '.') - filename), 
		filename
	);

	struct File cfile;
	file_ctor(&cfile, cfilename.data, FILEMODE_WRITE);
	vec_pushwitharr(cfile.content, code->data, code->len);
	file_dtor(&cfile);

	struct Str command;
	str_ctorfmt(
		&command, 
		"gcc %s -o %.*s -Wall -Wextra -Wshadow -Wstrict-prototypes\\\n"
			"\t-Wdouble-promotion -Wjump-misses-init -Wnull-dereference\\\n"
			"\t-Wrestrict -Wlogical-op -Wduplicated-branches "
			"-Wduplicated-cond\\\n"
#ifdef NDEBUG
			"\t-O3 -march=native -mtune=native" //Should this be -O2?
#else
			"\t-Og -g3"
#endif
			" -lm",
		cfilename.data,
		(int)(strchr(filename, '.') - filename), 
		filename
	);

	printf("Command: \n\t%s\n\n", command.data);

	int ret = system(command.data);
	remove(cfilename.data);
	str_dtor(&command);
	str_dtor(&cfilename);

	if(ret) //NOTE: This does not seem to work
	{ 
		putchar('\n');
		log_error("C Compilation failed");
	}
}

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

static void onerror(void* udata)
{ 
	(void)udata;
	abort();
}

#include <time.h>

int main(int argc, char* argv[])
{
	double timetotal = getperformancecount();
	struct ArgParserLongOpt options[] = { 
		{"file", "Which file to compile", 1},
		{"tokenize", "Output tokens", 0},
		{"parse", "Output abstract syntax tree", 0},
		{"symtable", "Output the symbol table", 0},
		{"generate", "Output C code", 0},
		{"compile", "Compile the C code", 0},
		{"all", "Enable all options", 0},
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

	if(argparser.results[6].used)
	{
		argparser.results[1].used = 1;
		argparser.results[2].used = 1;
		argparser.results[3].used = 1;
		argparser.results[4].used = 1;
		argparser.results[5].used = 1;
	}

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

		uint64_t timestart = getperformancecount();
		Vec(struct Str) lines = getlines(file.content);
		Vec(struct erw_Token) tokens = erw_tokenize(file.content, lines);
		uint64_t timestop = getperformancecount();
		double timeelapsed = (timestop - timestart) * 1000.0 
			/ getperformancefreq();
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
			printf("\n(%f ms)\n\n", timeelapsed);
		}

		timestart = getperformancecount();
		struct erw_ASTNode* ast = erw_parse(tokens, lines);
		timestop = getperformancecount();
		timeelapsed = (timestop - timestart) * 1000.0 / getperformancefreq();
		if(argparser.results[2].used)
		{ 
			ansicode_printf(&titlecolor, "\nAbstract Syntax Tree:\n\n");
			erw_ast_print(ast);
			putchar('\n');
			printf("(%f ms)\n\n", timeelapsed);
		}

		timestart = getperformancecount();
		struct erw_Scope* scope = erw_checksemantics(ast, lines);
		timestop = getperformancecount();
		timeelapsed = (timestop - timestart) * 1000.0 / getperformancefreq();
		if(argparser.results[3].used)
		{ 
			ansicode_printf(&titlecolor, "\nSymbol Table:\n\n");
			erw_scope_print(scope);
			putchar('\n');
			printf("(%f ms)\n\n", timeelapsed);
		}

		/*
		//erw_optimize(ast, scope);
		//erw_interpret(ast, scope);

		start = clock();
		struct Str code = erw_generate(ast, scope);
		stop = clock();
		elapsed = (double)(stop - start) * 1000.0 / CLOCKS_PER_SEC;
		total += elapsed;

		if(argparser.results[4].used)
		{
			ansicode_printf(&titlecolor, "\nGenerated C code:\n\n");
			puts(code.data);
			printf("(%f ms)\n\n", elapsed);
		}

		if(argparser.results[5].used)
		{
			ansicode_printf(&titlecolor, "Compiler Output:\n\n");
			start = clock();
			compile(&code, argparser.results[0].arg);
			stop = clock();
			elapsed = (double)(stop - start) * 1000.0 / CLOCKS_PER_SEC;
			total += elapsed;

			putchar('\n');
			printf("(%f ms)\n\n", elapsed);
		}
		*/

		//Cleanup
		erw_scope_dtor(scope);
		erw_ast_dtor(ast);
		for(size_t i = 0; i < vec_getsize(tokens); i++)
		{
			vec_dtor(tokens[i].text);
		}

		vec_dtor(tokens);
		for(size_t i = 0; i < vec_getsize(lines); i++)
		{
			str_dtor(&lines[i]);
		}

		vec_dtor(lines);
		file_dtor(&file);

		printf(
			"\nTotal time: %f ms\n", 
			(getperformancecount() - timetotal) * 1000.0 / getperformancefreq()
		);
	}
	else
	{
		log_error("Expected a file as argument");
	}

	argparser_dtor(&argparser);
}

