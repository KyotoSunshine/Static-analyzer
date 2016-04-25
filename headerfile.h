
#ifndef HEADERFILE_H_
#define HEADERFILE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define ERROR 1
#define OK 0

/*Sizes*/
#define KEY_SIZE 25
#define MAXSTRSIZE 1024

//Symbol classification

#define 	CNULL	1
#define 	CLPAREN	2
#define 	CDIGIT	3
#define 	CALPHA	4
#define 	CEOF	5
#define 	CRPAREN	6
#define 	CSTAR	7
#define 	CCOLON	8
#define 	CPLUS	9
#define 	CSEMI	10
#define 	CLSQP	11
#define 	CLBRACE	12
#define 	CCOMMA	13
#define 	CLESS	14
#define 	CMINUS	15
#define 	CEQUAL	16
#define 	CRSQP	17
#define 	CRBRACE	18
#define 	CDOT	19
#define 	CGREAT	20
#define 	CSQUOTE	21
#define 	CSLASH	22



/*Tokens*/
#define NUMOFTOKEN	46

#define	TNAME		1	/*NAME*/
#define	TPROGRAM	2	/*program*/
#define	TVAR		3	/*var*/
#define	TARRAY	4	/*array*/
#define	TOF		5	/*of*/
#define	TBEGIN		6	/*begin*/
#define	TEND		7  	/*end*/
#define	TIF		8  	/*if*/
#define	TTHEN		9	/*then*/
#define	TELSE		10	/*else*/
#define	TPROCEDURE	11	/*procedure*/
#define	TRETURN	12	/*return*/
#define	TCALL	13	/*call*/
#define	TWHILE		14	/*while*/
#define	TDO		15 	/*do*/
#define	TNOT		16	/*not*/
#define	TOR		17	/*or*/
#define	TDIV		18 	/*div*/
#define	TAND		19 	/*and*/
#define	TCHAR		20	/*char*/
#define	TINTEGER	21	/*integer*/
#define	TBOOLEAN	22 	/*boolean*/
#define	TREADLN	23	/*readln*/
#define	TWRITELN	24	/*writeln*/
#define	TTRUE		25	/*true*/
#define	TFALSE		26	/*false*/
#define	TNUMBER	27	/*unsigned integer*/
#define	TSTRING	28	/*string*/
#define	TPLUS		29	/*+*/
#define	TMINUS		30 	/*-*/
#define	TSTAR		31 	/*:*/
#define	TEQUAL		32 	/*=*/
#define	TNOTEQ		33 	/*<>*/
#define	TLE		34 	/*<*/
#define	TLEEQ		35 	/*<=*/
#define	TGR		36	/*>*/
#define	TGREQ		37	/*>=*/
#define	TLPAREN	38 	/*(*/
#define	TRPAREN	39 	/*)*/
#define	TLSQPAREN	40	/*[*/
#define	TRSQPAREN	41 	/*]*/
#define	TASSIGN	42	/*:=*/
#define	TDOT		43 	/*.*/
#define	TCOMMA	44	/*,*/
#define	TCOLON		45	/*:*/
#define	TSEMI		46	/*;*/

//syntax start

/* Type tokens*/
#define TYPE_INTEGER 21 //value is made to match that of TINTEGER
#define TYPE_CHAR 20 //value is made to match that of TCHAR
#define TYPE_BOOLEAN 22 //value is made to match that of TBOOLEAN
#define TYPE_INTEGER_ARRAY 210 //value is made to match that of TINTEGER*10
#define TYPE_CHAR_ARRAY 200 //value is made to match that of TCHAR*10
#define TYPE_BOOLEAN_ARRAY 220 //value is made to match that of TBOOLEAN*10
#define TYPE_PROCEDURE 1000

/* return values of save_name */
#define DUPLICATE_DECLARATION -1
#define INCORRECT_TYPE_IN_ARGUMENTS -2

//syntax end

struct TYPE{
	int type; //can be corresponding tokens for int, char, boolean, array_int, array_char, array_bool, proc
	int array_size; //size of array if type is array
	struct TYPE *procedure_arguments_types; //linked list with arguments' types if it is procedure
};

struct LINE{ //linked list with line numbers of NAME reference
	int reference_line_number;
	struct LINE *next_line_number;
};

 // A list to handle symbol table
struct ID {
	char *name; // string with the name of the NAME
	char *procedure_name; // string with the name of the procedure in which it is seen, null if it is global
	struct TYPE *type; // type of the NAME
	int is_in_procedure_argument; // 1 if this variable is seen in procedure arguments, 0 otherwise
	int line_number_of_declaration; // line number where name is declared
	struct LINE *line_numbers_of_references; // a linked list of line numbers
	struct LINE *line_numbers_of_references_tail; // a pointer to the end of line_numbers_of_references list
	struct ID *next_name; //pointer to next NAME
};






void end_scan();
void char_scan();
void error(char *mes);
void id_countup(char *np);
int get_linenum(void);
int init_scan(char *filename);
int which_keyword();
int scan();

int program();
int block();
int variable_declaration();
int variable_names();
int type();
int procedure();
int argument();
int composite_statement();
int statement();
int if_statement();
int while_statement();
int equation();
int expression();
int item();
int element();
int input_output();
int output_specification();

//syntax start
struct ID* search_in_tables(char * name);
struct ID* search_in_global_symbol_table(char*name);
//syntax end

#endif /* HEADERFILE_H_ */
