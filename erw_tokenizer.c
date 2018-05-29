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

#include "erw_tokenizer.h"
#include "erw_error.h"
#include "log.h"

#include <ctype.h>

//Wall of erw_TokenType initializations
const struct erw_TokenType* const erw_TOKENTYPE_KEYWORD_RETURN =
	&(struct erw_TokenType){"Keyword 'return'"};
const struct erw_TokenType* const erw_TOKENTYPE_KEYWORD_FUNC =
	&(struct erw_TokenType){"Keyword 'func'"};
const struct erw_TokenType* const erw_TOKENTYPE_KEYWORD_LET =
	&(struct erw_TokenType){"Keyword 'let'"};
const struct erw_TokenType* const erw_TOKENTYPE_KEYWORD_MUT =
	&(struct erw_TokenType){"Keyword 'mut'"};
const struct erw_TokenType* const erw_TOKENTYPE_KEYWORD_TYPE =
	&(struct erw_TokenType){"Keyword 'type'"};
const struct erw_TokenType* const erw_TOKENTYPE_KEYWORD_IF =
	&(struct erw_TokenType){"Keyword 'if'"};
const struct erw_TokenType* const erw_TOKENTYPE_KEYWORD_ELSEIF =
	&(struct erw_TokenType){"Keyword 'elseif'"};
const struct erw_TokenType* const erw_TOKENTYPE_KEYWORD_ELSE =
	&(struct erw_TokenType){"Keyword 'else'"};
const struct erw_TokenType* const erw_TOKENTYPE_KEYWORD_CAST =
	&(struct erw_TokenType){"Keyword 'cast'"};
const struct erw_TokenType* const erw_TOKENTYPE_KEYWORD_DEFER =
	&(struct erw_TokenType){"Keyword 'defer'"};
const struct erw_TokenType* const erw_TOKENTYPE_KEYWORD_WHILE =
	&(struct erw_TokenType){"Keyword 'while'"};
const struct erw_TokenType* const erw_TOKENTYPE_KEYWORD_STRUCT =
	&(struct erw_TokenType){"Keyword 'struct'"};
const struct erw_TokenType* const erw_TOKENTYPE_KEYWORD_UNION =
	&(struct erw_TokenType){"Keyword 'union'"};
const struct erw_TokenType* const erw_TOKENTYPE_KEYWORD_ENUM = 
	&(struct erw_TokenType){"Keyword 'enum'"};
const struct erw_TokenType* const erw_TOKENTYPE_KEYWORD_UNSAFE =
	&(struct erw_TokenType){"Keyword 'unsafe'"};
const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_DECLR =
	&(struct erw_TokenType){"Operator 'Declaration'"};
const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_ADD =
	&(struct erw_TokenType){"Operator 'Add'"};
const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_SUB =
	&(struct erw_TokenType){"Operator 'Subtract'"};
const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_MUL =
	&(struct erw_TokenType){"Operator 'Multiply'"};
const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_DIV =
	&(struct erw_TokenType){"Operator 'Divide'"};
const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_MOD =
	&(struct erw_TokenType){"Operator 'Modulo'"};
const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_POW =
	&(struct erw_TokenType){"Operator 'Exponentiate'"};
const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_RETURN =
	&(struct erw_TokenType){"Operator 'Return'"};
const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_EQUAL =
	&(struct erw_TokenType){"Operator 'Equal'"};
const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_NOT =
	&(struct erw_TokenType){"Operator 'Not'"};
const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_NOTEQUAL =
	&(struct erw_TokenType){"Operator 'Not Equal'"};
const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_LESS =
	&(struct erw_TokenType){"Operator 'Less Than'"};
const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_GREATER =
	&(struct erw_TokenType){"Operator 'Greater Than'"};
const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_LESSOREQUAL =
	&(struct erw_TokenType){"Operator 'Less Than or Equal'"};
const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_GREATEROREQUAL =
	&(struct erw_TokenType){"Operator 'Greater Than or Equal'"};
const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_AND =
	&(struct erw_TokenType){"Operator 'Logical And'"};
const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_OR =
	&(struct erw_TokenType){"Operator 'Logical Or'"};
const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_ASSIGN =
	&(struct erw_TokenType){"Operator 'Assign'"};
const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_ADDASSIGN =
	&(struct erw_TokenType){"Operator 'Add and Assign'"};
const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_SUBASSIGN =
	&(struct erw_TokenType){"Operator 'Subtract and Assign'"};
const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_MULASSIGN =
	&(struct erw_TokenType){"Operator 'Multiply and Assign'"};
const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_DIVASSIGN =
	&(struct erw_TokenType){"Operator 'Divide and Assign'"};
const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_MODASSIGN =
	&(struct erw_TokenType){"Operator 'Modulo and Assign'"};
const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_POWASSIGN =
	&(struct erw_TokenType){"Operator 'Exponentiate and assign'"};
const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_BITOR =
	&(struct erw_TokenType){"Operator 'Bitwise Or'"};
const struct erw_TokenType* const erw_TOKENTYPE_OPERATOR_BITAND =
	&(struct erw_TokenType){"Operator 'Bitwise And'"};
const struct erw_TokenType* const erw_TOKENTYPE_ACCESS =
	&(struct erw_TokenType){"Operator 'Access'"};
const struct erw_TokenType* const erw_TOKENTYPE_LITERAL_INT =
	&(struct erw_TokenType){"Literal Int"};
const struct erw_TokenType* const erw_TOKENTYPE_LITERAL_FLOAT =
	&(struct erw_TokenType){"Literal Float"};
const struct erw_TokenType* const erw_TOKENTYPE_LITERAL_STRING =
	&(struct erw_TokenType){"Literal String"};
const struct erw_TokenType* const erw_TOKENTYPE_LITERAL_CHAR =
	&(struct erw_TokenType){"Literal Char"};
const struct erw_TokenType* const erw_TOKENTYPE_LITERAL_BOOL =
	&(struct erw_TokenType){"Literal Bool"};
const struct erw_TokenType* const erw_TOKENTYPE_IDENT =
	&(struct erw_TokenType){"Identifier"};
const struct erw_TokenType* const erw_TOKENTYPE_TYPE =
	&(struct erw_TokenType){"Type"};
const struct erw_TokenType* const erw_TOKENTYPE_END =
	&(struct erw_TokenType){"End (';')"};
const struct erw_TokenType* const erw_TOKENTYPE_COMMA =
	&(struct erw_TokenType){"Comma"};
const struct erw_TokenType* const erw_TOKENTYPE_LPAREN =
	&(struct erw_TokenType){"Left Parenthesis"};
const struct erw_TokenType* const erw_TOKENTYPE_RPAREN =
	&(struct erw_TokenType){"Right Parenthesis"};
const struct erw_TokenType* const erw_TOKENTYPE_LCURLY =
	&(struct erw_TokenType){"Left Curly Bracket"};
const struct erw_TokenType* const erw_TOKENTYPE_RCURLY =
	&(struct erw_TokenType){"Right Curly Bracket"};
const struct erw_TokenType* const erw_TOKENTYPE_LBRACKET =
	&(struct erw_TokenType){"Left Bracket"};
const struct erw_TokenType* const erw_TOKENTYPE_RBRACKET =
	&(struct erw_TokenType){"Right Bracket"};
const struct erw_TokenType* const erw_TOKENTYPE_FOREIGN =
	&(struct erw_TokenType){"Foreign function call"};

Vec(struct erw_Token) erw_tokenize(const char* source, Vec(struct Str) lines)
{
	log_assert(lines, "is NULL");

	Vec(struct erw_Token) tokens = vec_ctor(struct erw_Token, 0);

	size_t pos = 0;
	size_t line = 1;
	size_t column = 1;

	while(source[pos] != '\0')
	{
		if(isblank(source[pos]))
		{
			column++;
			pos++;
			continue;
		}
		else if(source[pos] == '\n')
		{
			column = 1;
			line++;
			pos++;
			continue;
		}
		else if(source[pos] == '#')
		{
			if(source[pos + 1] == '[') //Multiline
			{
				size_t startcolumn = column + 1;
				size_t startline = line;
				size_t nested = 0;
				do {
					pos++;
					if(source[pos] == '#')
					{
						if(source[pos + 1] == '[')
						{
							nested++;
							column++;
							pos++;
						}
						else if(source[pos + 1] == ']')
						{
							if(nested)
							{
								nested--;
							}
							else
							{
								pos += 2;
								column++;
								break;
							}
						}
					}
					else if(source[pos] == '\n')
					{
						column = 1;
						line++;
					}
					else
					{
						column++;
					}
				} while(source[pos] != '\0');

				if(nested || source[pos - 1] != ']')
				{
					struct Str msg;
					str_ctor(
						&msg,
						"No comment ending (expected '#]')"
					);

					erw_error(
						msg.data,
						lines[startline - 1].data,
						startline,
						startcolumn - 1,
						column + 1
					);
					str_dtor(&msg);
				}
			}
			else if(source[pos + 1] == ']') //Multiline
			{
				struct Str msg;
				str_ctor(
					&msg,
					"Unexpected comment ending ('#]')"
				);

				erw_error(
					msg.data,
					lines[line - 1].data,
					line,
					column,
					column + 1
				);
				str_dtor(&msg);
			}
			else //Single line comment
			{
				do {
					column++;
					pos++;
				}
				while(
					source[pos] != '\n' &&
					//source[pos] != ';' &&
					source[pos] != '\0'
				);
			}

			continue;
		}

		struct erw_Token token = {
			.text = vec_ctor(char, 0),
			.linenum = line,
			.column = column
		};

		if(isupper(source[pos]))
		{
			do {
				column++;
				vec_pushback(token.text, source[pos]);
				pos++;
			}
			while(isalnum(source[pos]) || source[pos] == '_');
			token.type = erw_TOKENTYPE_TYPE;
		}
		else if(islower(source[pos]))
		{
			do {
				column++;
				vec_pushback(token.text, source[pos]);
				pos++;
			}
			while(isalnum(source[pos]) || source[pos] == '_');

			if(sizeof("let") - 1 == vec_getsize(token.text) &&
				!memcmp("let", token.text, sizeof("let") - 1))
			{
				token.type = erw_TOKENTYPE_KEYWORD_LET;
			}
			else if(sizeof("mut") - 1 == vec_getsize(token.text) &&
				!memcmp("mut", token.text, sizeof("mut") - 1))
			{
				token.type = erw_TOKENTYPE_KEYWORD_MUT;
			}
			else if(sizeof("func") - 1 == vec_getsize(token.text) &&
				!memcmp("func", token.text, sizeof("func") - 1))
			{
				token.type = erw_TOKENTYPE_KEYWORD_FUNC;
			}
			else if(sizeof("type") - 1 == vec_getsize(token.text) &&
				!memcmp("type", token.text, sizeof("type") - 1))
			{
				token.type = erw_TOKENTYPE_KEYWORD_TYPE;
			}
			else if(sizeof("return") - 1 == vec_getsize(token.text) &&
				!memcmp("return", token.text, sizeof("return") - 1))
			{
				token.type = erw_TOKENTYPE_KEYWORD_RETURN;
			}
			else if(sizeof("if") - 1 == vec_getsize(token.text) &&
				!memcmp("if", token.text, sizeof("if") - 1))
			{
				token.type = erw_TOKENTYPE_KEYWORD_IF;
			}
			else if(sizeof("elseif") - 1 == vec_getsize(token.text) &&
				!memcmp("elseif", token.text, sizeof("elseif") - 1))
			{
				token.type = erw_TOKENTYPE_KEYWORD_ELSEIF;
			}
			else if(sizeof("else") - 1 == vec_getsize(token.text) &&
				!memcmp("else", token.text, sizeof("else") - 1))
			{
				token.type = erw_TOKENTYPE_KEYWORD_ELSE;
			}
			else if(sizeof("cast") - 1 == vec_getsize(token.text) &&
				!memcmp("cast", token.text, sizeof("cast") - 1))
			{
				token.type = erw_TOKENTYPE_KEYWORD_CAST;
			}
			else if(sizeof("defer") - 1 == vec_getsize(token.text) &&
				!memcmp("defer", token.text, sizeof("defer") - 1))
			{
				token.type = erw_TOKENTYPE_KEYWORD_DEFER;
			}
			else if(sizeof("while") - 1 == vec_getsize(token.text) &&
				!memcmp("while", token.text, sizeof("while") - 1))
			{
				token.type = erw_TOKENTYPE_KEYWORD_WHILE;
			}
			else if(sizeof("struct") - 1 == vec_getsize(token.text) &&
				!memcmp("struct", token.text, sizeof("struct") - 1))
			{
				token.type = erw_TOKENTYPE_KEYWORD_STRUCT;
			}
			else if(sizeof("union") - 1 == vec_getsize(token.text) &&
				!memcmp("union", token.text, sizeof("union") - 1))
			{
				token.type = erw_TOKENTYPE_KEYWORD_UNION;
			}
			else if(sizeof("enum") - 1 == vec_getsize(token.text) &&
				!memcmp("enum", token.text, sizeof("enum") - 1))
			{
				token.type = erw_TOKENTYPE_KEYWORD_ENUM;
			}
			else if(sizeof("unsafe") - 1 == vec_getsize(token.text) &&
				!memcmp("unsafe", token.text, sizeof("unsafe") - 1))
			{
				token.type = erw_TOKENTYPE_KEYWORD_UNSAFE;
			}
			else if(sizeof("and") - 1 == vec_getsize(token.text) &&
				!memcmp("and", token.text, sizeof("and") - 1))
			{
				token.type = erw_TOKENTYPE_OPERATOR_AND;
			}
			else if(sizeof("or") - 1 == vec_getsize(token.text) &&
				!memcmp("or", token.text, sizeof("or") - 1))
			{
				token.type = erw_TOKENTYPE_OPERATOR_OR;
			}
			else if(sizeof("true") - 1 == vec_getsize(token.text) &&
				!memcmp("true", token.text, sizeof("true") - 1))
			{
				token.type = erw_TOKENTYPE_LITERAL_BOOL;
			}
			else if(sizeof("false") - 1 == vec_getsize(token.text) &&
				!memcmp("false", token.text, sizeof("false") - 1))
			{
				token.type = erw_TOKENTYPE_LITERAL_BOOL;
			}
			else
			{
				token.type = erw_TOKENTYPE_IDENT;
			}
		}
		else if(isdigit(source[pos]))
		{
			do {
				column++;
				vec_pushback(token.text, source[pos]);
				pos++;
			}
			while(isdigit(source[pos]));

			if(source[pos] == '.')
			{
				do {
					column++;
					vec_pushback(token.text, source[pos]);
					pos++;
				}
				while(isdigit(source[pos]));
				token.type = erw_TOKENTYPE_LITERAL_FLOAT;
			}
			else
			{
				token.type = erw_TOKENTYPE_LITERAL_INT;
			}
		}
		else
		{
			switch(source[pos])
			{
			case ':':
				token.type = erw_TOKENTYPE_OPERATOR_DECLR;
				break;

			case ';':
				token.type = erw_TOKENTYPE_END;
				break;

			case ',':
				token.type = erw_TOKENTYPE_COMMA;
				break;

			case '.':
				token.type = erw_TOKENTYPE_ACCESS;
				break;

			case '+':
				if(source[pos + 1] == '=')
				{
					token.type = erw_TOKENTYPE_OPERATOR_ADDASSIGN;
					column++;
					vec_pushback(token.text, source[pos]);
					pos++;
				}
				else
				{
					token.type = erw_TOKENTYPE_OPERATOR_ADD;
				}
				break;

			case '-':
				if(source[pos + 1] == '=')
				{
					token.type = erw_TOKENTYPE_OPERATOR_SUBASSIGN;
					column++;
					vec_pushback(token.text, source[pos]);
					pos++;
				}
				else if(source[pos + 1] == '>')
				{
					token.type = erw_TOKENTYPE_OPERATOR_RETURN;
					column++;
					vec_pushback(token.text, source[pos]);
					pos++;
				}
				else
				{
					token.type = erw_TOKENTYPE_OPERATOR_SUB;
				}
				break;

			case '*':
				if(source[pos + 1] == '=')
				{
					token.type = erw_TOKENTYPE_OPERATOR_MULASSIGN;
					column++;
					vec_pushback(token.text, source[pos]);
					pos++;
				}
				else
				{
					token.type = erw_TOKENTYPE_OPERATOR_MUL;
				}
				break;

			case '/':
				if(source[pos + 1] == '=')
				{
					token.type = erw_TOKENTYPE_OPERATOR_DIVASSIGN;
					column++;
					vec_pushback(token.text, source[pos]);
					pos++;
				}
				else
				{
					token.type = erw_TOKENTYPE_OPERATOR_DIV;
				}
				break;

			case '%':
				if(source[pos + 1] == '=')
				{
					token.type = erw_TOKENTYPE_OPERATOR_MODASSIGN;
					column++;
					vec_pushback(token.text, source[pos]);
					pos++;
				}
				else
				{
					token.type = erw_TOKENTYPE_OPERATOR_MOD;
				}
				break;

			case '^':
				if(source[pos + 1] == '=')
				{
					token.type = erw_TOKENTYPE_OPERATOR_POWASSIGN;
					column++;
					vec_pushback(token.text, source[pos]);
					pos++;
				}
				else
				{
					token.type = erw_TOKENTYPE_OPERATOR_POW;
				}
				break;

			case '=':
				if(source[pos + 1] == '=')
				{
					token.type = erw_TOKENTYPE_OPERATOR_EQUAL;
					column++;
					vec_pushback(token.text, source[pos]);
					pos++;
				}
				else
				{
					token.type = erw_TOKENTYPE_OPERATOR_ASSIGN;
				}
				break;

			case '!':
				if(source[pos + 1] == '=')
				{
					token.type = erw_TOKENTYPE_OPERATOR_NOTEQUAL;
					column++;
					vec_pushback(token.text, source[pos]);
					pos++;
				}
				else
				{
					token.type = erw_TOKENTYPE_OPERATOR_NOT;
				}
				break;

			case '<':
				if(source[pos + 1] == '=')
				{
					token.type = erw_TOKENTYPE_OPERATOR_LESSOREQUAL;
					column++;
					vec_pushback(token.text, source[pos]);
					pos++;
				}
				else
				{
					token.type = erw_TOKENTYPE_OPERATOR_LESS;
				}
				break;

			case '>':
				if(source[pos + 1] == '=')
				{
					token.type = erw_TOKENTYPE_OPERATOR_GREATEROREQUAL;
					column++;
					vec_pushback(token.text, source[pos]);
					pos++;
				}
				else
				{
					token.type = erw_TOKENTYPE_OPERATOR_GREATER;
				}
				break;

			case '&':
				/*if(source[pos + 1] == '&')
				{
					token.type = erw_TOKENTYPE_OPERATOR_AND;
					column++;
					vec_pushback(token.text, source[pos]);
					pos++;
				}
				else*/
				{
					token.type = erw_TOKENTYPE_OPERATOR_BITAND;
				}
				break;

			case '|':
				/*if(source[pos + 1] == '|')
				{
					token.type = erw_TOKENTYPE_OPERATOR_OR;
					column++;
					vec_pushback(token.text, source[pos]);
					pos++;
				}
				else*/
				{
					token.type = erw_TOKENTYPE_OPERATOR_BITOR;
				}
				break;

			case '[':
				token.type = erw_TOKENTYPE_LBRACKET;
				break;

			case ']':
				token.type = erw_TOKENTYPE_RBRACKET;
				break;

			case '{':
				token.type = erw_TOKENTYPE_LCURLY;
				break;

			case '}':
				token.type = erw_TOKENTYPE_RCURLY;
				break;

			case '(':
				token.type = erw_TOKENTYPE_LPAREN;
				break;

			case ')':
				token.type = erw_TOKENTYPE_RPAREN;
				break;

			case '"':
			{
				size_t startcolumn = column;
				do {
					column++;
					vec_pushback(token.text, source[pos]);
					pos++;
				}
				while(
					source[pos] != '"' &&
					source[pos] != '\n' &&
					source[pos] != '\0'
				);
				token.type = erw_TOKENTYPE_LITERAL_STRING;

				if(source[pos] != '"') //Does this work as intended?
				{
					struct Str msg;
					str_ctor(
						&msg,
						"Non-terminated string"
					);

					erw_error(
						msg.data,
						lines[line - 1].data,
						line,
						startcolumn,
						column
					);
					str_dtor(&msg);
				}
				break;
			}
			case '\'': //TODO: Fix '\n' and etc.
		 	{
				size_t startcolumn = column;
				do {
					column++;
					vec_pushback(token.text, source[pos]);
					pos++;
				}
				while(
					source[pos] != '\'' &&
					source[pos] != '\n' &&
					source[pos] != '\0'
				);
				token.type = erw_TOKENTYPE_LITERAL_CHAR;

				if(source[pos] != '\'' ||
					vec_getsize(token.text) != 2)
				{
					struct Str msg;
					str_ctorfmt(
						&msg,
						"Invalid %s (expected '<single letter>')",
						erw_TOKENTYPE_LITERAL_CHAR->name
					);

					erw_error(
						msg.data,
						lines[line - 1].data,
						line,
						startcolumn,
						column
					);
					str_dtor(&msg);
				}
				break;
			}
			case '@':
				column++;
				pos++;

				if(islower(source[pos]))
				{
					do {
						column++;
						vec_pushback(token.text, source[pos]);
						pos++;
					}
					while(isalnum(source[pos]) || source[pos] == '_');
				}
				else
				{
					struct Str msg;
					str_ctor(
						&msg,
						"Invalid foreign function call (expected 'identifier'"
							" after '@')"
					);

					erw_error(
						msg.data,
						lines[line - 1].data,
						line,
						column - 1,
						column - 1
					);
					str_dtor(&msg);
				}

				token.type = erw_TOKENTYPE_FOREIGN;
				goto done; //XXX skip vec_pushback in the end

			default:;
				struct Str msg;
				str_ctorfmt(
					&msg,
					"Invalid character (num: |%i|)",
					source[pos]
				);

				erw_error(
					msg.data,
					lines[line - 1].data,
					line,
					column,
					column
				);
				str_dtor(&msg);
			}

			vec_pushback(token.text, source[pos]);
			column++;
			pos++;
		}

	done: //XXX
		vec_pushback(token.text, '\0');
		vec_pushback(tokens, token);
	}

	return tokens;
}

void erw_tokens_delete(Vec(struct erw_Token) tokens)
{
	log_assert(tokens, "is NULL");
	for(size_t i = 0; i < vec_getsize(tokens); i++)
	{
		vec_dtor(tokens[i].text);
	}

	vec_dtor(tokens);
}

