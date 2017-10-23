#include "scope.h"
#include "ast.h"
#include "log.h"

//NOTE: scope_add* depends on scope_get*

struct Scope* scope_ctor(struct Scope* self, struct Scope* parent)
{
	log_assert(self, "is NULL");

	self->functions = vec_ctor(struct ASTNode*, 0);
	self->variables = vec_ctor(struct ASTNode* , 0);
	self->types = vec_ctor(struct ASTNode* , 0);
	self->parent = parent;

	return self;
}

struct ASTNode* scope_getvariable(struct Scope* self, const char* name)
{
	log_assert(self, "is NULL");
	log_assert(name, "is NULL");

	do {
		for(size_t i = 0; i < vec_getsize(self->variables); i++)
		{
			if(!strcmp(self->variables[i]->branches[0]->token.text, name))
			{
				return self->variables[i];
			}
		}

		self = self->parent;
	} while(self);

	return NULL;
}

struct ASTNode* scope_getfunction(struct Scope* self, const char* name)
{
	log_assert(self, "is NULL");
	log_assert(name, "is NULL");

	do {
		for(size_t i = 0; i < vec_getsize(self->functions); i++)
		{
			if(!strcmp(self->functions[i]->branches[0]->token.text, name))
			{
				return self->functions[i];
			}
		}

		self = self->parent;
	} while(self);

	return NULL;
}

struct ASTNode* scope_gettype(struct Scope* self, const char* name)
{
	log_assert(self, "is NULL");
	log_assert(name, "is NULL");

	do {
		for(size_t i = 0; i < vec_getsize(self->types); i++)
		{
			if(!strcmp(self->types[i]->branches[0]->token.text, name))
			{
				return self->types[i];
			}
		}

		self = self->parent;
	} while(self);

	return NULL;
}

void scope_addvariable(struct Scope* self, struct ASTNode* var)
{
	log_assert(self, "is NULL");
	log_assert(var, "is NULL");
	
	struct ASTNode* functest = scope_getfunction(
		self,
		var->branches[0]->token.text
	);

	if(functest)
	{
		log_error(
			"Semantic error: Variable '%s' (at line %zu, column %zu)"
				" has already been declared as a function at line "
				"%zu, column %zu",
			var->branches[0]->token.text,
			var->branches[0]->token.line,
			var->branches[0]->token.column,
			functest->branches[0]->token.line,
			functest->branches[0]->token.column
		);
	}

	struct ASTNode* vartest = scope_getvariable(
		self,
		var->branches[0]->token.text
	);

	if(vartest)
	{
		log_error(
			"Semantic error: Variable '%s' (at line %zu, column %zu)"
				" has already been declared as a variable at line "
				"%zu, column %zu",
			var->branches[0]->token.text,
			var->branches[0]->token.line,
			var->branches[0]->token.column,
			vartest->branches[0]->token.line,
			vartest->branches[0]->token.column
		);
	}

	vec_pushback(self->variables, var);
}

void scope_addfunction(struct Scope* self, struct ASTNode* func)
{
	log_assert(self, "is NULL");
	log_assert(func, "is NULL");

	struct ASTNode* functest = scope_getfunction(
		self,
		func->branches[0]->token.text
	);

	if(functest)
	{
		log_error(
			"Semantic error: Function '%s' (at line %zu, column %zu)"
				" has already been declared as a function at line "
				"%zu, column %zu",
			func->branches[0]->token.text,
			func->branches[0]->token.line,
			func->branches[0]->token.column,
			functest->branches[0]->token.line,
			functest->branches[0]->token.column
		);
	}

	struct ASTNode* vartest = scope_getvariable(
		self,
		func->branches[0]->token.text
	);

	if(vartest)
	{
		log_error(
			"Semantic error: Function '%s' (at line %zu, column %zu)"
				" has already been declared as a variable at line "
				"%zu, column %zu",
			func->branches[0]->token.text,
			func->branches[0]->token.line,
			func->branches[0]->token.column,
			vartest->branches[0]->token.line,
			vartest->branches[0]->token.column
		);
	}

	vec_pushback(self->functions, func);
}

void scope_addtype(struct Scope* self, struct ASTNode* type)
{
	log_assert(self, "is NULL");
	log_assert(type, "is NULL");

	struct ASTNode* typetest = scope_gettype(
		self,
		type->branches[0]->token.text
	);

	if(typetest)
	{
		log_error(
			"Semantic error: Type '%s' (at line %zu, column %zu)"
				" has already been declared at line %zu, column %zu",
			type->branches[0]->token.text,
			type->branches[0]->token.line,
			type->branches[0]->token.column,
			typetest->branches[0]->token.line,
			typetest->branches[0]->token.column
		);
	}

	vec_pushback(self->types, type);
}

void scope_dtor(struct Scope* self) 
{
	log_assert(self, "is NULL");

	vec_dtor(self->functions);
	vec_dtor(self->variables);
	vec_dtor(self->types);
}

