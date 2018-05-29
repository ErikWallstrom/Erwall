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

#include "erw_error.h"
#include "ansicode.h"
#include "log.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void erw_error(
	const char* msg, 
	const char* line, 
	size_t linenum, 
	size_t column, 
	size_t to)
{
	log_assert(msg, "is NULL");

	struct ANSICode errcolor = {.fg = ANSICODE_FG_RED, .bold = 1};
	struct ANSICode numcolor = {.fg = ANSICODE_FG_BLUE, .bold = 1};
	struct ANSICode markcolor = {.fg = ANSICODE_FG_MAGENTA, .bold = 1};

	ansicode_fprintf(&errcolor, stderr, "\nError: ");
	if(line)
	{
		fprintf(stderr, "(line ");
		ansicode_fprintf(&numcolor, stderr, "%zu", linenum);
		fprintf(stderr, ", column ");
		ansicode_fprintf(&numcolor, stderr, "%zu", column);
		fprintf(stderr, "): %s", msg);

		size_t printpos = 0;
		for(; printpos < strlen(line); printpos++)
		{
			if(!isblank(line[printpos]))
			{
				break;
			}
		}

		fprintf(stderr, "\n\n    %s\n    ", line + printpos);
		for(size_t i = 0; i < column - printpos - 1; i++)
		{
			fprintf(stderr, " ");
		}

		ansicode_fprintf(&markcolor, stderr, "^");
		for(size_t i = column; i < to; i++)
		{
			ansicode_fprintf(&markcolor, stderr, "~");
		}
		fprintf(stderr, "\n\n");
	}
	else
	{
		fprintf(stderr, "%s\n", msg);
	}

	abort();
}

void erw_warning(
	const char* msg, 
	const char* line, 
	size_t linenum, 
	size_t column, 
	size_t to
)
{
	log_assert(msg, "is NULL");

	struct ANSICode warncolor = {.fg = ANSICODE_FG_YELLOW, .bold = 1};
	struct ANSICode numcolor = {.fg = ANSICODE_FG_BLUE, .bold = 1};
	struct ANSICode markcolor = {.fg = ANSICODE_FG_MAGENTA, .bold = 1};

	ansicode_fprintf(&warncolor, stderr, "\nWarning: ");
	fprintf(stderr, "(line ");
	ansicode_fprintf(&numcolor, stderr, "%zu", linenum);
	fprintf(stderr, ", column ");
	ansicode_fprintf(&numcolor, stderr, "%zu", column);

	size_t printpos = 0;
	for(; printpos < strlen(line); printpos++)
	{
		if(!isblank(line[printpos]))
		{
			break;
		}
	}

	fprintf(stderr, "): %s", msg);
	if(linenum)
	{
		fprintf(stderr, "\n\n    %s\n    ", line + printpos);
		for(size_t i = 0; i < column - printpos - 1; i++)
		{
			fprintf(stderr, " ");
		}

		ansicode_fprintf(&markcolor, stderr, "^");
		for(size_t i = column; i < to; i++)
		{
			ansicode_fprintf(&markcolor, stderr, "~");
		}
		fprintf(stderr, "\n");
	}
}
