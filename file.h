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

#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include "vec.h"

enum FileMode
{
	FILEMODE_READ = 1 << 0,
	FILEMODE_WRITE = 1 << 1,
};

struct File
{
	FILE* raw;
	Vec(char) content;
	//NOTE: 'extension' simply points to path, no memory is allocated
	const char* extension;
	enum FileMode mode;
	//Should file name be included?
};

struct File* file_ctor(
	struct File* self, 
	const char* path, 
	enum FileMode mode
);
void file_flush(struct File* self);
void file_dtor(struct File* self);

#endif
