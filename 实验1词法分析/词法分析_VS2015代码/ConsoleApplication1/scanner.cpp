#include "scanner.h"
#include <iostream>
#pragma  warning (disable:4996) 
/*
 * function name: scan
 * Author: Mtx
 * Time: 2020��6��7��20��26��
 * parameter: file
 * return type: char
 * ���ܣ� ���ζ�ȡ�ļ��е��ַ���ÿһ�ε��÷���һ��
 * ��Ϊ�ļ�ָ����Զ��ƶ����������ζ�ȡ��δ��ȡ��������
 */
int count = 0;
char scan(FILE *file)
{
	char ch;
	fseek(file, count, SEEK_SET);
	if ((fscanf(file, "%c", &ch)) == EOF)
		ch = -1;
	count++;
	return ch;
}

void backOne(FILE *file)
{
	count--;
	fseek(file, count, SEEK_SET);
}

bool scan(FILE* file, char ch)
{
	if (ch != scan(file))
	{
		backOne(file);
		return false;
	}
	return true;
}