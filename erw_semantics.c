#include "erw_semantics.h"
#include "erw_error.h"
#include "log.h"

static void erw_checkboolean(
	struct erw_Scope* scope,
	struct erw_TypeSymbol* ret,
	struct erw_ASTNode* firstnode,
	struct erw_ASTNode* lastnode,
	struct Str* lines)
{
	log_assert(scope, "is NULL");
	log_assert(ret, "is NULL");
	log_assert(firstnode, "is NULL");
	log_assert(lastnode, "is NULL");
	log_assert(lines, "is NULL");

	while(ret->type) //Get base type
	{
		ret = ret->type;
	}

	if(strcmp(ret->name, "Bool"))
	{
		struct Str msg;
		str_ctorfmt(&msg, "Expected type 'Bool', got type '%s'", ret->name);
		erw_error(
			msg.data, 
			lines[firstnode->token.linenum - 1].data,
			firstnode->token.linenum, 
			firstnode->token.column,
			(lastnode->token.linenum == firstnode->token.linenum) 
				? (size_t)(lastnode->token.column + vec_getsize(
						lastnode->token.text)) - 2
				: lines[firstnode->token.linenum - 1].len
		);
		str_dtor(&msg);
	}
}

static void erw_checknumerical(
	struct erw_Scope* scope,
	struct erw_TypeSymbol* ret,
	struct erw_ASTNode* firstnode,
	struct erw_ASTNode* lastnode,
	struct Str* lines)
{
	log_assert(scope, "is NULL");
	log_assert(ret, "is NULL");
	log_assert(firstnode, "is NULL");
	log_assert(lastnode, "is NULL");
	log_assert(lines, "is NULL");
 
	while(ret->type) //Get base type
	{
		ret = ret->type;
	}

	if(strcmp(ret->name, "Int8") &&
		strcmp(ret->name, "Int16") &&
		strcmp(ret->name, "Int32") &&
		strcmp(ret->name, "Int64") &&
		strcmp(ret->name, "UInt8") &&
		strcmp(ret->name, "UInt16") &&
		strcmp(ret->name, "UInt32") &&
		strcmp(ret->name, "UInt64") &&
		strcmp(ret->name, "Float32") &&
		strcmp(ret->name, "Float64")
	)
	{
		struct Str msg;
		str_ctorfmt(&msg, "Expected numerical type, got type '%s'", ret->name);
		erw_error(
			msg.data, 
			lines[firstnode->token.linenum - 1].data,
			firstnode->token.linenum, 
			firstnode->token.column,
			(lastnode->token.linenum == firstnode->token.linenum) 
				? (size_t)(lastnode->token.column + vec_getsize(
						lastnode->token.text)) - 2
				: lines[firstnode->token.linenum - 1].len
		);
		str_dtor(&msg);
	}
}

static void erw_checkfunccall(
	struct erw_Scope* scope,
	struct erw_ASTNode* callnode,
	struct Str* lines
);

static struct erw_TypeSymbol* erw_getexprtype(
	struct erw_Scope* scope,
	struct erw_ASTNode* exprnode,
	struct Str* lines)
{
	log_assert(scope, "is NULL");
	log_assert(exprnode, "is NULL");
	log_assert(lines, "is NULL");

	struct erw_TypeSymbol* ret = NULL;
	if(exprnode->istoken)
	{
		if(exprnode->token.type == erw_TOKENTYPE_KEYWORD_CAST)
		{
			//TODO: Check that types are cast-compatible
			ret = erw_scope_gettype(
				scope, 
				&exprnode->branches[0]->token, 
				lines
			);
			erw_getexprtype(scope, exprnode->branches[1], lines);
		}
		else if(vec_getsize(exprnode->branches) == 2) //Binary operator
		{
			struct erw_TypeSymbol* typesym1 = erw_getexprtype(
				scope, 
				exprnode->branches[0], 
				lines
			);
			struct erw_TypeSymbol* typesym2 = erw_getexprtype(
				scope, 
				exprnode->branches[1], 
				lines
			);

			struct erw_ASTNode* firstnode = exprnode->branches[0];
			while(vec_getsize(firstnode->branches))
			{
				firstnode = firstnode->branches[0];
			}

			struct erw_ASTNode* lastnode = exprnode->branches[1];
			while(vec_getsize(lastnode->branches))
			{
				lastnode = lastnode->branches[
					vec_getsize(lastnode->branches) - 1];
			}

			const char* type1 = typesym1->name;
			const char* type2 = typesym2->name;

			if(strcmp(type1, type2))
			{
				struct Str msg;
				str_ctorfmt(
					&msg,
					"%s expected type '%s', got type '%s'",
					exprnode->token.type->name,
					type1,
					type2
				);
				erw_error(
					msg.data, 
					lines[firstnode->token.linenum - 1].data,
					firstnode->token.linenum, 
					lastnode->token.column,
					(lastnode->token.linenum == firstnode->token.linenum) 
						? (size_t)(lastnode->token.column + vec_getsize(
								lastnode->token.text)) - 2
						: lines[firstnode->token.linenum - 1].len
				);
				str_dtor(&msg);
			}

			if(exprnode->token.type == erw_TOKENTYPE_OPERATOR_LESS ||
				exprnode->token.type == erw_TOKENTYPE_OPERATOR_LESSOREQUAL ||
				exprnode->token.type == erw_TOKENTYPE_OPERATOR_GREATER ||
				exprnode->token.type == erw_TOKENTYPE_OPERATOR_GREATEROREQUAL)
			{
				struct erw_Token token = {
					.type = erw_TOKENTYPE_TYPE, 
					.text = "Bool"
				};
				ret = erw_scope_gettype(scope, &token, lines);
				erw_checknumerical(scope, typesym1, firstnode, lastnode, lines);
			}
			else if(exprnode->token.type == erw_TOKENTYPE_OPERATOR_EQUAL ||
				exprnode->token.type == erw_TOKENTYPE_OPERATOR_NOTEQUAL)
			{
				struct erw_Token token = {
					.type = erw_TOKENTYPE_TYPE, 
					.text = "Bool"
				};
				ret = erw_scope_gettype(scope, &token, lines);
			}
			else if(exprnode->token.type == erw_TOKENTYPE_OPERATOR_OR ||
				exprnode->token.type == erw_TOKENTYPE_OPERATOR_AND)
			{
				struct erw_Token token = {
					.type = erw_TOKENTYPE_TYPE, 
					.text = "Bool"
				};
				ret = erw_scope_gettype(scope, &token, lines);
				erw_checkboolean(scope, typesym1, firstnode, lastnode, lines);
			}
			else
			{
				ret = typesym1;
				erw_checknumerical(scope, ret, firstnode, lastnode, lines);
			}
		}
		else if(vec_getsize(exprnode->branches) == 1) //Unary operator
		{
			struct erw_ASTNode* firstnode = exprnode->branches[0];
			while(vec_getsize(firstnode->branches))
			{
				firstnode = firstnode->branches[0];
			}

			struct erw_ASTNode* lastnode = exprnode->branches[0];
			while(vec_getsize(lastnode->branches))
			{
				lastnode = lastnode->branches[
					vec_getsize(lastnode->branches) - 1];
			}

			ret = erw_getexprtype(scope, exprnode->branches[0], lines);
			if(exprnode->token.type == erw_TOKENTYPE_OPERATOR_NOT)
			{
				erw_checkboolean(scope, ret, firstnode, lastnode, lines);
			}
			else if(exprnode->token.type == erw_TOKENTYPE_OPERATOR_SUB)
			{
				erw_checknumerical(scope, ret, firstnode, lastnode, lines);
			}
		}
		else //Literal/identifier
		{
			if(exprnode->token.type == erw_TOKENTYPE_LITERAL_BOOL)
			{ 
				struct erw_Token token = {
					.type = erw_TOKENTYPE_TYPE, 
					.text = "Bool"
				};
				ret = erw_scope_gettype(scope, &token, lines);
			}
			else if(exprnode->token.type == erw_TOKENTYPE_LITERAL_INT)
			{
				struct erw_Token token = {
					.type = erw_TOKENTYPE_TYPE, 
					.text = "Int32"
				};
				ret = erw_scope_gettype(scope, &token, lines);
			}
			else if(exprnode->token.type == erw_TOKENTYPE_LITERAL_FLOAT)
			{
				struct erw_Token token = {
					.type = erw_TOKENTYPE_TYPE, 
					.text = "Float32"
				};
				ret = erw_scope_gettype(scope, &token, lines);
			}
			else if(exprnode->token.type == erw_TOKENTYPE_LITERAL_CHAR)
			{
				struct erw_Token token = {
					.type = erw_TOKENTYPE_TYPE, 
					.text = "Char"
				};
				ret = erw_scope_gettype(scope, &token, lines);
			}
			else if(exprnode->token.type == erw_TOKENTYPE_IDENT)
			{
				struct erw_VariableSymbol* var = erw_scope_getvariable(
					scope, 
					&exprnode->token,
					lines
				);

				if(!var->hasvalue)
				{
					struct Str msg;
					str_ctorfmt(
						&msg, 
						"Uninitialized variable used in expression. Declared at"
							" line %zu, column %zu",
						var->node->branches[0]->token.linenum,
						var->node->branches[0]->token.column
					);
					erw_error(
						msg.data, 
						lines[exprnode->token.linenum - 1].data,
						exprnode->token.linenum, 
						exprnode->token.column,
						exprnode->token.column +
							vec_getsize(exprnode->token.text) - 2
					);
					str_dtor(&msg);
				}

				var->used = 1;
				ret = erw_scope_gettype(
					scope, 
					&var->node->branches[1]->token, 
					lines
				);
			}
			else
			{
				log_info(
					"This shouldn't happen (1): %s (%s) (%zu, %zu)", 
					exprnode->token.text, 
					exprnode->token.type->name,
					exprnode->token.linenum,
					exprnode->token.column
				);
			}
		}
	}
	else //Cast/function call
	{ 
		if(exprnode->descriptor == erw_ASTNODETYPE_FUNC_CALL)
		{
			erw_checkfunccall(scope, exprnode, lines);
			struct erw_FunctionSymbol* func = erw_scope_getfunction(
				scope, 
				&exprnode->branches[0]->token,
				lines
			);

			if(vec_getsize(func->node->branches[2]->branches))
			{ 
				ret = erw_scope_gettype(
					scope, 
					&func->node->branches[2]->branches[0]->token,
					lines
				);
			}
			else
			{ 
				struct erw_ASTNode* firstnode = exprnode;
				while(vec_getsize(firstnode->branches))
				{
					firstnode = firstnode->branches[0];
				}

				struct erw_ASTNode* lastnode = exprnode;
				while(vec_getsize(lastnode->branches))
				{
					lastnode = lastnode->branches[
						vec_getsize(lastnode->branches) - 1];
				}

				struct Str msg;
				str_ctor(&msg, "Void function used in expression");
				erw_error(
					msg.data, 
					lines[firstnode->token.linenum - 1].data,
					firstnode->token.linenum, 
					firstnode->token.column,
					(lastnode->token.linenum == firstnode->token.linenum) 
						? (size_t)(lastnode->token.column + vec_getsize(
								lastnode->token.text)) - 2
						: lines[firstnode->token.linenum - 1].len
				);
				str_dtor(&msg);
			}
		}
		else
		{
			log_info(
				"This shouldn't happen (2): %s (%s)", 
				exprnode->token.text, 
				exprnode->token.type->name
			);
		}
	}

	if(ret) //XXX: Temporary 
	{
		ret->used = 1; //NOTE: !!!
	}
	return ret;
}

static void erw_checkexprtype(
	struct erw_Scope* scope,
	struct erw_ASTNode* exprnode,
	struct erw_TypeSymbol* type,
	struct Str* lines)
{
	log_assert(scope, "is NULL");
	log_assert(exprnode, "is NULL");
	log_assert(type, "is NULL");
	log_assert(lines, "is NULL");

	const char* type1 = type->name;
	const char* type2 = erw_getexprtype(scope, exprnode, lines)->name;

	if(strcmp(type1, type2))
	{
		struct erw_ASTNode* firstnode = exprnode;
		while(vec_getsize(firstnode->branches))
		{
			firstnode = firstnode->branches[0];
		}

		struct erw_ASTNode* lastnode = exprnode;
		while(vec_getsize(lastnode->branches))
		{
			lastnode = lastnode->branches[vec_getsize(lastnode->branches) - 1];
		}

		struct Str msg;
		str_ctorfmt(
			&msg,
			"Expected type '%s', got type '%s'",
			type1,
			type2
		);

		erw_error(
			msg.data, 
			lines[firstnode->token.linenum - 1].data,
			firstnode->token.linenum, 
			firstnode->token.column,
			(lastnode->token.linenum == firstnode->token.linenum) 
				? (size_t)(lastnode->token.column + vec_getsize(
						lastnode->token.text)) - 2
				: lines[firstnode->token.linenum - 1].len
		);
		str_dtor(&msg);
	}
}

static void erw_checkfunccall(
	struct erw_Scope* scope,
	struct erw_ASTNode* callnode,
	struct Str* lines)
{
	log_assert(scope, "is NULL");
	log_assert(callnode, "is NULL");
	log_assert(lines, "is NULL");

	struct erw_ASTNode* namenode = callnode->branches[0];
	struct erw_FunctionSymbol* funcnode = erw_scope_getfunction(
		scope, 
		&namenode->token, 
		lines
	);

	struct erw_ASTNode* funcargsnode = funcnode->node->branches[1];
	struct erw_ASTNode* callargsnode = callnode->branches[1];

	if(vec_getsize(funcargsnode->branches) != 
		vec_getsize(callargsnode->branches))
	{ 
		struct Str msg;
		str_ctorfmt(
			&msg,
			"Incorrect number of arguments in function call ('%s')."
				" Expected %zu, got %zu", 
			namenode->token.text,
			vec_getsize(funcargsnode->branches),
			vec_getsize(callargsnode->branches)
		);

		erw_error(
			msg.data, 
			lines[namenode->token.linenum - 1].data, 
			namenode->token.linenum, 
			namenode->token.column,
			namenode->token.column + 
				vec_getsize(namenode->token.text) - 2
		);
		str_dtor(&msg);
	}

	for(size_t i = 0; i < vec_getsize(funcargsnode->branches); i++)
	{
		struct erw_ASTNode* argnode = funcargsnode->branches[i];
		struct erw_ASTNode* argnodetype = argnode->branches[1];
		struct erw_TypeSymbol* argtype = erw_scope_gettype(
			scope, 
			&argnodetype->token, 
			lines
		);

		erw_checkexprtype(scope, callargsnode->branches[i], argtype, lines);
	}

	if(strcmp(scope->funcname, namenode->token.text)) 
		//Don't add as used if no extern function calls it
	{
		funcnode->used = 1;
	}
}

static void erw_checkfunc(
	struct erw_Scope* scope, 
	struct erw_ASTNode* funcnode, 
	struct Str* lines
);

static void erw_checkblock(
	struct erw_Scope* scope,
	struct erw_ASTNode* blocknode,
	struct Str* lines)
{
	log_assert(scope, "is NULL");
	log_assert(blocknode, "is NULL");
	log_assert(lines, "is NULL");

	for(size_t i = 0; i < vec_getsize(blocknode->branches); i++)
	{ 
		if(blocknode->branches[i]->istoken)
		{ 
			if(blocknode->branches[i]->token.type ==
				erw_TOKENTYPE_KEYWORD_FUNC)
			{
				struct erw_ASTNode* funcnode = blocknode->branches[i];
				erw_checkfunc(scope, funcnode, lines);
			}
			else if(blocknode->branches[i]->token.type ==
				erw_TOKENTYPE_KEYWORD_TYPE)
			{
				struct erw_ASTNode* typenode = blocknode->branches[i];
				erw_scope_addtype(scope, typenode, lines, 0);
			}
			else if(blocknode->branches[i]->token.type ==
					erw_TOKENTYPE_KEYWORD_LET || 
				blocknode->branches[i]->token.type == 	
					erw_TOKENTYPE_KEYWORD_MUT)
			{
				struct erw_ASTNode* varnode = blocknode->branches[i];
				struct erw_ASTNode* valuenode = varnode->branches[2];

				erw_scope_addvariable(scope, varnode, lines);
				if(vec_getsize(valuenode->branches))
				{ 
					struct erw_ASTNode* varnodetype = varnode->branches[1];
					struct erw_TypeSymbol* vartype = erw_scope_gettype(
						scope,
						&varnodetype->token,
						lines
					);

					erw_checkexprtype(
						scope, 
						valuenode->branches[0], 
						vartype, 
						lines
					);
				}
			}
			else if(blocknode->branches[i]->token.type == 
				erw_TOKENTYPE_KEYWORD_IF)
			{
				struct erw_ASTNode* ifnode = blocknode->branches[i];
				struct erw_ASTNode* exprnode = ifnode->branches[0];
				struct erw_ASTNode* ifblock = ifnode->branches[1];
				struct erw_TypeSymbol* iftype = erw_getexprtype(
					scope, 
					exprnode, 
					lines
				);

				struct erw_ASTNode* firstnode = exprnode;
				while(vec_getsize(firstnode->branches))
				{
					firstnode = firstnode->branches[0];
				}

				struct erw_ASTNode* lastnode = exprnode;
				while(vec_getsize(lastnode->branches))
				{
					lastnode = lastnode->branches[
						vec_getsize(lastnode->branches) - 1];
				}

				erw_checkboolean(scope, iftype, firstnode, lastnode, lines);

				struct erw_Scope* newscope = erw_scope_new(
					scope, 
					scope->funcname
				);

				erw_checkblock(newscope, ifblock, lines);
				for(size_t j = 2; j < vec_getsize(ifnode->branches); j++)
				{ 
					if(ifnode->branches[j]->token.type == 
						erw_TOKENTYPE_KEYWORD_ELSEIF)
					{ 
						struct erw_ASTNode* elseifnode = ifnode->branches[j];
						struct erw_ASTNode* elseifexpr = 
							elseifnode->branches[0];
						struct erw_TypeSymbol* elseiftype = erw_getexprtype(
							scope,
							elseifexpr, 
							lines
						);
						firstnode = elseifexpr;
						while(vec_getsize(firstnode->branches))
						{
							firstnode = firstnode->branches[0];
						}

						lastnode = elseifexpr;
						while(vec_getsize(lastnode->branches))
						{
							lastnode = lastnode->branches[
								vec_getsize(lastnode->branches) - 1];
						}

						erw_checkboolean(
							scope, 
							elseiftype, 
							firstnode, 
							lastnode, 
							lines
						);

						struct erw_ASTNode* elseifblock = 
							elseifnode->branches[1];

						newscope = erw_scope_new(
							scope, 
							scope->funcname
						);
						erw_checkblock(newscope, elseifblock, lines);
					}
					else //else statement
					{ 
						struct erw_ASTNode* elsenode = ifnode->branches[j];
						struct erw_ASTNode* elseblock = elsenode->branches[0];

						newscope = erw_scope_new(
							scope, 
							scope->funcname
						);
						erw_checkblock(newscope, elseblock, lines);
						erw_checkblock(newscope, elseblock, lines);
					}
				}
			}
			else if(blocknode->branches[i]->token.type == 
				erw_TOKENTYPE_KEYWORD_RETURN)
			{
				struct erw_ASTNode* retnode = blocknode->branches[i];
				struct erw_Token token = {
					.text = (char*)scope->funcname,
					.type = erw_TOKENTYPE_IDENT
				};
				struct erw_FunctionSymbol* func = erw_scope_getfunction(
					scope, 
					&token,
					lines
				);

				if(vec_getsize(func->node->branches[2]->branches)) 
					//Has return type, i.e not void
				{
					struct erw_TypeSymbol* rettype = erw_scope_gettype(
						scope, 
						&func->node->branches[2]->branches[0]->token,
						lines
					);

					if(!vec_getsize(retnode->branches)) 
					{
						struct Str msg;
						str_ctorfmt(
							&msg,
							"Function ('%s') should return a value of type "
								"'%s'.",
							func->name,
							func->type->name
						);

						erw_error(
							msg.data, 
							lines[retnode->token.linenum - 1].data, 
							retnode->token.linenum, 
							retnode->token.column,
							retnode->token.column + 
								vec_getsize(retnode->token.text) - 2
						);
						str_dtor(&msg);
					}

					erw_checkexprtype(
						scope, 
						retnode->branches[0], 
						rettype, 
						lines
					);
				}
				else
				{
					if(vec_getsize(retnode->branches)) 
					{
						struct Str msg;
						str_ctorfmt(
							&msg,
							"Function ('%s') should not return anything.",
							func->name
						);

						erw_error(
							msg.data, 
							lines[retnode->token.linenum - 1].data, 
							retnode->token.linenum, 
							retnode->token.column,
							retnode->token.column + 
								vec_getsize(retnode->token.text) - 2
						);
						str_dtor(&msg);
					}
				}
			}
			else if(blocknode->branches[i]->token.type == erw_TOKENTYPE_FOREIGN)
			{ 
				struct erw_ASTNode* foreignnode = blocknode->branches[i];
				struct erw_ASTNode* argsnode = foreignnode->branches[0];

				for(size_t j = 0; j < vec_getsize(argsnode->branches); j++)
				{ 
					erw_getexprtype(scope, argsnode->branches[j], lines);
				}
			}
		}
		else
		{ 
			if(blocknode->branches[i]->descriptor == 
				erw_ASTNODETYPE_FUNC_CALL)
			{
				struct erw_ASTNode* callnode = blocknode->branches[i];
				erw_checkfunccall(scope, callnode, lines);
			}
		}
	}
}

static void erw_checkfunc(
	struct erw_Scope* scope, 
	struct erw_ASTNode* funcnode, 
	struct Str* lines)
{
	log_assert(funcnode, "is NULL");
	log_assert(scope, "is NULL");
	log_assert(lines, "is NULL");

	erw_scope_addfunction(scope, funcnode, lines);
	struct erw_Scope* newscope = erw_scope_new(
		scope, 
		funcnode->branches[0]->token.text
	);

	struct erw_ASTNode* argsnode = funcnode->branches[1];
	for(size_t j = 0; j < vec_getsize(argsnode->branches); j++)
	{ 
		erw_scope_addvariable(newscope, argsnode->branches[j], lines);
	}

	struct erw_ASTNode* blocknode = funcnode->branches[3];
	erw_checkblock(newscope, blocknode, lines);
}

static void erw_checkunused(struct erw_Scope* scope, struct Str* lines)
{
	log_assert(scope, "is NULL");
	log_assert(lines, "is NULL");

	for(size_t i = 0; i < vec_getsize(scope->variables); i++)
	{
		struct erw_VariableSymbol* var = &scope->variables[i];
		if(!var->used)
		{
			struct Str msg;
			str_ctor(&msg, "Unused variable");
			erw_warning(
				msg.data, 
				lines[var->node->branches[0]->token.linenum - 1].data,
				var->node->branches[0]->token.linenum, 
				var->node->branches[0]->token.column,
				var->node->branches[0]->token.column + 
					vec_getsize(var->node->branches[0]->token.text) - 2
			);
			str_dtor(&msg);
		}
	}

	for(size_t i = 0; i < vec_getsize(scope->functions); i++)
	{
		struct erw_FunctionSymbol* func = &scope->functions[i];
		if(scope->funcname && strcmp(scope->funcname, func->name)) 
			//Function does not need to call itself
		{
			if(!func->used)
			{
				struct Str msg;
				str_ctor(&msg, "Unused function");
				erw_warning(
					msg.data, 
					lines[func->node->branches[0]->token.linenum - 1].data,
					func->node->branches[0]->token.linenum, 
					func->node->branches[0]->token.column,
					func->node->branches[0]->token.column +
						vec_getsize(func->node->branches[0]->token.text) - 2
				);
				str_dtor(&msg);
			}
		}
	}

	for(size_t i = 0; i < vec_getsize(scope->types); i++)
	{
		struct erw_TypeSymbol* type = &scope->types[i];
		if(type->type)
			/* XXX: Does not check for unused for empty types 
				(to make built-in types not crash the program)
			*/
		{
			if(!type->used)
			{
				struct Str msg;
				str_ctor(&msg, "Unused type");
				erw_warning(
					msg.data, 
					lines[type->node->branches[0]->token.linenum - 1].data,
					type->node->branches[0]->token.linenum, 
					type->node->branches[0]->token.column,
					type->node->branches[0]->token.column +
						vec_getsize(type->node->branches[0]->token.text) - 2
				);
				str_dtor(&msg);
			}
		}
	}

	for(size_t i = 0; i < vec_getsize(scope->children); i++)
	{
		erw_checkunused(scope->children[i], lines);
	}
}

static int erw_checkifreturn(struct erw_ASTNode* ifnode)
	//XXX: Ugly implementation
{
	log_assert(ifnode, "is NULL");

	if(vec_getsize(ifnode->branches) < 3 || 
		ifnode->branches[vec_getsize(ifnode->branches) - 1]->token.type !=
			erw_TOKENTYPE_KEYWORD_ELSE)
	{
		return 0;
	}

	struct erw_ASTNode* blocknode = ifnode->branches[1];
	if(!vec_getsize(blocknode->branches))
	{
		return 0;
	}

	struct erw_ASTNode* laststatement = blocknode->branches[
				vec_getsize(blocknode->branches) - 1];

	if(laststatement->istoken)
	{
		if(laststatement->token.type == erw_TOKENTYPE_KEYWORD_RETURN)
		{}
		else if(laststatement->token.type == erw_TOKENTYPE_KEYWORD_IF)
		{
			erw_checkifreturn(laststatement);
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}

	for(size_t i = 2; i < vec_getsize(ifnode->branches); i++)
	{
		if(ifnode->branches[i]->token.type == erw_TOKENTYPE_KEYWORD_ELSEIF)
		{
			struct erw_ASTNode* elseifnode = ifnode->branches[i];
			struct erw_ASTNode* eifblock = elseifnode->branches[1];
			if(!vec_getsize(eifblock->branches))
			{
				return 0;
			}

			laststatement = eifblock->branches[
				vec_getsize(eifblock->branches) - 1];

			if(laststatement->istoken)
			{
				if(laststatement->token.type == erw_TOKENTYPE_KEYWORD_RETURN)
				{}
				else if(laststatement->token.type == erw_TOKENTYPE_KEYWORD_IF)
				{
					erw_checkifreturn(laststatement);
				}
				else
				{
					return 0;
				}
			}
			else
			{
				return 0;
			}
		}
		else
		{
			struct erw_ASTNode* elseblock = ifnode->branches[i]->branches[0];
			if(!vec_getsize(elseblock->branches))
			{
				return 0;
			}

			laststatement = elseblock->branches[
				vec_getsize(elseblock->branches) - 1];

			if(laststatement->istoken)
			{
				if(laststatement->token.type == 
					erw_TOKENTYPE_KEYWORD_RETURN)
				{}
				else if(laststatement->token.type == erw_TOKENTYPE_KEYWORD_IF)
				{
					erw_checkifreturn(laststatement);
				}
				else
				{
					return 0;
				}
			}
			else
			{
				return 0;
			}
		}
	}

	return 1;
}

static void erw_checkreturn(struct erw_Scope* scope, struct Str* lines)
{
	log_assert(scope, "is NULL");
	log_assert(lines, "is NULL");

	for(size_t i = 0; i < vec_getsize(scope->functions); i++)
	{
		if(scope->functions[i].type) //Has return type
		{
			struct erw_ASTNode* blocknode = scope->functions[i].node->
				branches[3];

			int hasreturn = 0;
			if(vec_getsize(blocknode->branches))
			{
				struct erw_ASTNode* laststatement = blocknode->branches[
					vec_getsize(blocknode->branches) - 1];
			
				if(laststatement->istoken)
				{
					if(laststatement->token.type == erw_TOKENTYPE_KEYWORD_RETURN)
					{
						hasreturn = 1;
					}
					else if(laststatement->token.type == erw_TOKENTYPE_KEYWORD_IF)
					{
						hasreturn = erw_checkifreturn(laststatement);
					}
				}
			}

			if(!hasreturn)
			{
				struct Str msg;
				str_ctor(&msg, "Function expects return at the end");
				erw_error(
					msg.data, 
					lines[scope->functions->node->branches[0]->
						token.linenum - 1].data,
					scope->functions->node->branches[0]->token.linenum, 
					scope->functions->node->branches[0]->token.column,
					scope->functions->node->branches[0]->token.column +
						vec_getsize(
							scope->functions->node->branches[0]->token.text) - 2
				);
				str_dtor(&msg);
			}
		}
	}

	for(size_t i = 0; i < vec_getsize(scope->children); i++)
	{
		erw_checkreturn(scope->children[i], lines);
	}
}

struct erw_Scope* erw_checksemantics(struct erw_ASTNode* ast, struct Str* lines)
{
	log_assert(ast, "is NULL");
	log_assert(lines, "is NULL");

	struct erw_Scope* globalscope = erw_scope_new(NULL, NULL);
	//XXX: Ugly native types implementation
	char* types[] = { 
		"Char",
		"Bool",
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
	};

	struct erw_Token token = {.type = erw_TOKENTYPE_TYPE};
	struct erw_ASTNode* node;
	struct erw_Token typetoken = {
		.type = erw_TOKENTYPE_KEYWORD_TYPE, 
		.text = "type"
	};

	for(size_t i = 0; i < sizeof types / sizeof *types; i++)
		//NOTE: Memory leak here
	{  
		node = erw_ast_newfromtoken(typetoken);
		token.text = types[i];
		struct erw_ASTNode* tokennode = erw_ast_newfromtoken(token);
		erw_ast_addbranch(node, tokennode);
		erw_scope_addtype(globalscope, node, lines, 1);

		//NOTE: Does this work well? seems sketchy
		erw_ast_dtor(node);
	}

	//TODO: semantic check for main function
	for(size_t i = 0; i < vec_getsize(ast->branches); i++)
	{
		if(ast->branches[i]->token.type == erw_TOKENTYPE_KEYWORD_FUNC)
		{
			struct erw_ASTNode* funcnode = ast->branches[i];
			erw_checkfunc(globalscope, funcnode, lines);
		}
		else if(ast->branches[i]->token.type == erw_TOKENTYPE_KEYWORD_TYPE)
		{
			struct erw_ASTNode* typenode = ast->branches[i];
			erw_scope_addtype(globalscope, typenode, lines, 0);
		}
	}

	erw_checkunused(globalscope, lines);
	erw_checkreturn(globalscope, lines);
	return globalscope;
}

