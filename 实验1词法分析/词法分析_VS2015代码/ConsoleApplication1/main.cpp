// ConsoleApplication1.cpp : 定义控制台应用程序的入口点。
#pragma  warning (disable:4996)
#include "tokenClass.h"
#include "flags.h"
#include "scanner.h"
#include <iostream>
#include <vector>
using namespace std;

vector<string> after;
FILE *file;
int tokenNoExistFlag = 0;
//识别标识符
void idOrKeyword()
{
	char ch = scan(file);
	if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_')
	{
		Token* t;
		string name = "";
		do {
			name.push_back(ch);
			ch = scan(file);
		} while ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
			ch == '_' || (ch >= '0' && ch <= '9'));

		Tag tag = (new KeyWords())->getTag(name);
		if (tag == ID)
		{
			t = new Id(name);
		}
		else
		{
			t = new Token(tag);
		}
		after.push_back(t->toString());
		tokenNoExistFlag = 0;
	}
	else
		tokenNoExistFlag++;
	//结束之后回退一个位置
	backOne(file);
}

//识别常量——数字
void num()
{
	char ch = scan(file);
	if (ch >= '0' && ch <= '9')
	{
		Token *t = NULL;
		int val = 0;
		if (ch != '0')
		{
			do {
				val = (val << 3) + (val << 1) + ch - '0';
				ch = scan(file);
			} while (ch >= '0' && ch <= '9');
		}
		else
		{
			ch = scan(file);
			if (ch == 'x' || ch == 'X')//十六进制
			{
				ch = scan(file);
				if ((ch >= '0' && ch <= '9') ||
					(ch >= 'a' && ch <= 'f') ||
					(ch >= 'A' && ch <= 'F'))
				{
					do {
						val = (val << 4) + ch;
						if (ch >= '0' && ch <= '9')
							val -= '0';
						else if (ch >= 'a' && ch <= 'f')
							val -= 'a' + 10;
						else
							val -= 'A' + 10;
						ch = scan(file);
					} while ((ch >= '0' && ch <= '9') ||
						(ch >= 'a' && ch <= 'f') ||
						(ch >= 'A' && ch <= 'F'));
				}
				else
				{
					//出现错误——0x后无数据
					t = new Error(NUM_HEX_ERROR);
				}
			}
			else if (ch >= '0' && ch <= '7')//八进制
			{
				do {
					val = (val << 3) + ch - '0';
					ch = scan(file);
				} while (ch >= '0' && ch <= '7');
			}
		}
		if (!t)
			t = new Num(val);
		after.push_back(t->toString());
		tokenNoExistFlag = 0;
	}
	else
		tokenNoExistFlag++;
	//结束之后回退一个位置
	backOne(file);
}

//识别常量——字符
void ch()
{
	char ch = scan(file);
	if (ch == '\'')
	{
		char c;
		Token *t = NULL;
		ch = scan(file);
		if (ch == '\\')//转义字符
		{
			ch = scan(file);
			if (ch == 'n')
				c = '\n';
			else if (ch == 't')
				c = '\t';
			else if (ch == '\\')
				c = '\\';
			else if (ch == '0')
				c = '\0';
			else if (ch == '\'')
				c = '\'';
			else if (ch == -1 || ch == '\n')
			{
				//出现错误——文件结束或者换行
				t = new Error(CHAR_NO_R_QUTION);
			}
			else
				c = ch;
		}
		else if (ch == -1 || ch == '\n')
		{
			//出现错误——文件结束或者换行
			t = new Error(CHAR_NO_R_QUTION);
		}
		else if (ch == '\'')
		{
			//出现错误——无数据
			t = new Error(CHAR_NO_DATA);
		}
		else
			c = ch;
		if (!t)
		{
			ch = scan(file);
			if (ch == '\'')
			{
				t = new Char(c);
			}
			else
			{
				t = new Error(CHAR_NO_R_QUTION);
				backOne(file);//读错了，回退一步
			}
		}
		after.push_back(t->toString());
		tokenNoExistFlag = 0;
	}
	else
	{
		backOne(file);//读错了，回退一步
		tokenNoExistFlag++;
	}
}

//识别常量——字符串
void str()
{
	char ch = scan(file);
	if (ch == '"')
	{
		Token *t=NULL;
		string s = "";
		ch = scan(file);
		while (ch != '"')
		{
			char c;
			if (ch == '\\')//转义字符
			{
				ch = scan(file);
				if (ch == 'n')
					c = '\n';
				else if (ch == 't')
					c = '\t';
				else if (ch == '\\')
					c = '\\';
				else if (ch == '0')
					c = '\0';
				else if (ch == '"')
					c = '"';
				else if (ch == '\'')
					c = '\'';
				else if (ch == -1)
				{
					//出现错误——文件结束
					t = new Error(STR_NO_R_QUTION);
					break;
				}
				else
					c = ch;
				s.push_back(c);
			}
			else if (ch == -1)
			{
				//出现错误——文件结束
				t = new Error(STR_NO_R_QUTION);
				break;
			}
			else
				s.push_back(ch);
			ch = scan(file);
		}
		if (!t)
			t = new Str(s);
		after.push_back(t->toString());
		tokenNoExistFlag = 0;
	}
	else
	{
		backOne(file);//回退一步
		tokenNoExistFlag++;
	}
}

//最后判断——界符
void sign()
{
	char ch = scan(file);
	Token *t = NULL;
	switch (ch)
	{
	case '+':
		t = new Token(scan(file, '+') ? INC : ADD);
		break;
	case '-':
		t = new Token(scan(file, '-') ? DEC : SUB);
		break;
	case '*':
		t = new Token(MUL);
		break;
	case '/':								//需要考虑注释
		ch = scan(file);
		if (ch == '/')
		{
			ch = scan(file);
			//进入单行注释
			while (ch != '\n' && ch != -1)
				ch = scan(file);
			t = new Token(NOTE);
		}
		else if (ch == '*')
		{
			ch = scan(file);
			//进入多行注释
			while (ch != -1)
			{
				if (ch == '*')
				{
					ch = scan(file);
					while (ch == '*')
						ch = scan(file);
					if (ch == '/')
					{
						t = new Token(MULTINOTE);
						break;
					}
				}
				ch = scan(file);
			}
			if (!t && ch == -1)
			{
				t = new Error(COMMENT_NO_END);
			}
		}
		else
		{
			t = new Token(DIV);
			backOne(file);
		}
		break;
	case '%':
		t = new Token(MOD);
		break;
	case '>':
		t = new Token(scan(file, '=') ? GE : GT);
		break;
	case '<':
		t = new Token(scan(file, '=') ? LE : LT);
		break;
	case '=':
		t = new Token(scan(file, '=') ? EQU : ASSIGN);
		break;
	case '&':
		t = new Token(scan(file, '&') ? AND : LEA);
		break;
	case '|':
		if (!scan(file, '|'))
		{
			t = new Error(OR_NO_PAIR);
		}
		else
			t = new Token(OR);
		break;
	case '!':
		t = new Token(scan(file, '=') ? NEQU : NOT);
		break;
	case ',':
		t = new Token(COMMA);
		break;
	case ':':
		t = new Token(COLON);
		break;
	case ';':
		t = new Token(SEMICON);
		break;
	case '(':
		t = new Token(LPAREN);
		break;
	case ')':
		t = new Token(RPAREN);
		break;
	case '[':
		t = new Token(LBRACK);
		break;
	case ']':
		t = new Token(RBRACK);
		break;
	case '{':
		t = new Token(LBRACE);
		break;
	case '}':
		t = new Token(RBRACE);
		break;
	case -1:
		t = new Token(END);
		break;
	default:
		if (tokenNoExistFlag < 5)
			backOne(file);//不属于界符，会退一步
		else
			t = new Error(TOKEN_NO_EXIST);
		break;
	}
	if (t)
	{
		after.push_back(t->toString());
		tokenNoExistFlag=0;
	}
		

	//else
		//backOne(file);//不属于界符，会退一步
}

//无效词法符号
void spaceTabNewline()
{
	char ch = scan(file);
	while (ch == ' ' || ch == '\n' || ch == '\t')
	{
		tokenNoExistFlag = 0;
		ch = scan(file);
	}
	//结束之后回退一个位置
	backOne(file);
}

int main()
{
	string filename;
	cin >> filename;
	file = fopen(filename.c_str(), "r");
	if (!file)
		cout << "打开文件错误" << endl;
	while (true)
	{
		idOrKeyword();
		num();
		ch();
		str();
		spaceTabNewline();

		sign();
		char ch = scan(file);
		if (ch == -1)
			break;
		backOne(file);
	}
	int len = after.size();
	for (int i = 0; i < len; i++)
	{
		cout << after[i];
	}
	getchar();
	getchar();
	getchar();
    return 0;
}
