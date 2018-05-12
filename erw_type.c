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

struct erw_Type* erw_type_new(enum erw_TypeInfo info)
{
	log_assert(info < erw_TYPEINFO_COUNT, "invalid info (%i)", info);
	struct erw_Type* self = calloc(1, sizeof(struct erw_Type));
	if(!self)
	{
		log_error("calloc failed, in <%s>", __func__);
	}

	self->info = info;
	return self;
}

void erw_type_dtor(struct erw_Type* self)
{
	log_assert(self, "is NULL");
	free(self);
}

