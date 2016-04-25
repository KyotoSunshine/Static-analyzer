#include "headerfile.h"
int token;

//semantics
extern char string_attr[MAXSTRSIZE];
extern int num_attr;
extern char *buf;
extern int print_symbol_table();
char current_procedure_name[1024]; // name of the current procedure
int in_procedure_arguments=0; //are we processing the procedure arguments now?
extern int save_name(char * name, int type, int array_size);
extern int is_array(int a);
extern int is_standard_type(int a);
extern void end_of_procedure();
char * variables[1024]; // array to store the variable names in variable declaration
int num_variables=0; // number of variables in variables declaration
int size_of_array; //size of array in type declaration
struct ID *last_arguments=NULL; // to use with "call procedure(arguments)" stores last arguments seen in call
// end of semantics

extern int printed_error;

//By convention, we scan character, determine what structure it must be,
//and then check for the structure function to be correct.

// Program::= "program" "NAME" ";" "Block" "."
// # This name of program can be the same as some other names inside Block.
int program() // Parsing main program
{
	// checking tokens from the beginning
	if( token!=TPROGRAM)
	{
		error("No \"program\" word written at the beginning of the program");
		return ERROR;
	}
	token =scan();
	if(token!=TNAME)
	{
		error("No program name");
		return ERROR;
	}
	token =scan();
	 if(token!=TSEMI)
	{
		error("No semicolon after program name");
		return ERROR;
	}
	token =scan();
	if(block()==ERROR)
	{
		return(ERROR);
	}
	else
	{
		if(token !=TDOT)
		{
			error("No dot at the end of program");
			return(ERROR);
		}
		else return(OK);
	}
}

// Block::= {variable declaration | procedure declaration} composite statement
// # names used in "composite statement" must be declared in "variable declaration" part
// # procedure names used in "composite statement" must be declared in "procedure declaration part"
// # The scope of variables declared in "variable declaration" part is whole program.(only the part that is after the
// declaration)
// # All variables must be declared BEFORE usage in text of program (means that one compiler pass is enough)
int block() //Parsing main block
{
	while(token==TVAR||token==TPROCEDURE)
	{
		if (token==TVAR){
			token=scan();
			if(variable_declaration()==ERROR){
				error("No declaration after variable name");
				return(ERROR);
			}
		}else if (token==TPROCEDURE){
			token=scan();
			if(procedure()==ERROR){
				error("No procedure body");
				return(ERROR);
			}
		}
	}
	if(composite_statement()==ERROR)
		return(ERROR);
	else
		return(OK);
}


//variable declaration (after the word var)
// Variable declaration::= "var" variable names ":" type ";" {variable names ":" type ";"}
// # the type of each name is whatever is written after the next semicolon after the name.
int variable_declaration()
{
	if(variable_names()==ERROR)
	{
		error("Variable declaration incorrect");
		return ERROR;
	}
	do
	{
		if(token!=TCOLON)
		{
			error("No colon in variable declaration");
			return ERROR;
		}
		token=scan();
		if(type()==ERROR)return(ERROR);
		if(token!=TSEMI)
		{
			error("No semicolon in variable declaration");
			return ERROR;
		}
		token=scan();
	}while(variable_names()!=ERROR);
	return(OK);
}


// Variable names::= NAME {"," NAME}
// # There should not be more than one name called the same way that is in the same scope.
int variable_names()
{
	if(token!=TNAME)
	{
		return ERROR;
	}
	//semantics
	num_variables=1;
	variables[num_variables-1] = (char*) malloc( sizeof(char)*1024);
	strcpy(variables[num_variables-1],string_attr);
	//end semantics
	token=scan();
	while(token==TCOMMA)
	{
		token=scan();
		if(token!=TNAME)
		{
			error("Variable name must follow after comma in variable names declaration");
			return ERROR;
		}
		//semantics
		num_variables+=1;
/*
		for(i=0;i<num_variables;i++){
			if(!strcmp(string_attr,variables[i]){

			}
		}
*/
		variables[num_variables-1] = (char*) malloc( sizeof(char)*1024);
		strcpy(variables[num_variables-1],string_attr);
		//end semantics
		token=scan();
	}

	//semantics
/*	printf("\nVariables stored: ");
	for(i=0;i<num_variables;i++){
		printf("%s, ",variables[i]);
	}
	printf("\n");
*/
	//end semantics

	return(OK);
}

//Right part of variable declaration(type), final semicolon excluded
// Type::= "integer" | "boolean" | "char" | ("array" "[" signless integer "]" "of" ("integer" | "boolean" | "char"))
// # there are three standard types: integer, boolean, and char
// # the type of array is (standard type, length of array).
// # the number that can be inside parenthesis after array name must be in [0, length of array-1] range.
int type()
{
	int i;
	int return_value_of_save_name;
	if((token==TINTEGER) || (token==TBOOLEAN) ||(token==TCHAR))
	{
		//start of semantics
		for(i=0;i<num_variables;i++){
			return_value_of_save_name=save_name(variables[i],token,-1);
			if(return_value_of_save_name==DUPLICATE_DECLARATION){
				error("Duplicate declaration");
				return(ERROR);
			}
			if(return_value_of_save_name==INCORRECT_TYPE_IN_ARGUMENTS){
				error("Incorrect type in procedure arguments");
				return(ERROR);
			}

		}
		//end of semantics
		token=scan();
		return(OK);
	}
	else if(token!=TARRAY)
	{
		error("Type doesn't exist");
		return(ERROR);
	}
	token=scan();
	if(token!=TLSQPAREN)
	{
		error("No left square parenthesis after array declaration");
		return(ERROR);
	}
	token=scan();
	if(token!=TNUMBER)
	{
		error("Must be an unsigned integer in array declaration inside square parenthesis");
		return(ERROR);
	}
	//semantics start
	size_of_array=num_attr;
	//semantics end
	token=scan();
	if(token!=TRSQPAREN)
	{
		error("No right parenthesis after array declaration");
		return(ERROR);
	}
	token=scan();
	if(token!=TOF)
	{
		error("No \"of\" keyword after array square parenthesis");
		return(ERROR);
	}
	token=scan();
	if(!(token==TINTEGER || token==TBOOLEAN ||token==TCHAR))
	{
		error("Array type does not exist, must be integer, boolean, or char");
		return(ERROR);
	}
	//semantics start
	for(i=0;i<num_variables;i++){
		return_value_of_save_name=save_name(variables[i],token*10,size_of_array);
		if(return_value_of_save_name==DUPLICATE_DECLARATION){
			error("Duplicate declaration");
			return(ERROR);
		}
		if(return_value_of_save_name==INCORRECT_TYPE_IN_ARGUMENTS){
			error("Incorrect type in procedure arguments");
			return(ERROR);
		}
	}
	//semantics end
	token=scan();
	return(OK);
}




//procedure body (after "procedure" keyword)
//Note: procedure may not contain empty parenthesis after its name
//Finally scans token which is after procedures "end;" and returns
// procedure declaration::= "procedure" NAME [ argument ] ";" [ variable declaration ] composite statement ";"
// # scope of the procedure is the whole program(after the procedure declaration)
// # the name of procedure can not be used inside the procedure that it is declared in.(no recursion possible)
// # the scope of names in "argument" and "variable declaration" here is only inside the "composite statement"
// # all names must be declared BEFORE usage.
// # we use static scoping, which means that variable of procedure names can not be redeclared in the same scope.
int procedure()
{
	if(token!=TNAME)
	{
		error("A name for the procedure must follow after \"procedure\" keyword");
		return(ERROR);
	}
	//semantics start
	strcpy(current_procedure_name,string_attr);
	//semantics end
	token=scan();
	if(token==TLPAREN)
	{
		//semantics start
		in_procedure_arguments=1;
		//semantics end
		token=scan();
		if(argument()==ERROR)
			return(ERROR);
		//semantics start
		in_procedure_arguments=0;
		//semantics end

	}
	//semantics start
	if(save_name(current_procedure_name,TYPE_PROCEDURE,-1)==DUPLICATE_DECLARATION){
		error("Duplicate declaration");
		return(ERROR);
	}
	//semantics end
	if(token!=TSEMI)
	{
		error("No semicolon at end of line");
		return(ERROR);
	}
	token=scan();
	if(token==TVAR)
	{
		token=scan();
		if(variable_declaration()==ERROR)
			return(ERROR);
	}
	if(composite_statement()==ERROR){
		return(ERROR);
		error("Procedure body(\"begin\" - \"end;\") is absent");
	}
	if(token!=TSEMI)
	{
		error("No semicolon at end of line");
		return(ERROR);
	}
	//semantics start
	end_of_procedure();
	//semantics end
	token=scan();
	return(OK);
}


// Starts after left parenthesis, includes right parenthesis
// argument::= "(" variable names ":" type {";" variable names ":" type} ")"
// # the type of arguments can only be standard type, no arrays allowed (May be I can just change the syntaxis here?)
// # the type of variables is whatever is after the semicolon after their names
int argument()
{
	if(variable_names()==ERROR) return ERROR;
	if(token!=TCOLON)
	{
		error("No colon");
		return ERROR;
	}
	token=scan();
	if(type()==ERROR)
	{
		error("Incorrect/absent right side of variable declaration");
		return(ERROR);
	}
	while(token==TSEMI)
	{
		token=scan();
		if(variable_names()==ERROR)
		{
			error("Error in variable declaration");
			return ERROR;
		}
		if(token!=TCOLON)
		{
			error("No colon");
			return ERROR;
		}
		token=scan();
		if(type()==ERROR)return(ERROR);
	}
	if(token!=TRPAREN)
	{
		error(" \")\" is required");
		return(ERROR);
	}

	else
	{
		token=scan();
		return(OK);
	}
}


//commands enclosed by begin - end, i.e. "complex statement"
//does not include final semicolon
// composite statement::= "begin" statement {";" statement} "end"
int composite_statement()
{
	if(token!=TBEGIN)
	{
		error("No \"begin\" at the beginning of begin-end statement");
		return ERROR;
	}
	do
	{
		token=scan();
		if(statement()==ERROR){
			error("Not a statement");
			return ERROR;
		}
	}while(token==TSEMI);
	if(token!=TEND)
	{
		error("No \"end\"");
		return ERROR;
	}
	token=scan();
	return(OK);
}

// if, while, or other statements
// statement::= (NAME ["["equation"]"] ":=" equation) | if_statement | while_statement | ("call" NAME ["(" equation {","
// equation} ")"]) | "return" | input_output statement | composite statement | NOTHING
// # in "call name", name must be a procedure which has been declared
// # in "(" equation {"," equation} ")" part, the number of equations must be the same as in declaration of procedure,
// # and each equation must be of the same standard type as was declared in procedure declaration
// # in NAME ["["equation"]"] ":=" equation part, both left and right sides must be of standard type, not an array type.
// # Left "equation" must be of integer type. If there is [] part, then the type of left expression is type of array elements.
// # If there is no [], then type is type of "NAME"
int statement()
{
	struct ID *pname; //for search_in_tables
	struct TYPE *pname_counter; // for enumerating pname procedure arguments
	int ptype=0;
	int left_part_is_array=0; //whether left part in assignment is array
	switch(token)
	{
	case TIF:
		token=scan();
		if(if_statement()==ERROR)
			return(ERROR);
		else
			return(OK);
	case TWHILE:
		token=scan();
		if(while_statement()==ERROR)
			return(ERROR);
		else
			return(OK);
	case TBEGIN:
		//no need to scan token here, composite_statement is a bit different from others.
		if(composite_statement()==ERROR)
			return(ERROR);
		else
			return(OK);
	case TREADLN:
	case TWRITELN:
		if(input_output()==ERROR)return(ERROR);
		else return(OK);
	case TCALL:
		token=scan();
		if(token!=TNAME)
		{
			error("No name of the function");
			return(ERROR);
		}
		pname=search_in_tables(string_attr);
		if (pname==NULL){
			error("Undeclared procedure name");
			return (ERROR);
		}
		if (pname->type->type!=TYPE_PROCEDURE){
			error("Must be a procedure name");
			return (ERROR);
		}
		if (strcmp(pname->name,current_procedure_name)==0){
			error("Recursive calls are not permitted");
			return (ERROR);
		}
		//semantics start
		if(pname->line_numbers_of_references==NULL){
			pname->line_numbers_of_references=(struct LINE*)malloc(sizeof(struct LINE));
			pname->line_numbers_of_references->next_line_number=NULL;
			pname->line_numbers_of_references->reference_line_number=get_linenum();
			pname->line_numbers_of_references_tail=pname->line_numbers_of_references;
		}else{
			pname->line_numbers_of_references_tail->next_line_number=(struct LINE*)malloc(sizeof(struct LINE));
			pname->line_numbers_of_references_tail=pname->line_numbers_of_references_tail->next_line_number;
			pname->line_numbers_of_references_tail->next_line_number=NULL;
			pname->line_numbers_of_references_tail->reference_line_number=get_linenum();
		}
		//semantics end
		token=scan();
		if(token==TLPAREN)
		{
			pname_counter=pname->type->procedure_arguments_types;
			do
			{
				token=scan();
				if((ptype=equation())==ERROR)
				{
					error("Mistake in the formula");
					return ERROR;
				}
				if(pname_counter->type!=ptype){
					error("procedure arguments' types mismatch");
					return(ERROR);
				}else{
					pname_counter=pname_counter->procedure_arguments_types;
				}
			}
			while(token==TCOMMA);
			if(token!=TRPAREN)
			{
				error("No right parenthesis");
				return(ERROR);
			}
			token=scan();
		}else{
			if (pname->type->procedure_arguments_types!=NULL){
				error("this procedure must have arguments");
				return(ERROR);
			}
		}
		return(OK);
	case TRETURN:
		token=scan();
		return(OK);
	case TNAME:
		pname=search_in_tables(string_attr);
		if (pname==NULL){
			error("undeclared variable");
			return(ERROR);
		}
		if (pname->type->type==TPROCEDURE){
			error("procedure calls must be preceded by \"call\" keyword");
			return(ERROR);
		}
		//semantics start
		if(pname->line_numbers_of_references==NULL){
			pname->line_numbers_of_references=(struct LINE*)malloc(sizeof(struct LINE));
			pname->line_numbers_of_references->next_line_number=NULL;
			pname->line_numbers_of_references->reference_line_number=get_linenum();
			pname->line_numbers_of_references_tail=pname->line_numbers_of_references;
		}else{
			pname->line_numbers_of_references_tail->next_line_number=(struct LINE*)malloc(sizeof(struct LINE));
			pname->line_numbers_of_references_tail=pname->line_numbers_of_references_tail->next_line_number;
			pname->line_numbers_of_references_tail->next_line_number=NULL;
			pname->line_numbers_of_references_tail->reference_line_number=get_linenum();
		}
		//semantics end
		token=scan();
		if(token==TLSQPAREN)
		{
			left_part_is_array=1;
			if (!is_array(pname->type->type)){
				error("type must be array");
				return(ERROR);
			}
			token=scan();
			if((ptype=equation())==ERROR){
				error("invalid expression inside square parenthesis");
				return(ERROR);
			}else{
				if (ptype!=TYPE_INTEGER){
					error("array subscript must be of integer type");
					return(ERROR);
				}
			}
			if(token!=TRSQPAREN)
			{
				error("no right square parenthesis");
				return(ERROR);
			}
			token=scan();
		}else{
			if (is_array(pname->type->type)){
				error("left part in assignment can not be of type array");
				return (ERROR);
			}
		}
		if(token!=TASSIGN)
		{
			error("No \":=\"");
			return(ERROR);
		}
		token=scan();
		if((ptype=equation())==ERROR){
			error("invalid expression on the right side of equation");
			return(ERROR);
		}
		if(!is_standard_type(ptype)){
			error("right side of assignment expression must be of standard type(integer,char or boolean)");
			return(ERROR);
		}
		if(left_part_is_array){
			if (ptype!=pname->type->type/10){
				error("type of right part does not match type of left part");
				return(ERROR);
			}
		}else{
			if (ptype!=pname->type->type){
				error("type of right part does not match type of left part");
				return(ERROR);
			}

		}
		return(OK);
	default:
		return(OK);
	}
}

// if_statement::= "if" equation "then" statement ["else" statement]
// # "equation" after "if" must be of type boolean
int if_statement()
{
	int ptype=0;
	if((ptype=equation())==ERROR)return(ERROR);
	else{
		if (ptype!=TYPE_BOOLEAN){
			error("type must be boolean in if statement");
			return(ERROR);
		}
	}
	if(token!=TTHEN)
	{
		error("A keyword \"then\" must eventually follow keyword \"if\" ");
		return(ERROR);
	}
	token=scan();
	if(statement()==ERROR){
		error("Not a statement");
		return ERROR;
	}
	if(token==TELSE)
	{
		token=scan();
		if(statement()==ERROR){
			error("Not a statement");
			return ERROR;
		}
	}
	return OK;
}

// while_statement::= "while" equation "do" statement
// # "equation" after "while" must be of type boolean
int while_statement()
{
	int ptype=0;
	if((ptype=equation())==ERROR)return(ERROR);
	else{
		if (ptype!=TYPE_BOOLEAN){
			error("type must be boolean in while statement");
			return(ERROR);
		}
	}
	if(token!=TDO)
	{
		error("No \"do\" in while statement");
		return(ERROR);
	}
	token=scan();
	if(statement()==ERROR){
		error("Not a statement");
		return ERROR;
	}
	return OK;
}


// equation::= expression {("=" | "<>" | "<" | "<=" | ">" | ">=") expression}
// # if there is only one expression, then type of resulting equation is type of that expression
// # the equality comparison symbols must have the same standard type on both sides, and resulting type is boolean
// this function returns type
int equation()
{
	int ptype1=0;
	int ptype2=0;
	if((ptype1=expression())==ERROR)
		return(ERROR);
	while(token==TEQUAL || token==TNOTEQ ||token==TLE||token==TLEEQ||token==TGR||token==TGREQ)
	{
		token=scan();
		if((ptype2=expression())==ERROR)
		{
			error(" Problem in equation");
			return(ERROR);
		}
		if (ptype1!=ptype2){
			error("types on both sides of operator must be same");
			return(ERROR);
		}
		ptype1=TYPE_BOOLEAN;
	}
	return(ptype1);
}

// expression::= ["+"|"-"] item {("+" | "-" | "or") item}
// # if there is only one item, then the type of expression is the same as type of that item
// # if there is + or - at the beginning, then the type of first item must be integer, and type of expression is also integer.
// # both sides of "+" or "-" must have integer type, and resulting type is also integer
// # both sides of "or" must be boolean type and result is also boolean
// this function returns type
int expression()
{
	int ptype1=0;
	int ptype2=0;

	if((token==TPLUS) ||(token==TMINUS))
	{
		do
		{
			if (token==TOR){
				error("incorrect type");
				return(ERROR);
			}
			token=scan();
			if((ptype1=item())==ERROR)
			{
				error("problem in equation");
				return(ERROR);
			}
			if (ptype1!=TYPE_INTEGER){
				error("type of item after plus or minus sign must be integer");
				return(ERROR);
			}
		}while(token==TPLUS || token==TMINUS ||token==TOR);
	}
	else
	{
		if((ptype1=item())==ERROR)return(ERROR);
		if ((token==TPLUS || token==TMINUS)&&(ptype1!=TYPE_INTEGER)){
			error("type must be integer");
			return(ERROR);
		}
		if (token==TOR && (ptype1!=TYPE_BOOLEAN)){
			error("type must be boolean");
			return(ERROR);
		}

		while(token==TPLUS || token==TMINUS ||token==TOR)
		{
			token=scan();
			if((ptype2=item())==ERROR)
			{
				error("problem in expression");
				return(ERROR);
			}
			if(ptype2!=ptype1){
				error("incorrect type");
				return(ERROR);
			}
		}
	}
	return(ptype1);
}

// item::= element {("*" | "div" | "and")   element}
// # if there is only one element, then type of item is type of that element
// # "*" and "div" must have integer on both sides, the result is also integer.(div is probably modular division)
// # "and" must have boolean on both sides and result is also boolean
// this function returns type
int item()
{
	int ptype1=0;
	int ptype2=0;
	int operation=0;
	if((ptype1=element())==ERROR)return(ERROR);

	while(token==TSTAR ||token==TDIV ||token==TAND)
	{
		operation=token;
		token=scan();
		if((ptype2=element())==ERROR)
		{
			error("problem in equation");
			return(ERROR);
		}
		switch(operation){
		case TSTAR:
		case TDIV:
			if (ptype1!=TYPE_INTEGER || ptype2!=TYPE_INTEGER){
				error("both operands of multiplication or division must be of type integer");
				return(ERROR);
			}
			break;
		case TAND:
			if (ptype1!=TYPE_BOOLEAN || ptype2!=TYPE_BOOLEAN){
				error("both operands of \"AND\" must be of type integer");
				return(ERROR);
			}
			break;
		default:
			error("problem in item()");
			return(ERROR);
		}
	}
	return(ptype1);
}

// element::= (NAME ["[" equation "]"]) | (NUMBER | "false" | "true" | "string") | "(" equation ")" | "not" element
// # if it is variable or constant (first or second things), then type of element is type of those parts.
// # if it is "("equation")", then type is type of that equation
// # part after not must be boolean, the result is also boolean
// # if second part is NUMBER, then type is integer, if true or false then type is boolean
// # if string, then its length must be one, and its type will be char
// this function returns type
int element()
{
	int return_type=0;
	int ptype=0; // for processing types of called functions
	struct ID* pname; //for search_in_tables
	switch(token)
	{
		case TNOT:
			token=scan();
			if((return_type=element())==ERROR)return(ERROR);
			else{
				if (return_type==TYPE_BOOLEAN){
					return(TYPE_BOOLEAN);
				}else{
					error("Type of \"not\" operation must be boolean");
					return(ERROR);
				}
			}
		case TLPAREN:
			token=scan();
			if((return_type=equation())==ERROR)return(ERROR);
			if(token!=TRPAREN)
			{
				error("No right parenthesis");
				return(ERROR);
			}
			token=scan();
			return(return_type);
		case TNUMBER:
			token=scan();
			return(TYPE_INTEGER);
		case TSTRING:
			if(strlen(string_attr)!=1){
				error("char length must be 1");
				return(ERROR);
			}
			token=scan();
			return(TYPE_CHAR);
		case TTRUE:
		case TFALSE:
			token=scan();
			return(TYPE_BOOLEAN);
		case TNAME:
			pname=search_in_tables(string_attr);
			if (pname==NULL){
				error("Undeclared variable");
				return(ERROR);
			}
			//semantics start
			if(pname->line_numbers_of_references==NULL){
				pname->line_numbers_of_references=(struct LINE*)malloc(sizeof(struct LINE));
				pname->line_numbers_of_references->next_line_number=NULL;
				pname->line_numbers_of_references->reference_line_number=get_linenum();
				pname->line_numbers_of_references_tail=pname->line_numbers_of_references;
			}else{
				pname->line_numbers_of_references_tail->next_line_number=(struct LINE*)malloc(sizeof(struct LINE));
				pname->line_numbers_of_references_tail=pname->line_numbers_of_references_tail->next_line_number;
				pname->line_numbers_of_references_tail->next_line_number=NULL;
				pname->line_numbers_of_references_tail->reference_line_number=get_linenum();
			}
			//semantics end
			token=scan();
			if(token==TLSQPAREN)
			{
				if (!is_array(pname->type->type)){
					error("Variable must be an array");
					return(ERROR);
				}
				token=scan();
				if((ptype=equation())==ERROR)return(ERROR);
				else{
					if (ptype!=TYPE_INTEGER){
						error("array index must be of integer type");
						return(ERROR);
					}
				}
				if(token!=TRSQPAREN)
				{
					error("no right square parenthesis");
					return(ERROR);
				}
				token=scan();
				return(pname->type->type/10); //should perform integer division, makes TYPE_INTEGER from TYPE_INTEGER_ARRAY etc.
			}
			return(pname->type->type);
		default:
			return(ERROR);
	}
}


// input_output statement::= "readln" ["("(NAME ["[" equation "]"]) {"," (NAME ["[" equation "]"])}")"] | "writeln" [ "(" output
// specification { "," output specification } ")" ]
// # variables must be integer or char, can not be boolean or array.
int input_output()
{
	struct ID *pname;
	int ptype=0;
	switch(token)
	{
	case TREADLN:
		token=scan();
		if(token==TLPAREN)
		{
			do
			{
				token=scan();
				if(token==TNAME)
				{
					pname=search_in_tables(string_attr);
					if (pname==NULL){
						error("undeclared variable");
						return(ERROR);
					}
					//semantics start
					if(pname->line_numbers_of_references==NULL){
						pname->line_numbers_of_references=(struct LINE*)malloc(sizeof(struct LINE));
						pname->line_numbers_of_references->next_line_number=NULL;
						pname->line_numbers_of_references->reference_line_number=get_linenum();
						pname->line_numbers_of_references_tail=pname->line_numbers_of_references;
					}else{
						pname->line_numbers_of_references_tail->next_line_number=(struct LINE*)malloc(sizeof(struct LINE));
						pname->line_numbers_of_references_tail=pname->line_numbers_of_references_tail->next_line_number;
						pname->line_numbers_of_references_tail->next_line_number=NULL;
						pname->line_numbers_of_references_tail->reference_line_number=get_linenum();
					}
					//semantics end
					token=scan();
					if(token==TLSQPAREN)
					{
						if (!is_array(pname->type->type)){
							error("variable must be of array type");
							return(ERROR);
						}
						if(pname->type->type!=TYPE_INTEGER_ARRAY && pname->type->type!=TYPE_CHAR_ARRAY){
							error("array must be of integers or of chars");
							return(ERROR);
						}
						token=scan();
						if((ptype=equation())==ERROR)return(ERROR);
						else{
							if (ptype!=TYPE_INTEGER){
								error("array index must be of type integer");
								return(ERROR);
							}
						}
						if(token!=TRSQPAREN)
						{
							error("no right parenthesis");
							return(ERROR);
						}
						token=scan();
					}else{
						if(pname->type->type!=TYPE_INTEGER && pname->type->type!=TYPE_CHAR){
							error("variable must be of type integer or char");
							return(ERROR);
						}
					}
				}
				else
				{
					error("Error in variables");
					return ERROR;
				}
			}while(token==TCOMMA);
			if(token!=TRPAREN)
			{
				error("No right parenthesis");
				return ERROR;
			}
			else
			{
				token=scan();
				return(OK);
			}
		}
		return(OK);
		break;
	case TWRITELN:
		token=scan();
		if(token==TLPAREN)
		{
			do
			{
				token=scan();
				if(output_specification()==ERROR)return(ERROR);
			}while(token==TCOMMA);
			if(token!=TRPAREN)
			{
				error("No right parenthesis");
				return ERROR;
			}
			else
			{
				token=scan();
				return(OK);
			}
		}
		return(OK);
	}
	return ERROR;
}

// output specification::= equation [":" "unsigned integer" ] | string
// # equation must be of standard type, can not be array.
// # length of string must 0 or >=2.
int output_specification()
{
	int ptype=0;
	if((token==TSTRING) && (strlen(string_attr)!=1))
	{
		token=scan();
		return(OK);
	}
	else
	{
		if((ptype=equation())==ERROR)
		{
			error("\"writeln\" is incorrectly structured");
			return(ERROR);
		}else{
			if(!is_standard_type(ptype)){
				error("variables in output specification must be of standard types(integer, char or boolean)");
				return(ERROR);
			}
		}
		if(token==TCOLON)
		{
			token=scan();
			if(token!=TNUMBER)
			{
				error("Not an unsigned integer");
				return(ERROR);
			}
			else
			{
				token=scan();
				return(OK);
			}
		}
		return OK;
	}
}


int main(int argc,char *argv[])
{
	int i;
	if(argc!=2)
	{
		error("Incorrect number of arguments");
		return(ERROR);
	}
	//semantics start
	for(i=0;i<1024;i++){ //initiating variables array to NULLs
		variables[i]=NULL;
	};
	buf=(char*)malloc(sizeof(char)*1024);
	//semantics end

	if( init_scan(argv[1]) == ERROR)return(ERROR);
	token=scan();
	program();
	end_scan();
	if (!printed_error){
		print_symbol_table();
	}
	return 0;
}
