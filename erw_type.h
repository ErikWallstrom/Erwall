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

#ifndef ERW_TYPE_H
#define ERW_TYPE_H

#include "erw_ast.h"
#include "str.h"


enum erw_TypeBuiltin
{
	erw_TYPEBUILTIN_CHAR,
	erw_TYPEBUILTIN_BOOL,
	erw_TYPEBUILTIN_INT8,
	erw_TYPEBUILTIN_INT16,
	erw_TYPEBUILTIN_INT32,
	erw_TYPEBUILTIN_INT64,
	erw_TYPEBUILTIN_UINT8,
	erw_TYPEBUILTIN_UINT16,
	erw_TYPEBUILTIN_UINT32,
	erw_TYPEBUILTIN_UINT64,
	erw_TYPEBUILTIN_FLOAT32,
	erw_TYPEBUILTIN_FLOAT64,
	erw_TYPEBUILTIN_COUNT,
};

extern struct erw_Type* const erw_type_builtins[];

enum erw_TypeType
{
	erw_TYPETYPE_REFERENCE,
	erw_TYPETYPE_TYPEDEF,
	erw_TYPETYPE_STRUCT,
	erw_TYPETYPE_ARRAY,
	erw_TYPETYPE_UNION,
	erw_TYPETYPE_ENUM,
	erw_TYPETYPE_FUNC,
	erw_TYPETYPE_FLOAT,
	erw_TYPETYPE_CHAR,
	erw_TYPETYPE_BOOL,
	erw_TYPETYPE_INT,
};

struct erw_Type
{
	union
	{
		struct
		{
			struct erw_Type* type;
			int mutable;
		} reference_;

		struct
		{
			struct erw_Type* type;
			const char* name;
			struct erw_Token* token;
			int used;
		} typedef_;

		struct
		{
			Vec(struct erw_Type*) members;
		} struct_;

		struct
		{
			struct erw_Type* type;
			int mutable;
		} array_;

		struct
		{
			Vec(struct erw_Type*) members;
		} union_;

		struct
		{
			struct erw_Type* type;
			Vec(struct erw_Type*) params;
		} func_;

		struct 
		{
			size_t size;
		} float_;

		struct {} char_;

		struct {} bool_;

		struct 
		{
			size_t size;
			int signed_;
		} int_;
	};

	enum erw_TypeType type;
};

struct erw_Type* erw_type_newfromastnode(struct erw_ASTNode* node); 
struct Str erw_type_tostring(struct erw_Type* self);
int erw_type_compare(struct erw_Type* t1, struct erw_Type* t2);
struct erw_Type* erw_type_getbase(struct erw_Type* type);

#endif
