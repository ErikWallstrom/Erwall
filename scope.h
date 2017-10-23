#ifndef SCOPE_H
#define SCOPE_H

#include "vec.h"

struct Scope
{
	Vec(struct ASTNode*) variables;
	Vec(struct ASTNode*) functions;
	Vec(struct ASTNode*) types;
	struct Scope* parent;
};

struct Scope* scope_ctor(struct Scope* self, struct Scope* parent);
struct ASTNode* scope_getvariable(struct Scope* self, const char* name);
struct ASTNode* scope_getfunction(struct Scope* self, const char* name);
struct ASTNode* scope_gettype(struct Scope* self, const char* name);
void scope_addvariable(struct Scope* self, struct ASTNode* var);
void scope_addfunction(struct Scope* self, struct ASTNode* func);
void scope_addtype(struct Scope* self, struct ASTNode* type);
void scope_dtor(struct Scope* self);

#endif
