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

#include "erw_type.h"
#include "log.h"
#include "str.h"
#include <stdlib.h>

struct erw_Type* const erw_type_builtins[] = {
	&(struct erw_Type){
		.info = erw_TYPEINFO_NAMED,
		.named.name = "Char",
		.named.used = 1,
		.named.type = &(struct erw_Type){
			.info = erw_TYPEINFO_CHAR
		},
	},
	&(struct erw_Type){
		.info = erw_TYPEINFO_NAMED,
		.named.name = "Bool",
		.named.used = 1,
		.named.type = &(struct erw_Type){
			.info = erw_TYPEINFO_BOOL
		},
	},
	&(struct erw_Type){
		.info = erw_TYPEINFO_NAMED,
		.named.name = "Int8",
		.named.used = 1,
		.named.type = &(struct erw_Type){
			.info = erw_TYPEINFO_INT,
			.int_.size = 1,
			.int_.signed_ = 1
		},
	},
	&(struct erw_Type){
		.info = erw_TYPEINFO_NAMED,
		.named.name = "Int16",
		.named.used = 1,
		.named.type = &(struct erw_Type){
			.info = erw_TYPEINFO_INT,
			.int_.size = 2,
			.int_.signed_ = 1
		},
	},
	&(struct erw_Type){
		.info = erw_TYPEINFO_NAMED,
		.named.name = "Int32",
		.named.used = 1,
		.named.type = &(struct erw_Type){
			.info = erw_TYPEINFO_INT,
			.int_.size = 4,
			.int_.signed_ = 1
		},
	},
	&(struct erw_Type){
		.info = erw_TYPEINFO_NAMED,
		.named.name = "Int64",
		.named.used = 1,
		.named.type = &(struct erw_Type){
			.info = erw_TYPEINFO_INT,
			.int_.size = 8,
			.int_.signed_ = 1
		},
	},
	&(struct erw_Type){
		.info = erw_TYPEINFO_NAMED,
		.named.name = "UInt8",
		.named.used = 1,
		.named.type = &(struct erw_Type){
			.info = erw_TYPEINFO_INT,
			.int_.size = 1,
			.int_.signed_ = 0
		},
	},
	&(struct erw_Type){
		.info = erw_TYPEINFO_NAMED,
		.named.name = "UInt16",
		.named.used = 1,
		.named.type = &(struct erw_Type){
			.info = erw_TYPEINFO_INT,
			.int_.size = 2,
			.int_.signed_ = 0
		},
	},
	&(struct erw_Type){
		.info = erw_TYPEINFO_NAMED,
		.named.name = "UInt32",
		.named.used = 1,
		.named.type = &(struct erw_Type){
			.info = erw_TYPEINFO_INT,
			.int_.size = 4,
			.int_.signed_ = 0
		},
	},
	&(struct erw_Type){
		.info = erw_TYPEINFO_NAMED,
		.named.name = "UInt64",
		.named.used = 1,
		.named.type = &(struct erw_Type){
			.info = erw_TYPEINFO_INT,
			.int_.size = 8,
			.int_.signed_ = 0
		},
	},
	&(struct erw_Type){
		.info = erw_TYPEINFO_NAMED,
		.named.name = "Float32",
		.named.used = 1,
		.named.type = &(struct erw_Type){
			.info = erw_TYPEINFO_FLOAT,
			.float_.size = 4
		},
	},
	&(struct erw_Type){
		.info = erw_TYPEINFO_NAMED,
		.named.name = "Float64",
		.named.used = 1,
		.named.type = &(struct erw_Type){
			.info = erw_TYPEINFO_FLOAT,
			.float_.size = 8
		},
	},
};

struct erw_Type* erw_type_new(enum erw_TypeInfo info, struct erw_Type* parent)
{
	log_assert(info < erw_TYPEINFO_COUNT, "invalid info (%i)", info);
	struct erw_Type* self = calloc(1, sizeof(struct erw_Type));
	if(!self)
	{
		log_error("calloc failed, in <%s>", __func__);
	}

	if(info == erw_TYPEINFO_STRUCT)
	{
		self->struct_.members = vec_ctor(struct erw_TypeStructMember, 0);
	}
	else if(info == erw_TYPEINFO_UNION)
	{
		self->union_.members = vec_ctor(struct erw_Type*, 0);
	}
	else if(info == erw_TYPEINFO_ENUM)
	{
		self->enum_.members = vec_ctor(struct erw_TypeEnumMember, 0);
	}
	else if(info == erw_TYPEINFO_FUNC)
	{
		self->func.params = vec_ctor(struct erw_Type*, 0);
	}

	self->parent = parent;
	self->info = info;
	return self;
}

struct Str erw_type_tostring(struct erw_Type* type)
{
	log_assert(type, "is NULL");

	struct Str str;
	str_ctor(&str, "");
	while(type->info != erw_TYPEINFO_NAMED 
		&& type->info != erw_TYPEINFO_STRUCT
		&& type->info != erw_TYPEINFO_UNION
		&& type->info != erw_TYPEINFO_ENUM
		&& type->info != erw_TYPEINFO_CHAR
		&& type->info != erw_TYPEINFO_BOOL
		&& type->info != erw_TYPEINFO_INT
		&& type->info != erw_TYPEINFO_FLOAT
		&& type->info != erw_TYPEINFO_EMPTY)
	{
		if(type->info == erw_TYPEINFO_REFERENCE)
		{
			str_append(&str, "&");
			type = type->reference.type;
		}
		else if(type->info == erw_TYPEINFO_SLICE)
		{
			str_append(&str, "[]");
			type = type->slice.type;
		}
		else if(type->info == erw_TYPEINFO_ARRAY)
		{
			str_appendfmt(&str, "[%zu]", type->array.elements);
			type = type->array.type;
		}
		else if(type->info == erw_TYPEINFO_FUNC)
		{
			str_append(&str, "func(");
			for(size_t i = 0; i < vec_getsize(type->func.params); i++)
			{	
				struct Str tmp = erw_type_tostring(type->func.params[i]);
				str_append(&str, tmp.data);
				str_dtor(&tmp);

				if(i < vec_getsize(type->func.params) - 1)
				{
					str_append(&str, ", ");
				}
			}

			str_append(&str, ")");
			
			if(type->func.type)
			{
				struct Str tmp = erw_type_tostring(type->func.type);
				str_appendfmt(&str, " -> %s", tmp.data);
				str_dtor(&tmp);
			}
			break;
		}
		else
		{
			log_assert(0, "this shouldn't happen (%i)", type->info);
		}
	}

	if(type->info == erw_TYPEINFO_NAMED)
	{
		str_append(&str, type->named.name);
	}
	else if(type->info == erw_TYPEINFO_STRUCT)
	{
		str_append(&str, "struct"); //TODO: Improve this
	}
	else if(type->info == erw_TYPEINFO_UNION)
	{
		str_append(&str, "union"); //TODO: Improve this
	}
	else if(type->info == erw_TYPEINFO_ENUM)
	{
		str_append(&str, "enum"); //TODO: Improve this
	}
	else if(type->info == erw_TYPEINFO_CHAR)
	{
		str_append(&str, "Builtin Char"); //TODO: Improve this
	}
	else if(type->info == erw_TYPEINFO_BOOL)
	{
		str_append(&str, "Builtin Bool"); //TODO: Improve this
	}
	else if(type->info == erw_TYPEINFO_INT)
	{
		str_append(&str, "Builtin Int"); //TODO: Improve this
	}
	else if(type->info == erw_TYPEINFO_FLOAT)
	{
		str_append(&str, "Builtin Float"); //TODO: Improve this
	}
	else if(type->info == erw_TYPEINFO_EMPTY)
	{
		str_append(&str, "Builtin Empty"); //TODO: Improve this
	}
	else if(type->info == erw_TYPEINFO_FUNC) { }
	else
	{
		log_assert(0, "this shouldn't happen (%i)", type->info);
	}

	return str;
}

int erw_type_compare(struct erw_Type* type1, struct erw_Type* type2)
{
	if(type1->info != type2->info)
	{
		return 0;
	}

	if(type1->info == erw_TYPEINFO_NAMED)
	{
		if(!strcmp(type1->named.name, type2->named.name))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else if(type1->info == erw_TYPEINFO_REFERENCE)
	{
		if(type1->reference.mutable != type2->reference.mutable)
		{
			return 0;
		}
	}
	else if(type1->info == erw_TYPEINFO_SLICE)
	{
		if(type1->slice.mutable != type2->slice.mutable)
		{
			return 0;
		}
	}
	else if(type1->info == erw_TYPEINFO_ARRAY)
	{
		if(type1->array.elements != type2->array.elements)
		{
			return 0;
		}

		if(type1->array.mutable != type2->array.mutable)
		{
			return 0;
		}
	}
	else if(type1->info == erw_TYPEINFO_FUNC)
	{
		size_t numparams1 = vec_getsize(type1->func.params);
		size_t numparams2 = vec_getsize(type2->func.params);
		if(numparams1 != numparams2)
		{
			return 0;
		}

		for(size_t i = 0; i < numparams1; i++)
		{
			int result = erw_type_compare(
				type1->func.params[i], 
				type2->func.params[i]
			);

			if(!result)
			{
				return 0;
			}
		}
	}
	else
	{
		log_assert(0, "this shouldn't happen (%i)'", type1->info);
	}

	return erw_type_compare(type1->reference.type, type2->reference.type);
}

void erw_type_dtor(struct erw_Type* self)
{
	log_assert(self, "is NULL");
	for(size_t i = 0; i < erw_TYPEBUILTIN_COUNT; i++)
	{
		if(self == erw_type_builtins[i])
		{
			return;
		}
	}

	if(self->info == erw_TYPEINFO_STRUCT)
	{
		vec_dtor(self->struct_.members);
	}
	else if(self->info == erw_TYPEINFO_UNION)
	{
		vec_dtor(self->union_.members);
	}
	else if(self->info == erw_TYPEINFO_ENUM)
	{
		vec_dtor(self->enum_.members);
	}
	else if(self->info == erw_TYPEINFO_FUNC)
	{
		vec_dtor(self->func.params);
	}
	/*
	else if(self->info == erw_TYPEINFO_NAMED)
	{
		erw_type_dtor(self->named.type);
	}
	else if(self->info == erw_TYPEINFO_ARRAY)
	{
		erw_type_dtor(self->array.type);
	}
	else if(self->info == erw_TYPEINFO_SLICE)
	{
		erw_type_dtor(self->slice.type);
	}
	else if(self->info == erw_TYPEINFO_REFERENCE)
	{
		erw_type_dtor(self->reference.type);
	}

	free(self);
	*/
}

