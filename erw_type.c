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
		.type = erw_TYPETYPE_TYPEDEF, 
		.typedef_.name = "Char", 
		.typedef_.token = NULL,
		.typedef_.used = 1,
		.typedef_.type = &(struct erw_Type){
			.type = erw_TYPETYPE_CHAR
		},
	},
	&(struct erw_Type){
		.type = erw_TYPETYPE_TYPEDEF, 
		.typedef_.name = "Bool", 
		.typedef_.token = NULL,
		.typedef_.used = 1,
		.typedef_.type = &(struct erw_Type){
			.type = erw_TYPETYPE_BOOL
		},
	},
	&(struct erw_Type){
		.type = erw_TYPETYPE_TYPEDEF, 
		.typedef_.name = "Int8", 
		.typedef_.token = NULL,
		.typedef_.used = 1,
		.typedef_.type = &(struct erw_Type){
			.type = erw_TYPETYPE_INT,
			.int_.size = 1,
			.int_.signed_ = 1
		},
	},
	&(struct erw_Type){
		.type = erw_TYPETYPE_TYPEDEF, 
		.typedef_.name = "Int16", 
		.typedef_.token = NULL,
		.typedef_.used = 1,
		.typedef_.type = &(struct erw_Type){
			.type = erw_TYPETYPE_INT,
			.int_.size = 2,
			.int_.signed_ = 1
		},
	},
	&(struct erw_Type){
		.type = erw_TYPETYPE_TYPEDEF, 
		.typedef_.name = "Int32", 
		.typedef_.token = NULL,
		.typedef_.used = 1,
		.typedef_.type = &(struct erw_Type){
			.type = erw_TYPETYPE_INT,
			.int_.size = 4,
			.int_.signed_ = 1
		},
	},
	&(struct erw_Type){
		.type = erw_TYPETYPE_TYPEDEF, 
		.typedef_.name = "Int64", 
		.typedef_.token = NULL,
		.typedef_.used = 1,
		.typedef_.type = &(struct erw_Type){
			.type = erw_TYPETYPE_INT,
			.int_.size = 8,
			.int_.signed_ = 1
		},
	},
	&(struct erw_Type){
		.type = erw_TYPETYPE_TYPEDEF, 
		.typedef_.name = "UInt8", 
		.typedef_.token = NULL,
		.typedef_.used = 1,
		.typedef_.type = &(struct erw_Type){
			.type = erw_TYPETYPE_INT,
			.int_.size = 1,
			.int_.signed_ = 0
		},
	},
	&(struct erw_Type){
		.type = erw_TYPETYPE_TYPEDEF, 
		.typedef_.name = "UInt16", 
		.typedef_.token = NULL,
		.typedef_.used = 1,
		.typedef_.type = &(struct erw_Type){
			.type = erw_TYPETYPE_INT,
			.int_.size = 2,
			.int_.signed_ = 0
		},
	},
	&(struct erw_Type){
		.type = erw_TYPETYPE_TYPEDEF, 
		.typedef_.name = "UInt32", 
		.typedef_.token = NULL,
		.typedef_.used = 1,
		.typedef_.type = &(struct erw_Type){
			.type = erw_TYPETYPE_INT,
			.int_.size = 4,
			.int_.signed_ = 0
		},
	},
	&(struct erw_Type){
		.type = erw_TYPETYPE_TYPEDEF, 
		.typedef_.name = "UInt64", 
		.typedef_.token = NULL,
		.typedef_.used = 1,
		.typedef_.type = &(struct erw_Type){
			.type = erw_TYPETYPE_INT,
			.int_.size = 8,
			.int_.signed_ = 0
		},
	},
	&(struct erw_Type){
		.type = erw_TYPETYPE_TYPEDEF, 
		.typedef_.name = "Float32", 
		.typedef_.token = NULL,
		.typedef_.used = 1,
		.typedef_.type = &(struct erw_Type){
			.type = erw_TYPETYPE_FLOAT,
			.float_.size = 4
		},
	},
	&(struct erw_Type){
		.type = erw_TYPETYPE_TYPEDEF, 
		.typedef_.name = "Float64", 
		.typedef_.token = NULL,
		.typedef_.used = 1,
		.typedef_.type = &(struct erw_Type){
			.type = erw_TYPETYPE_FLOAT,
			.float_.size = 8
		},
	},
};

struct erw_Type* erw_type_newfromastnode(struct erw_ASTNode* node)
{
	log_assert(node, "is NULL");

	struct erw_Type* self = malloc(sizeof(struct erw_Type));
	if(!self)
	{
		log_error("Memory allocation failed in <%s>", __func__);
	}

	struct erw_Type* type = self;
	struct erw_Type* lasttype = NULL;
	int lastwasarrayorref = 0;
	while(1)
	{
		if(lastwasarrayorref)
		{
			lastwasarrayorref = 0;
			lasttype->array_.type = type;
		}

		if(!node->istoken)
		{
			if(node->descriptor == erw_ASTNODETYPE_REFERENCE)
			{
				type->type = erw_TYPETYPE_REFERENCE;
				type->reference_.mutable = 0; //Temporary
				lastwasarrayorref = 1;
			}
			else if(node->descriptor == erw_ASTNODETYPE_ARRAY)
			{
				type->type = erw_TYPETYPE_ARRAY;
				type->array_.mutable = 0; //Temporary
				lastwasarrayorref = 1;
			}
			else
			{
				log_assert(0, "should not happen");
			}
		}
		else
		{
			log_assert(node->token.type == erw_TOKENTYPE_TYPE, "???");

			type->type = erw_TYPETYPE_TYPEDEF;
			type->typedef_.name = node->token.text;
			type->typedef_.type = NULL;
			type->typedef_.token = &node->token;
			type->typedef_.used = 0;
		}

		if(vec_getsize(node->branches))
		{
			node = node->branches[0];
			lasttype = type;
			type = malloc(sizeof(struct erw_Type));
			if(!type)
			{
				log_error("Memory allocation failed in <%s>", __func__);
			}
		}
		else
		{
			break;
		}
	}

	return self;
}

struct Str erw_type_tostring(struct erw_Type* self)
{
	log_assert(self, "is NULL");
	struct Str result;
	str_ctor(&result, "");
	while(1)
	{
		if(self->type == erw_TYPETYPE_ARRAY)
		{
			str_prepend(&result, "[]");
			self = self->array_.type;
		}
		else if(self->type == erw_TYPETYPE_REFERENCE)
		{
			str_prepend(&result, "*");
			self = self->reference_.type;
		}
		else
		{
			//TODO: Generate all other types
			break;
		}
	}

	if(self->type == erw_TYPETYPE_TYPEDEF)
	{
		str_prepend(&result, self->typedef_.name); //!
	}
	return result;
}

int erw_type_compare(struct erw_Type* t1, struct erw_Type* t2)
{
	log_assert(t1, "is NULL");
	log_assert(t1, "is NULL");

	while(1)
	{
		if(t1->type != t2->type)
		{
			return 0;
		}

		if(t1->type == erw_TYPETYPE_TYPEDEF)
		{
			return !strcmp(t1->typedef_.name, t2->typedef_.name);
		}
		//TODO: Implement the rest
		
		t1 = t1->reference_.type;
		t2 = t2->reference_.type;
	}
}

struct erw_Type* erw_type_getbase(struct erw_Type* type)
{
	log_assert(type, "is NULL");
	
	while(1)
	{
		if(type->type == erw_TYPETYPE_TYPEDEF)
		{
			if(type->typedef_.type)
			{
				type = type->typedef_.type;
			}
			else
			{
				return type;
			}
		}
		else if(type->type == erw_TYPETYPE_REFERENCE || 
			type->type == erw_TYPETYPE_ARRAY)
		{
			type = type->reference_.type;
		}
		else //TODO: Implement the rest (?)
		{
			return type;
		}
	}
}

