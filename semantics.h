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

#ifndef SEMANTICS_H
#define SEMANTICS_H

#include "parser.h"

struct FunctionSemantics
{
	int isglobal;
	int used;
};

struct FunctionCallSemantics
{
	int isglobal;
};

struct VariableSemantics
{
	int used;
	int hasvalue;
};

void checksemantics(struct ASTNode* ast);

#endif
