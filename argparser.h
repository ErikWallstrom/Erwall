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

#ifndef ARGPARSER_H
#define ARGPARSER_H

#include <stddef.h>
#include "vec.h"

struct ArgParserLongOpt
{ 
	const char* opt;
	const char* info;
	int hasarg;
};

struct ArgParserResult
{ 
	const char* arg;
	int used;
};

struct ArgParser
{ 
	Vec(struct ArgParserResult) results;
	struct ArgParserLongOpt* opts;
	char** argv;
	size_t numopts;
};

struct ArgParser* argparser_ctor(
	struct ArgParser* self, 
	int argc,
	char** argv,
	struct ArgParserLongOpt* opts, 
	size_t numopts
);
void argparser_printhelp(struct ArgParser* self);
void argparser_dtor(struct ArgParser* self);

#endif
