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

#ifndef ERW_TOKENIZER_H
#define ERW_TOKENIZER_H

#include "str.h"
#include "vec.h"

struct erw_TokenType
{
	const char* name;
};

extern const struct erw_TokenType* const erw_TOKENTYPE_KEYWORD_RETURN;
extern const struct erw_TokenType* const erw_TOKENTYPE_KEYWORD_FUNC;
extern const struct erw_TokenType* const erw_TOKENTYPE_KEYWORD_LET;
extern const struct erw_TokenType* const erw_TOKENTYPE_KEYWORD_MUT;
extern const struct erw_TokenType* const erw_TOKENTYPE_KEYWORD_TYPE;
extern const struct erw_TokenType* const erw_TOKENTYPE_KEYWORD_IF;
extern const struct erw_TokenType* const erw_TOKENTYPE_KEYWORD_ELSEIF;
extern const struct erw_TokenType* const erw_TOKENTYPE_KEYWORD_ELSE;
extern const struct erw_TokenType* const erw_TOKENTYPE_KEYWORD_CAST;
extern const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_DECLR;
extern const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_ADD;
extern const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_SUB;
extern const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_MUL;
extern const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_DIV;
extern const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_MOD;
extern const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_POW;
extern const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_RETURN;
extern const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_EQUAL;
extern const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_NOT;
extern const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_NOTEQUAL;
extern const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_LESS;
extern const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_GREATER;
extern const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_LESSOREQUAL;
extern const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_GREATEROREQUAL;
extern const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_AND;
extern const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_OR;
extern const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_ASSIGN;
extern const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_ADDASSIGN;
extern const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_SUBASSIGN;
extern const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_MULASSIGN;
extern const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_DIVASSIGN;
extern const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_MODASSIGN;
extern const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_POWASSIGN;
extern const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_BITOR;
extern const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_BITAND;
extern const struct erw_TokenType* const erw_TOKENTYPE_LITERAL_INT;
extern const struct erw_TokenType* const erw_TOKENTYPE_LITERAL_FLOAT;
extern const struct erw_TokenType* const erw_TOKENTYPE_LITERAL_STRING;
extern const struct erw_TokenType* const erw_TOKENTYPE_LITERAL_CHAR;
extern const struct erw_TokenType* const erw_TOKENTYPE_LITERAL_BOOL;
extern const struct erw_TokenType* const erw_TOKENTYPE_IDENT;
extern const struct erw_TokenType* const erw_TOKENTYPE_TYPE;
extern const struct erw_TokenType* const erw_TOKENTYPE_END;
extern const struct erw_TokenType* const erw_TOKENTYPE_COMMA;
extern const struct erw_TokenType* const erw_TOKENTYPE_LPAREN;
extern const struct erw_TokenType* const erw_TOKENTYPE_RPAREN;
extern const struct erw_TokenType* const erw_TOKENTYPE_LCURLY;
extern const struct erw_TokenType* const erw_TOKENTYPE_RCURLY;
extern const struct erw_TokenType* const erw_TOKENTYPE_LBRACKET;
extern const struct erw_TokenType* const erw_TOKENTYPE_RBRACKET;
extern const struct erw_TokenType* const erw_TOKENTYPE_FOREIGN; //?

struct erw_Token
{
	Vec(char) text;
	const struct erw_TokenType* type;
	size_t linenum;
	size_t column;
};

Vec(struct erw_Token) erw_tokenize(const char* source, Vec(struct Str) lines);

#endif
