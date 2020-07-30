%{
	#include <bits/stdc++.h>
	#include "symboltable.h"
	#include "lex.yy.c"

	using namespace std;

	int yyerror(char *msg);

	#define SYMBOL_TABLE symbol_table_list[current_scope].symbol_table

  	extern entry_t** constant_table;

	int current_dtype; /*当前识别到的数据类型*/

	table_t symbol_table_list[NUM_TABLES]; /*在symboltable.h中声明的外部变量的定义*/

	int is_declaration = 0;
	int is_loop = 0;
	int is_func = 0;
	int func_type;

	int param_list[10];  /*参数列表*/
	int p_idx = 0;  /*param_list中当前实际用到的参数个数*/
	int p=0;   /*用于标记是否进入了一个新的作用域，需要开启新的符号表*/
  	int rhs = 0; /*右值*/

	/*函数声明*/
	void type_check(int,int,int);  /*类型检查*/
	vector<int> merge(vector<int>& v1, vector<int>& v2);  /*合并两个链（回填时用到）*/
	void backpatch(vector<int>&, int);  /*回填*/
	void gencode(string); /*根据传进来的字符串生成中间代码*/
	void gencode_math(content_t* & lhs, content_t* arg1, content_t* arg2, const string& op);
	void gencode_rel(content_t* & lhs, content_t* arg1, content_t* arg2, const string& op);

	int nextinstr = 0;  /*中间代码中，下一条指令的指令号*/
	int temp_var_number = 0;   /*临时变量的数量，即三地址代码中的t0、t1...*/

	vector<string> ICG;  /*记录生成的中间代码*/

%}

%union
{
	int data_type;
	entry_t* entry;
	content_t* content;
	string* op;  /*操作符*/
	vector<int>* nextlist;
	int instr;   /*指令号*/
}

%token <entry> IDENTIFIER

 /* 常量 */
%token <entry> DEC_CONSTANT HEX_CONSTANT CHAR_CONSTANT FLOAT_CONSTANT STRING

 /* 逻辑运算符和关系运算符 */
%token LOGICAL_AND LOGICAL_OR LS_EQ GR_EQ EQ NOT_EQ

 /* 赋值运算符 */
%token MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN SUB_ASSIGN
%token INCREMENT DECREMENT

 /* 数据类型 */
%token SHORT INT LONG LONG_LONG SIGNED UNSIGNED CONST VOID CHAR FLOAT CHAR_STAR

 /* 关键字 */
%token IF FOR WHILE CONTINUE BREAK RETURN

%type <entry> identifier
%type <entry> constant
%type <entry> array_index

%type <op> assign;
%type <data_type> function_call  /*函数调用*/

%type <content> lhs
%type <content> sub_expr
%type <content> expression
%type <content> expression_stmt
%type <content> unary_expr  /*一元表达式，a++,a--,++a,--a*/
%type <content> arithmetic_expr
%type <content> assignment_expr
%type <content> array_access   /*获取数组元素*/

%type <content> if_block
%type <content> for_block
%type <content> while_block
%type <content> compound_stmt  /*复合语句，{...}*/

%type <content> statements
%type <content> single_stmt
%type <content> stmt


%type <instr> M   /*回填技术中记录要回填的行号*/
%type <content> N  /*回填技术中处理nextlist*/

/*定义结合性和优先级*/
%left ','
%right '='
%left LOGICAL_OR
%left LOGICAL_AND
%left EQ NOT_EQ
%left '<' '>' LS_EQ GR_EQ
%left '+' '-'
%left '*' '/' '%'
%right '!'

/*没有结合性*/
%nonassoc UMINUS
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE   


%%

 /* 程序由多个builder组成 */
starter: starter builder
			 | builder;

 /* 每个builder是一个函数或一个声明 */
builder: function
			 | declaration
			 ;

 /* 函数 */
function: type identifier 	
			{
				func_type = current_dtype;
				is_declaration = 0;  /*只有前面有type的identifier才是声明*/
				current_scope = create_new_scope();  /*函数进入了新的作用域，要创建新的符号表*/
				gencode($2->lexeme + string(":"));
			}

		 '(' argument_list ')' 	
		 	{
				is_declaration = 0;
				fill_parameter_list($2,param_list,p_idx);
				p_idx = 0;  /*记录param_list中参数数量的变量置0*/
				is_func = 1;
				p=1;   /*标记进入了新的作用域*/
			}

		 compound_stmt	{   is_func = 0;  }
          
		;
 
 /* 类型 */
type : data_type pointer    {is_declaration = 1; }
     | data_type		    {is_declaration = 1; }
		 ;

pointer: '*' pointer
    	| '*'
       ;

data_type : sign_specifier type_specifier
    	| type_specifier
    	;

sign_specifier : SIGNED
    		| UNSIGNED
    		;

type_specifier :INT                    {current_dtype = INT;}
    |SHORT INT                         {current_dtype = SHORT;}
    |SHORT                             {current_dtype = SHORT;}
    |LONG                              {current_dtype = LONG;}
	|LONG INT                          {current_dtype = LONG;}
    |LONG_LONG                         {current_dtype = LONG_LONG;}
    |LONG_LONG INT                     {current_dtype = LONG_LONG;}
	|CHAR 							   {current_dtype = CHAR;}
	|FLOAT 							   {current_dtype = FLOAT;}
	|VOID							   {current_dtype = VOID;}
	|CHAR_STAR					 	   {current_dtype = STRING;}
    ;

 /* 参数列表 */
 /* 参数列表可以为空 */
argument_list : arguments
    	|
    	;
 /* 参数由多个type-id对组成，以逗号分隔 */
arguments : arguments ',' arg
    	| arg
    	;

 /* 每个参数是一个type-id对 */
arg : type identifier	{
							param_list[p_idx++] = $2->data_type;
							gencode(string("arg ") + $2->lexeme);
						}
    ;

 /* 语句，可以是复合语句或单条语句 */
stmt:compound_stmt		{$$ = new content_t(); $$=$1;}
    |single_stmt		{$$ = new content_t(); $$=$1;}
    ;

 /* 复合语句是由花括号包裹的多条语句 */
compound_stmt :
				'{' 	
				
				{
				/*如果前面已经创建了新符号表，就不再创建*/
					if(!p)current_scope = create_new_scope(); 
					else p = 0;
				}
				
				statements
				
				'}' 
				
				{
					current_scope = exit_scope();  /*离开作用域，退回到父符号表*/
					$$ = new content_t();
					$$ = $3;
				}
    ;

statements:statements M stmt	{
									backpatch($1->nextlist,$2);
									$$ = new content_t();
									$$->nextlist = $3->nextlist;
									$$->breaklist = merge($1->breaklist,$3->breaklist);
									$$->continuelist = merge($1->continuelist,$3->continuelist);
								}

    |							{	$$ = new content_t();	}
    ;

 /* 单条语句 */
single_stmt :if_block	{
							$$ = new content_t();
							$$ = $1;
							backpatch($$->nextlist, nextinstr);
						}

		    |for_block	{
							$$ = new content_t();
							$$ = $1;
							backpatch($$->nextlist, nextinstr);
						}
		
	    	|while_block {
							$$ = new content_t();
							$$ = $1;
							backpatch($$->nextlist, nextinstr);
						 }
	    	|declaration 		{$$ = new content_t();}
	    	|function_call ';'	{$$ = new content_t();}
			|RETURN ';'	  {
								if(is_func)
								{
									if(func_type != VOID)
										yyerror("return type (VOID) does not match function type");
								}
							  	else yyerror("return statement not inside function definition");
							}
	
			|CONTINUE ';'	{
								if(!is_loop)
									yyerror("Illegal use of continue");
								$$ = new content_t();
								$$->continuelist = {nextinstr};
								gencode("goto _");
							}
	
			|BREAK ';'      {
								if(!is_loop) {yyerror("Illegal use of break");}
								$$ = new content_t();
								$$->breaklist = {nextinstr};
								gencode("goto _");
						    }
	
			|RETURN sub_expr ';' 
							{
								if(is_func)
								{
									if(func_type != $2->data_type)
										yyerror("return type does not match function type");
								}
								else yyerror("return statement not in function definition");
							}
	    ;

for_block: FOR '(' expression_stmt M expression_stmt M expression ')' {is_loop = 1;} N M stmt {is_loop = 0;}
	         {
				backpatch($5->truelist,$11);
				backpatch($12->nextlist,$6);
				backpatch($12->continuelist, $6);
				backpatch($10->nextlist, $4);
				$$ = new content_t();
				$$->nextlist = merge($5->falselist,$12->breaklist);
				gencode(string("goto ") + to_string($6));
			 }

    		 ;

if_block:IF '(' expression ')' M stmt 	%prec LOWER_THAN_ELSE  
	 		{
			 /*本条规则具有与LOWER_THAN_ELSE相同的优先级，单if语句比else优先级低，保证了if-if-else结构中else和内层if配对*/
				backpatch($3->truelist,$5);
				$$ = new content_t();
				$$->nextlist = merge($3->falselist,$6->nextlist);
				$$->breaklist = $6->breaklist;
				$$->continuelist = $6->continuelist;
			}

		|IF '(' expression ')' M stmt  ELSE N M stmt
			{
				backpatch($3->truelist,$5);
				backpatch($3->falselist,$9);

				$$ = new content_t();
				vector<int> temp = merge($6->nextlist,$8->nextlist);
				$$->nextlist = merge(temp,$10->nextlist);
				$$->breaklist = merge($10->breaklist,$6->breaklist);
				$$->continuelist = merge($10->continuelist,$6->continuelist);
			}
    ;

while_block: WHILE M '(' expression	')' M {is_loop = 1;} stmt {is_loop = 0;}
			{
				backpatch($8->nextlist,$2);
				backpatch($4->truelist,$6);
				backpatch($8->continuelist, $2);
				$$ = new content_t();
				$$->nextlist = merge($4->falselist,$8->breaklist);
				gencode(string("goto ") + to_string($2));
			}
		;

/*声明*/
declaration: type  declaration_list ';'			{is_declaration = 0;  /*声明结束之后置这个标记为0*/}
			 | declaration_list ';'
			 | unary_expr ';'


declaration_list: declaration_list ',' sub_decl
					|sub_decl
					;

sub_decl: assignment_expr
    		|identifier
    		|array_access
			;

/* 表达式语句，在循环语句中可能会有空表达式 */
expression_stmt: expression ';'	 
					{
						$$ = new content_t(); 
						$$->truelist = $1->truelist; 
						$$->falselist = $1->falselist;
					}
    			
				| ';'	{	$$ = new content_t();	}
    			;

expression: expression ',' sub_expr
				{
					$$ = new content_t();
					$$->truelist = $3->truelist; 
					$$->falselist = $3->falselist;
				}
    		| sub_expr	
				{
					$$ = new content_t(); 
					$$->truelist = $1->truelist; 
					$$->falselist = $1->falselist;
				}
			;

sub_expr:

		sub_expr '>' sub_expr	
			{
				type_check($1->data_type,$3->data_type,2);
				$$ = new content_t();
				gencode_rel($$, $1, $3, string(" > "));  
			}
		| sub_expr '<' sub_expr
			{
				type_check($1->data_type,$3->data_type,2);
				$$ = new content_t();
				gencode_rel($$, $1, $3, string(" < "));
			}

		| sub_expr EQ sub_expr
			{
				type_check($1->data_type,$3->data_type,2);
				$$ = new content_t();
				gencode_rel($$, $1, $3, string(" == "));
			}

		| sub_expr NOT_EQ sub_expr
			{
				type_check($1->data_type,$3->data_type,2);
				$$ = new content_t();
				gencode_rel($$, $1, $3, string(" != "));
			}

		| sub_expr GR_EQ sub_expr
			{
				type_check($1->data_type,$3->data_type,2);
				$$ = new content_t();
				gencode_rel($$, $1, $3, string(" >= "));
			}

		| sub_expr LS_EQ sub_expr
			{
				type_check($1->data_type,$3->data_type,2);
				$$ = new content_t();
				gencode_rel($$, $1, $3, string(" <= "));
			}

		|sub_expr LOGICAL_AND M sub_expr
			{
				type_check($1->data_type,$4->data_type,2);
				$$ = new content_t();
				$$->data_type = $1->data_type;
				backpatch($1->truelist,$3);
				$$->truelist = $4->truelist;
				$$->falselist = merge($1->falselist,$4->falselist);
			}

		|sub_expr LOGICAL_OR M sub_expr
			{
				type_check($1->data_type,$4->data_type,2);
				$$ = new content_t();
				$$->data_type = $1->data_type;
				backpatch($1->falselist,$3);
				$$->truelist = merge($1->truelist,$4->truelist);
				$$->falselist = $4->falselist;
			}

		|'!' sub_expr
			{
				$$ = new content_t();
				$$->data_type = $2->data_type;
				$$->truelist = $2->falselist;
				$$->falselist = $2->truelist;
			}

		|arithmetic_expr
			{
				$$ = new content_t(); 
				$$->data_type = $1->data_type; 
				$$->addr = $1->addr;
			}
    	|assignment_expr
			{
				$$ = new content_t(); 
				$$->data_type = $1->data_type;
			}
		|unary_expr	
			{
				$$ = new content_t(); 
				$$->data_type = $1->data_type;
			}
    ;

/*赋值表达式，左边是一个左值，右边可以是算术表达式，数组元素，函数调用或一元表达式*/
assignment_expr :
	lhs assign arithmetic_expr	
			{
				type_check($1->entry->data_type,$3->data_type,1);
		 		$$ = new content_t();
				$$->data_type = $3->data_type;
		 		$$->code = $1->entry->lexeme + *$2 + $3->addr;
				gencode($$->code);
		 		rhs = 0;
			}

    |lhs assign array_access
			{
				type_check($1->entry->data_type,$3->data_type,1);
	 			$$ = new content_t();
				$$->data_type = $3->data_type;
	 			$$->code = $1->entry->lexeme + *$2 + $3->code;
				gencode($$->code);
	 			rhs = 0;
			}

    |lhs assign function_call
			{
				type_check($1->entry->data_type,$3,1); 
				$$ = new content_t(); 
				$$->data_type = $3;
			}

	|lhs assign unary_expr  
	        {
				type_check($1->entry->data_type,$3->data_type,1);
			 	$$ = new content_t();
				$$->data_type = $3->data_type;
			 	$$->code = $1->entry->lexeme + *$2 + $3->code;
				gencode($$->code);
			 	rhs = 0;
			}
    ;

/*一元表达式*/
unary_expr:	
	identifier INCREMENT	
			{
				$$ = new content_t();
				$$->data_type = $1->data_type;
				$$->code = string($1->lexeme) + string("++");
				gencode($$->code);
			}

 	| identifier DECREMENT		
	 		{
				$$ = new content_t();
				$$->data_type = $1->data_type;
				$$->code = string($1->lexeme) + string("--");
				gencode($$->code);
			}

	| DECREMENT identifier	
			{
				$$ = new content_t();
				$$->data_type = $2->data_type;
				$$->code = string("--") + string($2->lexeme);
				gencode($$->code);
			}

	| INCREMENT identifier
			{
				$$ = new content_t();
				$$->data_type = $2->data_type;
				$$->code = string("++") + string($2->lexeme);
				gencode($$->code);
			}

lhs: identifier		{$$ = new content_t(); $$->entry = $1;}
   | array_access	{$$ = new content_t(); $$->code = $1->code;}
	 ;

identifier:IDENTIFIER
                {
                    if(is_declaration && !rhs)
                    {
                      $1 = insert(SYMBOL_TABLE,yytext,INT_MAX,current_dtype);
                      if($1 == NULL) 
					  	yyerror("Redeclaration of variable");
                    }
                    else
                    {
                      $1 = search_recursive(yytext);
                      if($1 == NULL) 
					  	yyerror("Variable not declared");
                    }
                    
					$$ = $1;
                }
    		 ;

assign:'=' 			{rhs=1; $$ = new string(" = ");}
    |ADD_ASSIGN 	{rhs=1; $$ = new string(" += ");}
    |SUB_ASSIGN 	{rhs=1; $$ = new string(" -= ");}
    |MUL_ASSIGN 	{rhs=1; $$ = new string(" *= ");}
    |DIV_ASSIGN 	{rhs=1;	$$ = new string(" /= ");}
    |MOD_ASSIGN 	{rhs=1; $$ = new string(" %= ");}
    ;

/*算术表达式*/
arithmetic_expr: arithmetic_expr '+' arithmetic_expr
					 {
						type_check($1->data_type,$3->data_type,0);
						$$ = new content_t();
						$$->data_type = $1->data_type;
						gencode_math($$, $1, $3, string(" + "));
					 }

			| arithmetic_expr '-' arithmetic_expr
			  		 {
						type_check($1->data_type,$3->data_type,0);
						$$ = new content_t();
						$$->data_type = $1->data_type;
						gencode_math($$, $1, $3, string(" - "));
					 }

			| arithmetic_expr '*' arithmetic_expr
					 {
						type_check($1->data_type,$3->data_type,0);
						$$ = new content_t();
		 				$$->data_type = $1->data_type;
						gencode_math($$, $1, $3, string(" * "));
					 }

			| arithmetic_expr '/' arithmetic_expr
					 {
						type_check($1->data_type,$3->data_type,0);
						$$ = new content_t();
						$$->data_type = $1->data_type;
						gencode_math($$, $1, $3, string(" / "));
					 }

		    | arithmetic_expr '%' arithmetic_expr
					 {
						type_check($1->data_type,$3->data_type,0);
						$$ = new content_t();
						$$->data_type = $1->data_type;
						gencode_math($$, $1, $3, string(" % "));
				 	 }

			|'(' arithmetic_expr ')'
					 {
						$$ = new content_t();
						$$->data_type = $2->data_type;
						$$->addr = $2->addr;
						$$->code = $2->code;
					 }

    		|'-' arithmetic_expr %prec UMINUS	
					 {
						$$ = new content_t();
						$$->data_type = $2->data_type;
						$$->addr = "t" + to_string(temp_var_number);
						std::string expr = $$->addr + " = " + "minus " + $2->addr;
						$$->code = $2->code + expr;
						gencode(expr);
						temp_var_number++;
				 	 }

    	    |identifier
					 {
						$$ = new content_t();
						$$->data_type = $1->data_type;
	 					$$->addr = $1->lexeme;
			   		 }

    		|constant
					 {
						$$ = new content_t();
						$$->data_type = $1->data_type;
						$$->addr = to_string($1->value);
					 }
    		 ;

constant: DEC_CONSTANT 			{$1->is_constant=1; $$ = $1;}
    	| HEX_CONSTANT			{$1->is_constant=1; $$ = $1;}
		| CHAR_CONSTANT			{$1->is_constant=1; $$ = $1;}
		| FLOAT_CONSTANT		{$1->is_constant=1; $$ = $1;}
    ;

array_access: identifier '[' array_index ']'					
				{
					if(is_declaration)
					{
						if($3->value <= 0)
							yyerror("size of array is not positive");
						else if($3->is_constant)
							$1->array_dimension = $3->value;
					}
					else if($3->is_constant)
					{
						if($3->value > $1->array_dimension)
							yyerror("Array index out of bound");
						if($3->value < 0)
							yyerror("Array index cannot be negative");
					}
					
					$$ = new content_t();
					$$->data_type = $1->data_type;
					
					if($3->is_constant)
						$$->code = string($1->lexeme) + string("[") + to_string($3->value) + string("]");
					else
						$$->code = string($1->lexeme) + string("[") + string($3->lexeme) + string("]");
					$$->entry = $1;
				}

array_index: constant		{$$ = $1;}
		   | identifier		{$$ = $1;}
					 ;

function_call: identifier '(' parameter_list ')'
				{
					$$ = $1->data_type;
					check_parameter_list($1,param_list,p_idx);
					p_idx = 0;
					gencode(string("call ") + $1->lexeme);
				}

             | identifier '(' ')'	
			 	{
					$$ = $1->data_type;
				 	check_parameter_list($1,param_list,p_idx);
				 	p_idx = 0;
	 				gencode(string("call ") + $1->lexeme);
				}
         ;

parameter_list:
              parameter_list ','  parameter
              |parameter
              ;

parameter: sub_expr	
				{
					param_list[p_idx++] = $1->data_type;
					gencode(string("param ") + $1->addr);
				}
		 | STRING	
		 		{
					param_list[p_idx++] = STRING;
					gencode(string("param ") + $1->lexeme);
				}
		 ;

M: 			{$$ = nextinstr;}
 ;

N:			{
				$$ = new content_t;
				$$->nextlist = {nextinstr};
				gencode("goto _");
			}
	;

%%


//生成中间代码
void gencode(string x)
{
	std::string instruction;

	instruction = to_string(nextinstr) + string(": ") + x;
	ICG.push_back(instruction);
	nextinstr++;
}

//生成一个包含关系运算符的中间代码，这里会处理回填技术的truelist和falselist
void gencode_rel(content_t* & lhs, content_t* arg1, content_t* arg2, const string& op)
{
	lhs->data_type = arg1->data_type;

	lhs->truelist = {nextinstr};
	lhs->falselist = {nextinstr + 1};

	std::string code;

	code = string("if ") + arg1->addr + op + arg2->addr + string(" goto _");
	gencode(code);

	code = string("goto _");
	gencode(code);
}

//生成一个类似t0=a+b的中间代码
void gencode_math(content_t* & lhs, content_t* arg1, content_t* arg2, const string& op)
{
	lhs->addr = "t" + to_string(temp_var_number);
	std::string expr = lhs->addr + string(" = ") + arg1->addr + op + arg2->addr;
	lhs->code = arg1->code + arg2->code + expr;

	temp_var_number++;

	gencode(expr);
}

//回填
void backpatch(vector<int>& v1, int number)
{
	for(int i = 0; i<v1.size(); i++)
	{
		string instruction = ICG[v1[i]];

		if(instruction.find("_") < instruction.size())
		{
			instruction.replace(instruction.find("_"),1,to_string(number));
			ICG[v1[i]] = instruction;
		}
	}
}

//合并两个链
vector<int> merge(vector<int>& v1, vector<int>& v2)
{
	vector<int> concat;
	concat.reserve(v1.size() + v2.size());
	concat.insert(concat.end(), v1.begin(), v1.end());
	concat.insert(concat.end(), v2.begin(), v2.end());

	return concat;
}

//类型检查
void type_check(int left, int right, int flag)
{
	if(left != right)
	{
		switch(flag)
		{
			case 0: yyerror("Type mismatch in arithmetic expression"); break;
			case 1: yyerror("Type mismatch in assignment expression"); break;
			case 2: yyerror("Type mismatch in logical expression"); break;
		}
	}
}

//输出符号表到指定的文件中
void displayICG()
{
	ofstream outfile("ICG.code");

	for(int i=0; i<ICG.size();i++)
	outfile << ICG[i] <<endl;

	outfile << nextinstr << ": exit";

	outfile.close();
}

int main(int argc, char *argv[])
{
	 int i;
	 for(i=0; i<NUM_TABLES;i++) //创建符号表数组，每个符号表的父表置-1
	 {
	  symbol_table_list[i].symbol_table = NULL;
	  symbol_table_list[i].parent = -1;
	 }

	constant_table = create_table();  //创建常量表
  symbol_table_list[0].symbol_table = create_table(); //创建符号表数组的第0项为默认当前文件的第一个符号表
	yyin = fopen(argv[1], "r");

	if(!yyparse())
	{
		printf("\nPARSING COMPLETE\n\n\n");
	}
	else
	{
			printf("\nPARSING FAILED!\n\n\n");
	}

	displayICG(); //输出中间代码
/*
	printf("SYMBOL TABLES\n\n");
	display_all();

	printf("CONSTANT TABLE");
	display_constant_table(constant_table);*/

}

int yyerror(const char *msg)
{
	printf("Line no: %d Error message: %s Token: %s\n", yylineno, msg, yytext);
	exit(0);
}
