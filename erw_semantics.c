#include "erw_semantics.h"
#include "erw_error.h"
#include "log.h"

static void erw_checkboolean(
	struct erw_Type* type,
	struct erw_ASTNode* firstnode,
	struct erw_ASTNode* lastnode,
	struct Str* lines)
{
	log_assert(type, "is NULL");
	log_assert(
		type->info == erw_TYPEINFO_NAMED, 
		"invalid type (%i)",
		type->info
	);
	log_assert(firstnode, "is NULL");
	log_assert(lastnode, "is NULL");
	log_assert(lines, "is NULL");

	struct erw_Type* base = type;
	while(base->info == erw_TYPEINFO_NAMED)
	{
		base = base->named.type;
	}

	if(base->info != erw_TYPEINFO_BOOL)
	{
		struct Str typename = erw_type_tostring(type);
		struct Str msg;
		str_ctorfmt(
			&msg, 
			"Expected expression of type '%s', got expression of type '%s'", 
			erw_type_builtins[erw_TYPEBUILTIN_BOOL]->named.name,
			typename.data
		);

		erw_error(
			msg.data, 
			lines[firstnode->token->linenum - 1].data,
			firstnode->token->linenum, 
			firstnode->token->column,
			(lastnode->token->linenum == firstnode->token->linenum) 
				? (size_t)(lastnode->token->column + vec_getsize(
						lastnode->token->text)) - 2
				: lines[firstnode->token->linenum - 1].len
		);
		str_dtor(&msg);
	}
}

static void erw_checknumerical(
	struct erw_Type* type,
	struct erw_ASTNode* firstnode,
	struct erw_ASTNode* lastnode,
	struct Str* lines)
{
	log_assert(type, "is NULL");
	log_assert(
		type->info == erw_TYPEINFO_NAMED, 
		"invalid type (%i)",
		type->info
	);
	log_assert(firstnode, "is NULL");
	log_assert(lastnode, "is NULL");
	log_assert(lines, "is NULL");
 
	struct erw_Type* base = type;
	while(base->info == erw_TYPEINFO_NAMED)
	{
		base = base->named.type;
	}

	if(base->info != erw_TYPEINFO_INT && base->info != erw_TYPEINFO_FLOAT)
	{
		struct Str typename = erw_type_tostring(type);
		struct Str msg;
		str_ctorfmt(
			&msg, 
			"Expected numerical type, got type '%s'", 
			typename.data
		);

		erw_error(
			msg.data, 
			lines[firstnode->token->linenum - 1].data,
			firstnode->token->linenum, 
			firstnode->token->column,
			(lastnode->token->linenum == firstnode->token->linenum) 
				? (size_t)(lastnode->token->column + vec_getsize(
						lastnode->token->text)) - 2
				: lines[firstnode->token->linenum - 1].len
		);
		str_dtor(&msg);
	}
}

/*
static void erw_checkint(
	struct erw_Type* type,
	struct erw_ASTNode* firstnode,
	struct erw_ASTNode* lastnode,
	struct Str* lines)
{
	log_assert(type, "is NULL");
	log_assert(firstnode, "is NULL");
	log_assert(lastnode, "is NULL");
	log_assert(lines, "is NULL");
 
	struct erw_Type* base = erw_type_getbase(type);
	if(type->type != erw_TYPETYPE_TYPEDEF || base->type != erw_TYPETYPE_INT)
	{
		struct Str typename = erw_type_tostring(type);
		struct Str msg;
		str_ctorfmt(
			&msg, 
			"Expected integral type, got type '%s'", 
			typename.data
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
*/
static void erw_checkfunccall(
	struct erw_Scope* scope,
	struct erw_ASTNode* callnode,
	struct Str* lines
);

static struct erw_Type* erw_getexprtype(
	struct erw_Scope* scope,
	struct erw_ASTNode* exprnode,
	struct Str* lines)
{
	log_assert(scope, "is NULL");
	log_assert(exprnode, "is NULL");
	log_assert(lines, "is NULL");

	struct erw_Type* ret = NULL;
	if(exprnode->type == erw_ASTNODETYPE_CAST)
	{
		//TODO: Check if types are compatible
		ret = erw_scope_createtype(scope, exprnode->cast.type, lines);
		erw_getexprtype(scope, exprnode->cast.expr, lines); //Check for errors
	}
	else if(exprnode->type == erw_ASTNODETYPE_BINEXPR)
	{
		struct erw_Type* typesym1 = erw_getexprtype(
			scope, 
			exprnode->binexpr.expr1, 
			lines
		);
		struct erw_Type* typesym2 = erw_getexprtype(
			scope, 
			exprnode->binexpr.expr2, 
			lines
		);

		struct erw_ASTNode* firstnode = exprnode->binexpr.expr1;
		while(firstnode->type == erw_ASTNODETYPE_UNEXPR 
			|| firstnode->type == erw_ASTNODETYPE_BINEXPR
			|| firstnode->type == erw_ASTNODETYPE_CAST
			|| firstnode->type == erw_ASTNODETYPE_FUNCCALL)
		{
			if(firstnode->type == erw_ASTNODETYPE_UNEXPR)
			{
				firstnode = firstnode->unexpr.expr;
			}
			else if(firstnode->type == erw_ASTNODETYPE_BINEXPR)
			{
				firstnode = firstnode->binexpr.expr1;
			}
			else if(firstnode->type == erw_ASTNODETYPE_CAST
				|| firstnode->type == erw_ASTNODETYPE_FUNCCALL)
			{
				break;
			}
		}

		struct erw_ASTNode* lastnode = exprnode->binexpr.expr2;
		while(lastnode->type == erw_ASTNODETYPE_UNEXPR 
			|| lastnode->type == erw_ASTNODETYPE_BINEXPR)
		{
			if(lastnode->type == erw_ASTNODETYPE_UNEXPR)
			{
				lastnode = lastnode->unexpr.expr;
			}
			else if(lastnode->type == erw_ASTNODETYPE_BINEXPR)
			{
				lastnode = lastnode->binexpr.expr2;
			}
			else if(lastnode->type == erw_ASTNODETYPE_CAST)
			{
				lastnode = lastnode->cast.expr;
			}
			else if(lastnode->type == erw_ASTNODETYPE_FUNCCALL)
			{
				lastnode = lastnode->funccall.args[
					vec_getsize(lastnode->funccall.args) - 1
				];
			}
		}

		if(!erw_type_compare(typesym1, typesym1))
		{
			struct Str typename1 = erw_type_tostring(typesym1);
			struct Str typename2 = erw_type_tostring(typesym2);
			struct Str msg;
			str_ctorfmt(
				&msg,
				"%s expected type '%s', got type '%s'",
				exprnode->token->type->name,
				typename1.data,
				typename2.data
			);
			erw_error(
				msg.data, 
				lines[firstnode->token->linenum - 1].data,
				firstnode->token->linenum, 
				lastnode->token->column,
				(lastnode->token->linenum == firstnode->token->linenum) 
					? (size_t)(lastnode->token->column + vec_getsize(
							lastnode->token->text)) - 2
					: lines[firstnode->token->linenum - 1].len
			);
			str_dtor(&msg);
			str_dtor(&typename2);
			str_dtor(&typename1);
		}

		if(exprnode->token->type == erw_TOKENTYPE_OPERATOR_LESS 
			|| exprnode->token->type == erw_TOKENTYPE_OPERATOR_LESSOREQUAL 
			|| exprnode->token->type == erw_TOKENTYPE_OPERATOR_GREATER 
			|| exprnode->token->type == erw_TOKENTYPE_OPERATOR_GREATEROREQUAL)
		{
			ret = erw_type_builtins[erw_TYPEBUILTIN_BOOL];
			erw_checknumerical(typesym1, firstnode, lastnode, lines);
		}
		else if(exprnode->token->type == erw_TOKENTYPE_OPERATOR_EQUAL 
			|| exprnode->token->type == erw_TOKENTYPE_OPERATOR_NOTEQUAL)
		{
			ret = erw_type_builtins[erw_TYPEBUILTIN_BOOL];
		}
		else if(exprnode->token->type == erw_TOKENTYPE_OPERATOR_OR 
			|| exprnode->token->type == erw_TOKENTYPE_OPERATOR_AND)
		{
			ret = erw_type_builtins[erw_TYPEBUILTIN_BOOL];
			erw_checkboolean(typesym1, firstnode, lastnode, lines);
		}
		else
		{
			ret = typesym1;
			erw_checknumerical(ret, firstnode, lastnode, lines);
		}
	}
	else if(exprnode->type == erw_ASTNODETYPE_UNEXPR)
	{
		struct erw_ASTNode* firstnode = exprnode->unexpr.expr;
		while(firstnode->type == erw_ASTNODETYPE_UNEXPR 
			|| firstnode->type == erw_ASTNODETYPE_BINEXPR
			|| firstnode->type == erw_ASTNODETYPE_CAST
			|| firstnode->type == erw_ASTNODETYPE_FUNCCALL)
		{
			if(firstnode->type == erw_ASTNODETYPE_UNEXPR)
			{
				firstnode = firstnode->unexpr.expr;
			}
			else if(firstnode->type == erw_ASTNODETYPE_BINEXPR)
			{
				firstnode = firstnode->binexpr.expr1;
			}
			else if(firstnode->type == erw_ASTNODETYPE_CAST
				|| firstnode->type == erw_ASTNODETYPE_FUNCCALL)
			{
				break;
			}
		}

		struct erw_ASTNode* lastnode = exprnode->unexpr.expr;
		while(lastnode->type == erw_ASTNODETYPE_UNEXPR 
			|| lastnode->type == erw_ASTNODETYPE_BINEXPR)
		{
			if(lastnode->type == erw_ASTNODETYPE_UNEXPR)
			{
				lastnode = lastnode->unexpr.expr;
			}
			else if(lastnode->type == erw_ASTNODETYPE_BINEXPR)
			{
				lastnode = lastnode->binexpr.expr2;
			}
			else if(lastnode->type == erw_ASTNODETYPE_CAST)
			{
				lastnode = lastnode->cast.expr;
			}
			else if(lastnode->type == erw_ASTNODETYPE_FUNCCALL)
			{
				lastnode = lastnode->funccall.args[
					vec_getsize(lastnode->funccall.args) - 1
				];
			}
		}

		if(exprnode->token->type == erw_TOKENTYPE_OPERATOR_BITAND)
		{
			if(exprnode->unexpr.left)
			{
				//Should it only handle identifiers?
				struct erw_Type* type = erw_type_new(
					erw_TYPEINFO_REFERENCE, 
					NULL
				);

				type->reference.mutable = 0; //NOTE: Temporary
				type->reference.size = sizeof(void*); //NOTE: Temporary
				type->reference.type = erw_getexprtype(
					scope, 
					exprnode->unexpr.expr, 
					lines
				);

				ret = type;
			}
			else
			{
				struct erw_Type* type = erw_getexprtype(
					scope,
					exprnode->unexpr.expr,
					lines
				);
				
				if(type->info != erw_TYPEINFO_REFERENCE)
				{
					struct Str str = erw_type_tostring(type);
					struct Str msg;
					str_ctorfmt(
						&msg, 
						"Trying to dereference a non-reference type (%s)",
						str.data
					);
					erw_error(
						msg.data, 
						lines[firstnode->token->linenum - 1].data,
						firstnode->token->linenum, 
						firstnode->token->column,
						(lastnode->token->linenum == firstnode->token->linenum) 
							? (size_t)(lastnode->token->column + vec_getsize(
									lastnode->token->text)) - 2
							: lines[firstnode->token->linenum - 1].len
					);
					str_dtor(&msg);
					str_dtor(&str);
				}

				ret = type->reference.type;
			}
		}
		else
		{
			ret = erw_getexprtype(scope, exprnode->unexpr.expr, lines);
			if(exprnode->token->type == erw_TOKENTYPE_OPERATOR_NOT)
			{
				erw_checkboolean(ret, firstnode, lastnode, lines);
			}
			else if(exprnode->token->type == erw_TOKENTYPE_OPERATOR_SUB)
			{
				erw_checknumerical(ret, firstnode, lastnode, lines);
			}
			else
			{
				log_assert(
					0, 
					"this shouldn't happen (%s)", 
					exprnode->token->type->name
				);
			}
		}
	}
	else if(exprnode->type == erw_ASTNODETYPE_FUNCCALL)
	{
		erw_checkfunccall(scope, exprnode, lines);
		struct erw_FuncDeclr* func = erw_scope_getfunc(
			scope, 
			exprnode->funccall.name,
			lines
		);

		if(func->type)
		{ 
			ret = func->type;
		}
		else
		{ 
			struct erw_Token* token = exprnode->funccall.name;
			struct Str msg;
			str_ctor(&msg, "Void function used in expression");
			erw_error(
				msg.data, 
				lines[token->linenum - 1].data,
				token->linenum, 
				token->column,
				token->column + vec_getsize(token->text) - 2 //?
			);
			str_dtor(&msg);
		}
	}
	else if(exprnode->type == erw_ASTNODETYPE_LITERAL)
	{
		if(exprnode->token->type == erw_TOKENTYPE_LITERAL_BOOL)
		{ 
			ret = erw_type_builtins[erw_TYPEBUILTIN_BOOL];
		}
		else if(exprnode->token->type == erw_TOKENTYPE_LITERAL_INT)
		{
			ret = erw_type_builtins[erw_TYPEBUILTIN_INT32];
		}
		else if(exprnode->token->type == erw_TOKENTYPE_LITERAL_FLOAT)
		{
			ret = erw_type_builtins[erw_TYPEBUILTIN_FLOAT32];
		}
		else if(exprnode->token->type == erw_TOKENTYPE_LITERAL_CHAR)
		{
			ret = erw_type_builtins[erw_TYPEBUILTIN_CHAR];
		}
		else if(exprnode->token->type == erw_TOKENTYPE_IDENT)
		{
			struct erw_VarDeclr* var = erw_scope_getvar(
				scope, 
				exprnode->token,
				lines
			);

			/* TODO: Implement this
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
			*/

			var->used = 1;
			ret = var->type;
		}
		else
		{
			log_assert(
				0,
				"this shouldn't happen: %s (%s) (%zu, %zu)", 
				exprnode->token->text, 
				exprnode->token->type->name,
				exprnode->token->linenum,
				exprnode->token->column
			);
		}
	}

	return ret;
}

static void erw_checkexprtype(
	struct erw_Scope* scope,
	struct erw_ASTNode* exprnode,
	struct erw_Type* type,
	struct Str* lines)
{
	log_assert(scope, "is NULL");
	log_assert(exprnode, "is NULL");
	log_assert(type, "is NULL");
	log_assert(lines, "is NULL");

	struct erw_Type* type2 = erw_getexprtype(scope, exprnode, lines);
	if(!erw_type_compare(type, type2))
	{
		struct erw_ASTNode* firstnode = exprnode;
		while(firstnode->type == erw_ASTNODETYPE_UNEXPR 
			|| firstnode->type == erw_ASTNODETYPE_BINEXPR
			|| firstnode->type == erw_ASTNODETYPE_CAST
			|| firstnode->type == erw_ASTNODETYPE_FUNCCALL)
		{
			if(firstnode->type == erw_ASTNODETYPE_UNEXPR)
			{
				firstnode = firstnode->unexpr.expr;
			}
			else if(firstnode->type == erw_ASTNODETYPE_BINEXPR)
			{
				firstnode = firstnode->binexpr.expr1;
			}
			else if(firstnode->type == erw_ASTNODETYPE_CAST
				|| firstnode->type == erw_ASTNODETYPE_FUNCCALL)
			{
				break;
			}
		}

		struct erw_ASTNode* lastnode = exprnode;
		while(lastnode->type == erw_ASTNODETYPE_UNEXPR 
			|| lastnode->type == erw_ASTNODETYPE_BINEXPR)
		{
			if(lastnode->type == erw_ASTNODETYPE_UNEXPR)
			{
				lastnode = lastnode->unexpr.expr;
			}
			else if(lastnode->type == erw_ASTNODETYPE_BINEXPR)
			{
				lastnode = lastnode->binexpr.expr2;
			}
			else if(lastnode->type == erw_ASTNODETYPE_CAST)
			{
				lastnode = lastnode->cast.expr;
			}
			else if(lastnode->type == erw_ASTNODETYPE_FUNCCALL)
			{
				lastnode = lastnode->funccall.args[
					vec_getsize(lastnode->funccall.args) - 1
				];
			}
		}

		struct Str typestr = erw_type_tostring(type);
		struct Str type2str = erw_type_tostring(type2);
		struct Str msg;
		str_ctorfmt(
			&msg,
			"Expected expression of type '%s', got expression of type '%s'",
			typestr.data,
			type2str.data
		);

		erw_error(
			msg.data, 
			lines[firstnode->token->linenum - 1].data,
			firstnode->token->linenum, 
			firstnode->token->column,
			(lastnode->token->linenum == firstnode->token->linenum) 
				? (size_t)(lastnode->token->column + vec_getsize(
						lastnode->token->text)) - 2
				: lines[firstnode->token->linenum - 1].len
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
	log_assert(
		callnode->type == erw_ASTNODETYPE_FUNCCALL, 
		"invalid type (%s)",
		callnode->type->name
	);
	log_assert(lines, "is NULL");

	struct erw_FuncDeclr* func = erw_scope_getfunc(
		scope, 
		callnode->funccall.name, 
		lines
	);

	if(vec_getsize(func->node->funcdef.params) != 
		vec_getsize(callnode->funccall.args))
	{ 
		struct Str msg;
		str_ctorfmt(
			&msg,
			"Incorrect number of arguments in function call ('%s')."
				" Expected %zu, got %zu", 
			callnode->funccall.name->text,
			vec_getsize(func->node->funcdef.params),
			vec_getsize(callnode->funccall.args)
		);

		erw_error(
			msg.data, 
			lines[callnode->funccall.name->linenum - 1].data, 
			callnode->funccall.name->linenum, 
			callnode->funccall.name->column,
			callnode->funccall.name->column + 
				vec_getsize(callnode->funccall.name->text) - 2
		);
		str_dtor(&msg);
	}

	for(size_t i = 0; i < vec_getsize(func->node->funcdef.params); i++)
	{
		struct erw_Type* type = erw_scope_createtype(
			scope,
			func->node->funcdef.params[i],
			lines
		);

		erw_checkexprtype(
			scope, 
			callnode->funccall.args[i], 
			type,
			lines
		);
		erw_type_dtor(type);
	}

	if(strcmp(scope->funcname, callnode->funccall.name->text)) 
		//Don't flag as used if no extern function calls it
	{
		func->used = 1;
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
	log_assert(
		blocknode->type == erw_ASTNODETYPE_BLOCK, 
		"invalid size (%s)", 
		blocknode->type->name
	);
	log_assert(lines, "is NULL");

	for(size_t i = 0; i < vec_getsize(blocknode->block.stmts); i++)
	{ 
		if(blocknode->block.stmts[i]->type == erw_ASTNODETYPE_FUNCDEF)
		{
			erw_checkfunc(scope, blocknode->block.stmts[i], lines);
		}
		else if(blocknode->block.stmts[i]->type == erw_ASTNODETYPE_TYPEDECLR)
		{
			erw_scope_addtypedeclr(scope, blocknode->block.stmts[i], lines);
		}
		else if(blocknode->block.stmts[i]->type == erw_ASTNODETYPE_VARDECLR)
		{
			erw_scope_addvardeclr(scope, blocknode->block.stmts[i], lines);
			if(blocknode->block.stmts[i]->vardeclr.value)
			{ 
				struct erw_Type* vartype = erw_scope_createtype(
					scope,
					blocknode->block.stmts[i]->vardeclr.type,
					lines
				);

				erw_checkexprtype(
					scope, 
					blocknode->block.stmts[i]->vardeclr.value, 
					vartype, 
					lines
				);
			}
		}
		else if(blocknode->block.stmts[i]->type == erw_ASTNODETYPE_IF)
		{
			struct erw_Type* iftype = erw_getexprtype(
				scope, 
				blocknode->block.stmts[i]->if_.expr, 
				lines
			);

			struct erw_ASTNode* firstnode = blocknode->block.stmts[i]->if_.expr;
			while(firstnode->type == erw_ASTNODETYPE_UNEXPR 
				|| firstnode->type == erw_ASTNODETYPE_BINEXPR
				|| firstnode->type == erw_ASTNODETYPE_CAST
				|| firstnode->type == erw_ASTNODETYPE_FUNCCALL)
			{
				if(firstnode->type == erw_ASTNODETYPE_UNEXPR)
				{
					firstnode = firstnode->unexpr.expr;
				}
				else if(firstnode->type == erw_ASTNODETYPE_BINEXPR)
				{
					firstnode = firstnode->binexpr.expr1;
				}
				else if(firstnode->type == erw_ASTNODETYPE_CAST
					|| firstnode->type == erw_ASTNODETYPE_FUNCCALL)
				{
					break;
				}
			}

			struct erw_ASTNode* lastnode = blocknode->block.stmts[i]->if_.expr;
			while(lastnode->type == erw_ASTNODETYPE_UNEXPR 
				|| lastnode->type == erw_ASTNODETYPE_BINEXPR)
			{
				if(lastnode->type == erw_ASTNODETYPE_UNEXPR)
				{
					lastnode = lastnode->unexpr.expr;
				}
				else if(lastnode->type == erw_ASTNODETYPE_BINEXPR)
				{
					lastnode = lastnode->binexpr.expr2;
				}
				else if(lastnode->type == erw_ASTNODETYPE_CAST)
				{
					lastnode = lastnode->cast.expr;
				}
				else if(lastnode->type == erw_ASTNODETYPE_FUNCCALL)
				{
					lastnode = lastnode->funccall.args[
						vec_getsize(lastnode->funccall.args) - 1
					];
				}
			}

			erw_checkboolean(iftype, firstnode, lastnode, lines);
			struct erw_Scope* newscope = erw_scope_new(
				scope, 
				scope->funcname,
				vec_getsize(scope->children),
				0
			);

			erw_checkblock(
				newscope, 
				blocknode->block.stmts[i]->if_.block, 
				lines)
			;
			for(size_t j = 0; 
				j < vec_getsize(blocknode->block.stmts[i]->if_.elseifs); 
				j++)
			{ 
				struct erw_Type* elseiftype = erw_getexprtype(
					scope,
					blocknode->block.stmts[i]->if_.elseifs[j]->elseif.expr, 
					lines
				);
				firstnode = blocknode->block.stmts[i]->if_.elseifs[j]->elseif
					.expr;
				while(firstnode->type == erw_ASTNODETYPE_UNEXPR 
					|| firstnode->type == erw_ASTNODETYPE_BINEXPR
					|| firstnode->type == erw_ASTNODETYPE_CAST
					|| firstnode->type == erw_ASTNODETYPE_FUNCCALL)
				{
					if(firstnode->type == erw_ASTNODETYPE_UNEXPR)
					{
						firstnode = firstnode->unexpr.expr;
					}
					else if(firstnode->type == erw_ASTNODETYPE_BINEXPR)
					{
						firstnode = firstnode->binexpr.expr1;
					}
					else if(firstnode->type == erw_ASTNODETYPE_CAST
						|| firstnode->type == erw_ASTNODETYPE_FUNCCALL)
					{
						break;
					}
				}

				lastnode = blocknode->block.stmts[i]->if_.elseifs[j]->elseif
					.expr;
				while(lastnode->type == erw_ASTNODETYPE_UNEXPR 
					|| lastnode->type == erw_ASTNODETYPE_BINEXPR)
				{
					if(lastnode->type == erw_ASTNODETYPE_UNEXPR)
					{
						lastnode = lastnode->unexpr.expr;
					}
					else if(lastnode->type == erw_ASTNODETYPE_BINEXPR)
					{
						lastnode = lastnode->binexpr.expr2;
					}
					else if(lastnode->type == erw_ASTNODETYPE_CAST)
					{
						lastnode = lastnode->cast.expr;
					}
					else if(lastnode->type == erw_ASTNODETYPE_FUNCCALL)
					{
						lastnode = lastnode->funccall.args[
							vec_getsize(lastnode->funccall.args) - 1
						];
					}
				}

				erw_checkboolean(elseiftype, firstnode, lastnode, lines);
				newscope = erw_scope_new(
					scope, 
					scope->funcname,
					vec_getsize(scope->children),
					0
				);
				erw_checkblock(
					newscope, 
					blocknode->block.stmts[i]->if_.elseifs[j]->elseif.block, 
					lines
				);
			}

			if(blocknode->block.stmts[i]->if_.else_)
			{ 
				newscope = erw_scope_new(
					scope, 
					scope->funcname,
					vec_getsize(scope->children),
					0
				);
				erw_checkblock(
					newscope, 
					blocknode->block.stmts[i]->if_.else_->else_.block, 
					lines
				);
			}
		}
		else if(blocknode->block.stmts[i]->type == erw_ASTNODETYPE_RETURN)
		{
			struct erw_Token token = { //XXX: Ugly
				.text = (char*)scope->funcname, 
				.type = erw_TOKENTYPE_IDENT
			};
			struct erw_FuncDeclr* func = erw_scope_getfunc(
				scope, 
				&token,
				lines
			);

			if(func->node->funcdef.type) //Has return type, i.e not void
			{
				if(!blocknode->block.stmts[i]->return_.expr) 
				{
					struct Str typename = erw_type_tostring(func->type);
					struct Str msg;
					str_ctorfmt(
						&msg,
						"Function ('%s') should return a value of type "
							"'%s'.",
						func->node->funcdef.name->text,
						typename.data
					);

					erw_error(
						msg.data, 
						lines[blocknode->block.stmts[i]->token->linenum - 1]
							.data, 
						blocknode->block.stmts[i]->token->linenum, 
						blocknode->block.stmts[i]->token->column,
						blocknode->block.stmts[i]->token->column + 
							vec_getsize(blocknode->block.stmts[i]->token->text)
							- 2
					);
					str_dtor(&msg);
				}

				erw_checkexprtype(
					scope, 
					blocknode->block.stmts[i]->return_.expr, 
					func->type, 
					lines
				);
			}
			else
			{
				if(blocknode->block.stmts[i]->return_.expr) 
				{
					struct Str msg;
					str_ctorfmt(
						&msg,
						"Function ('%s') should not return anything.",
						func->node->funcdef.name->text
					);

					erw_error(
						msg.data, 
						lines[blocknode->block.stmts[i]->token->linenum - 1]
							.data, 
						blocknode->block.stmts[i]->token->linenum, 
						blocknode->block.stmts[i]->token->column,
						blocknode->block.stmts[i]->token->column + 
							vec_getsize(blocknode->block.stmts[i]->token->text)
							- 2
					);
					str_dtor(&msg);
				}
			}
		}
		/*
		else if(blocknode->block.stmts[i]->type == erw_TOKENTYPE_FOREIGN)
		{ 
			struct erw_ASTNode* foreignnode = blocknode->branches[i];
			struct erw_ASTNode* argsnode = foreignnode->branches[0];

			for(size_t j = 0; j < vec_getsize(argsnode->branches); j++)
			{ 
				erw_getexprtype(scope, argsnode->branches[j], lines);
			}
		}
		*/
		else if(blocknode->block.stmts[i]->type == erw_ASTNODETYPE_DEFER)
		{
			vec_pushback(
				scope->finalizers,
				(struct erw_Finalizer){
					.node = blocknode->block.stmts[i]->defer.block,
					.index = vec_getsize(scope->children)
				}
			);

			struct erw_Scope* newscope = erw_scope_new(
				scope, 
				scope->funcname,
				vec_getsize(scope->children),
				0
			);

			erw_checkblock(
				newscope, 
				blocknode->block.stmts[i]->defer.block, 
				lines
			);
		}
		else if(blocknode->block.stmts[i]->type == erw_ASTNODETYPE_WHILE)
		{
			struct erw_Type* exprtype = erw_getexprtype(
				scope, 
				blocknode->block.stmts[i]->while_.expr, 
				lines
			);

			struct erw_ASTNode* firstnode = blocknode->block.stmts[i]->while_
				.expr;
			while(firstnode->type == erw_ASTNODETYPE_UNEXPR 
				|| firstnode->type == erw_ASTNODETYPE_BINEXPR
				|| firstnode->type == erw_ASTNODETYPE_CAST
				|| firstnode->type == erw_ASTNODETYPE_FUNCCALL)
			{
				if(firstnode->type == erw_ASTNODETYPE_UNEXPR)
				{
					firstnode = firstnode->unexpr.expr;
				}
				else if(firstnode->type == erw_ASTNODETYPE_BINEXPR)
				{
					firstnode = firstnode->binexpr.expr1;
				}
				else if(firstnode->type == erw_ASTNODETYPE_CAST
					|| firstnode->type == erw_ASTNODETYPE_FUNCCALL)
				{
					break;
				}
			}

			struct erw_ASTNode* lastnode = blocknode->block.stmts[i]->while_
				.expr;
			while(lastnode->type == erw_ASTNODETYPE_UNEXPR 
				|| lastnode->type == erw_ASTNODETYPE_BINEXPR)
			{
				if(lastnode->type == erw_ASTNODETYPE_UNEXPR)
				{
					lastnode = lastnode->unexpr.expr;
				}
				else if(lastnode->type == erw_ASTNODETYPE_BINEXPR)
				{
					lastnode = lastnode->binexpr.expr2;
				}
				else if(lastnode->type == erw_ASTNODETYPE_CAST)
				{
					lastnode = lastnode->cast.expr;
				}
				else if(lastnode->type == erw_ASTNODETYPE_FUNCCALL)
				{
					lastnode = lastnode->funccall.args[
						vec_getsize(lastnode->funccall.args) - 1
					];
				}
			}

			erw_checkboolean(exprtype, firstnode, lastnode, lines);
			struct erw_Scope* newscope = erw_scope_new(
				scope, 
				scope->funcname,
				vec_getsize(scope->children),
				0
			);

			erw_checkblock(
				newscope, 
				blocknode->block.stmts[i]->while_.block, 
				lines
			);
		}
		else if(blocknode->block.stmts[i]->type == erw_ASTNODETYPE_ASSIGNMENT)
		{
			struct erw_VarDeclr* var = erw_scope_getvar(
				scope, 
				blocknode->block.stmts[i]->assignment.assignee->token, 
				lines
			);

			/* TODO: Implement this
			if(var->node->vardeclr.mutable || !var->hasvalue)
			{
				if(blocknode->branches[i]->token.type != 
					erw_TOKENTYPE_OPERATOR_ASSIGN)
				{
					if(!var->hasvalue)
					{
						struct Str msg;
						str_ctorfmt(
							&msg, 
							"Operation '%s' is not allowed on an"
								" uninitialized variable",
							assignnode->token.text
						);

						erw_error(
							msg.data, 
							lines[identnode-> token.linenum - 1].data,
							identnode->token.linenum, 
							identnode->token.column,
							identnode->token.column +
								vec_getsize( identnode->token.text) - 2
						);
						str_dtor(&msg);
					}
				}
				*/

				erw_checkexprtype(
					scope, 
					blocknode->block.stmts[i]->assignment.expr, 
					var->type, 
					lines
				);
				//var->hasvalue = 1;
			/*
			}
			else
			{
				struct Str msg;
				str_ctor(&msg, "Reassignment of immutable variable");
				erw_error(
					msg.data, 
					lines[identnode-> token.linenum - 1].data,
					identnode->token.linenum, 
					identnode->token.column,
					identnode->token.column +
						vec_getsize( identnode->token.text) - 2
				);
				str_dtor(&msg);
			}
			*/
		}
		else if(blocknode->block.stmts[i]->type == erw_ASTNODETYPE_FUNCCALL)
		{
			erw_checkfunccall(scope, blocknode->block.stmts[i], lines);
		}
		else
		{
			log_info(
				"this shouldn't happen (%s)", 
				blocknode->block.stmts[i]->type->name
			);
		}
	}
}

static void erw_checkfunc(
	struct erw_Scope* scope, 
	struct erw_ASTNode* funcnode, 
	struct Str* lines)
{
	log_assert(scope, "is NULL");
	log_assert(funcnode, "is NULL");
	log_assert(
		funcnode->type == erw_ASTNODETYPE_FUNCDEF, 
		"invalid size (%s)", 
		funcnode->type->name
	);
	log_assert(lines, "is NULL");

	erw_scope_addfuncdeclr(scope, funcnode, lines);
	struct erw_Scope* newscope = erw_scope_new(
		scope, 
		funcnode->funcdef.name->text,
		vec_getsize(scope->children),
		1
	);

	for(size_t j = 0; j < vec_getsize(funcnode->funcdef.params); j++)
	{ 
		erw_scope_addvardeclr(newscope, funcnode->funcdef.params[j], lines);
	}

	erw_checkblock(newscope, funcnode->funcdef.block, lines);
}

static int erw_checkifreturn(struct erw_ASTNode* ifnode)
{
	log_assert(ifnode, "is NULL");
	log_assert(
		ifnode->type == erw_ASTNODETYPE_IF, 
		"invalid size (%s)", 
		ifnode->type->name
	);

	if(!ifnode->if_.else_)
	{
		return 0;
	}

	if(!vec_getsize(ifnode->if_.block->block.stmts))
	{
		return 0;
	}

	struct erw_ASTNode* laststatement = ifnode->if_.block->block.stmts[
		vec_getsize(ifnode->if_.block->block.stmts) - 1
	];

	if(laststatement->type == erw_ASTNODETYPE_RETURN) {}
	else if(laststatement->type == erw_ASTNODETYPE_IF)
	{
		if(!erw_checkifreturn(laststatement))
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}

	for(size_t i = 0; i < vec_getsize(ifnode->if_.elseifs); i++)
	{
		if(!vec_getsize(ifnode->if_.elseifs[i]->elseif.block))
		{
			return 0;
		}

		laststatement = ifnode->if_.elseifs[i]->elseif.block->block.stmts[
			vec_getsize(ifnode->if_.elseifs[i]->elseif.block->block.stmts) - 1
		];

		if(laststatement->type == erw_ASTNODETYPE_RETURN) {}
		else if(laststatement->type == erw_ASTNODETYPE_IF)
		{
			if(!erw_checkifreturn(laststatement))
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}

	if(ifnode->if_.else_)
	{
		if(!vec_getsize(ifnode->if_.else_->else_.block))
		{
			return 0;
		}

		laststatement = ifnode->if_.else_->else_.block->block.stmts[
			vec_getsize(ifnode->if_.else_->else_.block->block.stmts) - 1
		];

		if(laststatement->type == erw_ASTNODETYPE_RETURN) {}
		else if(laststatement->type == erw_ASTNODETYPE_IF)
		{
			if(!erw_checkifreturn(laststatement))
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
		return 0;
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
			struct erw_ASTNode* blocknode = scope->functions[i].node->funcdef
				.block;
			int hasreturn = 0;
			if(vec_getsize(blocknode->block.stmts))
			{
				struct erw_ASTNode* laststatement = blocknode->block.stmts[
					vec_getsize(blocknode->block.stmts) - 1
				];
			
				if(laststatement->type == erw_ASTNODETYPE_RETURN)
				{
					hasreturn = 1;
				}
				else if(laststatement->type == erw_ASTNODETYPE_IF)
				{
					hasreturn = erw_checkifreturn(laststatement);
				}
			}

			if(!hasreturn)
			{
				struct Str msg;
				str_ctor(&msg, "Function expects return at the end");
				erw_error(
					msg.data, 
					lines[scope->functions[i].node->funcdef.name->linenum - 1]
						.data,
					scope->functions[i].node->funcdef.name->linenum, 
					scope->functions[i].node->funcdef.name->column,
					scope->functions[i].node->funcdef.name->column +
						vec_getsize(
							scope->functions[i].node->funcdef.name->text) - 2
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

static void erw_checkunused(struct erw_Scope* scope, struct Str* lines)
{
	log_assert(scope, "is NULL");
	log_assert(lines, "is NULL");

	for(size_t i = 0; i < vec_getsize(scope->variables); i++)
	{
		struct erw_VarDeclr* var = &scope->variables[i];
		if(!var->used)
		{
			struct Str msg;
			str_ctor(&msg, "Unused variable");
			erw_warning(
				msg.data, 
				lines[var->node->vardeclr.name->linenum - 1].data,
				var->node->vardeclr.name->linenum, 
				var->node->vardeclr.name->column,
				var->node->vardeclr.name->column + 
					vec_getsize(var->node->vardeclr.name->text) - 2
			);
			str_dtor(&msg);
		}
	}

	for(size_t i = 0; i < vec_getsize(scope->functions); i++)
	{
		struct erw_FuncDeclr* func = &scope->functions[i];
		if(!func->used)
		{
			struct Str msg;
			str_ctor(&msg, "Unused function");
			erw_warning(
				msg.data, 
				lines[func->node->funcdef.name->linenum - 1].data,
				func->node->funcdef.name->linenum, 
				func->node->funcdef.name->column,
				func->node->funcdef.name->column +
					vec_getsize(func->node->funcdef.name->text) - 2
			);
			str_dtor(&msg);
		}
	}

	for(size_t i = 0; i < vec_getsize(scope->types); i++)
	{
		struct erw_TypeDeclr* type = &scope->types[i];
		if(!type->type->named.used)
		{
			struct Str msg;
			str_ctor(&msg, "Unused type");
			erw_warning(
				msg.data,
				lines[type->node->typedeclr.name->linenum - 1].data,
				type->node->typedeclr.name->linenum,
				type->node->typedeclr.name->column,
				type->node->typedeclr.name->column +
					vec_getsize(type->node->typedeclr.name->text) - 2
			);
			str_dtor(&msg);
		}
	}

	for(size_t i = 0; i < vec_getsize(scope->children); i++)
	{
		erw_checkunused(scope->children[i], lines);
	}
}


struct erw_Scope* erw_checksemantics(struct erw_ASTNode* ast, struct Str* lines)
{
	log_assert(ast, "is NULL");
	log_assert(
		ast->type == 
		erw_ASTNODETYPE_START, 
		"invalid type (%s)", 
		ast->type->name
	);
	log_assert(lines, "is NULL");

	//NOTE: Global scope is temporarily named NULL
	struct erw_Scope* globalscope = erw_scope_new(NULL, NULL, 0, 1); 

	//Add all builtin types
	for(size_t i = 0; i < erw_TYPEBUILTIN_COUNT; i++)
	{  
		//Should this use erw_scope_addtypedeclr?
		vec_pushback(
			globalscope->types, 
			(struct erw_TypeDeclr){
				.type = erw_type_builtins[i],
				.node = NULL
			}
		);
	}

	//TODO: semantic check for main function
	for(size_t i = 0; i < vec_getsize(ast->start.children); i++)
	{
		if(ast->start.children[i]->type == erw_ASTNODETYPE_FUNCDEF)
		{
			erw_checkfunc(globalscope, ast->start.children[i], lines);
		}
		else if(ast->start.children[i]->type == erw_ASTNODETYPE_TYPEDECLR)
		{
			erw_scope_addtypedeclr(globalscope, ast->start.children[i], lines);
		}
	}

	erw_checkreturn(globalscope, lines);
	erw_checkunused(globalscope, lines);
	return globalscope;
}

