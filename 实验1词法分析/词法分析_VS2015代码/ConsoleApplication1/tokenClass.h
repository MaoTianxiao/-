#pragma once
#include "flags.h"
#include <string>
using std::string;
/*
�ʷ��Ǻ���
*/
class Token
{
public:
	Tag tag;
	Token(Tag t);
	virtual string toString();
	virtual ~Token() {};
};

/*��ʶ���Ǻ���*/
class Id :public Token
{
public:
	string name;
	Id(string n);
	virtual string toString();
};
/*���ּǺ���*/
class Num :public Token
{
public:
	int val;
	Num(int v);
	virtual string toString();
};
/*�ַ��Ǻ���*/
class Char :public Token
{
public:
	char ch;
	Char(char c);
	virtual string toString();
};
/*�ַ����Ǻ���*/
class Str :public Token
{
public:
	string str;
	Str(string s);
	virtual string toString();
};
/*�ؼ��ֱ���*/
class KeyWords
{
public:
	string keywords[47];
	KeyWords();
	void giveValue();
	Tag getTag(string name);
};

class Error : public Token
{
public:
	Err err;
	Error(Err e);
	virtual string toString();
};