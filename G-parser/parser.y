%define api.pure full
%parse-param {Node *parent}

%define parse.trace
%define parse.error verbose

%{
#include "parser.hpp"

void yyerror (Node* node, char const *msg);

int yylex (YYSTYPE * node);

extern unordered_map<string, int>* keywords ;

extern unordered_map<string, VariableDefinition*>* variables ;

extern unordered_map<string, FunctionDefinition*>* functions ;

extern string* operators ;

extern char * yytext;
extern string* now;
%}

%code requires
{
#include "statement.h"


}

%define api.value.type union /* Generate YYSTYPE from these types:  */
%token <const char *>    NUMBER  291   "number"
%token <const char *>   STRING  292   "string"
%token <string*>   FUNC_LINE   293  "func_line"
%token <string*> TOK_CPP_OPERATOR  294   "cpp_operator"


%token TOK_EOF 0 "EOF"
%token TOK_INSERT 261  "insert"
%token TOK_FUNC 262  "func"
%token TOK_BEFORE 265  "before"
%token TOK_AFTER  266  "after"
%token TOK_CALL  267   "call"
%token TOK_LPARAN 268 "("
%token TOK_RPARAN 269 ")"
%token TOK_LBRACKET 270 "["
%token TOK_RBRACKET 271 "]"
%token TOK_LBPARAN 272 "{"
%token TOK_RBPARAN 273 "}"
%token TOK_NAMESPACE 274 "::"

%token TOK_MAP 275 "map"
%token TOK_SET 276 "set"
%token TOK_UNIVERSE 277 "universe"
%token TOK_BOTTOM 278 "bottom"
%token TOK_SIZEOF 279  "sizeof"
%token TOK_SYNC 281 "sync"
%token TOK_FUNC_LBPARAN "{"
%token TOK_FUNC_RBPARAN "}"

%token IF ELSE RETURN
%token IDENTIFIER CONSTANT STRING_LITERAL SIZEOF FLOAT_CONSTANT
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID


%token TOK_LIMIT     ":"
%token TOK_DEFINE     ":="
%token TOK_EQUAL     "="
%token TOK_COMMA     ","
%token TOK_POINT     "."

%type <Node*> Stmts
%type <Node*> Stmt
%type <VariableDefinition*> TyDef
%type <FunctionDefinition*> FuncDef
%type <VariableDefinition*> VarDeclare
%type <vector<FunctionParamDefinition*>*>      FuncParams
%type  <string*> VarName
%type  <string*> VarType
%type  <string*> VarDescriptor
%type <InsertStatement*> InsertDef
%type <CombinedStatements*>     FuncStmtList
%type <Statement*>   statement
%type <vector<string*>*> InsertFuncParams
%type <CompoundStatement*>    compound_statement
%type <ExpressionStatement*>    expression_statement
%type <IfStatement*>        selection_statement
%type <ReturnStatement*>        return_statement
%type <Expression*> expression
%type <AssignmentExpression*> assignment_expression
%type <string*> assignment_operator
%type <ConditionalExpression*>        conditional_expression
%type <LogicalOrExpression*>    logical_or_expression
%type <LogicalAndExpression*>    logical_and_expression
%type <InclusiveOrExpression*>  inclusive_or_expression
%type <ExclusiveOrExpression*>        exclusive_or_expression
%type <AndExpression*>    and_expression
%type <EqualityExpression*>    equality_expression
%type <RelationalExpression*>   relational_expression
%type <ShiftExpression*> shift_expression
%type <AdditiveExpression*> additive_expression
%type <MultiplicativeExpression*> multiplicative_expression
%type <CastExpression*> cast_expression
%type <string*> unary_operator;
%type <UnaryExpression*> unary_expression
%type <ArgumentExpressionList*> argument_expression_list
%type <PostfixExpression*> postfix_expression
%type <PrimaryExpression*> primary_expression
%type <string*> IDENTIFIER
%type <string*> CONSTANT
%type <string*> FLOAT_CONSTANT
%type <string*> STRING_LITERAL

%%
%start Stmts;

Stmts:
	Stmt            {
	        RootNode* root = (RootNode*)parent;
            root->addStmt($1);
            $$ = $1;
	}
	| Stmts Stmt    {
	    RootNode* root = (RootNode*)parent;
	    root->addStmt($2);
	    $$ = $1;
	}
;
VarName:
    STRING        {
                        $$ = new string(yytext);
                    }
;
Stmt:
    TyDef          { printf("vvv\n"); }
    | FuncDef       { cout<<"SYMBOL "<<$1<<endl; };
    | InsertDef     {}
    | VarDeclare
;

VarType:
    STRING        {
                        $$ = new string(yytext);
                    }
;

VarDescriptor:
    /* empty */       {
            $$ = nullptr;
        }
    | "sync"   {
            $$ = new string(yytext);
    }
    | "number" {
            $$ = new string(yytext);
    }
;

TyDef:

    VarName ":="  VarName {

                                BuiltinCPPType* inst = new BuiltinCPPType( $3);
                                VariableDefinition* varDef = new VariableDefinition($1, inst, true);
                                variables->insert(make_pair(*$1, varDef));
                                $$ = varDef;
    }
    | VarName ":="  VarName ":" "number" {

                                BuiltinCPPType* inst = new BuiltinCPPType( $3, now);
                                VariableDefinition* varDef = new VariableDefinition($1, inst, true);
                                variables->insert(make_pair(*$1, varDef));
                                $$ = varDef;
    }
    | VarName ":="  VarName ":" "sync" {

                                BuiltinCPPType* inst = new BuiltinCPPType( $3, true);
                                VariableDefinition* varDef = new VariableDefinition($1, inst, true);
                                variables->insert(make_pair(*$1, varDef));
                                $$ = varDef;
    }
    | VarName ":="  VarName ":" "sync" "number" {

                                BuiltinCPPType* inst = new BuiltinCPPType( $3, now, true);
                                VariableDefinition* varDef = new VariableDefinition($1, inst, true);
                                variables->insert(make_pair(*$1, varDef));
                                $$ = varDef;
    }
;

FuncDef:
    VarName "(" FuncParams ")" "{" FuncStmtList "}" {
        FunctionType* funcType = new FunctionType($1, $1);
        FunctionDefinition* funcDef = new FunctionDefinition($1, funcType, false, $3, $6);
        functions->insert(make_pair(*$1, funcDef));
        $$ = funcDef;
    }
    |     VarName VarName "(" FuncParams ")" "{" FuncStmtList "}" {
              FunctionType* funcType = new FunctionType($2, $2);
              VariableDefinition* varDef = variables->find(*$1)->second;
              FunctionDefinition* funcDef = new FunctionDefinition($2, funcType, false, $4, $7, varDef);
              functions->insert(make_pair(*$2, funcDef));
              $$ = funcDef;
    }
    |VarName "(" FuncParams "," "." "." "." ")" "{" FuncStmtList "}" {
         FunctionType* funcType = new FunctionType($1, $1);
         FunctionDefinition* funcDef = new FunctionDefinition($1, funcType, false, $3, $10);
         funcDef->set_var_arg(true);
         functions->insert(make_pair(*$1, funcDef));
         $$ = funcDef;
    }
    |     VarName VarName "(" FuncParams "," "." "." "." ")"  "{" FuncStmtList "}" {
               FunctionType* funcType = new FunctionType($2, $2);
               VariableDefinition* varDef = variables->find(*$1)->second;
               FunctionDefinition* funcDef = new FunctionDefinition($2, funcType, false, $4, $11, varDef);
               funcDef->set_var_arg(true);
               functions->insert(make_pair(*$2, funcDef));
               $$ = funcDef;
    }
;
FuncParams
    : /* empty */       {
        $$ = new vector<FunctionParamDefinition*>();
    }
    | VarName           {
        vector<FunctionParamDefinition*>* ans = new vector<FunctionParamDefinition*>();
        FunctionParamDefinition* funcParam = new FunctionParamDefinition(variables->find(*$1)->second, nullptr);
        ans->push_back(funcParam);
        $$ = ans;
    }
    | VarName VarName           {
        vector<FunctionParamDefinition*>* ans = new vector<FunctionParamDefinition*>();
        FunctionParamDefinition* funcParam = new FunctionParamDefinition(variables->find(*$1)->second, $2);
        ans->push_back(funcParam);
        $$ = ans;
    }
    | FuncParams "," VarName    {
        FunctionParamDefinition* funcParam = new FunctionParamDefinition(variables->find(*$3)->second, nullptr);
        $1->push_back(funcParam);
        $$ = $1;
    }
    | FuncParams "," VarName  VarName  {
        FunctionParamDefinition* funcParam = new FunctionParamDefinition(variables->find(*$3)->second, $4);
        $1->push_back(funcParam);
        $$ = $1;
    }
;

FuncStmtList:
    statement     {
        CombinedStatements* combstmt = new CombinedStatements();
        combstmt->addStatement($1);
        $$ = combstmt;
    }
    | FuncStmtList statement  {
        $1->addStatement($2);
        $$ = $1;
    }
;

statement
	: compound_statement { $$ = $1; }
	| expression_statement { $$ = $1; }
	| selection_statement { $$ = $1; }
	| return_statement { $$ = $1; }
	;

expression
	: assignment_expression {
	    Expression * expr = new Expression();
	    expr->addExprs($1);
	    $1->setParent(expr);
	    $$ = expr;
	}
	| expression ',' assignment_expression {
	    $1->addExprs($3);
	    $3->setParent($1);
        $$ = $1;
	}
	;
return_statement
    : RETURN expression ';' {
        ReturnStatement* return_statement = new ReturnStatement();
        return_statement->setExpression($2);
        $$ = return_statement;
    }
    | RETURN ';' {
        ReturnStatement* return_statement = new ReturnStatement();
        $$ = return_statement;
    }
;
compound_statement
	: '{' '}' { $$ = new CompoundStatement(); }
	| '{' FuncStmtList '}' {
	    CompoundStatement* cstmt = new CompoundStatement();
	    for(Statement* stmt : $2->getStatements()){
	        cstmt->addStatement(stmt);
	        stmt->setParent(cstmt);
	    }
	    $$=cstmt;
	}
	| '{' statement '}' {
	    CompoundStatement* cstmt = new CompoundStatement();
	    cstmt->addStatement($2);
	    $2->setParent(cstmt);
	    $$=cstmt;
	}
	;

expression_statement
	: ';' { $$ = new ExpressionStatement(); }
	| expression ';' {
	    ExpressionStatement* exprStmt = new ExpressionStatement();
	    exprStmt->setExpression($1);
	    $1->setParent(exprStmt);
	    $$=exprStmt;
	}
	;


primary_expression
	: IDENTIFIER {
	    PrimaryExpression* primaryExpr = new PrimaryExpression();
	    primaryExpr->setIdentifier(now);
	    primaryExpr->setContent(1);
	    $$ = primaryExpr;
	}
	| FLOAT_CONSTANT {
	    PrimaryExpression* primaryExpr = new PrimaryExpression();
	    primaryExpr->setIdentifier(new string(yytext));
	    primaryExpr->setContent(2);
	    $$ = primaryExpr;
	}
	| CONSTANT {
	    PrimaryExpression* primaryExpr = new PrimaryExpression();
	    primaryExpr->setIdentifier(new string(yytext));
	    primaryExpr->setContent(3);
	    $$ = primaryExpr;
	}
	| STRING_LITERAL {
	    PrimaryExpression* primaryExpr = new PrimaryExpression();
	    primaryExpr->setIdentifier(new string(yytext));
	    primaryExpr->setContent(4);
	    $$ = primaryExpr;
	}
	| '(' expression ')' {
	    PrimaryExpression* primaryExpr = new PrimaryExpression();
	    primaryExpr->setExpr($2);
	    $2->setParent(primaryExpr);
	    $$ = primaryExpr;
	    cout<<yytext<<" (identif) "<<$2->toString()<<endl;
	}
	;

postfix_expression
	: primary_expression {
	    PostfixExpression* postExpr = new PostfixExpression();
	    postExpr->setPrimaryExpression($1);
	    $1->setParent(postExpr);
	    $$ = postExpr;
	}
	| postfix_expression '[' expression ']' {
	    PostfixExpression* postExpr = new PostfixExpression();
	    postExpr->setPostfixExpression($1);
	    $1->setParent(postExpr);
	    postExpr->setExpression($3);
	    $3->setParent(postExpr);
	    $$ = postExpr;
	}
	| postfix_expression '(' ')' {
	    PostfixExpression* postExpr = new PostfixExpression();
	    postExpr->setPostfixExpression($1);
	    $1->setParent(postExpr);
	    $$ = postExpr;
	}
	| postfix_expression '(' argument_expression_list ')' {
	    PostfixExpression* postExpr = new PostfixExpression();
	    postExpr->setPostfixExpression($1);
	    $1->setParent(postExpr);
	    postExpr->setArgumentExpressionList($3);
	    $3->setParent(postExpr);
	    $$ = postExpr;
	}
	| postfix_expression '.' IDENTIFIER {
	    PostfixExpression* postExpr = new PostfixExpression();
	    postExpr->setPostfixExpression($1);
	    $1->setParent(postExpr);
	    postExpr->setOp(new string("."));
	    postExpr->setIdentifier(new string(yytext));
	    $$ = postExpr;
	}
	| postfix_expression PTR_OP IDENTIFIER {
	    PostfixExpression* postExpr = new PostfixExpression();
	    postExpr->setPostfixExpression($1);
	    $1->setParent(postExpr);
	    postExpr->setOp(new string("*"));
	    postExpr->setIdentifier(new string(yytext));
	    $$ = postExpr;
	}
	| postfix_expression INC_OP {
	    PostfixExpression* postExpr = new PostfixExpression();
	    postExpr->setPostfixExpression($1);
	    $1->setParent(postExpr);
	    postExpr->setOp(new string("++"));
	    $$ = postExpr;
	}
	| postfix_expression DEC_OP {
	    PostfixExpression* postExpr = new PostfixExpression();
	    postExpr->setPostfixExpression($1);
	    $1->setParent(postExpr);
	    postExpr->setOp(new string("--"));
	    $$ = postExpr;
	}
	;

argument_expression_list
	: assignment_expression {
	    ArgumentExpressionList* argList = new ArgumentExpressionList();
	    argList->addStatements($1);
	    $1->setParent(argList);
	    $$ = argList;
	}
	| argument_expression_list ',' assignment_expression {
	    $1->addStatements($3);
	    $3->setParent($1);
	    $$ = $1;
	}
	;

unary_expression
	: postfix_expression {
	    UnaryExpression* unaryExpr = new UnaryExpression();
	    unaryExpr->setPostfixExpression($1);
	    $1->setParent(unaryExpr);
	    $$ = unaryExpr;
	}
	| INC_OP unary_expression {
	    UnaryExpression* unaryExpr = new UnaryExpression();
	    unaryExpr->setUnaryExpression($2);
	    $2->setParent(unaryExpr);
	    unaryExpr->setOp(new string("++"));
	    $$ = unaryExpr;
	}
	| DEC_OP unary_expression {
	    UnaryExpression* unaryExpr = new UnaryExpression();
	    unaryExpr->setUnaryExpression($2);
	    $2->setParent(unaryExpr);
	    unaryExpr->setOp(new string("--"));
	    $$ = unaryExpr;
	}
	| unary_operator cast_expression {
	    UnaryExpression* unaryExpr = new UnaryExpression();
	    unaryExpr->setCastExpression($2);
	    $2->setParent(unaryExpr);
	    unaryExpr->setOp($1);
	    $$ = unaryExpr;
	}
	| SIZEOF '(' IDENTIFIER ')' {
	    UnaryExpression* unaryExpr = new UnaryExpression();
	    unaryExpr->setIdentifier($3);
	    $$ = unaryExpr;
	}
	;

unary_operator
	: '&' {$$ = new string("&");}
	| '*' {$$ = new string("*");}
	| '+' {$$ = new string("+");}
	| '-' {$$ = new string("-");}
	| '~' {$$ = new string("~");}
	| '!' {$$ = new string("!");}
	;

cast_expression
	: unary_expression {
	    CastExpression* castExpr = new CastExpression();
        castExpr->setUnaryExpression($1);
        $1->setParent(castExpr);
        $$ = castExpr;
	 }
	| '(' IDENTIFIER ')' cast_expression {
	    CastExpression* castExpr = new CastExpression();
	    castExpr->setCastExpression($4);
	    $4->setParent(castExpr);
	    castExpr->setIdentifier($2);
	    $$ = castExpr;
	}
	;

multiplicative_expression
	: cast_expression {
	    MultiplicativeExpression* multiExpr = new MultiplicativeExpression();
	    multiExpr->setCastExpression($1);
	    $1->setParent(multiExpr);
	    $$ = multiExpr;
	}
	| multiplicative_expression '*' cast_expression {
	    MultiplicativeExpression* multiExpr = new MultiplicativeExpression();
	    multiExpr->setMultiplicativeExpression($1);
	    $1->setParent(multiExpr);
	    multiExpr->setOp(new string("*"));
	    multiExpr->setCastExpression($3);
	    $3->setParent(multiExpr);
	    $$ = multiExpr;
	}
	| multiplicative_expression '/' cast_expression {
	    MultiplicativeExpression* multiExpr = new MultiplicativeExpression();
	    multiExpr->setMultiplicativeExpression($1);
	    $1->setParent(multiExpr);
	    multiExpr->setOp(new string("/"));
	    multiExpr->setCastExpression($3);
	    $3->setParent(multiExpr);
	    $$ = multiExpr;
	}
	| multiplicative_expression '%' cast_expression {
	    MultiplicativeExpression* multiExpr = new MultiplicativeExpression();
	    multiExpr->setMultiplicativeExpression($1);
	    $1->setParent(multiExpr);
	    multiExpr->setOp(new string("%"));
	    multiExpr->setCastExpression($3);
	    $3->setParent(multiExpr);
	    $$ = multiExpr;
	}
	;

additive_expression
	: multiplicative_expression {
	    AdditiveExpression* addExpr = new AdditiveExpression();
	    addExpr->setMultiplicativeExpression($1);
	    $1->setParent(addExpr);
	    $$ = addExpr;
	}
	| additive_expression '+' multiplicative_expression {
	    AdditiveExpression* addExpr = new AdditiveExpression();
	    addExpr->setAdditiveExpression($1);
	    $1->setParent(addExpr);
	    addExpr->setOp(new string("+"));
	    addExpr->setMultiplicativeExpression($3);
	    $3->setParent(addExpr);
	    $$ = addExpr;
	}
	| additive_expression '-' multiplicative_expression {
	    AdditiveExpression* addExpr = new AdditiveExpression();
	    addExpr->setAdditiveExpression($1);
	    $1->setParent(addExpr);
	    addExpr->setOp(new string("-"));
	    addExpr->setMultiplicativeExpression($3);
	    $3->setParent(addExpr);
	    $$ = addExpr;
	}
	;

shift_expression
	: additive_expression {
	    ShiftExpression* shiftExpr = new ShiftExpression();
	    shiftExpr->setAdditiveExpression($1);
	    $1->setParent(shiftExpr);
	    $$ = shiftExpr;
	}
	| shift_expression LEFT_OP additive_expression {
	    ShiftExpression* shiftExpr = new ShiftExpression();
	    shiftExpr->setShiftExpression($1);
	    $1->setParent(shiftExpr);
	    shiftExpr->setOp(new string("<<"));
	    shiftExpr->setAdditiveExpression($3);
	    $3->setParent(shiftExpr);
	    $$ = shiftExpr;
	}
	| shift_expression RIGHT_OP additive_expression {
	    ShiftExpression* shiftExpr = new ShiftExpression();
	    shiftExpr->setShiftExpression($1);
	    $1->setParent(shiftExpr);
	    shiftExpr->setOp(new string(">>"));
	    shiftExpr->setAdditiveExpression($3);
	    $3->setParent(shiftExpr);
	    $$ = shiftExpr;
	}
	;

relational_expression
	: shift_expression {
	    RelationalExpression* relatExpr = new RelationalExpression();
	    relatExpr->setShiftExpression($1);
	    $1->setParent(relatExpr);
	    $$ = relatExpr;
	}
	| relational_expression '<' shift_expression {
	    RelationalExpression* relatExpr = new RelationalExpression();
	    relatExpr->setRelationalExpression($1);
	    $1->setParent(relatExpr);
	    relatExpr->setOp(new string("<"));
	    relatExpr->setShiftExpression($3);
	    $3->setParent(relatExpr);
	    $$ = relatExpr;
	}
	| relational_expression '>' shift_expression {
	    RelationalExpression* relatExpr = new RelationalExpression();
	    relatExpr->setRelationalExpression($1);
	    $1->setParent(relatExpr);
	    relatExpr->setOp(new string(">"));
	    relatExpr->setShiftExpression($3);
	    $3->setParent(relatExpr);
	    $$ = relatExpr;
	}
	| relational_expression LE_OP shift_expression {
	    RelationalExpression* relatExpr = new RelationalExpression();
	    relatExpr->setRelationalExpression($1);
	    $1->setParent(relatExpr);
	    relatExpr->setOp(new string("<="));
	    relatExpr->setShiftExpression($3);
	    $3->setParent(relatExpr);
	    $$ = relatExpr;
	}
	| relational_expression GE_OP shift_expression {
	    RelationalExpression* relatExpr = new RelationalExpression();
	    relatExpr->setRelationalExpression($1);
	    $1->setParent(relatExpr);
	    relatExpr->setOp(new string(">="));
	    relatExpr->setShiftExpression($3);
	    $3->setParent(relatExpr);
	    $$ = relatExpr;
	}
	;

equality_expression
	: relational_expression {
	    EqualityExpression* equalExpr = new EqualityExpression();
	    equalExpr->setRelationalExpression($1);
	    $1->setParent(equalExpr);
	    $$ = equalExpr;
	}
	| equality_expression EQ_OP relational_expression {
	    EqualityExpression* equalExpr = new EqualityExpression();
	    equalExpr->setEqualityExpression($1);
	    $1->setParent(equalExpr);
	    equalExpr->setOp(new string("=="));
	    equalExpr->setRelationalExpression($3);
	    $3->setParent(equalExpr);
	    $$ = equalExpr;
	}
	| equality_expression NE_OP relational_expression {
	    EqualityExpression* equalExpr = new EqualityExpression();
	    equalExpr->setEqualityExpression($1);
	    $1->setParent(equalExpr);
	    equalExpr->setOp(new string("!="));
	    equalExpr->setRelationalExpression($3);
	    $3->setParent(equalExpr);
	    $$ = equalExpr;
	}
	;

and_expression
	: equality_expression {
	    AndExpression* andExpr = new AndExpression();
	    andExpr->setEqualityExpression($1);
	    $1->setParent(andExpr);
	    $$ = andExpr;
	}
	| and_expression '&' equality_expression {
	    AndExpression* andExpr = new AndExpression();
	    andExpr->setAndExpression($1);
	    $1->setParent(andExpr);
	    andExpr->setEqualityExpression($3);
	    $3->setParent(andExpr);
	    $$ = andExpr;
    }
	;

exclusive_or_expression
	: and_expression {
	    ExclusiveOrExpression* exOrExpr = new ExclusiveOrExpression();
	    exOrExpr->setAndExpression($1);
	    $1->setParent(exOrExpr);
	    $$ = exOrExpr;
	}
	| exclusive_or_expression '^' and_expression {
	    ExclusiveOrExpression* exOrExpr = new ExclusiveOrExpression();
	    exOrExpr->setExclusiveOrExpression($1);
	    $1->setParent(exOrExpr);
	    exOrExpr->setAndExpression($3);
	    $3->setParent(exOrExpr);
	    $$ = exOrExpr;
	}
	;

inclusive_or_expression
	: exclusive_or_expression {
	    InclusiveOrExpression* inOrExpr = new InclusiveOrExpression();
	    inOrExpr->setExclusiveOrExpression($1);
	    $1->setParent(inOrExpr);
	    $$ = inOrExpr;
	}
	| inclusive_or_expression '|' exclusive_or_expression{
	    InclusiveOrExpression* inOrExpr = new InclusiveOrExpression();
	    inOrExpr->setInclusiveOrExpression($1);
	    $1->setParent(inOrExpr);
	    inOrExpr->setExclusiveOrExpression($3);
	    $3->setParent(inOrExpr);
	    $$ = inOrExpr;
	}
	;

logical_and_expression
	: inclusive_or_expression {
	    LogicalAndExpression* logicalAndExpr = new LogicalAndExpression();
	    logicalAndExpr->setInclusiveOrExpression($1);
	    $1->setParent(logicalAndExpr);
	    $$ = logicalAndExpr;
	}
	| logical_and_expression AND_OP inclusive_or_expression {
	    LogicalAndExpression* logicalAndExpr = new LogicalAndExpression();
	    logicalAndExpr->setLogicalAndExpression($1);
	    $1->setParent(logicalAndExpr);
	    logicalAndExpr->setInclusiveOrExpression($3);
	    $3->setParent(logicalAndExpr);
	    $$ = logicalAndExpr;
	}
	;

logical_or_expression
	: logical_and_expression {
	    LogicalOrExpression* logicOrExpr = new LogicalOrExpression();
	    logicOrExpr->setLogicalAndExpression($1);
	    $1->setParent(logicOrExpr);
	    $$ = logicOrExpr;
	}
	| logical_or_expression OR_OP logical_and_expression{
	    LogicalOrExpression* logicOrExpr = new LogicalOrExpression();
	    logicOrExpr->setLogicalOrExpression($1);
	    $1->setParent(logicOrExpr);
	    logicOrExpr->setLogicalAndExpression($3);
	    $3->setParent(logicOrExpr);
	    $$ = logicOrExpr;
    }
	;

conditional_expression
	: logical_or_expression {
	    ConditionalExpression* condExpr = new ConditionalExpression();
	    condExpr->setLogicalOrExpression($1);
	    $1->setParent(condExpr);
	    $$ = condExpr;
	}
	;

assignment_expression
	: conditional_expression {
	    AssignmentExpression* assignExpr = new AssignmentExpression();
	    assignExpr->setConditionalExpression($1);
	    $1->setParent(assignExpr);
	    $$ = assignExpr;
	}
	| unary_expression assignment_operator assignment_expression {
	    AssignmentExpression* assignExpr = new AssignmentExpression();
	    assignExpr->setUnaryExpression($1);
	    $1->setParent(assignExpr);
	    assignExpr->setOp($2);
	    assignExpr->setNext($3);
	    $3->setParent(assignExpr);
	    $$ = assignExpr;
	}
	;

assignment_operator
	: '=' { $$ = new string("="); }
	| MUL_ASSIGN { $$ = new string("*="); }
	| DIV_ASSIGN { $$ = new string("/="); }
	| MOD_ASSIGN { $$ = new string("%="); }
	| ADD_ASSIGN { $$ = new string("+="); }
	| SUB_ASSIGN { $$ = new string("-="); }
	| LEFT_ASSIGN { $$ = new string("<<="); }
	| RIGHT_ASSIGN { $$ = new string(">>="); }
	| AND_ASSIGN { $$ = new string("&="); }
	| XOR_ASSIGN { $$ = new string("^="); }
	| OR_ASSIGN { $$ = new string("|="); }
	;


selection_statement
	: IF '(' expression ')' statement {
	    IfStatement* ifStmt = new IfStatement();
	    if(auto tc = dynamic_cast<CompoundStatement*>($5)){
	        ifStmt->setT_case(tc);
	        $5->setParent(ifStmt);
	    }else{
	        CompoundStatement* tmp = new CompoundStatement();
	        tmp->addStatement($5);
	        $5->setParent(tmp);
	        ifStmt->setT_case(tmp);
	        tmp->setParent(ifStmt);
	    }
	    ifStmt->setExpression($3);
	    $3->setParent(ifStmt);
	    $$=ifStmt;
	}
	| IF '(' expression ')' statement ELSE statement {
	    IfStatement* ifStmt = new IfStatement();
	    ifStmt->setExpression($3);
	    $3->setParent(ifStmt);
	    if(auto tc = dynamic_cast<CompoundStatement*>($5)){
	        ifStmt->setT_case(tc);
	        $5->setParent(ifStmt);
	    }else{
	        CompoundStatement* tmp = new CompoundStatement();
	        tmp->addStatement($5);
	        $5->setParent(tmp);
	        ifStmt->setT_case(tmp);
	        tmp->setParent(ifStmt);
	    }
	    if(auto tc = dynamic_cast<CompoundStatement*>($7)){
	        ifStmt->setF_case(tc);
	        $7->setParent(ifStmt);
	    }else{
	        CompoundStatement* tmp = new CompoundStatement();
	        tmp->addStatement($7);
	        $7->setParent(tmp);
	        ifStmt->setF_case(tmp);
	        tmp->setParent(ifStmt);
	    }
	    $$=ifStmt;
	}
	;

VarDeclare:
    VarName { $$ = variables->find(*$1)->second; }
    | VarName "=" VarDeclare {
        VariableDefinition* varDef = new VariableDefinition($1, $3->getType());
        variables->insert(make_pair(*$1, varDef));
        $$ = varDef;
    }
    | VarName "=" "map" "(" VarDeclare "," VarDeclare ")"       {
        string* scope = new string("bottom");
        BuiltinMap* builtinMap = new BuiltinMap($1, *scope, $5, $7);

        VariableDefinition* varDef = new VariableDefinition($1, builtinMap);
        variables->insert(make_pair(*$1, varDef));
        $5->setParent(varDef);
        $7->setParent(varDef);
        $$ = varDef;
    }
    | VarName "=" "universe" "::" "map" "(" VarDeclare "," VarDeclare ")"   {
        string* scope = new string("universe");
        BuiltinMap* builtinMap = new BuiltinMap($1, *scope, $7, $9);
                VariableDefinition* varDef = new VariableDefinition($1, builtinMap);
                variables->insert(make_pair(*$1, varDef));
                $7->setParent(varDef);
                $9->setParent(varDef);
                $$ = varDef;
    }
    | VarName "=" "bottom" "::" "map" "(" VarDeclare "," VarDeclare ")"      {
    string* scope = new string("bottom");
        BuiltinMap* builtinMap = new BuiltinMap($1, *scope, $7, $9);
                        VariableDefinition* varDef = new VariableDefinition($1, builtinMap);
                        variables->insert(make_pair(*$1, varDef));
                        $7->setParent(varDef);
                        $9->setParent(varDef);
                        $$ = varDef;
    }
    | VarName "=" "universe" "::" "set" "(" VarDeclare ")"                  {
    string* scope = new string("universe");
        BuiltinSet* builtinSet = new BuiltinSet($1, *scope, $7);
                        VariableDefinition* varDef = new VariableDefinition($1, builtinSet);
                        variables->insert(make_pair(*$1, varDef));
                        $7->setParent(varDef);
                        $$ = varDef;
    }
    | VarName "=" "bottom" "::" "set" "(" VarDeclare ")"                    {
    string* scope = new string("bottom");
        BuiltinSet* builtinSet = new BuiltinSet($1, *scope, $7);
                                VariableDefinition* varDef = new VariableDefinition($1, builtinSet);
                                variables->insert(make_pair(*$1, varDef));
                                $7->setParent(varDef);
                                $$ = varDef;
    }
    | "universe" "::" "set" "(" VarDeclare ")"  {
        string* scope = new string("universe");
                BuiltinSet* builtinSet = new BuiltinSet(nullptr, *scope, $5);
                        VariableDefinition* varDef = new VariableDefinition(nullptr, builtinSet);
                        $5->setParent(varDef);
                        $$ = varDef;
    }
    | "bottom" "::" "set" "(" VarDeclare ")"    {
        string* scope = new string("bottom");
                BuiltinSet* builtinSet = new BuiltinSet(nullptr, *scope, $5);
                        VariableDefinition* varDef = new VariableDefinition(nullptr, builtinSet);
                        $5->setParent(varDef);
                        $$ = varDef;
    }
    | "set" "(" VarDeclare ")"                  {
        string* scope = new string("bottom");
                BuiltinSet* builtinSet = new BuiltinSet(nullptr, *scope, $3);
                        VariableDefinition* varDef = new VariableDefinition(nullptr, builtinSet);
                        $3->setParent(varDef);
                        $$ = varDef;
    }
    | "map" "(" VarDeclare "," VarDeclare ")"   {
            string* scope = new string("bottom");
            BuiltinMap* builtinMap = new BuiltinMap(nullptr, *scope, $3, $5);
            VariableDefinition* varDef = new VariableDefinition(nullptr, builtinMap);
            $3->setParent(varDef);
            $5->setParent(varDef);
            $$ = varDef;
    }
    | "universe" "map" "(" VarDeclare "," VarDeclare ")"    {
            string* scope = new string("universe");
            BuiltinMap* builtinMap = new BuiltinMap(nullptr, *scope, $4, $6);
            VariableDefinition* varDef = new VariableDefinition(nullptr, builtinMap);
            $4->setParent(varDef);
            $6->setParent(varDef);
            $$ = varDef;
    }
    | "bottom" "map" "(" VarDeclare "," VarDeclare ")"      {
            string* scope = new string("bottom");
            BuiltinMap* builtinMap = new BuiltinMap(nullptr, *scope, $4, $6);
            VariableDefinition* varDef = new VariableDefinition(nullptr, builtinMap);
            $4->setParent(varDef);
            $6->setParent(varDef);
            $$ = varDef;
    }
;

InsertFuncParams
    : /* empty */       {
        $$ = new vector<string*>();
    }
    | "sizeof" "(" VarName ")"           {
            vector<string*>* ans = new vector<string*>();
            string& tmp = *$3;
            tmp += " sizeof";
            ans->push_back(&tmp);
            $$ = ans;
    }
    | VarName           {
        vector<string*>* ans = new vector<string*>();
        ans->push_back(new string(yytext));
        $$ = ans;
    }
    | InsertFuncParams "," VarName    {
        $1->push_back($3);
        $$ = $1;
    }
    | InsertFuncParams "," "sizeof" "(" VarName ")"    {
        string& tmp = *$5;
        tmp += " sizeof";
        $1->push_back(&tmp);
        $$ = $1;
    }
;

InsertDef:
    "insert" "before" VarName "call" VarName    {
            LLVMInstructionType* inst = new LLVMInstructionType($3, $3);
            LLVMInstructionDefinition * instDef = new LLVMInstructionDefinition(nullptr, inst);
            FunctionDefinition * functionDef = functions->find(*$5)->second;
            InsertStatement* insert = new InsertStatement("before", instDef, functionDef);
            if(functionDef->getReturnValue()!= nullptr){
                string* s = new string("$.m");
                insert->setReturnDef(s);
            }
            $$ = insert;
    }
    | "insert" "after" VarName "call" VarName   {
            LLVMInstructionType* inst = new LLVMInstructionType($3, $3);
            LLVMInstructionDefinition * instDef = new LLVMInstructionDefinition(nullptr, inst);
            FunctionDefinition * functionDef = functions->find(*$5)->second;
            InsertStatement* insert = new InsertStatement("after", instDef, functionDef);
            if(functionDef->getReturnValue()!= nullptr){
                string* s = new string("$.m");
                insert->setReturnDef(s);
            }
            $$ = insert;
    }
    | "insert" "before" VarName "call" VarName  "(" InsertFuncParams ")"  {
            LLVMInstructionType* inst = new LLVMInstructionType($3, $3);
            LLVMInstructionDefinition * instDef = new LLVMInstructionDefinition(nullptr, inst);
            FunctionDefinition * functionDef = functions->find(*$5)->second;
            InsertStatement* insert = new InsertStatement("before", instDef, functionDef, $7);
            if(functionDef->getReturnValue()!= nullptr){
                string* s = new string("$.m");
                insert->setReturnDef(s);
            }
            $$ = insert;
    }
    | "insert" "after" VarName "call" VarName  "(" InsertFuncParams ")" {
            LLVMInstructionType* inst = new LLVMInstructionType($3, $3);
            LLVMInstructionDefinition * instDef = new LLVMInstructionDefinition(nullptr, inst);
            FunctionDefinition * functionDef = functions->find(*$5)->second;
            InsertStatement* insert = new InsertStatement("after", instDef, functionDef, $7);
            if(functionDef->getReturnValue()!= nullptr){
                string* s = new string("$.m");
                insert->setReturnDef(s);
            }
            $$ = insert;
    }
    | "insert" "before" "func" VarName "call" VarName   {
            LLVMInstructionType* inst = new LLVMInstructionType($4, $4);
            ExternalFunctionDefinition * instDef = new ExternalFunctionDefinition(nullptr, inst);
            FunctionDefinition * functionDef = functions->find(*$6)->second;
            InsertStatement* insert = new InsertStatement("before", instDef, functionDef);
            if(functionDef->getReturnValue()!= nullptr){
                string* s = new string("$.m");
                insert->setReturnDef(s);
            }
            $$ = insert;
    }
    | "insert" "after" "func" VarName "call" VarName  {
            LLVMInstructionType* inst = new LLVMInstructionType($4, $4);
            ExternalFunctionDefinition * instDef = new ExternalFunctionDefinition(nullptr, inst);
            FunctionDefinition * functionDef = functions->find(*$6)->second;
            InsertStatement* insert = new InsertStatement("after", instDef, functionDef);
            if(functionDef->getReturnValue()!= nullptr){
                string* s = new string("$.m");
                insert->setReturnDef(s);
            }
            $$ = insert;
    }
    | "insert" "before" "func" VarName "call" VarName "(" InsertFuncParams ")"   {
            LLVMInstructionType* inst = new LLVMInstructionType($4, $4);
            ExternalFunctionDefinition * instDef = new ExternalFunctionDefinition(nullptr, inst);
            FunctionDefinition * functionDef = functions->find(*$6)->second;
            InsertStatement* insert = new InsertStatement("before", instDef, functionDef, $8);
            if(functionDef->getReturnValue()!= nullptr){
                string* s = new string("$.m");
                insert->setReturnDef(s);
            }
            $$ = insert;
    }
    | "insert" "after" "func" VarName "call" VarName "(" InsertFuncParams ")" {
            LLVMInstructionType* inst = new LLVMInstructionType($4, $4);
            ExternalFunctionDefinition * instDef = new ExternalFunctionDefinition(nullptr, inst);
            FunctionDefinition * functionDef = functions->find(*$6)->second;
            InsertStatement* insert = new InsertStatement("after", instDef, functionDef, $8);
            if(functionDef->getReturnValue()!= nullptr){
                string* s = new string("$.m");
                insert->setReturnDef(s);
            }
            $$ = insert;
    }

;
%%
void yyerror (Node* node, char const *msg) {
	fprintf(stderr, "--> %s\n", msg);
}

