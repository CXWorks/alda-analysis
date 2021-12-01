%option noyywrap
%{
#include "parser.hpp"
#include "generator.h"
#include <string.h>
int mylineno = 0;
#undef YY_DECL
#define YY_DECL int yylex (YYSTYPE*)
string* now;

int left_counter = 0;

unordered_map<string, int>* keywords ;
unordered_map<string, VariableDefinition*>* variables ;
unordered_map<string, FunctionDefinition*>* functions ;
unordered_map<string, int>* cpp_keywords ;
string* operators ;

extern void parseAST(RootNode*);


void count(){cout<<yytext<<endl;}

static unordered_map<string, int>& init_keywords(){
    variables = new unordered_map<string, VariableDefinition*>();
    functions = new unordered_map<string, FunctionDefinition*>();
    operators = nullptr;
    keywords = new unordered_map<string, int >();
    keywords->insert(make_pair("insert", 261));
    keywords->insert(make_pair("func", 262));
    keywords->insert(make_pair("before", 265));
    keywords->insert(make_pair("after", 266));
    keywords->insert(make_pair("call", 267));
    keywords->insert(make_pair("(", 268));
    keywords->insert(make_pair(")", 269));
    keywords->insert(make_pair("[", 270));
    keywords->insert(make_pair("]", 271));
    keywords->insert(make_pair("{", 272));
    keywords->insert(make_pair("}", 273));
    keywords->insert(make_pair("map", 275));
    keywords->insert(make_pair("set", 276));
    keywords->insert(make_pair("universe", 277));
    keywords->insert(make_pair("bottom", 278));
    keywords->insert(make_pair("sizeof", 279));
    keywords->insert(make_pair("sync", 281));

    cpp_keywords = new unordered_map<string, int>();
    cpp_keywords->insert(make_pair("(", 268));
    cpp_keywords->insert(make_pair(")", 269));
    cpp_keywords->insert(make_pair("[", 270));
    cpp_keywords->insert(make_pair("]", 271));
    cpp_keywords->insert(make_pair("{", 272));
    cpp_keywords->insert(make_pair("}", 273));
    cpp_keywords->insert(make_pair("if", 374));
    cpp_keywords->insert(make_pair("else", 375));
    cpp_keywords->insert(make_pair("for", 376));
    cpp_keywords->insert(make_pair("while", 377));
    cpp_keywords->insert(make_pair(";", 378));
    cpp_keywords->insert(make_pair("return", 380));

    return *keywords;
}
%}

%x C_COMMENT
%x C_FUNCTION

string  \"[^\n"]+\"

ws      [ \t]+

alpha   [A-Za-z_]
dig     [0-9]
name    ({alpha}|{dig}|\$)({alpha}|{dig}|[_.\-/$])*
func_call_name    ({alpha}|{dig}|[\$\.])({alpha}|{dig}|[_.\-/$])*

num1    [-+]?{dig}+\.?([eE][-+]?{dig}+)?
num2    [-+]?{dig}*\.{dig}+([eE][-+]?{dig}+)?
number  {num1}|{num2}
bracket (\(|\[|\{|\}|\)|\]|\})

D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)
IS			(u|U|l|L)*


%%

{ws}    /* skip blanks and tabs */

"="     {cout<<"SYMBOL =  "<<yytext<<endl; return TOK_EQUAL;}

":="     {cout<<"DEFINE "<<yytext<<endl; return TOK_DEFINE;}

","         {cout<<"COMMA "<<yytext<<endl; return TOK_COMMA;}

":"         {cout<<"Define "<<yytext<<endl; return TOK_LIMIT;}

"::"            { return TOK_NAMESPACE; }

"/*"            { BEGIN(C_COMMENT); }

"//".*          { /* DO NOTHING */ }

"{"            {
                        BEGIN(C_FUNCTION);
                        left_counter = 1;
                        return 272;
                 }

<C_FUNCTION>"}"(\r\n|\n|\r)            {
                        left_counter--;
                        if(left_counter == 0){
                        BEGIN(INITIAL);
                        return 273;
                        }else
                        {
                            return('}');
                        }
                 }
<C_FUNCTION>0[xX]{H}+{IS}?		{ count(); return(CONSTANT); }
<C_FUNCTION>0{D}+{IS}?		{ count(); return(CONSTANT); }
<C_FUNCTION>{D}+{IS}?		{ count(); return(CONSTANT); }
<C_FUNCTION>L?'(\\.|[^\\'])+'	{ count(); return(CONSTANT); }

<C_FUNCTION>{D}+{E}{FS}?		{ count(); return(CONSTANT); }
<C_FUNCTION>{D}*"."{D}+({E})?{FS}?	{ count(); return(FLOAT_CONSTANT); }
<C_FUNCTION>{D}+"."{D}*({E})?{FS}?	{ count(); return(FLOAT_CONSTANT); }
<C_FUNCTION>L?\"(\\.|[^\\"])*\"	{ count(); cout<<yytext<<" string literal"<<endl; return(STRING_LITERAL); }
<C_FUNCTION>">>="			{ count(); return(RIGHT_ASSIGN); }
<C_FUNCTION>"<<="			{ count(); return(LEFT_ASSIGN); }
<C_FUNCTION>"+="			{ count(); return(ADD_ASSIGN); }
<C_FUNCTION>"-="			{ count(); return(SUB_ASSIGN); }
<C_FUNCTION>"*="			{ count(); return(MUL_ASSIGN); }
<C_FUNCTION>"/="			{ count(); return(DIV_ASSIGN); }
<C_FUNCTION>"%="			{ count(); return(MOD_ASSIGN); }
<C_FUNCTION>"&="			{ count(); return(AND_ASSIGN); }
<C_FUNCTION>"^="			{ count(); return(XOR_ASSIGN); }
<C_FUNCTION>"|="			{ count(); return(OR_ASSIGN); }
<C_FUNCTION>">>"			{ count(); return(RIGHT_OP); }
<C_FUNCTION>"<<"			{ count(); return(LEFT_OP); }
<C_FUNCTION>"++"			{ count(); return(INC_OP); }
<C_FUNCTION>"--"			{ count(); return(DEC_OP); }
<C_FUNCTION>"->"			{ count(); return(PTR_OP); }
<C_FUNCTION>"&&"			{ count(); return(AND_OP); }
<C_FUNCTION>"||"			{ count(); return(OR_OP); }
<C_FUNCTION>"<="			{ count(); return(LE_OP); }
<C_FUNCTION>">="			{ count(); return(GE_OP); }
<C_FUNCTION>"=="			{ count(); return(EQ_OP); }
<C_FUNCTION>"!="			{ count(); return(NE_OP); }
<C_FUNCTION>";"			{ count(); return(';'); }
<C_FUNCTION>"{"		{ count(); left_counter++; return('{'); }
<C_FUNCTION>"}"		{ count(); left_counter--; return('}'); }
<C_FUNCTION>","			{ count(); return(','); }
<C_FUNCTION>":"			{ count(); return(':'); }
<C_FUNCTION>"="			{ count(); return('='); }
<C_FUNCTION>"("			{ count(); return('('); }
<C_FUNCTION>")"			{ count();  return(')'); }
<C_FUNCTION>"["		{ count(); return('['); }
<C_FUNCTION>"]"		{ count(); return(']'); }
<C_FUNCTION>"."			{ count(); return('.'); }
<C_FUNCTION>"&"			{ count(); return('&'); }
<C_FUNCTION>"!"			{ count(); return('!'); }
<C_FUNCTION>"~"			{ count(); return('~'); }
<C_FUNCTION>"-"			{ count(); return('-'); }
<C_FUNCTION>"+"			{ count(); return('+'); }
<C_FUNCTION>"*"			{ count(); return('*'); }
<C_FUNCTION>"/"			{ count(); return('/'); }
<C_FUNCTION>"%"			{ count(); return('%'); }
<C_FUNCTION>"<"			{ count(); return('<'); }
<C_FUNCTION>">"			{ count(); return('>'); }
<C_FUNCTION>"^"			{ count(); return('^'); }
<C_FUNCTION>"|"			{ count(); return('|'); }
<C_FUNCTION>"?"			{ count(); return('?'); }

<C_FUNCTION>"if"			{ count(); return(IF); }
<C_FUNCTION>"else"			{ count(); return(ELSE); }
<C_FUNCTION>"return"			{ count(); return(RETURN); }

<C_FUNCTION>{alpha}({alpha}|{dig})*		{  cout<<"id: "<<yytext<<endl; now = new string(yytext); return(IDENTIFIER); }

<C_COMMENT>"*/" { BEGIN(INITIAL); }

<C_COMMENT>\n   { }



<C_COMMENT>.    { }

(\r\n)          {mylineno++; cout<<"EOL"<<endl;};

(\n|\r)        {mylineno++; cout<<"EOL"<<endl;};

{number} {cout<<"NUMBER "<<yytext<<endl; now = new string(yytext); return 291;}

{name}    {
if(keywords->find(yytext)!=keywords->end()){
            cout<<"KEYWORD "<<yytext<<endl;
            return keywords->find(yytext)->second;
}else

cout<<"NAME "<<yytext<<endl;
return STRING;
}



{bracket}   {cout<<"BRACKET "<<yytext<<endl; return keywords->find(yytext)->second;}

","         {cout<<"COMMA "<<yytext<<endl; return TOK_COMMA;}

"."         {cout<<"COMMA "<<yytext<<endl; return TOK_POINT;}

.           {cout<<"SYMBOL_GENERAL "<<yytext<<endl;}



%%

FILE * handle_include(char* param){
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    char include_name[255];
    memset(include_name, 0 ,255);

    fp = fopen(param, "r");
    if (fp == NULL)
        exit(-1);

    while ((read = getline(&line, &len, fp)) != -1) {

        if(strstr(line, "#include ")){
            int len_parent = strlen(param);
            int i=len_parent-1;
            for(;i>0;i--){
                if(param[i] == '/')
                    break;
            }
            int j = 0;
            while(line[j]!='"')
                j++;
            int m = j+1;
            while(line[m]!='"')
                m++;
            int t = i + m;
            strncpy(include_name, param, i+1);
            strncat(include_name, &line[j+1], m-j-1);
            break;
        }
    }
    fseek(fp, 0, SEEK_SET);
    if(strlen(include_name) != 0){

        FILE * nfp = fopen("/tmp/alda_tmp.txt", "w+");
        while ((read = getline(&line, &len, fp)) != -1) {
            if(strstr(line, "#include ")){
                FILE * include_file = fopen(include_name, "r");
                if (include_file == NULL)
                    exit(-1);
                while ((read = getline(&line, &len, include_file)) != -1) {
                    fputs(line, nfp);
                }
                fclose(include_file);
            }
            else{
                printf("%s\n", line);
                fputs(line, nfp);
            }
        }
        fseek(nfp, 0, SEEK_SET);
        fclose(fp);
        return nfp;
    } else{
        return fp;
    }
}

int main(int argc, char** argv)
{
#ifdef YYDEBUG
#endif

    if(argc > 1){
        yyin = handle_include(argv[1]);

    }
    if(argc>2)
        shift = atoi(argv[2]);
    init_keywords();
    RootNode* root = new RootNode();
    yyparse(root);

    parse_eliminate_copy_constructor(root);
    map_fusion(root);
    map_lookup_elimination(root);
    parseAST2opt(root);
    parseAST2cpp(root, shift);
    fclose(yyin);
    return 0;
}