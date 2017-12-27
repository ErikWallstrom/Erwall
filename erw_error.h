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

#ifndef ERW_ERROR
#define ERW_ERROR

#include <stddef.h>

void erw_error(
	const char* msg, 
	const char* line, 
	size_t linenum, 
	size_t column, 
	size_t to
);

void erw_warning(
	const char* msg, 
	const char* line, 
	size_t linenum, 
	size_t column, 
	size_t to
);

#endif
