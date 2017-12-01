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

#include "erw_optimizer.h"
#include "log.h"

static void erw_optimizefunc(struct erw_Scope* scope, struct Str* lines)
{
	for(size_t i = 0; i < vec_getsize(scope->functions); i++)
	{

	}

	for(size_t i = 0; i < vec_getsize(scope->children); i++)
	{
		erw_optimizefunc(scope->children[i], lines);
	}
}

void erw_optimize(
	struct erw_ASTNode* ast, 
	struct erw_Scope* scope, 
	struct Str* lines
)
{
	log_assert(ast, "is NULL");
	log_assert(scope, "is NULL");
	log_assert(lines, "is NULL");

	erw_optimizefunc(scope, lines);
}

