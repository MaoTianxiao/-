#include "tokenClass.h"


KeyWords::KeyWords()
{
	giveValue();
}

void
KeyWords::giveValue()
{
	keywords[KW_INT] = "int";
	keywords[KW_CHAR] = "char";
	keywords[KW_VOID] = "void";
	keywords[KW_IF] = "if";
	keywords[KW_ELSE] = "else";
	keywords[KW_SWITCH] = "switch";
	keywords[KW_CASE] = "case";
	keywords[KW_DEFAULT] = "default";
	keywords[KW_WHILE] = "while";
	keywords[KW_DO] = "do";
	keywords[KW_FOR] = "for";
	keywords[KW_BREAK] = "break";
	keywords[KW_CONTINUE] = "continue";
	keywords[KW_RETURN] = "return";
}

Tag
KeyWords::getTag(string name)
{
	for (int i = 0; i < 47; i++)
	{
		if (name == keywords[i])
			return (Tag)i;
	}
	return ID;
}

Token::Token(Tag t)
{
	tag = t;
}
string
Token::toString()
{
	string name;
	switch (tag)
	{
	case KW_INT:
		name = "KW_INT";
		break;
	case KW_CHAR:
		name = "KW_CHAR";
		break;
	case KW_VOID:
		name = "KW_VOID";
		break;
	case KW_IF:
		name = "KW_IF";
		break;
	case KW_ELSE:
		name = "KW_ELSE";
		break;
	case KW_SWITCH:
		name = "KW_SWITCH";
		break;
	case KW_CASE:
		name = "KW_CASE";
		break;
	case KW_DEFAULT:
		name = "KW_DEFAULT";
		break;
	case KW_WHILE:
		name = "KW_WHILE";
		break;
	case KW_DO:
		name = "KW_DO";
		break;
	case KW_FOR:
		name = "KW_FOR";
		break;
	case KW_BREAK:
		name = "KW_BREAK";
		break;
	case KW_CONTINUE:
		name = "KW_CONTINUE";
		break;
	case KW_RETURN:
		name = "KW_RETURN";
		break;
	case INC:
		name = "INC";
		break;
	case ADD:
		name = "ADD";
		break;
	case DEC:
		name = "DEC";
		break;
	case SUB:
		name = "SUB";
		break;
	case MUL:
		name = "MUL";
		break;
	case DIV:
		name = "DIV";
		break;
	case MOD:
		name = "MOD";
		break;
	case GE:
		name = "GE";
		break;
	case GT:
		name = "GT";
		break;
	case LE:
		name = "LT";
		break;
	case EQU:
		name = "EQU";
		break;
	case ASSIGN:
		name = "ASSIGN";
		break;
	case AND:
		name = "AND";
		break;
	case LEA:
		name = "LEA";
		break;
	case OR:
		name = "OR";
		break;
	case NEQU:
		name = "NEQU";
		break;
	case NOT:
		name = "NOT";
		break;
	case COMMA:
		name = "COMMA";
		break;
	case COLON:
		name = "COLON";
		break;
	case SEMICON:
		name = "SEMICON";
		break;
	case LPAREN:
		name = "LPAREN";
		break;
	case RPAREN:
		name = "RPAREN";
		break;
	case LBRACE:
		name = "LBRACE";
		break;
	case LBRACK:
		name = "LBRACK";
		break;
	case RBRACE:
		name = "RBRACE";
		break;
	case RBRACK:
		name = "RBRACK";
		break;
	case END:
		name = "END";
		break;
	case NOTE:
		name = "NOTE";
		break;
	case MULTINOTE:
		name = "MULTINOTE";
		break;
	}
	name = "<" + name + " >\n";
	return name;
}

Id::Id(string n):Token(ID)
{
	name = n;
}
string
Id::toString()
{
	return "<ID," + name + ">\n";
}

Num::Num(int v):Token(NUM)
{
	val = v;
}
string
Num::toString()
{
	return "<NUM," + std::to_string(val) + ">\n";
}

Char::Char(char c) :Token(CH)
{
	ch = c;
}
string
Char::toString()
{
	return "<CH," + string(1,ch) + ">\n";
}

Str::Str(string s) :Token(STR)
{
	str = s;
}
string
Str::toString()
{
	return "<STR," + str + ">\n";
}

Error::Error(Err e) :Token(ERR)
{
	err = e;
}
string
Error::toString()
{
	string name;
	switch (err)
	{
		
	case NUM_HEX_ERROR:
		name = "NUM_HEX_ERROR";
		break;
	case CHAR_NO_R_QUTION:
		name = "CHAR_NO_R_QUTION";
		break;
	case CHAR_NO_DATA:
		name = "CHAR_NO_DATA";
		break;
	case STR_NO_R_QUTION:
		name = "STR_NO_R_QUTION";
		break;
	case OR_NO_PAIR:
		name = "OR_NO_PAIR";
		break;
	case TOKEN_NO_EXIST:
		name = "TOKEN_NO_EXIST";
		break;
	case COMMENT_NO_END:
		name = "COMMENT_NO_END";
		break;
	}
	return "<ERR," + name + ">\n";
}
