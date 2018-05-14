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

#include "vec.h"

enum erw_TypeBuiltIn
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

enum erw_TypeInfo
{
	erw_TYPEINFO_REFERENCE,
	erw_TYPEINFO_STRUCT,
	erw_TYPEINFO_NAMED,
	erw_TYPEINFO_ARRAY,
	erw_TYPEINFO_SLICE,
	erw_TYPEINFO_UNION,
	erw_TYPEINFO_ENUM,
	erw_TYPEINFO_FUNC,
	erw_TYPEINFO_FLOAT,
	erw_TYPEINFO_CHAR,
	erw_TYPEINFO_BOOL,
	erw_TYPEINFO_INT,
	erw_TYPEINFO_EMPTY,
	erw_TYPEINFO_COUNT,
};

struct erw_TypeStructMember
{
	struct erw_Type* type;
	const char* name;
};

struct erw_Type
{
	union
	{
		size_t size;
		struct
		{
			size_t size;
			struct erw_Type* type;
			int mutable;
		} reference;

		struct
		{
			size_t size;
			struct erw_Type* type;
			const char* name;
			int used;
		} named;

		struct
		{
			size_t size;
			Vec(struct erw_TypeStructMember) members;
		} struct_;

		struct
		{
			size_t size;
			struct erw_Type* type;
			size_t elements;
			int mutable;
		} array;

		struct
		{
			size_t size;
			struct erw_Type* type;
			int mutable;
		} slice;

		struct
		{
			size_t size;
			Vec(struct erw_Type*) members;
		} union_;

		struct
		{
			size_t size;
			Vec(const char*) members;
		} enum_;

		struct
		{
			size_t size;
			struct erw_Type* type;
			Vec(struct erw_Type*) params; //Should this have names?
		} func;

		struct
		{
			size_t size;
		} float_;

		struct
		{
			size_t size;
			int signed_;
		} int_;
	};

	struct erw_Type* parent;
	enum erw_TypeInfo info;
};

struct erw_Type* erw_type_new(enum erw_TypeInfo info, struct erw_Type* parent);
struct Str erw_type_tostring(struct erw_Type* type);
int erw_type_compare(struct erw_Type* type1, struct erw_Type* type2);
void erw_type_dtor(struct erw_Type* self);

#endif
