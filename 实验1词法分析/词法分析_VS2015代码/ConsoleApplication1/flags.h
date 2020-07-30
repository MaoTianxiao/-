#pragma once



enum Tag
{
	ERR,									//�����쳣
	END,									//�ļ��������
	ID,										//��ʶ��
	KW_INT,	KW_CHAR, KW_VOID,				//��������
	NUM,CH,STR,								//����
	NOT,LEA,								//��Ŀ����� �� & - *
	ADD,SUB,MUL,DIV,MOD,					//���������
	INC,DEC,								//�Լ��Լ�
	GT,GE,LT,LE,EQU,NEQU,					//�Ƚ������
	AND,OR,									//�߼������
	LPAREN,RPAREN,							//()
	LBRACK,RBRACK,							//[]
	LBRACE,RBRACE,							//{}
	COMMA,COLON,SEMICON,					//���ţ�ð�ţ��ֺ�
	ASSIGN,									//��ֵ
	KW_IF,KW_ELSE,							//if-else
	KW_SWITCH,KW_CASE,KW_DEFAULT,			//switch-case-default
	KW_WHILE,KW_DO,KW_FOR,					//ѭ��
	KW_BREAK,KW_CONTINUE,KW_RETURN,			//break,continue,return
	NOTE,MULTINOTE,							//ע�ͣ�����ע��

};

enum Err
{
	NUM_HEX_ERROR,							//ʮ������
	CHAR_NO_R_QUTION,						//�ַ�ת�����в���
	CHAR_NO_DATA,							//�ַ�������
	STR_NO_R_QUTION,						//�ַ�����������
	OR_NO_PAIR,								//|û��һ��
	TOKEN_NO_EXIST,							//����Ĵ˷�����
	COMMENT_NO_END,							//����ע���޽���
};