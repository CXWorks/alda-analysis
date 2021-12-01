//
// Created by cxworks on 19-5-26.
//

#ifndef G_PARSER_STATEMENT_H
#define G_PARSER_STATEMENT_H

#include <unordered_map>
#include <cstring>
#include <iostream>
#include <vector>
#include <queue>
#include <stdio.h>

using namespace std;

class VariableDefinition;

class Expression;

class AssignmentExpression;

class CastExpression;

class Type;

class FunctionParamDefinition;

class Node {
protected:
    Node *parent;
public:
    bool isRoot() {
        return parent == nullptr;
    }

    Node *getParent();

    void setParent(Node *parent) {
        this->parent = parent;
    }

    Node() {

    }

    virtual ~Node() = default;
};


class G_Context {
private:
    unordered_map<string, Type *> scalar_type;
public:
    G_Context();

    G_Context(G_Context &another) {
        this->scalar_type = another.scalar_type;
    }

    unordered_map<string, Type *> &getScalar_type();

    Type *query_scalar_type(string scalar);

    Type *query_method_type(string scalar);

    void setScalar_type(const unordered_map<string, Type *> &scalar_type);
};

class FunctionBodyNode : public Node {
public:
    virtual string &toString() = 0;
};


class FunctionBodyStatement : public FunctionBodyNode {
public:
    virtual string &toString() = 0;
};

class FunctionBodyExpression : public FunctionBodyNode {
public:
    virtual Type *resolve_type(G_Context &context) {
        return nullptr;
    };

    virtual string &toString() = 0;
};

class Statement : public FunctionBodyStatement {
public:
    virtual string &toString() = 0;
};

class CombinedStatements : public FunctionBodyStatement {
private:
    vector<Statement *> statements;
public:
    void addStatement(Statement *stmt) {
        statements.push_back(stmt);
    }

    vector<Statement *> &getStatements() {
        return statements;
    }

    string &toString();
};


class MultiplicativeExpression : public FunctionBodyExpression {
private:
    CastExpression *castExpression{};
    MultiplicativeExpression *multiplicativeExpression{};
    string *op{};
public:

    CastExpression *getCastExpression() const;

    MultiplicativeExpression *getMultiplicativeExpression() const;

    string *getOp() const;

    void setCastExpression(CastExpression *castExpression);


    void setMultiplicativeExpression(MultiplicativeExpression *multiplicativeExpression);


    void setOp(string *op);

    bool operator==(MultiplicativeExpression &another);

    bool isCastExpr() {
        return castExpression != nullptr && op == nullptr && multiplicativeExpression == nullptr;
    }

    bool isMultiExpr() {
        return castExpression != nullptr && op != nullptr && multiplicativeExpression != nullptr;
    }

    Type *resolve_type(G_Context &context);

    string &toString();
};

class AdditiveExpression : public FunctionBodyExpression {
private:
    MultiplicativeExpression *multiplicativeExpression{};
    AdditiveExpression *additiveExpression{};
    string *op{};
public:

    void setMultiplicativeExpression(MultiplicativeExpression *multiplicativeExpression);

    void setAdditiveExpression(AdditiveExpression *additiveExpression);

    MultiplicativeExpression *getMultiplicativeExpression() const;

    AdditiveExpression *getAdditiveExpression() const;

    string *getOp() const;

    void setOp(string *op);

    bool operator==(AdditiveExpression &another);

    bool isAdditiveExpr() {
        return multiplicativeExpression != nullptr && op != nullptr && additiveExpression != nullptr;
    }

    bool isMultiExpr() {
        return multiplicativeExpression != nullptr && op == nullptr && additiveExpression == nullptr;
    }

    string &toString();

    Type *resolve_type(G_Context &context);
};

class ShiftExpression : public FunctionBodyExpression {
private:
    AdditiveExpression *additiveExpression{};
    ShiftExpression *shiftExpression{};
    string *op{};
public:

    void setAdditiveExpression(AdditiveExpression *additiveExpression);


    void setShiftExpression(ShiftExpression *shiftExpression);

    AdditiveExpression *getAdditiveExpression() const;

    ShiftExpression *getShiftExpression() const;

    string *getOp() const;

    void setOp(string *op);

    bool operator==(ShiftExpression &another);

    bool isAdditiveExpr() {
        return additiveExpression != nullptr && shiftExpression == nullptr && op == nullptr;
    }

    bool isShiftExpr() {
        return additiveExpression != nullptr && shiftExpression != nullptr && op != nullptr;
    }

    string &toString();

    Type *resolve_type(G_Context &context);
};

class RelationalExpression : public FunctionBodyExpression {
private:
    ShiftExpression *shiftExpression{};
    RelationalExpression *relationalExpression{};
    string *op{};
public:

    void setShiftExpression(ShiftExpression *shiftExpression);


    void setRelationalExpression(RelationalExpression *relationalExpression);

    ShiftExpression *getShiftExpression() const;

    RelationalExpression *getRelationalExpression() const;

    string *getOp() const;

    void setOp(string *op);

    bool operator==(RelationalExpression &another);

    bool isRelationExpr() {
        return relationalExpression != nullptr && op != nullptr && shiftExpression != nullptr;
    }

    bool isShiftExpr() {
        return relationalExpression == nullptr && op == nullptr && shiftExpression != nullptr;
    }

    string &toString();

    Type *resolve_type(G_Context &context);
};

class EqualityExpression : public FunctionBodyExpression {
private:
    RelationalExpression *relationalExpression{};
    EqualityExpression *equalityExpression{};
    string *op{};
public:

    void setRelationalExpression(RelationalExpression *relationalExpression);

    void setEqualityExpression(EqualityExpression *equalityExpression);

    RelationalExpression *getRelationalExpression() const;

    EqualityExpression *getEqualityExpression() const;

    string *getOp() const;

    void setOp(string *op);

    bool operator==(EqualityExpression &another);

    bool isRelationExpr() {
        return relationalExpression != nullptr && equalityExpression == nullptr && op == nullptr;
    }

    bool isEqualityExpr() {
        return relationalExpression != nullptr && equalityExpression != nullptr && op != nullptr;
    }

    string &toString();

    Type *resolve_type(G_Context &context);

};

class AndExpression : public FunctionBodyExpression {
private:
    EqualityExpression *equalityExpression{};
    AndExpression *andExpression{};
public:

    void setEqualityExpression(EqualityExpression *equalityExpression);

    void setAndExpression(AndExpression *andExpression);

    EqualityExpression *getEqualityExpression() const;

    AndExpression *getAndExpression() const;

    bool operator==(AndExpression &another);

    bool isAndExpr() {
        return equalityExpression != nullptr && andExpression != nullptr;
    }

    bool isEqualityExpr() {
        return equalityExpression != nullptr && andExpression == nullptr;
    }

    string &toString();

    Type *resolve_type(G_Context &context);
};

class ExclusiveOrExpression : public FunctionBodyExpression {
private:
    AndExpression *andExpression{};
    ExclusiveOrExpression *exclusiveOrExpression{};
public:

    void setAndExpression(AndExpression *andExpression);

    AndExpression *getAndExpression() const;

    ExclusiveOrExpression *getExclusiveOrExpression() const;

    void setExclusiveOrExpression(ExclusiveOrExpression *exclusiveOrExpression);

    bool operator==(ExclusiveOrExpression &another);

    bool isExclusiveOrExpr() {
        return andExpression != nullptr && exclusiveOrExpression != nullptr;
    }

    bool isAndExpr() {
        return andExpression != nullptr && exclusiveOrExpression == nullptr;
    }

    string &toString();

    Type *resolve_type(G_Context &context);
};

class InclusiveOrExpression : public FunctionBodyExpression {
private:
    ExclusiveOrExpression *exclusiveOrExpression{};
    InclusiveOrExpression *inclusiveOrExpression{};
public:

    void setExclusiveOrExpression(ExclusiveOrExpression *exclusiveOrExpression);

    ExclusiveOrExpression *getExclusiveOrExpression() const;

    InclusiveOrExpression *getInclusiveOrExpression() const;

    void setInclusiveOrExpression(InclusiveOrExpression *inclusiveOrExpression);

    bool operator==(InclusiveOrExpression &another);

    bool isInclusiveOrExpr() {
        return inclusiveOrExpression != nullptr && exclusiveOrExpression != nullptr;
    }

    bool isExclusiveOrExpr() {
        return inclusiveOrExpression == nullptr && exclusiveOrExpression != nullptr;
    }

    string &toString();

    Type *resolve_type(G_Context &context);
};

class LogicalAndExpression : public FunctionBodyExpression {
private:
    InclusiveOrExpression *inclusiveOrExpression{};
    LogicalAndExpression *logicalAndExpression{};
public:

    void setInclusiveOrExpression(InclusiveOrExpression *inclusiveOrExpression);

    InclusiveOrExpression *getInclusiveOrExpression() const;

    LogicalAndExpression *getLogicalAndExpression() const;

    void setLogicalAndExpression(LogicalAndExpression *logicalAndExpression);

    bool operator==(LogicalAndExpression &another);

    bool isLogicalAndExpr() {
        return logicalAndExpression != nullptr && inclusiveOrExpression != nullptr;
    }

    bool isInclusiveOrExpr() {
        return logicalAndExpression == nullptr && inclusiveOrExpression != nullptr;
    }

    string &toString();

    Type *resolve_type(G_Context &context);
};

class LogicalOrExpression : public FunctionBodyExpression {
private:
    LogicalOrExpression *logicalOrExpression{};
    LogicalAndExpression *logicalAndExpression{};
public:

    void setLogicalOrExpression(LogicalOrExpression *logicalOrExpression);

    LogicalOrExpression *getLogicalOrExpression() const;

    LogicalAndExpression *getLogicalAndExpression() const;

    void setLogicalAndExpression(LogicalAndExpression *logicalAndExpression);

    bool operator==(LogicalOrExpression &another);

    bool isLogicalAndExpr() {
        return logicalAndExpression != nullptr && logicalOrExpression == nullptr;
    }

    bool isLogicalOrExpr() {
        return logicalAndExpression != nullptr && logicalOrExpression != nullptr;
    }

    string &toString();

    Type *resolve_type(G_Context &context);
};

class ConditionalExpression : public FunctionBodyExpression {
private:
    LogicalOrExpression *logicalOrExpression{};
    Expression *expression{};
    ConditionalExpression *conditionalExpression{};
public:

    void setLogicalOrExpression(LogicalOrExpression *logicalOrExpression);

    void setExpression(Expression *expression);

    LogicalOrExpression *getLogicalOrExpression() const;

    Expression *getExpression() const;

    ConditionalExpression *getConditionalExpression() const;

    void setConditionalExpression(ConditionalExpression *conditionalExpression);

    bool operator==(ConditionalExpression &another);

    bool isLogicalOrExpr() {
        return logicalOrExpression != nullptr && expression == nullptr && conditionalExpression == nullptr;
    }

    bool is3OpCondition() {
        return logicalOrExpression != nullptr && expression != nullptr && conditionalExpression != nullptr;
    }

    string &toString();

    Type *resolve_type(G_Context &context);
};


class ArgumentExpressionList : public FunctionBodyExpression {
private:
    vector<AssignmentExpression *> statements;
    vector<string *> str_statements;
public:
    const vector<AssignmentExpression *> &getStatements() const;

    void addStatements(AssignmentExpression *assign_expr);

    void addStatements(string *assign_expr);

    bool operator==(ArgumentExpressionList &another);

    string &toString();
};

class PrimaryExpression : public FunctionBodyExpression {
private:
    string *identifier{};
    Expression *expr{};
    int content = 0;
public:


    void setIdentifier(string *identifier);

    string *getIdentifier() const;

    Expression *getExpr() const;

    void setExpr(Expression *expr);

    string &toString();

    bool isIdentifier() { return identifier != nullptr; }

    bool isExpression() { return expr != nullptr; };

    bool operator==(PrimaryExpression &another);

    int getContent() const;

    void setContent(int content);

    Type *resolve_type(G_Context &conext);
};

class PostfixExpression : public FunctionBodyExpression {
private:
    PostfixExpression *postfixExpression{};
    Expression *expression{};
    ArgumentExpressionList *argumentExpressionList{};
    PrimaryExpression *primaryExpression{};
    string *identifier{};
    string *op{};
public:
    string &toString();

    void setExpression(Expression *expression);

    void setArgumentExpressionList(ArgumentExpressionList *argumentExpressionList);

    void setIdentifier(string *identifier);

    void setOp(string *op);

    void setPostfixExpression(PostfixExpression *postfixExpression);

    PostfixExpression *getPostfixExpression() const;

    Expression *getExpression() const;

    ArgumentExpressionList *getArgumentExpressionList() const;

    string *getIdentifier() const;

    string *getOp() const;

    string resolve_inner_scalar() {
        if (isPrimaryExpr()) {
            if (primaryExpression->isIdentifier())
                return *primaryExpression->getIdentifier();
            else
                return "";
        } else
            return postfixExpression->resolve_inner_scalar();
    }

    PrimaryExpression *getPrimaryExpression() const;

    bool operator==(PostfixExpression &another);

    bool isPrimaryExpr() {
        return primaryExpression != nullptr && postfixExpression == nullptr && expression == nullptr
               && argumentExpressionList == nullptr && identifier == nullptr && op == nullptr;
    }

    bool isArrayLookup() {
        return primaryExpression == nullptr && postfixExpression != nullptr && expression != nullptr
               && argumentExpressionList == nullptr && identifier == nullptr && op == nullptr;
    }

    bool isArgMethodCall() {
        return primaryExpression == nullptr && postfixExpression != nullptr && expression == nullptr
               && argumentExpressionList != nullptr && identifier == nullptr && op == nullptr;
    }

    bool isNoArgMethodCall() {
        return primaryExpression == nullptr && postfixExpression != nullptr && expression == nullptr
               && argumentExpressionList == nullptr && identifier == nullptr && op == nullptr;
    }

    bool isFieldAccess() {
        return primaryExpression == nullptr && postfixExpression != nullptr && expression == nullptr
               && argumentExpressionList == nullptr && identifier != nullptr && op != nullptr;
    }

    bool isSelfOp() {
        return primaryExpression == nullptr && postfixExpression != nullptr && expression == nullptr
               && argumentExpressionList == nullptr && identifier == nullptr && op != nullptr;
    }

    void setPrimaryExpression(PrimaryExpression *primaryExpression);

    Type *resolve_type(G_Context &context);

};

class UnaryExpression : public FunctionBodyExpression {
private:
    string *op{};
    UnaryExpression *unaryExpression{};
    string *identifier{};
    CastExpression *castExpression{};
    PostfixExpression *postfixExpression{};
public:

    void setOp(string *op);

    void setUnaryExpression(UnaryExpression *unaryExpression);


    void setIdentifier(string *identifier);

    void setCastExpression(CastExpression *castExpression);

    string *getOp() const;

    UnaryExpression *getUnaryExpression() const;

    string *getIdentifier() const;

    CastExpression *getCastExpression() const;

    PostfixExpression *getPostfixExpression() const;

    void setPostfixExpression(PostfixExpression *postfixExpression);

    bool isPostfixExpr() {
        return postfixExpression != nullptr && unaryExpression == nullptr && castExpression == nullptr
               && identifier == nullptr && op == nullptr;
    }

    bool isUnaryExpr() {
        return postfixExpression == nullptr && unaryExpression != nullptr && castExpression == nullptr
               && identifier == nullptr && op != nullptr;
    }

    bool isCastExpr() {
        return postfixExpression == nullptr && unaryExpression == nullptr && castExpression != nullptr
               && identifier == nullptr && op != nullptr;
    }

    bool isSizeofExpr() {
        return postfixExpression == nullptr && unaryExpression == nullptr && castExpression == nullptr
               && identifier != nullptr && op == nullptr;
    }

    bool operator==(UnaryExpression &another);

    Type *resolve_type(G_Context &context);

    string &toString();

};

class CastExpression : public FunctionBodyExpression {
private:
    string *identifier{};
    CastExpression *castExpression{};
    UnaryExpression *unaryExpression{};
public:

    void setIdentifier(string *identifier);

    void setCastExpression(CastExpression *castExpression);

    string *getIdentifier() const;

    CastExpression *getCastExpression() const;

    UnaryExpression *getUnaryExpression() const;

    void setUnaryExpression(UnaryExpression *unaryExpression);

    bool operator==(CastExpression &another);

    bool isUnaryExpr() {
        return unaryExpression != nullptr && castExpression == nullptr && identifier == nullptr;
    }

    bool isCastExpr() {
        return unaryExpression == nullptr && castExpression != nullptr && identifier != nullptr;
    }

    string &toString();

    Type *resolve_type(G_Context &context);
};


class AssignmentExpression : public FunctionBodyExpression {
private:
    ConditionalExpression *conditionalExpression{};
    UnaryExpression *unaryExpression{};
    string *op{};
    AssignmentExpression *next{};
    PostfixExpression *postfixExpression;
    string *scalar_type{};
    string *scalar_name{};
public:

    void setUnaryExpression(UnaryExpression *unaryExpression);

    void setOp(string *op);

    void setNext(AssignmentExpression *next);

    ConditionalExpression *getConditionalExpression() const;

    UnaryExpression *getUnaryExpression() const;

    string *getOp() const;

    AssignmentExpression *getNext() const;

    void setConditionalExpression(ConditionalExpression *conditionalExpression);

    bool operator==(AssignmentExpression &another);

    bool isConditionalExpr() {
        return conditionalExpression != nullptr && unaryExpression == nullptr && op == nullptr && next == nullptr;
    }

    bool isAssignExpr() {
        return unaryExpression != nullptr && op != nullptr && next != nullptr && conditionalExpression == nullptr;
    }

    bool isOptAssignExpr() {
        return scalar_type != nullptr && scalar_name != nullptr && postfixExpression != nullptr;
    }

    virtual string &toString();

    Type *resolve_type(G_Context &context);

    string *getScalar_type() const;

    void setScalar_type(string *scalar_type);

    string *getScalar_name() const;

    void setScalar_name(string *scalar_name);

    PostfixExpression *getPostfixExpression() const;

    void setPostfixExpression(PostfixExpression *postfixExpression);
};


class Expression : public FunctionBodyExpression {
private:
    vector<AssignmentExpression *> exprs;
public:
    const vector<AssignmentExpression *> &getExprs() const;

    void addExprs(AssignmentExpression *expr);

    bool operator==(Expression &another);

    virtual string &toString();
};


class CompoundStatement : public Statement {
private:
    vector<Statement *> statements;
public:
    void addStatement(Statement *stmt);

    vector<Statement *> &getStatements();

    string &toString();
};

class ExpressionStatement : public Statement {
private:
    Expression *expression{};
public:
    Expression *getExpression() const {
        return expression;
    }

    void setExpression(Expression *expression) {
        ExpressionStatement::expression = expression;
    }

    string &toString();
};

class ReturnStatement : public Statement {
private:
    Expression *expression{};
public:
    Expression *getExpression() const {
        return expression;
    }

    void setExpression(Expression *expression) {
        ReturnStatement::expression = expression;
    }

    string &toString();
};

class IfStatement : public Statement {
private:
    Expression *expression{};
    CompoundStatement *t_case{};
    CompoundStatement *f_case{};
public:

    void setExpression(Expression *expression);

    void setT_case(CompoundStatement *t_case);

    Expression *getExpression() const;

    CompoundStatement *getT_case() const;

    CompoundStatement *getF_case() const;

    void setF_case(CompoundStatement *f_case);

    bool hasFCase();

    string &toString();
};


class RootNode : public Node {
private:
    vector<Node *> statements;
public:
    RootNode() : Node() {
        this->parent = nullptr;
    }

    void addStmt(Node *child) {
        statements.push_back(child);
    }

    void setStatements(const vector<Node *> &statements);

    vector<Node *> &getChildren() {
        return statements;
    }
};

class Type : public Node {
private:
    string *value;
public:
    explicit Type(string *val);

    string &getValue();

    ~Type() override = default;

    virtual string typeName() {
        return string("Type");
    }

    int get_priority();

    bool operator==(Type *another);

    bool operator<(Type *another);

    bool operator>(Type *another);

    virtual string &toString() { throw "Unsupported"; };

};

class BuiltinType : public Type {
public:
    explicit BuiltinType(string *value);

    string typeName() override;

    virtual string &toString() = 0;

};

class BuiltinElementType : public BuiltinType {
public:
    explicit BuiltinElementType(string *value) : BuiltinType(value) {}

    virtual string typeName();

    virtual string &get_real_cpp_type() = 0;

    virtual string &toString() = 0;
};

class BuiltinVoidType : public BuiltinElementType {
private:
    string Type;
    string real_cpp_Type = "void";
public:
    BuiltinVoidType(string *value) : BuiltinElementType(value) {}

    virtual string typeName() {
        return string("BuiltinVoidType");
    }

    string &get_real_cpp_type() override {
        return this->real_cpp_Type;
    }

    string &toString() { return real_cpp_Type; }
};

class BuiltinBoolType : public BuiltinElementType {
private:
    string Type;
    string real_cpp_Type = "bool";
public:
    BuiltinBoolType(string *value) : BuiltinElementType(value) {}

    virtual string typeName() {
        return string("BuiltinBoolType");
    }

    string &get_real_cpp_type() override {
        return this->real_cpp_Type;
    }

    string &toString() { return real_cpp_Type; }
};

class BuiltinStringType : public BuiltinElementType {
private:
    string Type;
    string real_cpp_Type = "string";
public:
    BuiltinStringType(string *value) : BuiltinElementType(value) {}

    virtual string typeName() {
        return string("BuiltinStringType");
    }

    string &get_real_cpp_type() override {
        return this->real_cpp_Type;
    }

    string &toString() { return real_cpp_Type; }
};

class BuiltinDoubleType : public BuiltinElementType {
private:
    string Type;
    string real_cpp_Type = "double";
public:
    BuiltinDoubleType(string *value) : BuiltinElementType(value) {}

    virtual string typeName() {
        return string("BuiltinDoubleType");
    }

    string &get_real_cpp_type() override {
        return this->real_cpp_Type;
    }

    string &toString() { return real_cpp_Type; }
};

class BuiltinIntType : public BuiltinElementType {
private:
    string Type;
    string real_cpp_Type = "int";
public:
    BuiltinIntType(string *value) : BuiltinElementType(value) {}

    virtual string typeName() {
        return string("BuiltinIntType");
    }

    string &get_real_cpp_type() override {
        return this->real_cpp_Type;
    }

    string &toString() { return real_cpp_Type; }
};


class BuiltinPointerType : public BuiltinElementType {
private:
    string type;
    string real_cpp_Type = "void*";
    bool sync = false;
public:
    explicit BuiltinPointerType(string *value);

    BuiltinPointerType(string *value, bool sync);

    string &getType() {
        return this->type;
    }


    virtual string typeName() {
        return string("BuiltinPointerType");
    }

    string &get_real_cpp_type() override {
        return this->real_cpp_Type;
    }

    string &toString() { return real_cpp_Type; }

    bool isSync() const;
};

class BuiltinThreadIdType : public BuiltinElementType {
private:
    string Type;
    string real_cpp_Type;
    int length;
public:
    BuiltinThreadIdType(string *value, const char *length) : BuiltinElementType(value) {
        this->Type = "tid";
        this->real_cpp_Type = "unsigned long";
        this->length = atoi(length);
    }

    virtual string typeName() {
        return string("BuiltinThreadIdType");
    }

    string &get_real_cpp_type() override {
        return this->real_cpp_Type;
    }

    string &toString() { return real_cpp_Type; }

    int getLength() { return length; }
};

class BuiltinOThreadIdType : public BuiltinElementType {
private:
    string Type;
    string real_cpp_Type;
    int length;
public:
    BuiltinOThreadIdType(string *value, const char *length) : BuiltinElementType(value) {
        this->Type = "otid";
        this->real_cpp_Type = "pthread_t";
        this->length = atoi(length);
    }

    virtual string typeName() {
        return string("BuiltinOThreadIdType");
    }

    string &get_real_cpp_type() override {
        return this->real_cpp_Type;
    }

    string &toString() { return real_cpp_Type; }

    int getLength() { return length; }
};

class BuiltinLockIdType : public BuiltinElementType {
private:
    string Type;
    string real_cpp_Type;
    int length;
public:
    BuiltinLockIdType(string *value, const char *length) : BuiltinElementType(value) {
        this->Type = "lockId";
        this->real_cpp_Type = "unsigned long";
        this->length = atoi(length);
    }

    string &getType() {
        return this->Type;
    }

    virtual string typeName() {
        return string("BuiltinLockIdType");
    }

    string &get_real_cpp_type() override {
        return this->real_cpp_Type;
    }

    string &toString() { return real_cpp_Type; }

    int getLength() { return length; }
};

class BuiltinCPPType : public BuiltinElementType {
private:
    string Type;
    string real_cpp_Type;
    int length = 0;
    bool sync = false;
public:
    BuiltinCPPType(string *value) : BuiltinElementType(value) {
        this->Type = *value;
        this->real_cpp_Type = *value;
    }

    BuiltinCPPType(string *value, const string *length) : BuiltinElementType(value) {
        this->Type = *value;
        this->real_cpp_Type = *value;
        this->length = atoi(length->c_str());
    }

    BuiltinCPPType(string *value, const bool sync) : BuiltinElementType(value) {
        this->Type = *value;
        this->real_cpp_Type = *value;
        this->sync = sync;
    }

    BuiltinCPPType(string *value, const string *length, const bool sync) : BuiltinElementType(value) {
        this->Type = *value;
        this->real_cpp_Type = *value;
        this->length = atoi(length->c_str());
        this->sync = sync;
    }

    string &getType() {
        return this->real_cpp_Type;
    }

    bool isSync(){
        return this->sync;
    }

    int getLength() {
        return length;
    }

    virtual string typeName() {
        return string("BuiltinCPPType");
    }

    string &get_real_cpp_type() override {
        return this->real_cpp_Type;
    }

    string &toString() { return real_cpp_Type; }
};

class BuiltinSizeofType : public BuiltinElementType {
private:
    string Type;
    string real_cpp_Type;
public:
    BuiltinSizeofType(string *value) : BuiltinElementType(value) {
        this->Type = "sizeof";
        this->real_cpp_Type = "unsigned long";
    }

    string &getType() {
        return this->Type;
    }

    string typeName() {
        return string("BuiltinSizeofType");
    }

    string &get_real_cpp_type() override {
        return this->real_cpp_Type;
    }

    string &toString() { return real_cpp_Type; }
};

class BuiltinCollectionType : public BuiltinType {
private:
    string scope;
public:
    BuiltinCollectionType(string *value, string &scope) : BuiltinType(value) {
        this->scope = scope;
    }

    string getScope() { return this->scope; }

    virtual string typeName() {
        return string("BuiltinCollectionType");
    }

    virtual string &toString() = 0;
};

class BuiltinMap : public BuiltinCollectionType {
private:
    VariableDefinition *key;
    VariableDefinition *value;
public:
    BuiltinMap(string *value, string &scope, VariableDefinition *k, VariableDefinition *v) : BuiltinCollectionType(
            value, scope) {
        this->key = k;
        this->value = v;
    }

    VariableDefinition *getKeyType() {
        return this->key;
    }

    VariableDefinition *getValueType() {
        return this->value;
    }

    virtual string typeName() {
        return string("BuiltinMap");
    }

    string &toString();
};

class BuiltinSet : public BuiltinCollectionType {
private:
    VariableDefinition *elementType;
public:
    BuiltinSet(string *value, string &scope, VariableDefinition *e) : BuiltinCollectionType(value, scope) {
        this->elementType = e;
    }

    VariableDefinition *getElementType() {
        return this->elementType;
    }

    virtual string typeName() {
        return string("BuiltinSet");
    }

    void setElementType(VariableDefinition *elementType);

    string &toString();
};

class ExternalFunctionType : public Type {
private:
    string *functionName;
public:
    ExternalFunctionType(string *value, string *functionName) : Type(value) {
        this->functionName = functionName;
    }

    string &getFunctionName() {
        return *this->functionName;
    }

    virtual string typeName() {
        return string("ExternalFunctionType");
    }
};

class FunctionType : public Type {
private:
    string *functionName;
public:
    FunctionType(string *value, string *functionName) : Type(value) {
        this->functionName = functionName;
    }

    string &getFunctionName() {
        return *this->functionName;
    }

    virtual string typeName() {
        return string("FunctionType");
    }
};

class LLVMInstructionType : public Type {
private:
    string *instName;
public:
    LLVMInstructionType(string *value, string *instName) : Type(value) {
        this->instName = instName;
    }

    virtual string typeName() {
        return string("LLVMInstructionType");
    }
};

class Definition : public Node {
protected:
    string *scalar;
    Type *type;
public:
    Definition(string *scalar, Type *type) {
        this->scalar = scalar;
        this->type = type;
    }

    virtual string &getScalarName() {
        return *scalar;
    }

    Type *getType() {
        return type;
    }
};

class FunctionDefinition : public Definition {
private:
    CombinedStatements *functionBody;
    bool sync;
    vector<FunctionParamDefinition *> *parameters;
    VariableDefinition *return_value;
    bool isvar_arg = false;
public:
    FunctionDefinition(string *scalar, Type *type, bool sync, vector<FunctionParamDefinition *> *parameters,
                       CombinedStatements *functionBody) : Definition(scalar, type) {
        this->sync = sync;
        this->functionBody = functionBody;
        this->parameters = parameters;
        this->return_value = nullptr;
    }

    FunctionDefinition(string *scalar, Type *type, bool sync, vector<FunctionParamDefinition *> *parameters,
                       CombinedStatements *functionBody, VariableDefinition *return_value) : Definition(scalar, type) {
        this->sync = sync;
        this->functionBody = functionBody;
        this->parameters = parameters;
        this->return_value = return_value;

    }


    vector<FunctionParamDefinition *> &getParameters() {
        return *parameters;
    }

    CombinedStatements *getBody() {
        return functionBody;
    }

    bool isSync() {
        return this->sync;
    }

    VariableDefinition *getReturnValue() {
        return this->return_value;
    }

    bool is_var_arg() {
        return this->isvar_arg;
    }

    void set_var_arg(bool b) {
        this->isvar_arg = b;
    }
};

class LLVMInstructionDefinition : public Definition {
public:
    LLVMInstructionDefinition(string *scalar, Type *type) : Definition(scalar, type) {

    }

    string &getScalarName() {
        return this->getType()->getValue();
    }
};

class VariableDefinition : public Definition {
public:
    bool isTypeDef = false;

    VariableDefinition(string *scalar, Type *type, bool isTypeDef = false) : Definition(scalar, type) {
        this->isTypeDef = isTypeDef;
    }

    string &getScalarName() override {
        if (scalar != nullptr)
            return *scalar;
        if (this->getParent() != nullptr) {
            if (auto p_var = dynamic_cast<VariableDefinition *>(this->getParent()))
                return p_var->getScalarName();
        }

        throw "Unexpected code";
    }
};

class FunctionParamDefinition : public Definition {
public:
    string *alias;
    VariableDefinition *variableDefinition;

    FunctionParamDefinition(VariableDefinition *variableDefinition, string *alias) : Definition(
            &variableDefinition->getScalarName(), variableDefinition->getType()) {
        this->variableDefinition = variableDefinition;
        this->alias = alias;
    }

    string &getAlias() {
        if (alias == nullptr) {
            return variableDefinition->getScalarName();
        } else
            return *alias;
    }

    VariableDefinition *getVariable() {
        return variableDefinition;
    }

    string &getScalarName() {
        return variableDefinition->getScalarName();
    }

    Type *getType() {
        return variableDefinition->getType();
    }
};

class ExternalFunctionDefinition : public LLVMInstructionDefinition {
public:
    ExternalFunctionDefinition(string *scalar, Type *type) : LLVMInstructionDefinition(scalar, type) {
        this->getType()->getValue().insert(0, "func ");
    }

    string &getScalarName() {
        return this->getType()->getValue();
    }
};

class ClazzType : public BuiltinType {
private:
    string *clazzName;
    unordered_map<string, Type *> fields{};
public:
    explicit ClazzType(string *name) : BuiltinType(name) {
        clazzName = name;
    }

    string &getClazzName() { return *clazzName; }

    unordered_map<string, Type *> &getFields();

    virtual string typeName() {
        return string("ClazzType");
    }

    string &toString() {
        return *clazzName;
    }
};


class InsertStatement : public Node {
private:
    string shift;
    LLVMInstructionDefinition *location;
    FunctionDefinition *calledFunction;
    vector<string *> *calledParams;
    string *return_def;
public:
    InsertStatement(const char *shift, LLVMInstructionDefinition *location, FunctionDefinition *functionDefinition)
            : Node() {
        this->shift = string(shift);
        this->location = location;
        this->calledFunction = functionDefinition;
    }

    InsertStatement(const char *shift, LLVMInstructionDefinition *location, FunctionDefinition *functionDefinition,
                    vector<string *> *calledParams)
            : Node() {
        this->shift = string(shift);
        this->location = location;
        this->calledFunction = functionDefinition;
        this->calledParams = calledParams;
        this->return_def = nullptr;
    }

    void setReturnDef(string *return_def) {
        this->return_def = return_def;
    }

    LLVMInstructionDefinition *getLocation() {
        return this->location;
    }

    FunctionDefinition *getFunctionDef() {
        return calledFunction;
    }

    string &getShift() {
        return shift;
    }

    vector<string *> *getCalledParams() {
        return this->calledParams;
    }

    string *getReturnDef() {
        return this->return_def;
    }
};


#endif //G_PARSER_STATEMENT_H
