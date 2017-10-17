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

