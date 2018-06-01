#include "erw_ast.h"
#include "log.h"
#include <stdlib.h>

const struct erw_ASTNodeType* const erw_ASTNODETYPE_START =
	&(struct erw_ASTNodeType){"Start"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_FUNCPROT =
	&(struct erw_ASTNodeType){"Function Prototype"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_FUNCDEF =
	&(struct erw_ASTNodeType){"Function Definition"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_TYPEDECLR =
	&(struct erw_ASTNodeType){"Type Declaration"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_VARDECLR =
	&(struct erw_ASTNodeType){"Variable Declaration"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_BLOCK =
	&(struct erw_ASTNodeType){"Block"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_IF =
	&(struct erw_ASTNodeType){"If Statement"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_ELSEIF =
	&(struct erw_ASTNodeType){"Elseif Statement"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_ELSE =
	&(struct erw_ASTNodeType){"Else Statement"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_RETURN =
	&(struct erw_ASTNodeType){"Return Statement"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_ASSIGNMENT =
	&(struct erw_ASTNodeType){"Assignment"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_UNEXPR =
	&(struct erw_ASTNodeType){"Unary Expression"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_BINEXPR =
	&(struct erw_ASTNodeType){"Binary Expression"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_FUNCCALL =
	&(struct erw_ASTNodeType){"Function Call"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_CAST =
	&(struct erw_ASTNodeType){"Type Cast"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_DEFER =
	&(struct erw_ASTNodeType){"Defer Statement"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_WHILE =
	&(struct erw_ASTNodeType){"While Statement"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_ENUM =
	&(struct erw_ASTNodeType){"Enum Type"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_ENUMMEMBER =
	&(struct erw_ASTNodeType){"Enum Member"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_STRUCT =
	&(struct erw_ASTNodeType){"Struct Type"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_UNION =
	&(struct erw_ASTNodeType){"Union Type"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_UNSAFE =
	&(struct erw_ASTNodeType){"Unsafe Statement"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_REFERENCE =
	&(struct erw_ASTNodeType){"Reference Type"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_ARRAY =
	&(struct erw_ASTNodeType){"Array Type"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_SLICE =
	&(struct erw_ASTNodeType){"Slice Type"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_LITERAL =
	&(struct erw_ASTNodeType){"Literal"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_TYPE = 
	&(struct erw_ASTNodeType){"Named Type"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_FUNCTYPE =
	&(struct erw_ASTNodeType){"Function Type"};
const struct erw_ASTNodeType* const erw_ASTNODETYPE_ACCESS = 
	&(struct erw_ASTNodeType){"Array Access"};

struct erw_ASTNode* erw_ast_new(
	const struct erw_ASTNodeType* type, 
	struct erw_Token* token)
{
	log_assert(type, "is NULL");
	struct erw_ASTNode* self = calloc(1, sizeof(struct erw_ASTNode));
	if(!self)
	{
		log_error("calloc failed, in <%s>", __func__);
	}

	self->token = token;
	self->type = type;
	if(self->type == erw_ASTNODETYPE_START)
	{
		self->start.children = vec_ctor(struct erw_ASTNode*, 0);
	}
	else if(self->type == erw_ASTNODETYPE_FUNCPROT)
	{
		self->funcprot.params = vec_ctor(struct erw_ASTNode*, 0);
	}
	else if(self->type == erw_ASTNODETYPE_FUNCDEF)
	{
		self->funcdef.params = vec_ctor(struct erw_ASTNode*, 0);
	}
	else if(self->type == erw_ASTNODETYPE_BLOCK)
	{
		self->block.stmts = vec_ctor(struct erw_ASTNode*, 0);
	}
	else if(self->type == erw_ASTNODETYPE_IF)
	{
		self->if_.elseifs = vec_ctor(struct erw_ASTNode*, 0);
	}
	else if(self->type == erw_ASTNODETYPE_FUNCCALL)
	{
		self->funccall.args = vec_ctor(struct erw_ASTNode*, 0);
	}
	else if(self->type == erw_ASTNODETYPE_ENUM)
	{
		self->enum_.members = vec_ctor(struct erw_ASTNode*, 0);
	}
	else if(self->type == erw_ASTNODETYPE_STRUCT)
	{
		self->struct_.members = vec_ctor(struct erw_ASTNode*, 0);
	}
	else if(self->type == erw_ASTNODETYPE_UNION)
	{
		self->union_.members = vec_ctor(struct erw_ASTNode*, 0);
	}
	else if(self->type == erw_ASTNODETYPE_FUNCTYPE)
	{
		self->functype.params = vec_ctor(struct erw_ASTNode*, 0);
	}

	return self;
}

static void erw_ast_printinternaltoken(struct erw_Token* token, size_t level)
{
	for(size_t i = 0; i < level; i++)
	{
		printf("    ");
		printf("│");
	}
	
	printf("─ %s (%s)\n", token->type->name, token->text);
}

static void erw_ast_printinternal(struct erw_ASTNode* ast, size_t level)
{
	if(ast != NULL)
	{
		for(size_t i = 0; i < level; i++)
		{
			printf("    ");
			printf("│");
		}
		
		if(ast->token)
		{
			printf("─ %s (%s)\n", ast->type->name, ast->token->text);
		}
		else
		{
			printf("─ %s\n", ast->type->name);
		}

		if(ast->type == erw_ASTNODETYPE_START)
		{
			for(size_t i = 0; i < vec_getsize(ast->start.children); i++)
			{
				erw_ast_printinternal(ast->start.children[i], level + 1);
			}
		}
		else if(ast->type == erw_ASTNODETYPE_FUNCPROT)
		{
			for(size_t i = 0; i < vec_getsize(ast->funcprot.params); i++)
			{
				erw_ast_printinternal(ast->funcprot.params[i], level + 1);
			}

			erw_ast_printinternaltoken(ast->funcprot.name, level + 1);
			erw_ast_printinternal(ast->funcprot.type, level + 1);
		}
		else if(ast->type == erw_ASTNODETYPE_FUNCDEF)
		{
			for(size_t i = 0; i < vec_getsize(ast->funcdef.params); i++)
			{
				erw_ast_printinternal(ast->funcdef.params[i], level + 1);
			}

			erw_ast_printinternaltoken(ast->funcdef.name, level + 1);
			erw_ast_printinternal(ast->funcdef.type, level + 1);
			erw_ast_printinternal(ast->funcdef.block, level + 1);
		}
		else if(ast->type == erw_ASTNODETYPE_TYPEDECLR)
		{
			erw_ast_printinternaltoken(ast->typedeclr.name, level + 1);
			erw_ast_printinternal(ast->typedeclr.type, level + 1);
		}
		else if(ast->type == erw_ASTNODETYPE_VARDECLR)
		{
			erw_ast_printinternaltoken(ast->vardeclr.name, level + 1);
			erw_ast_printinternal(ast->vardeclr.type, level + 1);
			erw_ast_printinternal(ast->vardeclr.value, level + 1);
		}
		else if(ast->type == erw_ASTNODETYPE_BLOCK)
		{
			for(size_t i = 0; i < vec_getsize(ast->block.stmts); i++)
			{
				erw_ast_printinternal(ast->block.stmts[i], level + 1);
			}
		}
		else if(ast->type == erw_ASTNODETYPE_IF)
		{
			for(size_t i = 0; i < vec_getsize(ast->if_.elseifs); i++)
			{
				erw_ast_printinternal(ast->if_.elseifs[i], level + 1);
			}

			erw_ast_printinternal(ast->if_.expr, level + 1);
			erw_ast_printinternal(ast->if_.block, level + 1);
			erw_ast_printinternal(ast->if_.else_, level + 1);
		}
		else if(ast->type == erw_ASTNODETYPE_ELSEIF)
		{
			erw_ast_printinternal(ast->elseif.expr, level + 1);
			erw_ast_printinternal(ast->elseif.block, level + 1);
		}
		else if(ast->type == erw_ASTNODETYPE_ELSE)
		{
			erw_ast_printinternal(ast->else_.block, level + 1);
		}
		else if(ast->type == erw_ASTNODETYPE_RETURN)
		{
			erw_ast_printinternal(ast->return_.expr, level + 1);
		}
		else if(ast->type == erw_ASTNODETYPE_ASSIGNMENT)
		{
			erw_ast_printinternal(ast->assignment.assignee, level + 1);
			erw_ast_printinternal(ast->assignment.expr, level + 1);
		}
		else if(ast->type == erw_ASTNODETYPE_UNEXPR)
		{
			erw_ast_printinternal(ast->unexpr.expr, level + 1);
		}
		else if(ast->type == erw_ASTNODETYPE_BINEXPR)
		{
			erw_ast_printinternal(ast->binexpr.expr1, level + 1);
			erw_ast_printinternal(ast->binexpr.expr2, level + 1);
		}
		else if(ast->type == erw_ASTNODETYPE_FUNCCALL)
		{
			erw_ast_printinternaltoken(ast->funccall.name, level + 1);
			for(size_t i = 0; i < vec_getsize(ast->funccall.args); i++)
			{
				erw_ast_printinternal(ast->funccall.args[i], level + 1);
			}
		}
		else if(ast->type == erw_ASTNODETYPE_DEFER)
		{
			erw_ast_printinternal(ast->defer.block, level + 1);
		}
		else if(ast->type == erw_ASTNODETYPE_UNSAFE)
		{
			erw_ast_printinternal(ast->unsafe.block, level + 1);
		}
		else if(ast->type == erw_ASTNODETYPE_CAST)
		{
			erw_ast_printinternal(ast->cast.type, level + 1);
			erw_ast_printinternal(ast->cast.expr, level + 1);
		}
		else if(ast->type == erw_ASTNODETYPE_WHILE)
		{
			erw_ast_printinternal(ast->while_.expr, level + 1);
			erw_ast_printinternal(ast->while_.block, level + 1);
		}
		else if(ast->type == erw_ASTNODETYPE_ENUM)
		{
			for(size_t i = 0; i < vec_getsize(ast->enum_.members); i++)
			{
				erw_ast_printinternal(ast->enum_.members[i], level + 1);
			}
		}
		else if(ast->type == erw_ASTNODETYPE_ENUMMEMBER)
		{
			erw_ast_printinternaltoken(ast->enummember.name, level + 1);
			erw_ast_printinternal(ast->enummember.value, level + 1);
		}
		else if(ast->type == erw_ASTNODETYPE_STRUCT)
		{
			for(size_t i = 0; i < vec_getsize(ast->struct_.members); i++)
			{
				erw_ast_printinternal(ast->struct_.members[i], level + 1);
			}
		}
		else if(ast->type == erw_ASTNODETYPE_UNION)
		{
			for(size_t i = 0; i < vec_getsize(ast->union_.members); i++)
			{
				erw_ast_printinternal(ast->union_.members[i], level + 1);
			}
		}
		else if(ast->type == erw_ASTNODETYPE_REFERENCE)
		{
			erw_ast_printinternal(ast->reference.type, level + 1);
		}
		else if(ast->type == erw_ASTNODETYPE_ARRAY)
		{
			erw_ast_printinternal(ast->array.type, level + 1);
			erw_ast_printinternal(ast->array.size, level + 1);
		}
		else if(ast->type == erw_ASTNODETYPE_SLICE)
		{
			erw_ast_printinternal(ast->slice.type, level + 1);
		}
		else if(ast->type == erw_ASTNODETYPE_FUNCTYPE)
		{
			for(size_t i = 0; i < vec_getsize(ast->functype.params); i++)
			{
				erw_ast_printinternal(ast->functype.params[i], level + 1);
			}

			erw_ast_printinternal(ast->functype.type, level + 1);
		}
		else if(ast->type == erw_ASTNODETYPE_TYPE) { }
		else if(ast->type == erw_ASTNODETYPE_LITERAL) { }
		else if(ast->type == erw_ASTNODETYPE_ACCESS) 
		{ 
			erw_ast_printinternal(ast->access.expr, level + 1);
			erw_ast_printinternal(ast->access.index, level + 1);
		}
		else
		{
			log_assert(0, "This shouldn't happen (%s)'", ast->type->name);
		}
	}
}

void erw_ast_print(struct erw_ASTNode* ast)
{
	log_assert(ast, "is NULL");
	erw_ast_printinternal(ast, 0);
}

void erw_ast_dtor(struct erw_ASTNode* ast)
{
	if(!ast)
	{
		return;
	}
	else if(ast->type == erw_ASTNODETYPE_START)
	{
		for(size_t i = 0; i < vec_getsize(ast->start.children); i++)
		{
			erw_ast_dtor(ast->start.children[i]);
		}

		vec_dtor(ast->start.children);
	}
	else if(ast->type == erw_ASTNODETYPE_FUNCPROT)
	{
		for(size_t i = 0; i < vec_getsize(ast->funcprot.params); i++)
		{
			erw_ast_dtor(ast->funcprot.params[i]);
		}

		vec_dtor(ast->funcprot.params);
		erw_ast_dtor(ast->funcprot.type);
	}
	else if(ast->type == erw_ASTNODETYPE_FUNCDEF)
	{
		for(size_t i = 0; i < vec_getsize(ast->funcdef.params); i++)
		{
			erw_ast_dtor(ast->funcdef.params[i]);
		}

		vec_dtor(ast->funcdef.params);
		erw_ast_dtor(ast->funcdef.type);
		erw_ast_dtor(ast->funcdef.block);
	}
	else if(ast->type == erw_ASTNODETYPE_TYPEDECLR)
	{
		erw_ast_dtor(ast->typedeclr.type);
	}
	else if(ast->type == erw_ASTNODETYPE_VARDECLR)
	{
		erw_ast_dtor(ast->vardeclr.type);
		erw_ast_dtor(ast->vardeclr.value);
	}
	else if(ast->type == erw_ASTNODETYPE_BLOCK)
	{
		for(size_t i = 0; i < vec_getsize(ast->block.stmts); i++)
		{
			erw_ast_dtor(ast->block.stmts[i]);
		}

		vec_dtor(ast->block.stmts);
	}
	else if(ast->type == erw_ASTNODETYPE_IF)
	{
		for(size_t i = 0; i < vec_getsize(ast->if_.elseifs); i++)
		{
			erw_ast_dtor(ast->if_.elseifs[i]);
		}

		vec_dtor(ast->if_.elseifs);
		erw_ast_dtor(ast->if_.expr);
		erw_ast_dtor(ast->if_.block);
		erw_ast_dtor(ast->if_.else_);
	}
	else if(ast->type == erw_ASTNODETYPE_ELSEIF)
	{
		erw_ast_dtor(ast->elseif.expr);
		erw_ast_dtor(ast->elseif.block);
	}
	else if(ast->type == erw_ASTNODETYPE_ELSE)
	{
		erw_ast_dtor(ast->else_.block);
	}
	else if(ast->type == erw_ASTNODETYPE_RETURN)
	{
		erw_ast_dtor(ast->return_.expr);
	}
	else if(ast->type == erw_ASTNODETYPE_ASSIGNMENT)
	{
		erw_ast_dtor(ast->assignment.assignee);
		erw_ast_dtor(ast->assignment.expr);
	}
	else if(ast->type == erw_ASTNODETYPE_UNEXPR)
	{
		erw_ast_dtor(ast->unexpr.expr);
	}
	else if(ast->type == erw_ASTNODETYPE_BINEXPR)
	{
		erw_ast_dtor(ast->binexpr.expr1);
		erw_ast_dtor(ast->binexpr.expr2);
	}
	else if(ast->type == erw_ASTNODETYPE_FUNCCALL)
	{
		for(size_t i = 0; i < vec_getsize(ast->funccall.args); i++)
		{
			erw_ast_dtor(ast->funccall.args[i]);
		}

		vec_dtor(ast->funccall.args);
	}
	else if(ast->type == erw_ASTNODETYPE_DEFER)
	{
		erw_ast_dtor(ast->defer.block);
	}
	else if(ast->type == erw_ASTNODETYPE_UNSAFE)
	{
		erw_ast_dtor(ast->unsafe.block);
	}
	else if(ast->type == erw_ASTNODETYPE_CAST)
	{
		erw_ast_dtor(ast->cast.type);
		erw_ast_dtor(ast->cast.expr);
	}
	else if(ast->type == erw_ASTNODETYPE_WHILE)
	{
		erw_ast_dtor(ast->while_.expr);
		erw_ast_dtor(ast->while_.block);
	}
	else if(ast->type == erw_ASTNODETYPE_ENUM)
	{
		for(size_t i = 0; i < vec_getsize(ast->enum_.members); i++)
		{
			erw_ast_dtor(ast->enum_.members[i]);
		}

		vec_dtor(ast->enum_.members);
	}
	else if(ast->type == erw_ASTNODETYPE_ENUMMEMBER)
	{
		erw_ast_dtor(ast->enummember.value);
	}
	else if(ast->type == erw_ASTNODETYPE_STRUCT)
	{
		for(size_t i = 0; i < vec_getsize(ast->struct_.members); i++)
		{
			erw_ast_dtor(ast->struct_.members[i]);
		}

		vec_dtor(ast->struct_.members);
	}
	else if(ast->type == erw_ASTNODETYPE_UNION)
	{
		for(size_t i = 0; i < vec_getsize(ast->union_.members); i++)
		{
			erw_ast_dtor(ast->union_.members[i]);
		}

		vec_dtor(ast->union_.members);
	}
	else if(ast->type == erw_ASTNODETYPE_REFERENCE)
	{
		erw_ast_dtor(ast->reference.type);
	}
	else if(ast->type == erw_ASTNODETYPE_ARRAY)
	{
		erw_ast_dtor(ast->array.type);
		erw_ast_dtor(ast->array.size);
	}
	else if(ast->type == erw_ASTNODETYPE_SLICE)
	{
		erw_ast_dtor(ast->slice.type);
	}
	else if(ast->type == erw_ASTNODETYPE_FUNCTYPE)
	{
		for(size_t i = 0; i < vec_getsize(ast->functype.params); i++)
		{
			erw_ast_dtor(ast->functype.params[i]);
		}

		vec_dtor(ast->functype.params);
		erw_ast_dtor(ast->functype.type);
	}
	else if(ast->type == erw_ASTNODETYPE_TYPE) { }
	else if(ast->type == erw_ASTNODETYPE_LITERAL) { }
	else if(ast->type == erw_ASTNODETYPE_ACCESS) 
	{ 
		erw_ast_dtor(ast->access.expr);
		erw_ast_dtor(ast->access.index);
	}
	else
	{
		log_assert(0, "This shouldn't happen (%s)'", ast->type->name);
	}

	free(ast);
}

