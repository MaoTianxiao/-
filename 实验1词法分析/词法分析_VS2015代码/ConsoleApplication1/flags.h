#pragma once



enum Tag
{
	ERR,									//错误，异常
	END,									//文件结束标记
	ID,										//标识符
	KW_INT,	KW_CHAR, KW_VOID,				//数据类型
	NUM,CH,STR,								//常量
	NOT,LEA,								//单目运算符 ！ & - *
	ADD,SUB,MUL,DIV,MOD,					//算术运算符
	INC,DEC,								//自加自减
	GT,GE,LT,LE,EQU,NEQU,					//比较运算符
	AND,OR,									//逻辑运算符
	LPAREN,RPAREN,							//()
	LBRACK,RBRACK,							//[]
	LBRACE,RBRACE,							//{}
	COMMA,COLON,SEMICON,					//逗号，冒号，分号
	ASSIGN,									//赋值
	KW_IF,KW_ELSE,							//if-else
	KW_SWITCH,KW_CASE,KW_DEFAULT,			//switch-case-default
	KW_WHILE,KW_DO,KW_FOR,					//循环
	KW_BREAK,KW_CONTINUE,KW_RETURN,			//break,continue,return
	NOTE,MULTINOTE,							//注释，多行注释

};

enum Err
{
	NUM_HEX_ERROR,							//十六进制
	CHAR_NO_R_QUTION,						//字符转义无有部分
	CHAR_NO_DATA,							//字符无数据
	STR_NO_R_QUTION,						//字符串无右引号
	OR_NO_PAIR,								//|没有一对
	TOKEN_NO_EXIST,							//错误的此法符号
	COMMENT_NO_END,							//多行注释无结束
};