#include "scanner.h"
#include <iostream>
#pragma  warning (disable:4996) 
/*
 * function name: scan
 * Author: Mtx
 * Time: 2020年6月7日20点26分
 * parameter: file
 * return type: char
 * 功能： 依次读取文件中的字符，每一次调用返回一个
 * 因为文件指针会自动移动，所以依次读取（未采取缓冲区）
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