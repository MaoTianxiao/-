#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>

int yyerror(const char *msg);

using namespace std;

#define HASH_TABLE_SIZE 100
#define NUM_TABLES 10

int table_index = 0;
int current_scope = 0;

/* 符号表条目结构体 */
struct entry_t
{
	char* lexeme; //词素
	int value;
	int data_type; //使用int类型是因为在bison文件中定义了这些类型的token，bison会自动为这些token分配编号
	int* parameter_list; // for functions
	int array_dimension;
	int is_constant;
	int num_params;
	struct entry_t* successor;  //使用拉链法解决哈希冲突时用到
};

/* 符号表结构体，包含父亲表的编号 */
struct table_t
{
	entry_t** symbol_table;
	int parent;
};

struct content_t
{
	vector<int> truelist;
	vector<int> falselist;
	vector<int> nextlist;
	vector<int> breaklist;
	vector<int> continuelist;
	string addr;
	string code;

	entry_t* entry;
	int data_type;
};

extern table_t symbol_table_list[NUM_TABLES]; //声明一个包含10个符号表的符号表数组外部变量symbol_table_list

void display_symbol_table(entry_t** hash_table_ptr);
/* 创建新符号表，哈希表 */
entry_t** create_table()
{
	entry_t** hash_table_ptr = NULL; // declare a pointer

	/* 分配空间， HASH_TABLE_SIZE */
	if( ( hash_table_ptr = (entry_t**) malloc( sizeof( entry_t* ) * HASH_TABLE_SIZE ) ) == NULL )
    	return NULL;

	int i;

	// 所有表项置空
    for( i = 0; i < HASH_TABLE_SIZE; i++ )
	{
		hash_table_ptr[i] = NULL;
	}

	return hash_table_ptr;
}
/*创建一个新的作用域*/
int create_new_scope()
{
	table_index++;
	// printf("Table index:%d\n",table_index);
	symbol_table_list[table_index].symbol_table = create_table(); //创建一个新符号表
	symbol_table_list[table_index].parent = current_scope; //连接到父亲上面

	return table_index;
}
/*退出作用域*/
int exit_scope() 
{
	return symbol_table_list[current_scope].parent;
}
/* 生成哈希值，index-[0, HASH_TABLE_SIZE) */
int myhash( char *lexeme )
{
	size_t i;
	int hashvalue;

	/* Apply jenkin's myhash function
	* https://en.wikipedia.org/wiki/Jenkins_hash_function#one-at-a-time
	*/

	for ( hashvalue = i = 0; i < strlen(lexeme); ++i ) {
        hashvalue += lexeme[i];
        hashvalue += ( hashvalue << 10 );
        hashvalue ^= ( hashvalue >> 6 );
    }
	hashvalue += ( hashvalue << 3 );
	hashvalue ^= ( hashvalue >> 11 );
    hashvalue += ( hashvalue << 15 );
	//cout<<(hashvalue% HASH_TABLE_SIZE + HASH_TABLE_SIZE) % HASH_TABLE_SIZE<<endl;
	return (hashvalue% HASH_TABLE_SIZE + HASH_TABLE_SIZE) % HASH_TABLE_SIZE; // return an index in [0, HASH_TABLE_SIZE)
}

/* 创建符号表条目，insert函数会调用 */
entry_t *create_entry( char *lexeme, int value, int data_type )
{
	entry_t *new_entry;

	/* 分配空间*/
	if( ( new_entry = (entry_t*) malloc( sizeof( entry_t ) ) ) == NULL ) {
		return NULL;
	}
	/* 将lexme复制到新条目的lexeme */
	if( ( new_entry->lexeme = strdup( lexeme ) ) == NULL ) {
		return NULL;
	}

	new_entry->value = value;
	new_entry->successor = NULL;
	new_entry->parameter_list = NULL;
	new_entry->array_dimension = -1;
	new_entry->is_constant = 0;
	new_entry->num_params = 0;
	new_entry->data_type = data_type;

	return new_entry;
}

/* 查表，返回条目指针，未找到返回NULL */
entry_t* search(entry_t** hash_table_ptr, char* lexeme)
{
	int idx = 0;
	entry_t* myentry;

    // 获取哈希值
	idx = myhash( lexeme );

	/* 在该位置的链表上进行查找 */
	myentry = hash_table_ptr[idx];

	while( myentry != NULL && strcmp( lexeme, myentry->lexeme ) != 0 )
	{
		myentry = myentry->successor;
	}

	if(myentry == NULL) // 未找到
		return NULL;

	else // 找到
		return myentry;

}

// 递归在父表中查询
entry_t* search_recursive(char* lexeme)
{
	int idx = current_scope;
	entry_t* finder = NULL;

	while(idx != -1)
	{
		finder = search(symbol_table_list[idx].symbol_table, lexeme);

		if(finder != NULL)
			return finder;

		idx = symbol_table_list[idx].parent;
	}

	return finder;
}
/* 插入一个表项 */
entry_t* insert( entry_t** hash_table_ptr, char* lexeme, int value, int data_type)
{
	// hash_table_ptr是当前作用域的哈希表指针
	entry_t* finder = search( hash_table_ptr, lexeme );
	if( finder != NULL) // 如果找到了，就不再插入
	{
		if(finder->is_constant) //当试图添加的符号表表项是一个常量且已经存在时，不再添加，直接返回原来的常量的表项指针
			return finder;
		return NULL; //非常量，说明试图重定义一个符号，返回NULL，在parse.y中会处理，输出一个错误提示
	}

	int idx;
	entry_t* new_entry = NULL;
	entry_t* head = NULL;

	idx = myhash( lexeme ); // 获取哈希值
	new_entry = create_entry( lexeme, value, data_type ); // 创建条目

	if(new_entry == NULL) 
	{
		printf("Insert failed. New entry could not be created.");
		exit(1);
	}

	head = hash_table_ptr[idx];

	if(head == NULL) // 没有冲突，直接插入
	{
		hash_table_ptr[idx] = new_entry;
	}
	else // 有冲突，拉链法解决冲突，将新条目指针插入链表表头
	{
		new_entry->successor = hash_table_ptr[idx];
		hash_table_ptr[idx] = new_entry;
	}

	return hash_table_ptr[idx];
}

// 进行函数调用时，检查参数列表（类型表）是否匹配
int check_parameter_list(entry_t* entry, int* list, int m)
{
	int* parameter_list = entry->parameter_list;

	if(m != entry->num_params)
	{
		yyerror("Number of parameters and arguments do not match");
	}

	int i;
	for(i=0; i<m; i++)
	{
		if(list[i] != parameter_list[i])
		yyerror("Parameter and argument types do not match");
	}

	return 1;
}

void fill_parameter_list(entry_t* entry, int* list, int n)
{
	entry->parameter_list = (int *)malloc(n*sizeof(int));

	int i;
	for(i=0; i<n; i++)
	{
		entry->parameter_list[i] = list[i];
	}
	entry->num_params = n;
}

/*以下函数均为按照格式输出符号表*/

void print_dashes(int n)
{
  printf("\n");

	int i;
	for(i=0; i< n; i++)
	printf("=");
	printf("\n");
}

void display_symbol_table(entry_t** hash_table_ptr)
{
	int i;
	entry_t* traverser;

	print_dashes(100);

  printf(" %-20s %-20s %-20s %-20s %-20s\n","lexeme","data-type","array_dimension","num_params","param_list");

	print_dashes(100);

	for( i=0; i < HASH_TABLE_SIZE; i++)
	{
		traverser = hash_table_ptr[i];
		while( traverser != NULL)
		{
			printf(" %-20s %-20d %-20d ", traverser->lexeme, traverser->data_type, traverser->array_dimension);

			printf(" %-20d", traverser->num_params);

			int j;
			for(j=0; j < traverser->num_params; j++)
			printf(" %d",traverser->parameter_list[j]);
			printf("\n");

			traverser = traverser->successor;
		}
	}

	print_dashes(100);

}

void display_constant_table(entry_t** hash_table_ptr)
{
	int i;
	entry_t* traverser;

	print_dashes(25);

	printf(" %-10s %-10s \n","lexeme","data-type");

	print_dashes(25);

	for( i=0; i < HASH_TABLE_SIZE; i++)
	{
		traverser = hash_table_ptr[i];
		while( traverser != NULL)
		{
			printf(" %-10s %-10d \n", traverser->lexeme, traverser->data_type);
			traverser = traverser->successor;
		}
	}

	print_dashes(25);
}

void display_all()
{
		int i;
		for(i=0; i<=table_index; i++)
		{
			printf("Scope: %d\n",i);
			display_symbol_table(symbol_table_list[i].symbol_table);
			printf("\n\n");
		}
}
