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

#include "generator.h"
#include "log.h"

static void generateexpr(struct Str* ccode, struct ASTNode* exprnode)
{ 
	if(exprnode->istoken)
	{ 
		if(vec_getsize(exprnode->branches) > 1)
		{ 
			if(exprnode->token.type == TOKENTYPE_OPERATOR_POW)
			{ 
				str_append(ccode, "pow(");
				generateexpr(ccode, exprnode->branches[0]);
				str_append(ccode, ", ");
				generateexpr(ccode, exprnode->branches[1]);
				str_append(ccode, ")");
			}
			else
			{ 
				str_append(ccode, "(");
				generateexpr(ccode, exprnode->branches[0]);
				str_appendfmt(ccode, " %s ", exprnode->token.text);
				generateexpr(ccode, exprnode->branches[1]);
				str_append(ccode, ")");
			}
		}
		else if(vec_getsize(exprnode->branches) == 1)
		{ 
			if(exprnode->branches[0]->token.type == TOKENTYPE_OPERATOR_NOT)
			{ 
				str_append(ccode, "!(");
				generateexpr(ccode, exprnode->branches[0]->branches[0]);
				str_append(ccode, ")");
			}
			else if(exprnode->branches[0]->token.type == TOKENTYPE_OPERATOR_SUB)
			{ 
				str_append(ccode, "-(");
				generateexpr(ccode, exprnode->branches[0]->branches[0]);
				str_append(ccode, ")");
			}
		}
		else
		{ 
			if(exprnode->token.type == TOKENTYPE_LITERAL_BOOL)
			{ 
				if(!strcmp(exprnode->token.text, "true"))
				{ 
					str_append(ccode, "erwall_true");
				}
				else
				{ 
					str_append(ccode, "erwall_false");
				}
			}
			else if(exprnode->token.type == TOKENTYPE_LITERAL_INT)
			{ 
				str_append(ccode, exprnode->token.text);
			}
			else if(exprnode->token.type == TOKENTYPE_LITERAL_FLOAT)
			{ 
				str_append(ccode, exprnode->token.text);
			}
			else if(exprnode->token.type == TOKENTYPE_LITERAL_CHAR)
			{ 
				str_append(ccode, exprnode->token.text);
			}
			else if(exprnode->token.type == TOKENTYPE_IDENT)
			{ 
				str_appendfmt(ccode, "erwall_%s", exprnode->token.text);
			}
			else if(exprnode->token.type == TOKENTYPE_LITERAL_STRING)
			{ 
				str_append(ccode, exprnode->token.text);
			}
		}
	}
	else
	{ 
		if(exprnode->descriptor == ASTNODETYPE_FUNC_CALL)
		{ 
			str_appendfmt(ccode, "erwall_%s(", exprnode->token.text);
			int first = 1;
			for(size_t i = 0; 
				i < vec_getsize(exprnode->branches[1]->branches); 
				i++)
			{ 
				if(!first)
				{ 
					str_append(ccode, ", ");
				}

				generateexpr(ccode, exprnode->branches[1]->branches[i]);
				first = 0;
			}
			str_append(ccode, ")");
		}
		else if(exprnode->descriptor == ASTNODETYPE_TYPECAST)
		{ 
			str_appendfmt(
				ccode, 
				"(erwall_%s)(", 
				exprnode->branches[0]->token.text
			);
			generateexpr(ccode, exprnode->branches[1]);
			str_append(ccode, ")");
		}
	}
}

static void generateblock(
	struct Str* ccode, 
	struct ASTNode* block, 
	size_t funcpos,
	const char* funcname)
{ 
	for(size_t i = 0; i < vec_getsize(block->branches); i++)
	{ 
		if(block->branches[i]->istoken)
		{ 
			if(block->branches[i]->token.type == TOKENTYPE_KEYWORD_FUNC)
			{ 
				struct ASTNode* funcnode = block->branches[i];
				struct ASTNode* blocknode = funcnode->branches[3];
				struct ASTNode* retnode = funcnode->branches[2];
				struct ASTNode* namenode = funcnode->branches[0];
				struct ASTNode* argsnode = funcnode->branches[1];

				struct Str funccode;
				str_ctor(&funccode, "");

				if(vec_getsize(retnode->branches))
				{ 
					str_appendfmt(
						&funccode, 
						"\nerwall_%s ", 
						retnode->branches[0]->token.text
					);
				}
				else
				{ 
					str_append(&funccode, "\nvoid ");
				}

				str_appendfmt(
					&funccode, 
					"erwall_%s_%s(", 
					funcname,
					namenode->token.text
				); 

				int first = 1;
				for(size_t j = 0; j < vec_getsize(argsnode->branches); j++)
				{ 
					if(!first)
					{ 
						str_append(&funccode, ", ");
					}

					struct ASTNode* varnode = argsnode->branches[j];
					if(varnode->token.type == TOKENTYPE_KEYWORD_LET)
					{ 
						str_append(&funccode, "const ");
					}

					str_appendfmt(
						&funccode, 
						"erwall_%s %s", 
						varnode->branches[1]->token.text,
						varnode->branches[0]->token.text
					);
					
					first = 0;
				}

				str_append(&funccode, ")\n{\n");
				generateblock(ccode, blocknode, funcpos, namenode->token.text);
				str_append(&funccode, "\n}\n");

				str_insert(ccode, funcpos, funccode.data);
				str_dtor(&funccode);
			}
			else if(block->branches[i]->token.type == TOKENTYPE_KEYWORD_TYPE)
			{ 
				struct ASTNode* typenode = block->branches[i];
				str_appendfmt(
					ccode, 
					"	typedef erwall_%s erwall_%s;\n", 
					typenode->branches[1]->token.text,
					typenode->branches[0]->token.text
				);
			}
			else if(block->branches[i]->token.type == TOKENTYPE_KEYWORD_LET)
			{ 
				struct ASTNode* letnode = block->branches[i];
				str_appendfmt(
					ccode,
					"	const erwall_%s erwall_%s",
					letnode->branches[1]->token.text,
					letnode->branches[0]->token.text
				);

				if(vec_getsize(letnode->branches[2]->branches))
				{ 
					str_append(ccode, " = ");
					generateexpr(ccode, letnode->branches[2]->branches[0]);
				}

				str_append(ccode, ";\n");
			}
			else if(block->branches[i]->token.type == TOKENTYPE_KEYWORD_MUT)
			{ 
				struct ASTNode* letnode = block->branches[i];
				str_appendfmt(
					ccode,
					"	erwall_%s erwall_%s",
					letnode->branches[1]->token.text,
					letnode->branches[0]->token.text
				);

				if(vec_getsize(letnode->branches[2]->branches))
				{ 
					str_append(ccode, " = ");
					generateexpr(ccode, letnode->branches[2]->branches[0]);
				}

				str_append(ccode, ";\n");
			}
			else if(block->branches[i]->token.type == TOKENTYPE_KEYWORD_IF)
			{ 
			}
			else if(block->branches[i]->token.type == TOKENTYPE_KEYWORD_RETURN)
			{ 
				str_append(ccode, "	return ");
				generateexpr(ccode, block->branches[i]->branches[0]);
				str_append(ccode, ";\n");
			}
			else if(block->branches[i]->token.type == TOKENTYPE_FOREIGN)
			{ 
				struct ASTNode* foreignnode = block->branches[i];
				str_appendfmt(ccode, "	%s(", foreignnode->token.text);
				int first = 1;
				for(size_t j = 0; 
					j < vec_getsize(foreignnode->branches[0]->branches); 
					j++)
				{ 
					if(!first)
					{ 
						str_append(ccode, ", ");
					}

					generateexpr(ccode, foreignnode->branches[0]->branches[j]);
					first = 0;
				}
				str_append(ccode, ");\n");
			}
		}
		else
		{ 
			if(block->branches[i]->descriptor == ASTNODETYPE_FUNC_CALL)
			{ 
				struct ASTNode* callnode = block->branches[i];
				str_appendfmt(
					ccode, 
					"	erwall_%s(", 
					callnode->branches[0]->token.text
				);

				int first = 1;
				for(size_t j = 0; 
					j < vec_getsize(callnode->branches[1]->branches); 
					j++)
				{ 
					if(!first)
					{ 
						str_append(ccode, ", ");
					}

					generateexpr(ccode, callnode->branches[1]->branches[j]);
					first = 0;
				}
				str_append(ccode, ");\n");
			}
		}
	}
}

struct Str generate(struct ASTNode* ast)
{ 
	const char header[] = { 
		"//Generated with Erwall\n\n"
		"#include <inttypes.h>\n"
		"#include <stdio.h>\n"
		"#include <math.h>\n\n"

		"enum {erwall_false, erwall_true};\n\n"

		"typedef int8_t		erwall_Int8;\n"
		"typedef int16_t 	erwall_Int16;\n"
		"typedef int32_t 	erwall_Int32;\n"
		"typedef int64_t 	erwall_Int64;\n"
		"typedef uint8_t 	erwall_UInt8;\n"
		"typedef uint16_t 	erwall_UInt16;\n"
		"typedef uint32_t 	erwall_UInt32;\n"
		"typedef uint64_t	erwall_UInt64;\n"
		"typedef float		erwall_Float32;\n" //XXX: Not portable
		"typedef double		erwall_Float64;\n" //XXX: Not portable
		"typedef _Bool		erwall_Bool;\n\n"
	};

	const char footer[] = { 
		"\nint main(int argc, char* argv[])\n"
		"{\n"
		"	return erwall_main();\n"
		"}\n"
	};

	struct Str ccode;
	str_ctor(&ccode, header);

	for(size_t i = 0; i < vec_getsize(ast->branches); i++)
	{ 
		if(ast->branches[i]->token.type == TOKENTYPE_KEYWORD_FUNC)
		{ 
			size_t funcpos = ccode.len;

			struct ASTNode* funcnode = ast->branches[i];
			struct ASTNode* blocknode = funcnode->branches[3];
			struct ASTNode* retnode = funcnode->branches[2];
			struct ASTNode* namenode = funcnode->branches[0];
			struct ASTNode* argsnode = funcnode->branches[1];

			if(vec_getsize(retnode->branches))
			{ 
				str_appendfmt(
					&ccode, 
					"\nerwall_%s ", 
					retnode->branches[0]->token.text
				);
			}
			else
			{ 
				str_append(&ccode, "\nvoid ");
			}

			str_appendfmt(&ccode, "erwall_%s(", namenode->token.text); 
			int first = 1;
			for(size_t j = 0; j < vec_getsize(argsnode->branches); j++)
			{ 
				if(!first)
				{ 
					str_append(&ccode, ", ");
				}

				struct ASTNode* varnode = argsnode->branches[j];
				if(varnode->token.type == TOKENTYPE_KEYWORD_LET)
				{ 
					str_append(&ccode, "const ");
				}

				str_appendfmt(
					&ccode, 
					"erwall_%s %s", 
					varnode->branches[1]->token.text,
					varnode->branches[0]->token.text
				);

				first = 0;
			}

			str_append(&ccode, ")\n{\n");
			generateblock(&ccode, blocknode, funcpos, namenode->token.text);
			str_append(&ccode, "\n}\n");
		}
		else if(ast->branches[i]->token.type == TOKENTYPE_KEYWORD_TYPE)
		{ 
			struct ASTNode* typenode = ast->branches[i];
			str_appendfmt(
				&ccode, 
				"typedef erwall_%s erwall_%s;\n", 
				typenode->branches[1]->token.text,
				typenode->branches[0]->token.text
			);
		}
	}

	str_append(&ccode, footer);
	return ccode;
}

