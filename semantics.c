#include "semantics.h"
#include "scope.h"
#include "log.h"
#include <string.h>

static void checkboolean(
	struct ASTNode* exprnode, 
	struct ASTNode* typenode, 
	struct Scope* scope)
{
	if(vec_getsize(typenode->branches) == 2)
	{
		struct ASTNode* type = scope_gettype(scope, typenode->branches[1]);
		checkboolean(exprnode, scope_gettype(scope, type->branches[0]), scope);
	}
	else //Build-in type
	{
		if(strcmp(typenode->branches[0]->token.text, "Bool"))
		{
			log_error(
				"Semantic error: Expected type '%s', got type "
					"'%s' (%s) (line %zu, column %zu)",
				"Bool",
				typenode->branches[0]->token.text,
				exprnode->istoken ? exprnode->token.text : 
					exprnode->branches[0]->token.text, //?
				exprnode->istoken ? exprnode->token.line :
					exprnode->branches[0]->token.line,
				exprnode->istoken ? exprnode->token.column : 
					exprnode->branches[0]->token.column
			);
		}
	}
}

static void checknumerical(
	struct ASTNode* exprnode, 
	struct ASTNode* typenode, 
	struct Scope* scope)
{
	if(vec_getsize(typenode->branches) == 2)
	{
		struct ASTNode* type = scope_gettype(scope, typenode->branches[1]);
		checknumerical(
			exprnode, 
			scope_gettype(scope, type->branches[0]), 
			scope
		);
	}
	else //Build-in type
	{
		if( strcmp(typenode->branches[0]->token.text, "Int8") &&
			strcmp(typenode->branches[0]->token.text, "Int16") &&
			strcmp(typenode->branches[0]->token.text, "Int32") &&
			strcmp(typenode->branches[0]->token.text, "Int64") &&
			strcmp(typenode->branches[0]->token.text, "UInt8") &&
			strcmp(typenode->branches[0]->token.text, "UInt16") &&
			strcmp(typenode->branches[0]->token.text, "UInt32") &&
			strcmp(typenode->branches[0]->token.text, "UInt64") &&
			strcmp(typenode->branches[0]->token.text, "Float32") &&
			strcmp(typenode->branches[0]->token.text, "Float64")
		)
		{
			log_error(
				"Semantic error: Expected a numerical type, got type "
					"'%s' (%s) (line %zu, column %zu)",
				typenode->branches[0]->token.text,
				exprnode->istoken ? exprnode->token.text : 
					exprnode->branches[0]->token.text, //?
				exprnode->istoken ? exprnode->token.line :
					exprnode->branches[0]->token.line,
				exprnode->istoken ? exprnode->token.column : 
					exprnode->branches[0]->token.column
			);
		}
	}
}

static void checkfunccall(struct ASTNode* callnode, struct Scope* scope);

static struct ASTNode* getexprtype(
	struct ASTNode* exprnode, 
	struct Scope* scope)
{ 
	struct ASTNode* ret = NULL;
	if(exprnode->istoken)
	{ 
		if(vec_getsize(exprnode->branches) > 1)
		{ 
			struct ASTNode* type1 = getexprtype(exprnode->branches[0], scope);
			struct ASTNode* type2 = getexprtype(exprnode->branches[1], scope);

			if(strcmp(
				type1->branches[0]->token.text, 
				type2->branches[0]->token.text))
			{
				log_error(
					"Semantic error: Expected type '%s', got type "
						"'%s' (%s) (line %zu, column %zu)",
					type1->branches[0]->token.text,
					type2->branches[0]->token.text,
					exprnode->istoken ? exprnode->token.text : 
						exprnode->branches[0]->token.text, //?
					exprnode->istoken ? exprnode->token.line :
						exprnode->branches[0]->token.line,
					exprnode->istoken ? exprnode->token.column : 
						exprnode->branches[0]->token.column
				);
			}

			if(exprnode->token.type == TOKENTYPE_OPERATOR_LESS ||
				exprnode->token.type == TOKENTYPE_OPERATOR_LESSOREQUAL ||
				exprnode->token.type == TOKENTYPE_OPERATOR_GREATER ||
				exprnode->token.type == TOKENTYPE_OPERATOR_GREATEROREQUAL)
			{
				struct Token token = {.type = TOKENTYPE_TYPE, .text = "Bool"};
				struct ASTNode* temp = ast_newfromtoken(token);
				ret = scope_gettype(scope, temp);
				checknumerical(exprnode, type1, scope);
				ast_dtor(temp);
			}
			else if(exprnode->token.type == TOKENTYPE_OPERATOR_EQUAL ||
				exprnode->token.type == TOKENTYPE_OPERATOR_NOTEQUAL)
			{
				struct Token token = {.type = TOKENTYPE_TYPE, .text = "Bool"};
				struct ASTNode* temp = ast_newfromtoken(token);
				ret = scope_gettype(scope, temp);
				ast_dtor(temp);
			}
			else if(exprnode->token.type == TOKENTYPE_OPERATOR_OR ||
				exprnode->token.type == TOKENTYPE_OPERATOR_AND)
			{
				struct Token token = {.type = TOKENTYPE_TYPE, .text = "Bool"};
				struct ASTNode* temp = ast_newfromtoken(token);
				ret = scope_gettype(scope, temp);
				checkboolean(exprnode, type1, scope);
				ast_dtor(temp);
			}
			else
			{
				checknumerical(exprnode, type1, scope);
				ret = type1;
			}
		}
		else if(vec_getsize(exprnode->branches) == 1) //Unary operator
		{ 
			ret = getexprtype(exprnode->branches[0], scope);
			if(exprnode->token.type == TOKENTYPE_OPERATOR_NOT)
			{
				checkboolean(exprnode, ret, scope);
			}
			else if(exprnode->token.type ==  TOKENTYPE_OPERATOR_SUB)
			{
				checknumerical(exprnode, ret, scope);
			}
		}
		else //Literal/identifier
		{ 
			if(exprnode->token.type == TOKENTYPE_LITERAL_BOOL)
			{ 
				struct Token token = {.type = TOKENTYPE_TYPE, .text = "Bool"};
				struct ASTNode* temp = ast_newfromtoken(token);
				ret = scope_gettype(scope, temp);
				ast_dtor(temp);
			}
			else if(exprnode->token.type == TOKENTYPE_LITERAL_INT)
			{ 
				struct Token token = {.type = TOKENTYPE_TYPE, .text = "Int32"};
				struct ASTNode* temp = ast_newfromtoken(token);
				ret = scope_gettype(scope, temp);
				ast_dtor(temp);
			}
			else if(exprnode->token.type == TOKENTYPE_LITERAL_FLOAT)
			{ 
				struct Token token = {
					.type = TOKENTYPE_TYPE, 
					.text = "Float32"
				};
				struct ASTNode* temp = ast_newfromtoken(token);
				ret = scope_gettype(scope, temp);
				ast_dtor(temp);
			}
			else if(exprnode->token.type == TOKENTYPE_LITERAL_CHAR)
			{ 
				struct Token token = {.type = TOKENTYPE_TYPE, .text = "Char"};
				struct ASTNode* temp = ast_newfromtoken(token);
				ret = scope_gettype(scope, temp);
				ast_dtor(temp);
			}
			else if(exprnode->token.type == TOKENTYPE_IDENT)
			{ 
				struct ASTNode* varnode = scope_getvariable(scope, exprnode);
				ret = scope_gettype(scope, varnode->branches[1]);
			}
			else
			{ 
				log_info(
					"This shouldn't happen 1' %s", 
					exprnode->token.type->name
				);
			}
		}
	}
	else //Cast/function call
	{ 
		if(exprnode->descriptor == ASTNODETYPE_FUNC_CALL)
		{ 
			checkfunccall(exprnode, scope);
			struct ASTNode* funcnode = scope_getfunction(
				scope, 
				exprnode->branches[0]
			);

			if(vec_getsize(funcnode->branches[2]->branches))
			{ 
				ret = scope_gettype(
					scope, 
					funcnode->branches[2]->branches[0]
				);
			}
			else
			{ 
				log_error(
					"Semantic error: Void function used in expression (%s)"
						" (line %zu, column %zu)",
					exprnode->branches[0]->token.text,
					exprnode->branches[0]->token.line,
					exprnode->branches[0]->token.column
				);
			}
		}
		else if(exprnode->descriptor == ASTNODETYPE_TYPECAST)
		{ 
			getexprtype(exprnode->branches[1], scope);
			ret = scope_gettype(scope, exprnode->branches[0]);
		}
		else
		{ 
			log_info("This shouldn't happen 2' %s", exprnode->descriptor->name);
		}
	}

	return ret;
}

static void checkexprtype(
	struct ASTNode* typenode, 
	struct ASTNode* exprnode, 
	struct Scope* scope)
{ 
	const char* type1 = typenode->branches[0]->token.text;
	const char* type2 = getexprtype(exprnode, scope)->branches[0]->token.text;

	if(strcmp(type1, type2))
	{ 
		log_error(
			"Semantic error: Expected type '%s', got type "
				"'%s' (%s) (line %zu, column %zu)",
			type1,
			type2,
			exprnode->istoken ? exprnode->token.text : //TODO: Fix output
				exprnode->branches[0]->token.text, 
			exprnode->istoken ? exprnode->token.line :
				exprnode->branches[0]->token.line,
			exprnode->istoken ? exprnode->token.column : 
				exprnode->branches[0]->token.column
		);
	}
}

static void checkfunccall(struct ASTNode* callnode, struct Scope* scope)
{ 
	struct ASTNode* namenode = callnode->branches[0];
	struct ASTNode* funcnode = scope_getfunction(scope, namenode);
	struct ASTNode* funcargsnode = funcnode->branches[1];
	struct ASTNode* callargsnode = callnode->branches[1];

	if(vec_getsize(funcargsnode->branches) != 
		vec_getsize(callargsnode->branches))
	{ 
		log_error(
			"Semantic error: Incorrect number of arguments in function call"
				" '%s' (line %zu, column %zu). Expected %zu, got %zu",
			namenode->token.text,
			namenode->token.line,
			namenode->token.column,
			vec_getsize(funcargsnode->branches),
			vec_getsize(callargsnode->branches)
		);
	}

	for(size_t i = 0; i < vec_getsize(funcargsnode->branches); i++)
	{ 
		struct ASTNode* argnode = funcargsnode->branches[i];
		struct ASTNode* argnodetype = argnode->branches[1];
		struct ASTNode* argtype = scope_gettype(scope, argnodetype);

		checkexprtype(argtype, callargsnode->branches[i], scope);
	}
}

static void checkblock(
	struct ASTNode* parentfuncnode, 
	struct ASTNode* blocknode, 
	struct Scope* scope,
	struct Scope* globalscope)
{ 
	for(size_t i = 0; i < vec_getsize(blocknode->branches); i++)
	{ 
		if(blocknode->branches[i]->istoken)
		{ 
			if(blocknode->branches[i]->token.type == TOKENTYPE_KEYWORD_FUNC)
			{
				struct ASTNode* funcnode = blocknode->branches[i];
				scope_addfunction(scope, funcnode);
				struct Scope newscope;
				scope_ctor(&newscope, globalscope);

				struct ASTNode* argsnode = funcnode->branches[1];
				for(size_t j = 0; j < vec_getsize(argsnode->branches); j++)
				{ 
					scope_addvariable(&newscope, argsnode->branches[j]);
				}

				struct ASTNode* newblocknode = funcnode->branches[3];
				checkblock(funcnode, newblocknode, &newscope, globalscope);
			}
			else if(blocknode->branches[i]->token.type == 
				TOKENTYPE_KEYWORD_TYPE)
			{
				struct ASTNode* typenode = blocknode->branches[i];
				scope_addtype(scope, typenode);
			}
			else if(blocknode->branches[i]->token.type == TOKENTYPE_KEYWORD_LET 
				|| blocknode->branches[i]->token.type == TOKENTYPE_KEYWORD_MUT)
			{ 
				struct ASTNode* varnode = blocknode->branches[i];
				struct ASTNode* valuenode = varnode->branches[2];
				if(vec_getsize(valuenode->branches))
				{ 
					struct ASTNode* varnodetype = varnode->branches[1];
					struct ASTNode* vartype = scope_gettype(
						scope,
						varnodetype
					);

					checkexprtype(vartype, valuenode->branches[0], scope);
				}

				scope_addvariable(scope, varnode);
			}
			else if(blocknode->branches[i]->token.type == TOKENTYPE_KEYWORD_IF)
			{ 
				struct ASTNode* ifnode = blocknode->branches[i];
				struct ASTNode* ifexpr = ifnode->branches[0];
				struct ASTNode* ifblock = ifnode->branches[1];
				struct ASTNode* iftype = getexprtype(ifexpr, scope);
				checkboolean(ifexpr, iftype, scope);
				checkblock(parentfuncnode, ifblock, scope, globalscope);

				for(size_t j = 2; j < vec_getsize(ifnode->branches); j++)
				{ 
					if(ifnode->branches[j]->token.type == 
						TOKENTYPE_KEYWORD_ELSEIF)
					{ 
						struct ASTNode* elseifnode = ifnode->branches[j];
						struct ASTNode* elseifexpr = elseifnode->branches[0];
						struct ASTNode* elseiftype = getexprtype(
							elseifexpr, 
							scope
						);
						checkboolean(elseifexpr, elseiftype, scope);
						struct ASTNode* elseifblock = elseifnode->branches[1];
						checkblock(
							parentfuncnode, 
							elseifblock, 
							scope, 
							globalscope
						);
					}
					else //else statement
					{ 
						struct ASTNode* elsenode = ifnode->branches[j];
						struct ASTNode* elseblock = elsenode->branches[0];
						checkblock(
							parentfuncnode, 
							elseblock, 
							scope, 
							globalscope
						);
					}
				}
			}
			else if(blocknode->branches[i]->token.type == 
				TOKENTYPE_KEYWORD_RETURN)
			{ 
				struct ASTNode* retnode = blocknode->branches[i];
				struct ASTNode* rettype = scope_gettype(
					scope, 
					parentfuncnode->branches[2]->branches[0]
				);

				checkexprtype(rettype, retnode->branches[0], scope);
			}
			else if(blocknode->branches[i]->token.type == TOKENTYPE_FOREIGN)
			{ 
				struct ASTNode* foreignnode = blocknode->branches[i];
				struct ASTNode* argsnode = foreignnode->branches[0];
				for(size_t j = 0; j < vec_getsize(argsnode->branches); j++)
				{ 
					getexprtype(argsnode->branches[j], scope);
				}
			}
		}
		else
		{ 
			if(blocknode->branches[i]->descriptor == ASTNODETYPE_FUNC_CALL)
			{ 
				struct ASTNode* callnode = blocknode->branches[i];
				checkfunccall(callnode, scope);
			}
		}
	}
}

void checksemantics(struct ASTNode* ast)
{
	struct Scope globalscope;
	scope_ctor(&globalscope, NULL);

	char* types[] = { 
		"Int8",
		"Int16",
		"Int32",
		"Int64",
		"UInt8",
		"UInt16",
		"UInt32",
		"UInt64",
		"Float32",
		"Float64",
		"Char",
		"Bool",
	};

	struct Token typetoken = {.type = TOKENTYPE_KEYWORD_TYPE, .text = "type"};
	struct Token token = {.type = TOKENTYPE_TYPE};
	struct ASTNode* node;

	for(size_t i = 0; i < sizeof types / sizeof *types; i++)
		//NOTE: Memory leak here
	{  
		node = ast_newfromtoken(typetoken);
		token.text = types[i];
		ast_addbranch(node, ast_newfromtoken(token));
		scope_addtype(&globalscope, node);
	}

	int hasmain = 0;
	for(size_t i = 0; i < vec_getsize(ast->branches); i++)
	{
		if(ast->branches[i]->token.type == TOKENTYPE_KEYWORD_FUNC)
		{
			struct ASTNode* funcnode = ast->branches[i];
			scope_addfunction(&globalscope, funcnode);

			if(!strcmp(funcnode->branches[0]->token.text, "main"))
			{ 
				log_assert(!hasmain, "this should not happen");
				hasmain = 1;
			}

			struct Scope scope;
			scope_ctor(&scope, &globalscope);

			struct ASTNode* argsnode = funcnode->branches[1];
			for(size_t j = 0; j < vec_getsize(argsnode->branches); j++)
			{ 
				scope_addvariable(&scope, argsnode->branches[j]);
			}

			struct ASTNode* blocknode = funcnode->branches[3];
			checkblock(funcnode, blocknode, &scope, &globalscope);
			scope_dtor(&scope);
		}
		else if(ast->branches[i]->token.type == TOKENTYPE_KEYWORD_TYPE)
		{
			struct ASTNode* typenode = ast->branches[i];
			scope_addtype(&globalscope, typenode);
		}
	}

	if(!hasmain)
	{ 
		log_error("Semantic error: Expected a main function");
	}

	scope_dtor(&globalscope);
}
