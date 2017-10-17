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

#include "tokenizer.h"
#include <stdio.h>
#include "file.h"
#include "log.h"

int main(int argc, char* argv[])
{
	log_seterrorfatal(NULL, NULL);
	if(argc == 2)
	{
		struct File file;
		file_ctor(&file, argv[1], FILEMODE_READ);
		Vec(struct Token) tokens = tokenize(file.content);

		for(size_t i = 0; i < vec_getsize(tokens); i++)
		{
			printf("%s: %s\n", tokens[i].type->name, tokens[i].text);
		}

		//Cleanup
		for(size_t i = 0; i < vec_getsize(tokens); i++)
		{
			vec_dtor(tokens[i].text);
		}

		vec_dtor(tokens);
		file_dtor(&file);
	}
	else
	{
		log_warning("Expected one file. Exiting without doing anything...");
	}
}

