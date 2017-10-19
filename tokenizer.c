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

#include "tokenizer.h"
#include "log.h"
#include <ctype.h>

//Wall of TokenType initializations
const struct TokenType* const TOKENTYPE_KEYWORD_RETURN 			= &(struct TokenType){"Keyword 'return'"};
const struct TokenType* const TOKENTYPE_KEYWORD_FUNC			= &(struct TokenType){"Keyword 'func'"};
const struct TokenType* const TOKENTYPE_KEYWORD_LET				= &(struct TokenType){"Keyword 'let'"};
const struct TokenType* const TOKENTYPE_KEYWORD_MUT 			= &(struct TokenType){"Keyword 'mut'"};
const struct TokenType* const TOKENTYPE_KEYWORD_TYPE 			= &(struct TokenType){"Keyword 'type'"};
const struct TokenType* const TOKENTYPE_KEYWORD_IF 				= &(struct TokenType){"Keyword 'if'"};
const struct TokenType* const TOKENTYPE_KEYWORD_CAST			= &(struct TokenType){"Keyword 'cast'"};
const struct TokenType* const TOKENTYPE_OPERATOR_DECLR			= &(struct TokenType){"Operator 'Declaration'"};
const struct TokenType* const TOKENTYPE_OPERATOR_ADD			= &(struct TokenType){"Operator 'Add'"};
const struct TokenType* const TOKENTYPE_OPERATOR_SUB			= &(struct TokenType){"Operator 'Subtract'"};
const struct TokenType* const TOKENTYPE_OPERATOR_MUL			= &(struct TokenType){"Operator 'Multiply'"};
const struct TokenType* const TOKENTYPE_OPERATOR_DIV			= &(struct TokenType){"Operator 'Divide'"};
const struct TokenType* const TOKENTYPE_OPERATOR_MOD			= &(struct TokenType){"Operator 'Modulo'"};
const struct TokenType* const TOKENTYPE_OPERATOR_POW			= &(struct TokenType){"Operator 'Exponentiate'"};
const struct TokenType* const TOKENTYPE_OPERATOR_RETURN 		= &(struct TokenType){"Operator 'Return'"};
const struct TokenType* const TOKENTYPE_OPERATOR_EQUAL			= &(struct TokenType){"Operator 'Equal'"};
const struct TokenType* const TOKENTYPE_OPERATOR_NOT  			= &(struct TokenType){"Operator 'Not'"};
const struct TokenType* const TOKENTYPE_OPERATOR_NOTEQUAL   	= &(struct TokenType){"Operator 'Not Equal'"};
const struct TokenType* const TOKENTYPE_OPERATOR_LESS			= &(struct TokenType){"Operator 'Less Than'"};
const struct TokenType* const TOKENTYPE_OPERATOR_GREATER		= &(struct TokenType){"Operator 'Greater Than'"};
const struct TokenType* const TOKENTYPE_OPERATOR_LESSOREQUAL	= &(struct TokenType){"Operator 'Less Than or Equal'"};
const struct TokenType* const TOKENTYPE_OPERATOR_GREATEROREQUAL = &(struct TokenType){"Operator 'Greater Than or Equal'"};
const struct TokenType* const TOKENTYPE_OPERATOR_AND			= &(struct TokenType){"Operator 'Logical And'"};
const struct TokenType* const TOKENTYPE_OPERATOR_OR				= &(struct TokenType){"Operator 'Logical Or'"};
const struct TokenType* const TOKENTYPE_OPERATOR_BITOR			= &(struct TokenType){"Operator 'Bitwise Or'"};
const struct TokenType* const TOKENTYPE_OPERATOR_BITAND			= &(struct TokenType){"Operator 'Bitwise And'"};
const struct TokenType* const TOKENTYPE_OPERATOR_ASSIGN 		= &(struct TokenType){"Operator 'Assign'"};
const struct TokenType* const TOKENTYPE_OPERATOR_ADDASSIGN		= &(struct TokenType){"Operator 'Add and Assign'"};
const struct TokenType* const TOKENTYPE_OPERATOR_SUBASSIGN		= &(struct TokenType){"Operator 'Subtract and Assign'"};
const struct TokenType* const TOKENTYPE_OPERATOR_MULASSIGN		= &(struct TokenType){"Operator 'Multiply and Assign'"};
const struct TokenType* const TOKENTYPE_OPERATOR_DIVASSIGN		= &(struct TokenType){"Operator 'Divide and Assign'"};
const struct TokenType* const TOKENTYPE_OPERATOR_MODASSIGN		= &(struct TokenType){"Operator 'Modulo and Assign'"};
const struct TokenType* const TOKENTYPE_OPERATOR_POWASSIGN		= &(struct TokenType){"Operator 'Exponentiate and assign'"};
const struct TokenType* const TOKENTYPE_LITERAL_INT				= &(struct TokenType){"Literal Int"};
const struct TokenType* const TOKENTYPE_LITERAL_FLOAT			= &(struct TokenType){"Literal Float"};
const struct TokenType* const TOKENTYPE_LITERAL_STRING			= &(struct TokenType){"Literal String"};
const struct TokenType* const TOKENTYPE_LITERAL_CHAR			= &(struct TokenType){"Literal Char"};
const struct TokenType* const TOKENTYPE_LITERAL_BOOL			= &(struct TokenType){"Literal Bool"};
const struct TokenType* const TOKENTYPE_IDENT					= &(struct TokenType){"Identifier"};
const struct TokenType* const TOKENTYPE_TYPE					= &(struct TokenType){"Type"};
const struct TokenType* const TOKENTYPE_END						= &(struct TokenType){"End"};
const struct TokenType* const TOKENTYPE_COMMA           	    = &(struct TokenType){"Comma"};
const struct TokenType* const TOKENTYPE_LPAREN					= &(struct TokenType){"Left Parenthesis"};
const struct TokenType* const TOKENTYPE_RPAREN					= &(struct TokenType){"Right Parenthesis"};
const struct TokenType* const TOKENTYPE_LCURLY					= &(struct TokenType){"Left Curly Bracket"};
const struct TokenType* const TOKENTYPE_RCURLY					= &(struct TokenType){"Right Curly Bracket"};
const struct TokenType* const TOKENTYPE_LBRACKET				= &(struct TokenType){"Left Bracket"};
const struct TokenType* const TOKENTYPE_RBRACKET				= &(struct TokenType){"Right Bracket"};

Vec(struct Token) tokenize(const char* source)
{
	log_assert(source, "is NULL");
	Vec(struct Token) tokens = vec_ctor(struct Token, 0);

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
					log_error(
						"Tokenizing error: No comment ending "
							"(expected '#]') after line %zu, column %zu", 
						startline,
						startcolumn
					);
				}
			}
			else if(source[pos + 1] == ']') //Multiline
			{
				log_error(
					"Tokenizing error: Unexpected comment ending "
						"('#]') at line %zu, column %zu", 
					line,
					column
				);
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

		struct Token token = {
			.text = vec_ctor(char, 0),
			.line = line,
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
			token.type = TOKENTYPE_TYPE;
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
				token.type = TOKENTYPE_KEYWORD_LET;
			}
			else if(sizeof("mut") - 1 == vec_getsize(token.text) &&
				!memcmp("mut", token.text, sizeof("mut") - 1))
			{
				token.type = TOKENTYPE_KEYWORD_MUT;
			}
			else if(sizeof("func") - 1 == vec_getsize(token.text) &&
				!memcmp("func", token.text, sizeof("func") - 1))
			{
				token.type = TOKENTYPE_KEYWORD_FUNC;
			}
			else if(sizeof("type") - 1 == vec_getsize(token.text) &&
				!memcmp("type", token.text, sizeof("type") - 1))
			{
				token.type = TOKENTYPE_KEYWORD_TYPE;
			}
			else if(sizeof("return") - 1 == vec_getsize(token.text) &&
				!memcmp("return", token.text, sizeof("return") - 1))
			{
				token.type = TOKENTYPE_KEYWORD_RETURN;
			}
			else if(sizeof("if") - 1 == vec_getsize(token.text) &&
				!memcmp("if", token.text, sizeof("if") - 1))
			{
				token.type = TOKENTYPE_KEYWORD_IF;
			}
			else if(sizeof("cast") - 1 == vec_getsize(token.text) &&
				!memcmp("cast", token.text, sizeof("cast") - 1))
			{
				token.type = TOKENTYPE_KEYWORD_CAST;
			}
			else if(sizeof("true") - 1 == vec_getsize(token.text) &&
				!memcmp("true", token.text, sizeof("true") - 1))
			{
				token.type = TOKENTYPE_LITERAL_BOOL;
			}
			else if(sizeof("false") - 1 == vec_getsize(token.text) &&
				!memcmp("false", token.text, sizeof("false") - 1))
			{
				token.type = TOKENTYPE_LITERAL_BOOL;
			}
			else
			{
				token.type = TOKENTYPE_IDENT;
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
				token.type = TOKENTYPE_LITERAL_FLOAT;
			}
			else
			{
				token.type = TOKENTYPE_LITERAL_INT;
			}
		}
		else
		{
			switch(source[pos])
			{
			case ':':
				token.type = TOKENTYPE_OPERATOR_DECLR;
				break;

			case ';':
				token.type = TOKENTYPE_END;
				break;

			case ',':
				token.type = TOKENTYPE_COMMA;
				break;

			case '+':
				if(source[pos + 1] == '=')
				{
					token.type = TOKENTYPE_OPERATOR_ADDASSIGN;
					column++;
					vec_pushback(token.text, source[pos]);
					pos++;
				}
				else
				{
					token.type = TOKENTYPE_OPERATOR_ADD;
				}
				break;

			case '-':
				if(source[pos + 1] == '=')
				{
					token.type = TOKENTYPE_OPERATOR_SUBASSIGN;
					column++;
					vec_pushback(token.text, source[pos]);
					pos++;
				}
				else if(source[pos + 1] == '>')
				{
					token.type = TOKENTYPE_OPERATOR_RETURN;
					column++;
					vec_pushback(token.text, source[pos]);
					pos++;
				}
				else
				{
					token.type = TOKENTYPE_OPERATOR_SUB;
				}
				break;

			case '*':
				if(source[pos + 1] == '=')
				{
					token.type = TOKENTYPE_OPERATOR_MULASSIGN;
					column++;
					vec_pushback(token.text, source[pos]);
					pos++;
				}
				else
				{
					token.type = TOKENTYPE_OPERATOR_MUL;
				}
				break;

			case '/':
				if(source[pos + 1] == '=')
				{
					token.type = TOKENTYPE_OPERATOR_DIVASSIGN;
					column++;
					vec_pushback(token.text, source[pos]);
					pos++;
				}
				else
				{
					token.type = TOKENTYPE_OPERATOR_DIV;
				}
				break;

			case '%':
				if(source[pos + 1] == '=')
				{
					token.type = TOKENTYPE_OPERATOR_MODASSIGN;
					column++;
					vec_pushback(token.text, source[pos]);
					pos++;
				}
				else
				{
					token.type = TOKENTYPE_OPERATOR_MOD;
				}
				break;

			case '^':
				if(source[pos + 1] == '=')
				{
					token.type = TOKENTYPE_OPERATOR_POWASSIGN;
					column++;
					vec_pushback(token.text, source[pos]);
					pos++;
				}
				else
				{
					token.type = TOKENTYPE_OPERATOR_POW;
				}
				break;

			case '=':
				if(source[pos + 1] == '=')
				{
					token.type = TOKENTYPE_OPERATOR_EQUAL;
					column++;
					vec_pushback(token.text, source[pos]);
					pos++;
				}
				else
				{
					token.type = TOKENTYPE_OPERATOR_ASSIGN;
				}
				break;

			case '!':
				if(source[pos + 1] == '=')
				{
					token.type = TOKENTYPE_OPERATOR_NOTEQUAL;
					column++;
					vec_pushback(token.text, source[pos]);
					pos++;
				}
				else
				{
					token.type = TOKENTYPE_OPERATOR_NOT;
				}
				break;

			case '<':
				if(source[pos + 1] == '=')
				{
					token.type = TOKENTYPE_OPERATOR_LESSOREQUAL;
					column++;
					vec_pushback(token.text, source[pos]);
					pos++;
				}
				else
				{
					token.type = TOKENTYPE_OPERATOR_LESS;
				}
				break;

			case '>':
				if(source[pos + 1] == '=')
				{
					token.type = TOKENTYPE_OPERATOR_GREATEROREQUAL;
					column++;
					vec_pushback(token.text, source[pos]);
					pos++;
				}
				else
				{
					token.type = TOKENTYPE_OPERATOR_GREATER;
				}
				break;

			case '&':
				if(source[pos + 1] == '&')
				{
					token.type = TOKENTYPE_OPERATOR_AND;
					column++;
					vec_pushback(token.text, source[pos]);
					pos++;
				}
				else
				{
					token.type = TOKENTYPE_OPERATOR_BITAND;
				}
				break;

			case '|':
				if(source[pos + 1] == '|')
				{
					token.type = TOKENTYPE_OPERATOR_OR;
					column++;
					vec_pushback(token.text, source[pos]);
					pos++;
				}
				else
				{
					token.type = TOKENTYPE_OPERATOR_BITOR;
				}
				break;

			case '[':
				token.type = TOKENTYPE_LBRACKET;
				break;

			case ']':
				token.type = TOKENTYPE_RBRACKET;
				break;

			case '{':
				token.type = TOKENTYPE_LCURLY;
				break;

			case '}':
				token.type = TOKENTYPE_RCURLY;
				break;

			case '(':
				token.type = TOKENTYPE_LPAREN;
				break;

			case ')':
				token.type = TOKENTYPE_RPAREN;
				break;

			case '"':
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
				token.type = TOKENTYPE_LITERAL_STRING;

				if(source[pos] != '"') //Does this work as intended?
				{
					log_error(
						"Tokenizing error: Non-terminated string"
							" at line %zu, column %zu", 
						line,
						column
					);
				}
				break;

			case '\'':
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
				token.type = TOKENTYPE_LITERAL_CHAR;

				if(source[pos] != '\'' ||
					vec_getsize(token.text) != 2)
				{
					log_error(
						"Tokenizing error: Invalid %s"
							"(expected '<single letter>', got %*.s)"
							" at line %zu, column %zu", 
						TOKENTYPE_LITERAL_CHAR->name,
						(int)vec_getsize(token.text),
						token.text,
						line,
						column
					);
				}
				break;
			
			default:
				log_error(
					"Tokenizing error: Invalid character "
						"('%c' |%i|)" " at line %zu, column %zu", 
					source[pos],
					source[pos],
					line,
					column
				);
			}

			vec_pushback(token.text, source[pos]);
			column++;
			pos++;
		}

		vec_pushback(token.text, '\0');
		vec_pushback(tokens, token);
	}

	return tokens;
}

