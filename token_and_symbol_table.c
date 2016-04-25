#include "headerfile.h"

FILE *finp;

int num_attr;    //If token is integer, contains its value
char string_attr[MAXSTRSIZE]; //If token is string, contains its value
int eof_flag=0;  //Equals one if scan reaches end of file
int count[NUMOFTOKEN+1]={0};  // Contains frequency data for each token
int ascii_code =0;  //ASCII codes in decimal notation
int linenum=1;  //Current scanning line number in the input file, lets start from 1
int i; //counter
int indent=0; //indentation level for pretty print
int prev_token=-1; //previous token for pretty print
int in_procedure=0; //for pretty print and semantics
int met_first_begin_in_procedure=0; //for pretty print
int met_first_begin_in_program=0; //for pretty print
int printed_error=0; // in order to only print one error
int linenum_controller=0; //a flag to control increasing of linenum
//semantics start
extern int in_procedure_arguments; // whether the variable(or current execution) is in procedure arguments
extern char current_procedure_name[1024];
char *buf; //return value for type_in_words function
//semantics end

char ctable[256]={
		CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,
		CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,
		CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,
		CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,
		CNULL,CEOF,CEOF,CEOF,CEOF,CEOF,CEOF,CSQUOTE,
		CLPAREN,CRPAREN,CSTAR,CPLUS,CCOMMA,CMINUS,CDOT,CSLASH,
		CDIGIT,CDIGIT,CDIGIT,CDIGIT,CDIGIT,CDIGIT,CDIGIT,CDIGIT,
		CDIGIT,CDIGIT,CCOLON,CSEMI,CLESS,CEQUAL,CGREAT,CEOF,
		CEOF,CALPHA,CALPHA,CALPHA,CALPHA,CALPHA,CALPHA,CALPHA,
		CALPHA,CALPHA,CALPHA,CALPHA,CALPHA,CALPHA,CALPHA,CALPHA,
		CALPHA,CALPHA,CALPHA,CALPHA,CALPHA,CALPHA,CALPHA,CALPHA,
		CALPHA,CALPHA,CALPHA,CLSQP,CEOF,CRSQP,CEOF,CEOF,
		CEOF,CALPHA,CALPHA,CALPHA,CALPHA,CALPHA,CALPHA,CALPHA,
		CALPHA,CALPHA,CALPHA,CALPHA,CALPHA,CALPHA,CALPHA,CALPHA,
		CALPHA,CALPHA,CALPHA,CALPHA,CALPHA,CALPHA,CALPHA,CALPHA,
		CALPHA,CALPHA,CALPHA,CLBRACE,CEOF,CRBRACE,CEOF,CNULL,
		CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,
		CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,
		CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,
		CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,
		CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,
		CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,
		CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,
		CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,
		CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,
		CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,
		CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,
		CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,
		CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,
		CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,
		CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,
		CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL,CNULL
};


//two helper functions
int is_standard_type(int a){ //returns 1 if argument is of standard type and 0 otherwise
	switch(a){
	case TYPE_INTEGER:
	case TYPE_CHAR:
	case TYPE_BOOLEAN:
	return 1;
	default:
		return 0;
	}
}

int is_array(int a){ // returns 1 if argument type is array and 0 otherwise
	switch(a){
		case TYPE_INTEGER_ARRAY:
		case TYPE_CHAR_ARRAY:
		case TYPE_BOOLEAN_ARRAY:
			return 1;
		default:
			return 0;
	}

}


struct ID *global_symbol_table=NULL, *local_symbol_table=NULL,*curr_local=NULL,*curr_global=NULL;


// saving declared variable into the nametable
int save_name(char *name, int type, int array_size){
	struct ID *temp;
	struct ID *pname; //for search_in_tables
	struct TYPE *gtemp;

	if (type==TYPE_PROCEDURE){
		pname=search_in_global_symbol_table(name);
		if (pname!=NULL){
			return DUPLICATE_DECLARATION;
		}
	}else{
		pname=search_in_tables(name);
	}

	if(pname!=NULL){
		if (in_procedure){
			if(strcmp(pname->procedure_name,"\0")!=0){ // if the name that we found is in local scope
				return DUPLICATE_DECLARATION;
			}
		}else{
			return DUPLICATE_DECLARATION;
		}
	}

	if(in_procedure_arguments&& !is_standard_type(type)){
		return INCORRECT_TYPE_IN_ARGUMENTS; //incorrect type in arguments
	}



	if (type==TYPE_PROCEDURE){
		if (global_symbol_table==NULL){
			global_symbol_table=(struct ID *)malloc(sizeof(struct ID));
			global_symbol_table->type=(struct TYPE *)malloc(sizeof(struct TYPE));
			curr_global=global_symbol_table;
			curr_global->type->procedure_arguments_types=NULL;
			curr_global->line_numbers_of_references=NULL;
			curr_global->line_numbers_of_references_tail=NULL;
			curr_global->next_name=NULL;
		}
		else{
			curr_global->next_name=(struct ID *)malloc(sizeof(struct ID));
			curr_global->next_name->type=(struct TYPE *)malloc(sizeof(struct TYPE));
			curr_global = curr_global->next_name;
			curr_global->type->procedure_arguments_types=NULL;
			curr_global->line_numbers_of_references=NULL;
			curr_global->line_numbers_of_references_tail=NULL;
			curr_global->next_name=NULL;
		}
		curr_global->procedure_name="\0";
		curr_global->name=(char*)malloc(sizeof(char)*1024);
		strcpy(curr_global->name,name);
		curr_global->type->array_size=array_size;
		curr_global->is_in_procedure_argument=in_procedure_arguments;
		curr_global->line_number_of_declaration=get_linenum();
		curr_global->type->type=type;

		temp=local_symbol_table;
		gtemp=curr_global->type;
		while (temp!=NULL){
			gtemp->procedure_arguments_types=(struct TYPE*)malloc(sizeof(struct TYPE));
			*(gtemp->procedure_arguments_types)=*(temp->type); //or maybe without stars
			gtemp->procedure_arguments_types->procedure_arguments_types=NULL;
			gtemp=gtemp->procedure_arguments_types;
			temp=temp->next_name;
		}
		return 0;
	}

	if (in_procedure==1){
		if (local_symbol_table==NULL){
			local_symbol_table=(struct ID *)malloc(sizeof(struct ID));
			local_symbol_table->type=(struct TYPE *)malloc(sizeof(struct TYPE));
			curr_local=local_symbol_table;

		}
		else{
			curr_local->next_name=(struct ID *)malloc(sizeof(struct ID));
			curr_local->next_name->type=(struct TYPE *)malloc(sizeof(struct TYPE));
			curr_local = curr_local->next_name;
		}
		curr_local->name=name;
		curr_local->procedure_name=(char*)malloc(sizeof(char)*1024);
		strcpy(curr_local->procedure_name,current_procedure_name);
		curr_local->type->array_size=array_size;
		curr_local->is_in_procedure_argument=in_procedure_arguments;
		curr_local->line_number_of_declaration=get_linenum();
		curr_local->type->type=type;
		curr_local->type->procedure_arguments_types=NULL;
		curr_local->line_numbers_of_references=NULL;
		curr_local->line_numbers_of_references_tail=NULL;
		curr_local->next_name=NULL;
	}
	else{
		if (global_symbol_table==NULL){
			global_symbol_table=(struct ID *)malloc(sizeof(struct ID));
			global_symbol_table->type=(struct TYPE *)malloc(sizeof(struct TYPE));
			curr_global=global_symbol_table;
		}
		else{
			curr_global->next_name=(struct ID *)malloc(sizeof(struct ID));
			curr_global->next_name->type=(struct TYPE *)malloc(sizeof(struct TYPE));
			curr_global = curr_global->next_name;
		}
		curr_global->procedure_name="\0";
		curr_global->name=name;
		curr_global->type->array_size=array_size;
		curr_global->is_in_procedure_argument=in_procedure_arguments;
		curr_global->line_number_of_declaration=get_linenum();
		curr_global->type->type=type;
		curr_global->type->procedure_arguments_types=NULL;
		curr_global->line_numbers_of_references=NULL;
		curr_global->line_numbers_of_references_tail=NULL;
		curr_global->next_name=NULL;
	}
	return 0;
}

struct ID *local_tables_storage[1024]; 	//when we exit procedure we store the past
										//local symbol table here.
int num_of_local_tables=0;


void end_of_procedure(){ // saving past local symbol table into archive
	num_of_local_tables+=1;
	local_tables_storage[num_of_local_tables-1]=local_symbol_table;
	local_symbol_table=NULL;
	strcpy(current_procedure_name,"\0");
}



void type_in_words(int type,int size){
	switch (type){
	case TYPE_INTEGER:
		strcpy(buf,"integer");
		break;
	case TYPE_CHAR:
		strcpy(buf,"char");
		break;
	case TYPE_BOOLEAN:
		strcpy(buf,"boolean");
		break;
	case TYPE_INTEGER_ARRAY:
		sprintf(buf,"%s%d%s", "array[", size, "] of integer");
		break;
	case TYPE_CHAR_ARRAY:
		sprintf(buf,"%s%d%s", "array[", size, "] of char");
		break;
	case TYPE_BOOLEAN_ARRAY:
		sprintf(buf,"%s%d%s", "array[", size, "] of boolean");
		break;
	case TYPE_PROCEDURE:
		strcpy(buf,"procedure");
		break;
	}
}


struct ID* search_in_tables(char*name){ //returns pointer to the variable in the symbol table(global or local)
	struct ID * p;
	if (!in_procedure){
		for(p=global_symbol_table;p!=NULL;p=p->next_name){
			if (strcmp(name,p->name)==0){
				return p;
			}
		}
		return NULL;
	}else{
		for(p=local_symbol_table;p!=NULL;p=p->next_name){
			if (strcmp(name,p->name)==0){
				return p;
			}
		}
		for(p=global_symbol_table;p!=NULL;p=p->next_name){
			if (strcmp(name,p->name)==0){
				return p;
			}
		}
		return NULL;
	}
}

struct ID* search_in_global_symbol_table(char*name){ //returns pointer to the variable in the global symbol table
	struct ID * p;
		for(p=global_symbol_table;p!=NULL;p=p->next_name){
			if (strcmp(name,p->name)==0){
				return p;
			}
		}
		return NULL;
}




/* string of each token */
char *token_str[NUMOFTOKEN+1] = {
	"",
	"NAME", "program", "var", "array", "of", "begin", "end", "if", "then", "else", "procedure", "return", "call",
	"while", "do", "not", "or", "div", "and", "char", "integer", "boolean", "readln",
	"writeln", "true", "false", "NUMBER", "STRING", "+", "-", "*", "=", "<>",
	"<", "<=", ">", ">=", "(", ")", "[", "]", ":=", ".",
	",", ":", ";"
};



struct KEY
{
	char *keyword;
	int symbol;
}  key[KEY_SIZE]={
		{"program",TPROGRAM},
		{"var",TVAR},
		{"array",TARRAY},
		{"of",TOF},
		{"begin",TBEGIN},
		{"end",TEND},
		{"if",TIF},
		{"then",TTHEN},
		{"else",TELSE},
		{"procedure",TPROCEDURE},
		{"return",TRETURN},
		{"call",TCALL},
		{"while",TWHILE},
		{"do",TDO},
		{"not",TNOT},
		{"or",TOR},
		{"div",TDIV},
		{"and",TAND},
		{"char",TCHAR},
		{"integer",TINTEGER},
		{"boolean",TBOOLEAN},
		{"readln",TREADLN},
		{"writeln",TWRITELN},
		{"true",TTRUE},
		{"false",TFALSE}
     };

int get_linenum(void)
{
	return(linenum);
}


void char_scan()
{
	if (linenum_controller==1){
		linenum++;
		linenum_controller=0;
	}
	if((ascii_code=fgetc(finp))!= EOF)
	{
	    if (ascii_code < 0 || ascii_code > 255)ascii_code=255;
        if(ascii_code=='\n')
        {
        	linenum_controller=1;
        }
	}
	else
	{
		eof_flag=1;
	}
}



int which_keyword()
{
	int i=0;

    while(i<KEY_SIZE)
    {
        if(!strcmp(string_attr,key[i].keyword))return(key[i].symbol);
	    i++;
    }
	return(TNAME);
}


// Returns integer code of next token
int scan()
{
	int next_token; // First holds the classification of next char, then will hold the value of next token
	int str_count=0;
	int is_comment=1; //whether last token was a comment or not, setting to comment at the beginning
	char *str_pointer;

	while (is_comment){
		is_comment=0;


		// Find first character which is not counted as NULL
		while( (next_token=ctable[ascii_code] )==CNULL && eof_flag==0)char_scan();


		//If found illegal display characters, then give error. These characters are only allowed in strings.
		if(next_token==CEOF)
		{
			error("Illegal character");
			return(-1);
		}

		//Setting next_token to CEOF so that it gets processed as end of file in next switch segment
		if(eof_flag==1)next_token=CEOF;

		switch (next_token)
		{
		// If alphabet character, then write it in string_attr and then check if it is name or keyword.
		// If it is name, then set next_token to TNAME, and count the name in id_countup. If it is
		// keyword, then set next_token to its integer value.
		case CALPHA:
			str_pointer=string_attr;
			do
			{
				*str_pointer++=ascii_code;
				char_scan();
			} while(ctable[ascii_code]==CALPHA || ctable[ascii_code]==CDIGIT);
			*str_pointer='\0';
			next_token=which_keyword();
			break;

			//if it is number then set next_token to TNUMBER
		case CDIGIT:
			num_attr=ascii_code-'0';
			char_scan();
			while('0' <= ascii_code && ascii_code <= '9')
			{
				num_attr= (num_attr * 10)+(ascii_code -'0');
				if(num_attr > 32767)
				{
					error("Number is larger than 32767");
					return(-1);
				}
				char_scan();
			}
			if( ctable[ascii_code]==CALPHA || ctable[ascii_code]==CEOF )
			{
				error("There are no tokens that start with digit and not end with digit");
				return(-1);
			}
			else next_token=TNUMBER;
			break;
	// Processes string. Accounts for the case when double single quotes means single quote(''->').
	// In case if string is too long, cuts it there, gives error and exits.
		case CSQUOTE:
			char_scan();
			str_pointer=string_attr;
			while(1)
			{
				if(ctable[ascii_code]==CSQUOTE)
				{
					char_scan();
					if(ctable[ascii_code]==CSQUOTE){}
					else
					{
						*str_pointer='\0';
						next_token=TSTRING;
						break;
					}
				}

				if(++str_count ==  MAXSTRSIZE)
				{
					*str_pointer='\0';
					next_token=TSTRING;
				}

				if(str_count > MAXSTRSIZE)
				{
					error("String too long");
					count[TSTRING]++;
					return(-1);
				}
				*str_pointer++=ascii_code;
				char_scan();
				if(eof_flag==1)
				{
					error("End of file");
					return(-1);
				}
			}
			break;

	// If character is <, then it could be <, <=, or <>
		case CLESS:
			char_scan();
			if(ctable[ascii_code]==CEQUAL)
			{
				next_token=TLEEQ;
				char_scan();
			}
			else if(ctable[ascii_code]==CGREAT)
			{
				next_token=TNOTEQ;
				char_scan();
			}else {
				next_token=TLE;
			}
			break;

	//If character is >, then it can be > or >=
		case CGREAT:
			char_scan();
			if(ctable[ascii_code]==CEQUAL)
			{
				next_token=TGREQ;
				char_scan();
			}else {
				next_token=TGR;
			}
			break;

	// If char is :, it can be : or :=
		case CCOLON:
			char_scan();
			if(ctable[ascii_code]==CEQUAL)
			{
				next_token=TASSIGN;
				char_scan();
			}else {
				next_token=TCOLON;
			}
			break;

	//.
		case CDOT:
			next_token=TDOT;
			char_scan();
			break;

	//(
		case CLPAREN:
			next_token=TLPAREN;
			char_scan();
			break;

		case CPLUS://+
			next_token=TPLUS;
			char_scan();
			break;


		case CMINUS://-
			next_token=TMINUS;
			char_scan();
			break;


		case CSTAR://*
			next_token=TSTAR;
			char_scan();
			break;


		case CEQUAL://=
			next_token=TEQUAL;
			char_scan();
			break;

		case CRPAREN://)
			next_token=TRPAREN;
			char_scan();
			break;

		case CLSQP://[
			next_token=TLSQPAREN;
			char_scan();
			break;


		case CRSQP: //]
			next_token=TRSQPAREN;
			char_scan();
			break;


		case CCOMMA://,
			next_token=TCOMMA;
			char_scan();
			break;

		case CSEMI://;
			next_token=TSEMI;
			char_scan();
			break;


	// There is no token for comments, so we just pass it.
		case CLBRACE://{Comment}
			while( ctable[ascii_code] != CRBRACE )
			{
				char_scan();
				if(eof_flag==1)
				{
					error("End of file");
					return(-1);
				}
			}
			is_comment=1;
			char_scan();
			break;
	//In case of / * comments
		case CSLASH:// /*  */
			char_scan();
			if(ctable[ascii_code]!=CSTAR)
			{
				error("'/' is not a token by itself");
				return(-1);
			}
			else
			{
				char_scan();
				while(1)
				{
					if(ascii_code=='*')
					{
						char_scan();
						if(eof_flag==1)
						{
							error("End of file");
							return(-1);
						}
						if(ascii_code=='/')
						{
							is_comment=1;
							char_scan();
							break;
						}
					}	else
					{
						char_scan();
						if(eof_flag==1)
						{
							error("End of file");
							return(-1);
						}
					}
				}
			}
			break;
		case CEOF:
			return(-1);
		}
	}


	//Printing every token for pretty print
	// I didn't delete this part because I use in_procedure variable in other functions

	//Before printing indentation changes
	if (next_token==TPROCEDURE){
		indent=1;
		met_first_begin_in_procedure=0;
		in_procedure=1;
	}
	if (next_token==TEND){
		indent--;
	}
	if (next_token==TBEGIN){
		if (in_procedure){
			if (met_first_begin_in_procedure==0){
				indent=1;
				met_first_begin_in_procedure=1;
			}
		}else{ //not in procedure
			if (met_first_begin_in_program==0){
				indent=0;
				met_first_begin_in_program=1;
			}
		}
	}
	if (next_token==TVAR){
		if (in_procedure){
				indent=2;
		}else{ //not in procedure
				indent=1;
		}
	}

	if (in_procedure && next_token==TEND && met_first_begin_in_procedure && indent==1){
		in_procedure=0;
	}

	//After printing indentation changes
	if (next_token==TBEGIN){
		indent++;
	}
	if (next_token==TVAR){
		indent++;
	}


	//prev_token becomes current token
	prev_token=next_token;

	// end of pretty print part


	return(next_token);
}


void end_scan()
{
	fclose(finp);
}


void error(char *mes)
{
	if (!printed_error){
		printf("\n[ERROR] : at line number %d, %s", get_linenum(),mes);
		fflush(stdout);
		printed_error=1;
	}
}

//Open input file
int init_scan(char *filename)
{
	if( (finp=fopen(filename,"r")) ==NULL )
	{
		error("Input file can't be opened");
		return(ERROR);
	}
	else return(OK);
}

// General procedure for parsing source code into tokens. Calls scan()
void Determine_Tokens(){
	int nexttoken;
	char_scan();
	if (eof_flag!=1) linenum=1;
	while(1)
	{
		if((nexttoken=scan())<0)break;
		else count[nexttoken]++;
	}
}

int print_symbol_table(){
	struct ID *p,*merged,*curr_merged,*counter,*temp1,*temp_name;//counter,temp1 and temp_name are for sorting
	char *oname1,*oname2; //official names to compare names
	struct TYPE* temp;
	struct LINE* reference=NULL;
	char *res;
	int i;
	res=(char*)malloc(sizeof(char)*MAXSTRSIZE*5); //max string size of type length


	//--merge lists together--
	merged=NULL;
	curr_merged=NULL;
	p=global_symbol_table; //merging global symbol table
	if (p!=NULL){
		merged=(struct ID*)malloc(sizeof(struct ID));
		*merged=*p;

		curr_merged=merged;
		while (p->next_name!=NULL){
			curr_merged->next_name=(struct ID*)malloc(sizeof(struct ID));
			*(curr_merged->next_name)=*(p->next_name);
			curr_merged->next_name->next_name=NULL;
			curr_merged=curr_merged->next_name;
			p=p->next_name;
		}
	}

	for(i=0;i<num_of_local_tables;i++){
		p=local_tables_storage[i]; //merging local symbol tables
		if (p!=NULL){
			curr_merged->next_name=(struct ID*)malloc(sizeof(struct ID));
			*(curr_merged->next_name)=*p;
			curr_merged->next_name->next_name=NULL;
			curr_merged=curr_merged->next_name;

			while (p->next_name!=NULL){
				curr_merged->next_name=(struct ID*)malloc(sizeof(struct ID));
				*(curr_merged->next_name)=*(p->next_name);
				curr_merged->next_name->next_name=NULL;
				curr_merged=curr_merged->next_name;
				p=p->next_name;
			}
		}
	}

//--end of merging--


	//sorting

	temp1=(struct ID*)malloc(sizeof(struct ID));
	oname1=(char*)malloc(sizeof(char)*2049);
	oname2=(char*)malloc(sizeof(char)*2049);
	if (merged!=NULL){
		for(p=merged; p->next_name != NULL; p = p->next_name)
		{
			for(counter = p->next_name; counter != NULL; counter = counter->next_name)
			{
				sprintf(oname1,"%s:%s",p->name,p->procedure_name);
				sprintf(oname2,"%s:%s",counter->name,counter->procedure_name);
				if(strcmp(oname1,oname2)>0)
				{
					*temp1 = *p;
					temp_name=p->next_name;
					*p = *counter;
					p->next_name=temp_name;
					temp_name=counter->next_name;
					*counter = *temp1;
					counter->next_name=temp_name;
				}
			}
		}
	}

	//end of sorting


	//merged crossreference table
	if (merged!=NULL){
		p=merged;
		printf("%-15s %-40s %-4s %-20s\n", "Name","Type","Def.","| Ref.");
		while (p!=NULL){
			if (p->type->type==TYPE_PROCEDURE){
				printf("%-15s ",p->name);
				temp=p->type;
				type_in_words(p->type->type,p->type->array_size);
				strcpy(res,buf);
				if (temp->procedure_arguments_types!=NULL){
					res=strcat(res,"(");
					type_in_words(temp->procedure_arguments_types->type,temp->procedure_arguments_types->array_size);
					res=strcat(res,buf);
					temp=temp->procedure_arguments_types;
					while(temp->procedure_arguments_types!=NULL){
						type_in_words(temp->procedure_arguments_types->type,temp->procedure_arguments_types->array_size);
						res=strcat(res,",");
						res=strcat(res,buf);
						temp=temp->procedure_arguments_types;
					}
					res=strcat(res,") ");
				}
				printf("%-40s ",res);
				strcpy(res,"\0");
				printf("%-4d | ",p->line_number_of_declaration);
				if (p->line_numbers_of_references!=NULL){
					printf("%d",p->line_numbers_of_references->reference_line_number);
					for(reference=p->line_numbers_of_references->next_line_number;reference!=NULL;reference=reference->next_line_number){
						printf(",%d",reference->reference_line_number);
					}
				}
				printf("\n");
			}else{
				if (strcmp(p->procedure_name,"\0")!=0){
					sprintf(oname1,"%s:%s",p->name,p->procedure_name);
				}else{
					strcpy(oname1,p->name);
				}
				printf("%-15s ",oname1);
				type_in_words(p->type->type,p->type->array_size);
				printf("%-40s ",buf);
				printf("%-4d | ",p->line_number_of_declaration);
				if (p->line_numbers_of_references!=NULL){
					printf("%d",p->line_numbers_of_references->reference_line_number);
					for(reference=p->line_numbers_of_references->next_line_number;reference!=NULL;reference=reference->next_line_number){
						printf(",%d",reference->reference_line_number);
					}
				}
				printf("\n");
			}
			p=p->next_name;
		}
	}else{
		printf("Cross-reference table is empty.");
	}



	//end of merged crossreference table

}

