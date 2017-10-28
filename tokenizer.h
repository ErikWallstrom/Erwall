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

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "vec.h"

struct TokenType
{
	const char* name;
};

extern const struct TokenType* const TOKENTYPE_KEYWORD_RETURN;
extern const struct TokenType* const TOKENTYPE_KEYWORD_FUNC;
extern const struct TokenType* const TOKENTYPE_KEYWORD_LET;
extern const struct TokenType* const TOKENTYPE_KEYWORD_MUT;
extern const struct TokenType* const TOKENTYPE_KEYWORD_TYPE;
extern const struct TokenType* const TOKENTYPE_KEYWORD_IF;
extern const struct TokenType* const TOKENTYPE_OPERATOR_DECLR;
extern const struct TokenType* const TOKENTYPE_OPERATOR_ADD;
extern const struct TokenType* const TOKENTYPE_OPERATOR_SUB;
extern const struct TokenType* const TOKENTYPE_OPERATOR_MUL;
extern const struct TokenType* const TOKENTYPE_OPERATOR_DIV;
extern const struct TokenType* const TOKENTYPE_OPERATOR_MOD;
extern const struct TokenType* const TOKENTYPE_OPERATOR_POW;
extern const struct TokenType* const TOKENTYPE_OPERATOR_RETURN;
extern const struct TokenType* const TOKENTYPE_OPERATOR_EQUAL;
extern const struct TokenType* const TOKENTYPE_OPERATOR_NOT;
extern const struct TokenType* const TOKENTYPE_OPERATOR_NOTEQUAL;
extern const struct TokenType* const TOKENTYPE_OPERATOR_LESS;
extern const struct TokenType* const TOKENTYPE_OPERATOR_GREATER;
extern const struct TokenType* const TOKENTYPE_OPERATOR_LESSOREQUAL;
extern const struct TokenType* const TOKENTYPE_OPERATOR_GREATEROREQUAL;
extern const struct TokenType* const TOKENTYPE_OPERATOR_AND;
extern const struct TokenType* const TOKENTYPE_OPERATOR_OR;
extern const struct TokenType* const TOKENTYPE_OPERATOR_BITOR;
extern const struct TokenType* const TOKENTYPE_OPERATOR_BITAND;
extern const struct TokenType* const TOKENTYPE_OPERATOR_ASSIGN;
extern const struct TokenType* const TOKENTYPE_OPERATOR_ADDASSIGN;
extern const struct TokenType* const TOKENTYPE_OPERATOR_SUBASSIGN;
extern const struct TokenType* const TOKENTYPE_OPERATOR_MULASSIGN;
extern const struct TokenType* const TOKENTYPE_OPERATOR_DIVASSIGN;
extern const struct TokenType* const TOKENTYPE_OPERATOR_MODASSIGN;
extern const struct TokenType* const TOKENTYPE_OPERATOR_POWASSIGN;
extern const struct TokenType* const TOKENTYPE_LITERAL_INT;
extern const struct TokenType* const TOKENTYPE_LITERAL_FLOAT;
extern const struct TokenType* const TOKENTYPE_LITERAL_STRING;
extern const struct TokenType* const TOKENTYPE_LITERAL_CHAR;
extern const struct TokenType* const TOKENTYPE_LITERAL_BOOL;
extern const struct TokenType* const TOKENTYPE_IDENT;
extern const struct TokenType* const TOKENTYPE_TYPE;
extern const struct TokenType* const TOKENTYPE_END;
extern const struct TokenType* const TOKENTYPE_COMMA;
extern const struct TokenType* const TOKENTYPE_LPAREN;
extern const struct TokenType* const TOKENTYPE_RPAREN;
extern const struct TokenType* const TOKENTYPE_LCURLY;
extern const struct TokenType* const TOKENTYPE_RCURLY;
extern const struct TokenType* const TOKENTYPE_LBRACKET;
extern const struct TokenType* const TOKENTYPE_RBRACKET;

struct Token
{
	const struct TokenType* type;
	Vec(char) text;
	size_t line;
	size_t column;
};

Vec(struct Token) tokenize(const char* source);

#endif
