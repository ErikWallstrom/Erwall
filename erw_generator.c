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

#include "erw_generator.h"
#include "log.h"

#define ERW_PREFIX "erw"

struct erw_BlockResult
{
	struct Str blockcode;
	struct Str header;
};

static struct Str erw_generatefunccall(
	struct erw_ASTNode* callnode, 
	struct erw_Scope* scope
);

static struct Str erw_generatetype(
	struct erw_ASTNode* typenode, 
	struct erw_Scope* scope
);

static struct Str erw_generateexpr(
	struct erw_ASTNode* exprnode,
	struct erw_Scope* scope)
{
	log_assert(exprnode, "is NULL");
	log_assert(scope, "is NULL");

	struct Str code;
	str_ctor(&code, "");
	if(exprnode->istoken)
	{ 
		if(vec_getsize(exprnode->branches) > 1)
		{ 
			if(exprnode->token.type == erw_TOKENTYPE_OPERATOR_POW)
			{ 
				struct Str expr1 = erw_generateexpr(
					exprnode->branches[0], 
					scope
				);

				struct Str expr2 = erw_generateexpr(
					exprnode->branches[1], 
					scope
				);

				//TODO: Fix generation for integers
				str_appendfmt(&code, "pow(%s, %s)", expr1.data, expr2.data);
			}
			else if(exprnode->token.type == erw_TOKENTYPE_KEYWORD_CAST)
			{
				struct Str type = erw_generatetype(
					exprnode->branches[0],
					scope
				);

				str_appendfmt(&code, "(%s)(", type.data);
				struct Str expr = erw_generateexpr(
					exprnode->branches[1], 
					scope
				);

				str_append(&code, expr.data);
				str_append(&code, ")");
			}
			else
			{
				str_append(&code, "("); //Is this needed?
				struct Str expr1 = erw_generateexpr(
					exprnode->branches[0], 
					scope
				);
				str_append(&code, expr1.data);
				str_appendfmt(&code, " %s ", exprnode->token.text);

				struct Str expr2 = erw_generateexpr(
					exprnode->branches[1], 
					scope
				);
				str_append(&code, expr2.data);
				str_append(&code, ")"); //Is this needed?
			}
		}
		else if(vec_getsize(exprnode->branches) == 1)
		{ 
			if(exprnode->branches[0]->token.type == erw_TOKENTYPE_OPERATOR_NOT)
			{ 
				str_append(&code, "!(");
				struct Str expr = erw_generateexpr(
					exprnode->branches[0]->branches[0], 
					scope
				);

				str_append(&code, expr.data);
				str_append(&code, ")");
			}
			else if(exprnode->branches[0]->token.type ==
				erw_TOKENTYPE_OPERATOR_SUB)
			{ 
				str_append(&code, "-(");
				struct Str expr = erw_generateexpr(
					exprnode->branches[0]->branches[0], 
					scope
				);

				str_append(&code, expr.data);
				str_append(&code, ")");
			}
		}
		else
		{
			if(exprnode->token.type == erw_TOKENTYPE_LITERAL_BOOL)
			{ 
				if(!strcmp(exprnode->token.text, "true"))
				{ 
					str_append(&code, ERW_PREFIX "_true");
				}
				else
				{ 
					str_append(&code, ERW_PREFIX "_false");
				}
			}
			else if(exprnode->token.type == erw_TOKENTYPE_IDENT)
			{ 
				str_appendfmt(&code, ERW_PREFIX "_%s", exprnode->token.text);
			}
			//TODO: Fix correct generation of different numerical types
			/*
			else if(exprnode->token.type == TOKENTYPE_LITERAL_INT)
			{ 
				str_append(&code, exprnode->token.text);
			}
			*/
			else if(exprnode->token.type == erw_TOKENTYPE_LITERAL_FLOAT)
			{ 
				str_appendfmt(&code, "%sf", exprnode->token.text);
			}
			/*
			else if(exprnode->token.type == TOKENTYPE_LITERAL_CHAR)
			{ 
				str_append(ccode, exprnode->token.text);
			}
			else if(exprnode->token.type == TOKENTYPE_LITERAL_STRING)
			{ 
				str_append(ccode, exprnode->token.text);
			}
			*/
			else
			{ 
				str_append(&code, exprnode->token.text);
			}
		}
	}
	else
	{
		if(exprnode->descriptor == erw_ASTNODETYPE_FUNC_CALL)
		{ 
			struct Str funccall = erw_generatefunccall(exprnode, scope);
			str_append(&code, funccall.data);
		}
	}

	return code;
}

static struct Str erw_generatefunccall(
	struct erw_ASTNode* callnode, 
	struct erw_Scope* scope)
{
	log_assert(callnode, "is NULL");
	log_assert(scope, "is NULL");

	struct Str code;
	str_ctor(&code, "");

	const char* lastname = "";
	int found = 0;
	int local = 0;

	struct erw_Scope* tempscope = scope;
	while(tempscope)
	{
		if(!found)
		{
			for(size_t i = 0; i < vec_getsize(tempscope->functions); i++)
			{
				if(!strcmp(
					tempscope->functions[i].name, 
					callnode->branches[0]->token.text))
				{
					if(tempscope->parent)
					{
						local = 1;
					}

					str_append(&code, tempscope->functions[i].name);
					found = 1;
					break;
				}
			}
		}

		if(found)
		{
			if(tempscope->isfunction)
			{
				if(tempscope->funcname)
				{
					if(strcmp(lastname, tempscope->funcname))
					{
						str_prependfmt(&code, "%s_", tempscope->funcname);
						lastname = tempscope->funcname;
					}
				}
			}
			else
			{
				str_prependfmt(&code, "%zu_", tempscope->index);
			}
		}

		tempscope = tempscope->parent;
	}

	if(local)
	{
		str_prepend(&code, ERW_PREFIX "__");
	}
	else
	{
		str_prepend(&code, ERW_PREFIX "_");
	}

	str_append(&code, "(");
	int first = 1;
	for(size_t i = 0; i < vec_getsize(callnode->branches[1]->branches); i++)
	{
		if(!first)
		{
			str_append(&code, ", ");
		}
		
		struct Str expr = erw_generateexpr(
			callnode->branches[1]->branches[i], 
			scope
		);

		str_append(&code, expr.data);
		first = 0;
	}

	str_append(&code, ")");
	return code;
}

static struct Str erw_generatefuncprot(
	struct erw_ASTNode* funcnode,
	struct erw_Scope* scope
);

static struct Str erw_generatetypedeclr(
	struct erw_ASTNode* typenode, 
	struct erw_Scope* scope
);

static struct erw_BlockResult erw_generateblock(
	struct erw_ASTNode* blocknode, 
	struct erw_Scope* blockscope,
	size_t indentlvl
)
{
	log_assert(blocknode, "is NULL");
	log_assert(blockscope, "is NULL");

	struct erw_BlockResult result;
	str_ctor(&result.blockcode, "");
	str_ctor(&result.header, "");
	size_t scopecounter = 0;

	for(size_t i = 0; i < indentlvl; i++)
	{ 
		str_append(&result.blockcode, "\t");
	}
	str_append(&result.blockcode, "{\n");

	for(size_t i = 0; i < vec_getsize(blocknode->branches); i++)
	{ 
		for(size_t j = 0; j < indentlvl + 1; j++)
		{ 
			str_append(&result.blockcode, "\t");
		}

		if(blocknode->branches[i]->istoken)
		{ 
			if(blocknode->branches[i]->token.type == erw_TOKENTYPE_KEYWORD_FUNC)
			{
				struct erw_ASTNode* funcnode = blocknode->branches[i];
				struct Str funcprot = erw_generatefuncprot(
					funcnode, 
					blockscope
				);

				struct erw_ASTNode* newblock = funcnode->branches[3];
				struct erw_BlockResult newresult = erw_generateblock(
					newblock, 
					blockscope->children[scopecounter], 
					0
				);

				str_append(&result.header, newresult.header.data);
				str_appendfmt(&result.header, "%s\n", funcprot.data);
				str_append(&result.header, newresult.blockcode.data);
				str_append(&result.header, "\n");
				str_append(&result.blockcode, "\n");
				scopecounter++;
			}
			else if(blocknode->branches[i]->token.type == 
				erw_TOKENTYPE_KEYWORD_TYPE)
			{
				struct erw_ASTNode* typenode = blocknode->branches[i];
				struct Str typecode = erw_generatetypedeclr(
					typenode, 
					blockscope
				);
				str_append(&result.header, typecode.data);
			}
			else if(blocknode->branches[i]->token.type == 
				erw_TOKENTYPE_KEYWORD_LET || 
				blocknode->branches[i]->token.type == erw_TOKENTYPE_KEYWORD_MUT)
			{
				struct erw_ASTNode* varnode = blocknode->branches[i];
				struct Str type = erw_generatetype(
					varnode->branches[1], 
					blockscope
				);

				str_appendfmt(
					&result.blockcode, 
					"%s " ERW_PREFIX "_%s",
					type.data,
					varnode->branches[0]->token.text
				);

				if(vec_getsize(varnode->branches[2]->branches))
				{ 
					str_append(&result.blockcode, " = ");
					struct Str expr = erw_generateexpr(
						varnode->branches[2]->branches[0], 
						blockscope
					);

					str_append(&result.blockcode, expr.data);
				}

				str_append(&result.blockcode, ";\n");
			}
			else if(blocknode->branches[i]->token.type == 
				erw_TOKENTYPE_KEYWORD_RETURN)
			{
				str_append(&result.blockcode, "return");
				if(vec_getsize(blocknode->branches[i]->branches))
				{
					struct Str expr = erw_generateexpr(
						blocknode->branches[i]->branches[0], 
						blockscope
					);
					str_appendfmt(&result.blockcode, " %s", expr.data);
				}
				str_append(&result.blockcode, ";\n");
			}
			else if(blocknode->branches[i]->token.type == 
				erw_TOKENTYPE_KEYWORD_IF)
			{
				struct erw_ASTNode* ifnode = blocknode->branches[i];
				str_append(&result.blockcode, "if(");
				struct Str expr = erw_generateexpr(
					ifnode->branches[0], 
					blockscope
				);
				str_appendfmt(&result.blockcode, "%s)\n", expr.data);

				struct erw_BlockResult newblock = erw_generateblock(
					ifnode->branches[1], 
					blockscope->children[scopecounter], 
					indentlvl + 1
				);

				str_append(&result.blockcode, newblock.blockcode.data);
				str_append(&result.header, newblock.header.data);
				for(size_t j = 2; j < vec_getsize(ifnode->branches); j++)
				{ 
					scopecounter++;
					if(ifnode->branches[j]->token.type == 
						erw_TOKENTYPE_KEYWORD_ELSEIF)
					{ 
						for(size_t k = 0; k < indentlvl + 1; k++)
						{ 
							str_append(&result.blockcode, "\t");
						}

						str_append(&result.blockcode, "else if(");
						struct Str newexpr = erw_generateexpr(
							ifnode->branches[j]->branches[0], 
							blockscope
						);

						str_append(&result.blockcode, newexpr.data);
						str_append(&result.blockcode, ")\n");

						struct erw_BlockResult block = erw_generateblock(
							ifnode->branches[j]->branches[1], 
							blockscope->children[scopecounter], 
							indentlvl + 1
						);
						str_append(&result.blockcode, block.blockcode.data);
						str_append(&result.header, block.header.data);
					}
					else //else statement
					{ 
						for(size_t k = 0; k < indentlvl + 1; k++)
						{ 
							str_append(&result.blockcode, "\t");
						}

						str_append(&result.blockcode, "else\n");
						struct erw_BlockResult block = erw_generateblock(
							ifnode->branches[j]->branches[0], 
							blockscope->children[scopecounter], 
							indentlvl + 1
						);
						str_append(&result.blockcode, block.blockcode.data);
						str_append(&result.header, block.header.data);
					}
				}
				str_append(&result.blockcode, "\n");
			}
			else if(blocknode->branches[i]->token.type == erw_TOKENTYPE_FOREIGN)
			{
				struct erw_ASTNode* foreignnode = blocknode->branches[i];
				str_appendfmt(
					&result.blockcode, 
					"%s(", 
					foreignnode->token.text
				);

				int first = 1;
				for(size_t j = 0; 
					j < vec_getsize(foreignnode->branches[0]->branches); 
					j++)
				{ 
					if(!first)
					{ 
						str_append(&result.blockcode, ", ");
					}

					struct Str expr = erw_generateexpr(
						foreignnode->branches[0]->branches[j],
						blockscope
					);

					str_append(&result.blockcode, expr.data);
					first = 0;
				}

				str_append(&result.blockcode, ");\n");
			}
		}
		else
		{
			if(blocknode->branches[i]->descriptor == erw_ASTNODETYPE_FUNC_CALL)
			{ 
				struct Str funccall = erw_generatefunccall(
					blocknode->branches[i],
					blockscope
				);
				str_appendfmt(&result.blockcode, "%s;\n", funccall.data);
			}
		}
	}

	for(size_t i = 0; i < indentlvl; i++)
	{ 
		str_append(&result.blockcode, "\t");
	}
	str_append(&result.blockcode, "}\n");

	return result;
}

static struct Str erw_generatefuncprot(
	struct erw_ASTNode* funcnode,
	struct erw_Scope* scope)
{
	log_assert(funcnode, "is NULL");
	log_assert(scope, "is NULL");

	struct Str code;
	str_ctor(&code, "");

	struct erw_ASTNode* retnode = funcnode->branches[2];
	struct erw_ASTNode* namenode = funcnode->branches[0];
	struct erw_ASTNode* argsnode = funcnode->branches[1];

	const char* lastname = "";
	int found = 0;
	int local = 0;
	struct erw_Scope* tempscope = scope;

	while(tempscope)
	{
		if(!found)
		{
			for(size_t i = 0; i < vec_getsize(tempscope->functions); i++)
			{
				if(!strcmp(
					tempscope->functions[i].name, 
					namenode->token.text))
				{
					if(tempscope->parent)
					{
						local = 1;
					}

					str_append(&code, tempscope->functions[i].name);
					found = 1;
					break;
				}
			}
		}

		if(found)
		{
			if(tempscope->isfunction)
			{
				if(tempscope->funcname)
				{
					if(strcmp(lastname, tempscope->funcname))
					{
						str_prependfmt(&code, "%s_", tempscope->funcname);
						lastname = tempscope->funcname;
					}
				}
			}
			else
			{
				str_prependfmt(&code, "%zu_", tempscope->index);
			}
		}

		tempscope = tempscope->parent;
	}

	if(local)
	{
		str_prepend(&code, ERW_PREFIX "__");
	}
	else
	{
		str_prepend(&code, ERW_PREFIX "_");
	}

	if(vec_getsize(retnode->branches))
	{
		struct Str type = erw_generatetype(retnode->branches[0], scope);
		str_prependfmt(
			&code, 
			"%s ", 
			type.data
		);
	}
	else
	{
		str_prepend(&code, "void ");
	}

	if(local)
	{
		str_prepend(&code, "static ");
	}

	str_append(&code, "("); 
	size_t numargs = vec_getsize(argsnode->branches);
	if(numargs)
	{ 
		int first = 1;
		for(size_t j = 0; j < numargs; j++)
		{ 
			if(!first)
			{ 
				str_append(&code, ", ");
			}

			struct erw_ASTNode* varnode = argsnode->branches[j];
			if(varnode->token.type == erw_TOKENTYPE_KEYWORD_LET)
			{ 
				str_append(&code, "const ");
			}

			struct Str type = erw_generatetype(varnode->branches[1], scope);
			str_appendfmt(
				&code, 
				"%s " ERW_PREFIX "_%s", 
				type.data,
				varnode->branches[0]->token.text
			);

			first = 0;
		}
	}
	else
	{ 
		str_append(&code, "void");
	}

	str_append(&code, ")");
	return code;
}

static struct Str erw_generatetype(
	struct erw_ASTNode* typenode, 
	struct erw_Scope* scope)
{
	log_assert(typenode, "is NULL");
	log_assert(scope, "is NULL");

	struct Str code;
	str_ctor(&code, "");

	const char* lastname = "";
	int found = 0;
	int local = 0;

	struct erw_Scope* tempscope = scope;
	while(tempscope)
	{
		if(!found)
		{
			for(size_t i = 0; i < vec_getsize(tempscope->types); i++)
			{
				if(!strcmp(
					tempscope->types[i].name, 
					typenode->token.text))
				{
					if(tempscope->parent)
					{
						local = 1;
					}

					str_append(&code, tempscope->types[i].name);
					found = 1;
					break;
				}
			}
		}

		if(found)
		{
			if(tempscope->isfunction)
			{
				if(tempscope->funcname)
				{
					if(strcmp(lastname, tempscope->funcname))
					{
						str_prependfmt(&code, "%s_", tempscope->funcname);
						lastname = tempscope->funcname;
					}
				}
			}
			else
			{
				str_prependfmt(&code, "%zu_", tempscope->index);
			}
		}

		tempscope = tempscope->parent;
	}

	if(local)
	{
		str_prepend(&code, ERW_PREFIX "__");
	}
	else
	{
		str_prepend(&code, ERW_PREFIX "_");
	}

	return code;
}

static struct Str erw_generatetypedeclr(
	struct erw_ASTNode* typenode, 
	struct erw_Scope* scope)
{
	log_assert(typenode, "is NULL");
	log_assert(scope, "is NULL");

	struct Str code;
	if(vec_getsize(typenode->branches) == 1)
	{ 
		str_ctor(&code, "typedef struct {char _;} ");
		struct Str type = erw_generatetype(typenode->branches[0], scope);
		str_append(&code, type.data);
		str_append(&code, ";\n");
	}
	else
	{ 
		str_ctor(&code, "typedef ");
		struct Str type1 = erw_generatetype(typenode->branches[0], scope);
		struct Str type2 = erw_generatetype(typenode->branches[1], scope);

		str_appendfmt(&code, "%s ", type2.data);
		str_appendfmt(&code, "%s;\n", type1.data);
	}

	return code;
}

struct Str erw_generate(struct erw_ASTNode* ast, struct erw_Scope* scope)
{ 
	log_assert(ast, "is NULL");
	log_assert(scope, "is NULL");

	const char header[] = { 
		"//Generated with Erwall\n\n" //TODO: Add date and time

		"#include <inttypes.h>\n"
		"#include <stdio.h>\n"
		"#include <math.h>\n\n"

		"typedef int8_t\t\t"  ERW_PREFIX "_Int8;\n"
		"typedef int16_t\t\t" ERW_PREFIX "_Int16;\n"
		"typedef int32_t\t\t" ERW_PREFIX "_Int32;\n"
		"typedef int64_t\t\t" ERW_PREFIX "_Int64;\n"
		"typedef uint8_t\t\t" ERW_PREFIX "_UInt8;\n"
		"typedef uint16_t\t"  ERW_PREFIX "_UInt16;\n"
		"typedef uint32_t\t"  ERW_PREFIX "_UInt32;\n"
		"typedef uint64_t\t"  ERW_PREFIX "_UInt64;\n"
		"typedef float\t\t"   ERW_PREFIX "_Float32;\n" //XXX: Not portable
		"typedef double\t\t"  ERW_PREFIX "_Float64;\n" //XXX: Not portable
		"typedef _Bool\t\t"   ERW_PREFIX "_Bool;\n\n"

		"enum {" ERW_PREFIX "_false, " ERW_PREFIX "_true};\n\n"
	};

	const char footer[] = { 
		"int main(int argc, char* argv[])\n"
		"{\n"
		"\treturn " ERW_PREFIX "_main();\n"
		"}\n\n"
	};

	struct Str code;
	str_ctor(&code, header);

	for(size_t i = 0; i < vec_getsize(scope->types); i++)
	{
		if(!scope->types[i].native)
		{
			struct erw_ASTNode* typenode = scope->types[i].node;
			struct Str typecode = erw_generatetypedeclr(typenode, scope);

			str_append(&code, typecode.data);
			str_dtor(&typecode);
		}
	}

	/*
	str_append(&code, "\n");
	for(size_t i = 0; i < vec_getsize(scope->functions); i++)
	{
		struct erw_ASTNode* funcnode = scope->functions[i].node;
		struct Str funcprot = erw_generatefuncprot(funcnode);

		str_append(&code, funcprot.data);
		str_append(&code, ";");
		str_dtor(&funcprot);
	}
	*/

	for(size_t i = 0; i < vec_getsize(scope->functions); i++)
	{
		struct erw_ASTNode* funcnode = scope->functions[i].node;
		struct Str funcprot = erw_generatefuncprot(funcnode, scope);

		struct erw_ASTNode* blocknode = funcnode->branches[3];
		struct erw_BlockResult result = erw_generateblock(
			blocknode, 
			scope->children[i], 
			0
		);

		str_append(&code, result.header.data);
		str_appendfmt(&code, "%s\n", funcprot.data);
		str_append(&code, result.blockcode.data);
		str_append(&code, "\n");;
		str_dtor(&funcprot);
	}

	str_append(&code, footer);
	return code;
}

