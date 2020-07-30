#pragma once
#include "flags.h"
#include <string>
using std::string;
/*
词法记号类
*/
class Token
{
public:
	Tag tag;
	Token(Tag t);
	virtual string toString();
	virtual ~Token() {};
};

/*标识符记号类*/
class Id :public Token
{
public:
	string name;
	Id(string n);
	virtual string toString();
};
/*数字记号类*/
class Num :public Token
{
public:
	int val;
	Num(int v);
	virtual string toString();
};
/*字符记号类*/
class Char :public Token
{
public:
	char ch;
	Char(char c);
	virtual string toString();
};
/*字符串记号类*/
class Str :public Token
{
public:
	string str;
	Str(string s);
	virtual string toString();
};
/*关键字表类*/
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