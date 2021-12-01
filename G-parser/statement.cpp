//
// Created by cxworks on 19-9-14.
//

#include "statement.h"


void AssignmentExpression::setOp(string *op) {
    AssignmentExpression::op = op;
}


void AssignmentExpression::setUnaryExpression(UnaryExpression *unaryExpression) {
    AssignmentExpression::unaryExpression = unaryExpression;
}


string &AssignmentExpression::toString() {
    if (isConditionalExpr()) {
        return conditionalExpression->toString();
    } else if (isAssignExpr()) {
        string &ans = unaryExpression->toString();
        ans += " " + *op + " ";
        ans += next->toString();
        return ans;
    } else if (isOptAssignExpr()) {
        string &ans = postfixExpression->toString();
        if ((*scalar_type).size() != 0)
            ans = *scalar_type + "& " + *scalar_name + " = " + ans;
        else
            ans = *scalar_type + " " + *scalar_name + " = " + ans;
        return ans;
    }
    return *new string("");
}

void AssignmentExpression::setNext(AssignmentExpression *next) {
    AssignmentExpression::next = next;
}


void AssignmentExpression::setConditionalExpression(ConditionalExpression *conditionalExpression) {
    AssignmentExpression::conditionalExpression = conditionalExpression;
}

bool AssignmentExpression::operator==(AssignmentExpression &another) {
    if (isConditionalExpr() && another.isConditionalExpr()) {
        return *getConditionalExpression() == *another.getConditionalExpression();
    } else if (isAssignExpr() && another.isAssignExpr()) {
        return *getUnaryExpression() == *another.getUnaryExpression() && *getOp() == *another.getOp() &&
               *getNext() == *another.getNext();
    }
    return false;
}

ConditionalExpression *AssignmentExpression::getConditionalExpression() const {
    return conditionalExpression;
}

UnaryExpression *AssignmentExpression::getUnaryExpression() const {
    return unaryExpression;
}

string *AssignmentExpression::getOp() const {
    return op;
}

AssignmentExpression *AssignmentExpression::getNext() const {
    return next;
}

Type *AssignmentExpression::resolve_type(G_Context &context) {
    if (isConditionalExpr())
        return conditionalExpression->resolve_type(context);
    if (isOptAssignExpr())
        return nullptr;
    auto left = unaryExpression->resolve_type(context);
    auto right = this->next->resolve_type(context);
    if (left != nullptr && right != nullptr && *left == right)
        return left;
    if (left == nullptr)
        return right;
    else if (right == nullptr)
        return left;
    return nullptr;
}

string *AssignmentExpression::getScalar_type() const {
    return scalar_type;
}

void AssignmentExpression::setScalar_type(string *scalar_type) {
    AssignmentExpression::scalar_type = scalar_type;
}

string *AssignmentExpression::getScalar_name() const {
    return scalar_name;
}

void AssignmentExpression::setScalar_name(string *scalar_name) {
    AssignmentExpression::scalar_name = scalar_name;
}

PostfixExpression *AssignmentExpression::getPostfixExpression() const {
    return postfixExpression;
}

void AssignmentExpression::setPostfixExpression(PostfixExpression *postfixExpression) {
    AssignmentExpression::postfixExpression = postfixExpression;
}

const vector<AssignmentExpression *> &Expression::getExprs() const {
    return exprs;
}

void Expression::addExprs(AssignmentExpression *expr) {
    exprs.push_back(expr);
}

string &Expression::toString() {
    string &ans = *(new string());
    for (int i = 0; i < exprs.size(); ++i) {
        ans += exprs[i]->toString();
        if (i != exprs.size() - 1)
            ans += ", ";
    }
    return ans;
}

bool Expression::operator==(Expression &another) {
    bool ans = this->exprs.size() == another.exprs.size();
    if (ans) {
        for (int i = 0; ans && i < this->exprs.size(); ++i) {
            ans = ans && *(this->exprs[i]) == *(another.exprs[i]);
        }
    }
    return ans;
}


void CompoundStatement::addStatement(Statement *stmt) {
    statements.push_back(stmt);
}

vector<Statement *> &CompoundStatement::getStatements() {
    return statements;
}

string &CompoundStatement::toString() {
    string &ans = *(new string("{\n"));
    for (auto stmt: statements) {
        ans += stmt->toString();
    }
    ans += "\n}\n";
    return ans;
}


void UnaryExpression::setOp(string *op) {
    UnaryExpression::op = op;
}


void UnaryExpression::setUnaryExpression(UnaryExpression *unaryExpression) {
    UnaryExpression::unaryExpression = unaryExpression;
}


string &UnaryExpression::toString() {
    if (postfixExpression != nullptr) {
        return postfixExpression->toString();
    } else if (unaryExpression != nullptr) {
        string &ans = *(new string(*op + " "));
        ans += unaryExpression->toString();
        return ans;
    } else if (identifier != nullptr) {
        string &ans = *(new string("sizeof("));
        ans += *identifier;
        ans += ") ";
        return ans;
    } else if (castExpression != nullptr) {
        string &ans = *(new string(*op + " "));
        ans += castExpression->toString();
        return ans;
    } else
        throw "Unable to determine unary type";
}


void UnaryExpression::setIdentifier(string *identifier) {
    UnaryExpression::identifier = identifier;
}

bool UnaryExpression::operator==(UnaryExpression &another) {
    if (isPostfixExpr() && another.isPostfixExpr())
        return *getPostfixExpression() == *another.getPostfixExpression();
    else if (isUnaryExpr() && another.isUnaryExpr()) {
        return *getOp() == *another.getOp() && *getUnaryExpression() == *another.getUnaryExpression();
    } else if (isCastExpr() && another.isCastExpr()) {
        return *getOp() == *another.getOp() && *getCastExpression() == *another.getCastExpression();
    } else if (isSizeofExpr() && another.isSizeofExpr()) {
        return *getIdentifier() == *another.getIdentifier();
    }
    return false;
}


void UnaryExpression::setCastExpression(CastExpression *castExpression) {
    UnaryExpression::castExpression = castExpression;
}


void UnaryExpression::setPostfixExpression(PostfixExpression *postfixExpression) {
    UnaryExpression::postfixExpression = postfixExpression;
}


string *UnaryExpression::getOp() const {
    return op;
}

UnaryExpression *UnaryExpression::getUnaryExpression() const {
    return unaryExpression;
}

string *UnaryExpression::getIdentifier() const {
    return identifier;
}

CastExpression *UnaryExpression::getCastExpression() const {
    return castExpression;
}

PostfixExpression *UnaryExpression::getPostfixExpression() const {
    return postfixExpression;
}

Type *UnaryExpression::resolve_type(G_Context &context) {
    if (isPostfixExpr())
        return postfixExpression->resolve_type(context);
    if (isUnaryExpr())
        return unaryExpression->resolve_type(context);
    if (isCastExpr())
        return castExpression->resolve_type(context);
    if (isSizeofExpr())
        return new BuiltinIntType(nullptr);
    return nullptr;
}


void PrimaryExpression::setIdentifier(string *identifier) {
    PrimaryExpression::identifier = identifier;
}


void PrimaryExpression::setExpr(Expression *expr) {
    PrimaryExpression::expr = expr;
}

string &PrimaryExpression::toString() {
    if (identifier != nullptr) {
        return *(new string(*identifier));
    } else if (expr != nullptr) {
        string &ans = *(new string("( "));
        ans += expr->toString();
        ans += " )";
        return ans;
    }
    throw "Unable to determine primary expr type";
}

bool PrimaryExpression::operator==(PrimaryExpression &another) {
    if (isIdentifier() && another.isIdentifier()) {
        return *getIdentifier() == *another.getIdentifier();
    } else if (isExpression() && another.isExpression()) {
        return *getExpr() == *another.getExpr();
    }
    return false;
}

string *PrimaryExpression::getIdentifier() const {
    return identifier;
}

Expression *PrimaryExpression::getExpr() const {
    return expr;
}

Type *PrimaryExpression::resolve_type(G_Context &conext) {
    if (this->isExpression())
        return this->expr->resolve_type(conext);
    else {
        switch (getContent()) {
            case 1:
                if (conext.query_scalar_type(*getIdentifier()) != nullptr) {
                    return conext.query_scalar_type(*getIdentifier());
                } else
                    return nullptr;
            case 2:
                return new BuiltinDoubleType(nullptr);
            case 3:
                return new BuiltinIntType(nullptr);
            case 4:
                return new BuiltinStringType(nullptr);
        }
    }
    return nullptr;
}

int PrimaryExpression::getContent() const {
    return content;
}

void PrimaryExpression::setContent(int content) {
    PrimaryExpression::content = content;
}

void PostfixExpression::setExpression(Expression *expression) {
    PostfixExpression::expression = expression;
}


void PostfixExpression::setArgumentExpressionList(ArgumentExpressionList *argumentExpressionList) {
    PostfixExpression::argumentExpressionList = argumentExpressionList;
}


void PostfixExpression::setIdentifier(string *identifier) {
    PostfixExpression::identifier = identifier;
}


void PostfixExpression::setOp(string *op) {
    PostfixExpression::op = op;
}

void PostfixExpression::setPostfixExpression(PostfixExpression *postfixExpression) {
    PostfixExpression::postfixExpression = postfixExpression;
}

string &PostfixExpression::toString() {
    if (primaryExpression != nullptr) {
        return primaryExpression->toString();
    } else if (expression != nullptr) {
        string &ans = postfixExpression->toString();
        ans += "[" + expression->toString() + "]";
        return ans;
    } else if (argumentExpressionList != nullptr) {
        string &ans = postfixExpression->toString();
        ans += "( " + argumentExpressionList->toString() + " )";
        return ans;
    } else if (op != nullptr) {
        string &ans = postfixExpression->toString();
        ans += *op;
        if (identifier != nullptr)
            ans += *identifier;
        return ans;
    } else if (postfixExpression != nullptr) {
        string &ans = postfixExpression->toString();
        ans += "()";
        return ans;
    }
    throw "Unable to determine postfix type";
}

void PostfixExpression::setPrimaryExpression(PrimaryExpression *primaryExpression) {
    PostfixExpression::primaryExpression = primaryExpression;
}

PostfixExpression *PostfixExpression::getPostfixExpression() const {
    return postfixExpression;
}

Expression *PostfixExpression::getExpression() const {
    return expression;
}

ArgumentExpressionList *PostfixExpression::getArgumentExpressionList() const {
    return argumentExpressionList;
}

string *PostfixExpression::getIdentifier() const {
    return identifier;
}

string *PostfixExpression::getOp() const {
    return op;
}

bool PostfixExpression::operator==(PostfixExpression &another) {
    if (isPrimaryExpr() && another.isPrimaryExpr())
        return *getPrimaryExpression() == *another.getPrimaryExpression();
    else if (isArrayLookup() && another.isArrayLookup()) {
        return *getPostfixExpression() == *another.getPostfixExpression() &&
               *getExpression() == *another.getExpression();
    } else if (isArgMethodCall() && another.isArgMethodCall()) {
        return *getPostfixExpression() == *another.getPostfixExpression() &&
               *getArgumentExpressionList() == *another.getArgumentExpressionList();
    } else if (isNoArgMethodCall() && another.isNoArgMethodCall()) {
        return *getPostfixExpression() == *another.getPostfixExpression();
    } else if (isFieldAccess() && another.isFieldAccess()) {
        return *getPostfixExpression() == *another.getPostfixExpression() &&
               *getIdentifier() == *another.getIdentifier() && *getOp() == *another.getOp();
    } else if (isSelfOp() && another.isSelfOp()) {
        return *getPostfixExpression() == *another.getPostfixExpression() && *getOp() == *another.getOp();
    }
    return false;
}

PrimaryExpression *PostfixExpression::getPrimaryExpression() const {
    return primaryExpression;
}

Type *PostfixExpression::resolve_type(G_Context &context) {
    if (this->isPrimaryExpr())
        return this->primaryExpression->resolve_type(context);
    if (this->isArrayLookup()) {
        Type *left_type = this->getPostfixExpression()->resolve_type(context);
        if (auto map_type = dynamic_cast<BuiltinMap *>(left_type)) {
            return map_type->getValueType()->getType();
        }
    }
    if (this->isNoArgMethodCall()) {
        string method_name = this->postfixExpression->toString();
        return context.query_method_type(method_name);
    }
    if (this->isArgMethodCall()) {
        string method_name = this->postfixExpression->toString();
        Type *base_type = context.query_method_type(method_name);
        if (auto builtin_set = dynamic_cast<BuiltinSet *>(base_type)) {
            Type *arg_type = this->getArgumentExpressionList()->getStatements()[0]->resolve_type(context);
            if (arg_type != nullptr)
                builtin_set->setElementType(new VariableDefinition(nullptr, arg_type));
        }
        return base_type;
    }
    if (this->isFieldAccess()) {
        Type *left_type = postfixExpression->resolve_type(context);
        if (auto clazz_type = dynamic_cast<ClazzType *>(left_type)) {
            return clazz_type->getFields()[*identifier];
        }
    }
    if (this->isSelfOp()) {
        return postfixExpression->resolve_type(context);
    }
    return nullptr;
}


const vector<AssignmentExpression *> &ArgumentExpressionList::getStatements() const {
    return statements;
}

void ArgumentExpressionList::addStatements(AssignmentExpression *assign_expr) {
    this->statements.push_back(assign_expr);
}

void ArgumentExpressionList::addStatements(string *assign_expr) {
    this->str_statements.push_back(assign_expr);
}

string &ArgumentExpressionList::toString() {

    if (this->statements.size() != 0) {
        string &ans = this->statements[0]->toString();
        for (int i = 1; i < statements.size(); ++i) {
            ans += ", " + statements[i]->toString();
        }
        if (!str_statements.empty()) {
            for (int i = 0; i < str_statements.size(); ++i) {
                ans += ", " + *str_statements[i];
            }
        }
        return ans;

    } else if (!str_statements.empty()) {
        string &ans = *(new string(*this->str_statements[0]));
        for (int i = 1; i < str_statements.size(); ++i) {
            ans += ", " + *str_statements[i];
        }
        return ans;
    } else {
        return *(new string());
    }
}

bool ArgumentExpressionList::operator==(ArgumentExpressionList &another) {
    if (getStatements().size() == another.getStatements().size()) {
        bool ans = true;
        for (int i = 0; ans && i < getStatements().size(); ++i) {
            ans = ans && *(getStatements()[i]) == *(another.getStatements()[i]);
        }
        return ans;
    }
    return false;
}

void CastExpression::setIdentifier(string *identifier) {
    CastExpression::identifier = identifier;
}

string &CastExpression::toString() {
    if (unaryExpression != nullptr) {
        return unaryExpression->toString();
    } else {
        string &ans = *(new string("( "));
        ans += *identifier + " )" + castExpression->toString();
        return ans;
    }
}


void CastExpression::setCastExpression(CastExpression *castExpression) {
    CastExpression::castExpression = castExpression;
}


void CastExpression::setUnaryExpression(UnaryExpression *unaryExpression) {
    CastExpression::unaryExpression = unaryExpression;
}

bool CastExpression::operator==(CastExpression &another) {
    if (isUnaryExpr() && another.isUnaryExpr()) {
        return *getUnaryExpression() == *another.getUnaryExpression();
    } else if (isCastExpr() && another.isCastExpr()) {
        return *getCastExpression() == *another.getCastExpression() && *getIdentifier() == *another.getIdentifier();
    }
    return false;
}

string *CastExpression::getIdentifier() const {
    return identifier;
}

CastExpression *CastExpression::getCastExpression() const {
    return castExpression;
}

UnaryExpression *CastExpression::getUnaryExpression() const {
    return unaryExpression;
}

Type *CastExpression::resolve_type(G_Context &context) {
    if (isUnaryExpr())
        return unaryExpression->resolve_type(context);
    if (isCastExpr()) {
        string &tar = *identifier;
        if (tar == "int" || tar == "long") {
            return new BuiltinIntType(nullptr);
        } else if (tar == "double" || tar == "float")
            return new BuiltinDoubleType(nullptr);
        else
            return castExpression->resolve_type(context);
    }
    return nullptr;
}


void ConditionalExpression::setLogicalOrExpression(LogicalOrExpression *logicalOrExpression) {
    ConditionalExpression::logicalOrExpression = logicalOrExpression;
}


void ConditionalExpression::setExpression(Expression *expression) {
    ConditionalExpression::expression = expression;
}


void ConditionalExpression::setConditionalExpression(ConditionalExpression *conditionalExpression) {
    ConditionalExpression::conditionalExpression = conditionalExpression;
}

string &ConditionalExpression::toString() {
    string &ans = logicalOrExpression->toString();
    if (expression != nullptr && conditionalExpression != nullptr) {
        ans += " ? " + expression->toString() + " : " + conditionalExpression->toString();
    }
    return ans;
}

bool ConditionalExpression::operator==(ConditionalExpression &another) {
    if (isLogicalOrExpr() && another.isLogicalOrExpr()) {
        return *getLogicalOrExpression() == *another.getLogicalOrExpression();
    } else if (is3OpCondition() && another.is3OpCondition()) {
        return *getLogicalOrExpression() == *another.getLogicalOrExpression() &&
               *getExpression() == *another.getExpression() &&
               *getConditionalExpression() == *another.getConditionalExpression();
    }
    return false;
}

LogicalOrExpression *ConditionalExpression::getLogicalOrExpression() const {
    return logicalOrExpression;
}

Expression *ConditionalExpression::getExpression() const {
    return expression;
}

ConditionalExpression *ConditionalExpression::getConditionalExpression() const {
    return conditionalExpression;
}

Type *ConditionalExpression::resolve_type(G_Context &context) {
    return logicalOrExpression->resolve_type(context);
}


void LogicalOrExpression::setLogicalOrExpression(LogicalOrExpression *logicalOrExpression) {
    LogicalOrExpression::logicalOrExpression = logicalOrExpression;
}


void LogicalOrExpression::setLogicalAndExpression(LogicalAndExpression *logicalAndExpression) {
    LogicalOrExpression::logicalAndExpression = logicalAndExpression;
}

string &LogicalOrExpression::toString() {
    if (logicalOrExpression != nullptr) {
        string &ans = logicalOrExpression->toString();
        ans += " || " + logicalAndExpression->toString();
        return ans;
    } else {
        string &ans = logicalAndExpression->toString();
        return ans;
    }
}

bool LogicalOrExpression::operator==(LogicalOrExpression &another) {
    if (isLogicalOrExpr() && another.isLogicalOrExpr()) {
        return *getLogicalOrExpression() == *another.getLogicalOrExpression() &&
               *getLogicalAndExpression() == *another.getLogicalAndExpression();
    } else if (isLogicalAndExpr() && another.isLogicalAndExpr()) {
        return *getLogicalAndExpression() == *another.getLogicalAndExpression();
    }
    return false;
}

LogicalOrExpression *LogicalOrExpression::getLogicalOrExpression() const {
    return logicalOrExpression;
}

LogicalAndExpression *LogicalOrExpression::getLogicalAndExpression() const {
    return logicalAndExpression;
}

Type *LogicalOrExpression::resolve_type(G_Context &context) {
    if (isLogicalAndExpr())
        return logicalAndExpression->resolve_type(context);
    return new BuiltinBoolType(nullptr);
}


void LogicalAndExpression::setInclusiveOrExpression(InclusiveOrExpression *inclusiveOrExpression) {
    LogicalAndExpression::inclusiveOrExpression = inclusiveOrExpression;
}


void LogicalAndExpression::setLogicalAndExpression(LogicalAndExpression *logicalAndExpression) {
    LogicalAndExpression::logicalAndExpression = logicalAndExpression;
}

string &LogicalAndExpression::toString() {
    if (logicalAndExpression != nullptr) {
        string &ans = logicalAndExpression->toString();
        ans += " && " + inclusiveOrExpression->toString();
        return ans;
    } else {
        string &ans = inclusiveOrExpression->toString();
        return ans;
    }
}

bool LogicalAndExpression::operator==(LogicalAndExpression &another) {
    if (isLogicalAndExpr() && another.isLogicalAndExpr()) {
        return *getLogicalAndExpression() == *another.getLogicalAndExpression() &&
               *getInclusiveOrExpression() == *another.getInclusiveOrExpression();
    } else if (isInclusiveOrExpr() && another.isInclusiveOrExpr()) {
        return *getInclusiveOrExpression() == *another.getInclusiveOrExpression();
    }
    return false;
}

InclusiveOrExpression *LogicalAndExpression::getInclusiveOrExpression() const {
    return inclusiveOrExpression;
}

LogicalAndExpression *LogicalAndExpression::getLogicalAndExpression() const {
    return logicalAndExpression;
}

Type *LogicalAndExpression::resolve_type(G_Context &context) {
    if (isInclusiveOrExpr())
        return inclusiveOrExpression->resolve_type(context);
    return new BuiltinBoolType(nullptr);
}


void InclusiveOrExpression::setExclusiveOrExpression(ExclusiveOrExpression *exclusiveOrExpression) {
    InclusiveOrExpression::exclusiveOrExpression = exclusiveOrExpression;
}


void InclusiveOrExpression::setInclusiveOrExpression(InclusiveOrExpression *inclusiveOrExpression) {
    InclusiveOrExpression::inclusiveOrExpression = inclusiveOrExpression;
}

string &InclusiveOrExpression::toString() {
    if (inclusiveOrExpression != nullptr) {
        string &ans = inclusiveOrExpression->toString();
        ans += " | " + exclusiveOrExpression->toString();
        return ans;
    } else {
        string &ans = exclusiveOrExpression->toString();
        return ans;
    }
}

bool InclusiveOrExpression::operator==(InclusiveOrExpression &another) {
    if (isInclusiveOrExpr() && another.isInclusiveOrExpr()) {
        return *getInclusiveOrExpression() == *another.getInclusiveOrExpression() &&
               *getExclusiveOrExpression() == *another.getExclusiveOrExpression();
    } else if (isExclusiveOrExpr() && another.isExclusiveOrExpr()) {
        return *getExclusiveOrExpression() == *another.getExclusiveOrExpression();
    }
    return false;
}

ExclusiveOrExpression *InclusiveOrExpression::getExclusiveOrExpression() const {
    return exclusiveOrExpression;
}

InclusiveOrExpression *InclusiveOrExpression::getInclusiveOrExpression() const {
    return inclusiveOrExpression;
}

Type *InclusiveOrExpression::resolve_type(G_Context &context) {
    if (isExclusiveOrExpr())
        return exclusiveOrExpression->resolve_type(context);
    auto left = inclusiveOrExpression->resolve_type(context);
    auto right = exclusiveOrExpression->resolve_type(context);
    if (left != nullptr && right != nullptr) {
        if (*left == right)
            return left;
        if (*left > right)
            return left;
        if (*left < right)
            return right;
    }
    return nullptr;
}


void ExclusiveOrExpression::setAndExpression(AndExpression *andExpression) {
    ExclusiveOrExpression::andExpression = andExpression;
}


void ExclusiveOrExpression::setExclusiveOrExpression(ExclusiveOrExpression *exclusiveOrExpression) {
    ExclusiveOrExpression::exclusiveOrExpression = exclusiveOrExpression;
}

string &ExclusiveOrExpression::toString() {
    if (exclusiveOrExpression != nullptr) {
        string &ans = exclusiveOrExpression->toString();
        ans += " ^ " + andExpression->toString();
        return ans;
    } else {
        string &ans = andExpression->toString();
        return ans;
    }
}

bool ExclusiveOrExpression::operator==(ExclusiveOrExpression &another) {
    if (isExclusiveOrExpr() && another.isExclusiveOrExpr())
        return *getAndExpression() == *another.getAndExpression() &&
               *getExclusiveOrExpression() == *another.getExclusiveOrExpression();
    else if (isAndExpr() && another.isAndExpr())
        return *getAndExpression() == *another.getAndExpression();
    return false;
}

AndExpression *ExclusiveOrExpression::getAndExpression() const {
    return andExpression;
}

ExclusiveOrExpression *ExclusiveOrExpression::getExclusiveOrExpression() const {
    return exclusiveOrExpression;
}

Type *ExclusiveOrExpression::resolve_type(G_Context &context) {
    if (isAndExpr())
        return andExpression->resolve_type(context);
    auto left = exclusiveOrExpression->resolve_type(context);
    auto right = andExpression->resolve_type(context);
    if (left != nullptr && right != nullptr) {
        if (*left == right)
            return left;
        if (*left > right)
            return left;
        if (*left < right)
            return right;
    }
    return nullptr;
}


void AndExpression::setEqualityExpression(EqualityExpression *equalityExpression) {
    AndExpression::equalityExpression = equalityExpression;
}


void AndExpression::setAndExpression(AndExpression *andExpression) {
    AndExpression::andExpression = andExpression;
}

string &AndExpression::toString() {
    if (andExpression != nullptr) {
        string &ans = andExpression->toString();
        ans += " & " + equalityExpression->toString();
        return ans;
    } else {
        string &ans = equalityExpression->toString();
        return ans;
    }
}

bool AndExpression::operator==(AndExpression &another) {
    if (isAndExpr() && another.isAndExpr())
        return *getEqualityExpression() == *another.getEqualityExpression() &&
               *getAndExpression() == *another.getAndExpression();
    else if (isEqualityExpr() && another.isEqualityExpr())
        return *getEqualityExpression() == *another.getEqualityExpression();
    return false;
}

EqualityExpression *AndExpression::getEqualityExpression() const {
    return equalityExpression;
}

AndExpression *AndExpression::getAndExpression() const {
    return andExpression;
}

Type *AndExpression::resolve_type(G_Context &context) {
    if (isEqualityExpr())
        return equalityExpression->resolve_type(context);
    auto left = andExpression->resolve_type(context);
    auto right = equalityExpression->resolve_type(context);
    if (left != nullptr && right != nullptr) {
        if (*left == right)
            return left;
        if (*left > right)
            return left;
        if (*left < right)
            return right;
    }
    return nullptr;
}


void EqualityExpression::setRelationalExpression(RelationalExpression *relationalExpression) {
    EqualityExpression::relationalExpression = relationalExpression;
}


void EqualityExpression::setEqualityExpression(EqualityExpression *equalityExpression) {
    EqualityExpression::equalityExpression = equalityExpression;
}

void EqualityExpression::setOp(string *op) {
    EqualityExpression::op = op;
}

string &EqualityExpression::toString() {
    if (equalityExpression != nullptr) {
        string &ans = equalityExpression->toString();
        ans += *op + relationalExpression->toString();
        return ans;
    } else {
        string &ans = relationalExpression->toString();
        return ans;
    }
}

bool EqualityExpression::operator==(EqualityExpression &another) {
    if (isEqualityExpr() && another.isEqualityExpr()) {
        return *getRelationalExpression() == *another.getRelationalExpression() &&
               *getEqualityExpression() == *another.getEqualityExpression() && *getOp() == *another.getOp();
    } else if (isRelationExpr() && another.isRelationExpr())
        return *getRelationalExpression() == *another.getRelationalExpression();
    return false;
}

RelationalExpression *EqualityExpression::getRelationalExpression() const {
    return relationalExpression;
}

EqualityExpression *EqualityExpression::getEqualityExpression() const {
    return equalityExpression;
}

string *EqualityExpression::getOp() const {
    return op;
}

Type *EqualityExpression::resolve_type(G_Context &context) {
    if (isRelationExpr())
        return relationalExpression->resolve_type(context);
    return new BuiltinBoolType(nullptr);
}

void RelationalExpression::setShiftExpression(ShiftExpression *shiftExpression) {
    RelationalExpression::shiftExpression = shiftExpression;
}


void RelationalExpression::setRelationalExpression(RelationalExpression *relationalExpression) {
    RelationalExpression::relationalExpression = relationalExpression;
}


void RelationalExpression::setOp(string *op) {
    RelationalExpression::op = op;
}

string &RelationalExpression::toString() {
    if (relationalExpression != nullptr) {
        string &ans = relationalExpression->toString();
        ans += *op + shiftExpression->toString();
        return ans;
    } else {
        string &ans = shiftExpression->toString();
        return ans;
    }
}

bool RelationalExpression::operator==(RelationalExpression &another) {
    if (isShiftExpr() && another.isShiftExpr())
        return *getShiftExpression() == *another.getShiftExpression();
    else if (isRelationExpr() && another.isRelationExpr())
        return *getRelationalExpression() == *another.getRelationalExpression() && *getOp() == *another.getOp() &&
               *getShiftExpression() == *another.getShiftExpression();
    return false;
}

ShiftExpression *RelationalExpression::getShiftExpression() const {
    return shiftExpression;
}

RelationalExpression *RelationalExpression::getRelationalExpression() const {
    return relationalExpression;
}

string *RelationalExpression::getOp() const {
    return op;
}

Type *RelationalExpression::resolve_type(G_Context &context) {
    if (isShiftExpr())
        return shiftExpression->resolve_type(context);
    return new BuiltinBoolType(nullptr);
}


void ShiftExpression::setAdditiveExpression(AdditiveExpression *additiveExpression) {
    ShiftExpression::additiveExpression = additiveExpression;
}


void ShiftExpression::setShiftExpression(ShiftExpression *shiftExpression) {
    ShiftExpression::shiftExpression = shiftExpression;
}


void ShiftExpression::setOp(string *op) {
    ShiftExpression::op = op;
}

string &ShiftExpression::toString() {
    if (shiftExpression != nullptr) {
        string &ans = shiftExpression->toString();
        ans += *op + additiveExpression->toString();
        return ans;
    } else {
        string &ans = additiveExpression->toString();
        return ans;
    }
}

bool ShiftExpression::operator==(ShiftExpression &another) {
    if (isAdditiveExpr() && another.isAdditiveExpr()) {
        return *getAdditiveExpression() == *another.getAdditiveExpression();
    } else if (isShiftExpr() && another.isShiftExpr())
        return *getShiftExpression() == *another.getShiftExpression() && *getOp() == *another.getOp() &&
               *getAdditiveExpression() == *another.getAdditiveExpression();
    return false;
}

AdditiveExpression *ShiftExpression::getAdditiveExpression() const {
    return additiveExpression;
}

ShiftExpression *ShiftExpression::getShiftExpression() const {
    return shiftExpression;
}

string *ShiftExpression::getOp() const {
    return op;
}

Type *ShiftExpression::resolve_type(G_Context &context) {
    if (isAdditiveExpr())
        return additiveExpression->resolve_type(context);

    Type *left_type = shiftExpression->resolve_type(context);
    if (left_type != nullptr) {
        return left_type;
    }
    return nullptr;
}


void AdditiveExpression::setMultiplicativeExpression(MultiplicativeExpression *multiplicativeExpression) {
    AdditiveExpression::multiplicativeExpression = multiplicativeExpression;
}


void AdditiveExpression::setAdditiveExpression(AdditiveExpression *additiveExpression) {
    AdditiveExpression::additiveExpression = additiveExpression;
}


void AdditiveExpression::setOp(string *op) {
    AdditiveExpression::op = op;
}

string &AdditiveExpression::toString() {
    if (additiveExpression != nullptr) {
        string &ans = additiveExpression->toString();
        ans += *op + multiplicativeExpression->toString();
        return ans;
    } else {
        string &ans = multiplicativeExpression->toString();
        return ans;
    }
}

bool AdditiveExpression::operator==(AdditiveExpression &another) {
    if (isAdditiveExpr() && another.isAdditiveExpr()) {
        return *getAdditiveExpression() == *another.getAdditiveExpression() && *getOp() == *another.getOp() &&
               *getMultiplicativeExpression() == *another.getMultiplicativeExpression();
    } else if (isMultiExpr())
        return *getMultiplicativeExpression() == *another.getMultiplicativeExpression();
    return false;
}

MultiplicativeExpression *AdditiveExpression::getMultiplicativeExpression() const {
    return multiplicativeExpression;
}

AdditiveExpression *AdditiveExpression::getAdditiveExpression() const {
    return additiveExpression;
}

string *AdditiveExpression::getOp() const {
    return op;
}

Type *AdditiveExpression::resolve_type(G_Context &context) {
    if (isMultiExpr())
        return multiplicativeExpression->resolve_type(context);

    Type *left_type = additiveExpression->resolve_type(context);
    Type *right_type = multiplicativeExpression->resolve_type(context);
    if (left_type != nullptr && right_type != nullptr) {
        if (*left_type == right_type)
            return left_type;
        if (*left_type < right_type)
            return right_type;
        if (*left_type > right_type)
            return left_type;
        return left_type;
    }
    return nullptr;
}


void MultiplicativeExpression::setCastExpression(CastExpression *castExpression) {
    MultiplicativeExpression::castExpression = castExpression;
}


void MultiplicativeExpression::setMultiplicativeExpression(MultiplicativeExpression *multiplicativeExpression) {
    MultiplicativeExpression::multiplicativeExpression = multiplicativeExpression;
}


void MultiplicativeExpression::setOp(string *op) {
    MultiplicativeExpression::op = op;
}

string &MultiplicativeExpression::toString() {
    if (multiplicativeExpression != nullptr) {
        string &ans = multiplicativeExpression->toString();
        ans += *op + castExpression->toString();
        return ans;
    } else {
        string &ans = castExpression->toString();
        return ans;
    };
}

bool MultiplicativeExpression::operator==(MultiplicativeExpression &another) {
    if (isCastExpr() && another.isCastExpr()) {
        return *getCastExpression() == *another.getCastExpression();
    } else if (isMultiExpr() && another.isMultiExpr()) {
        return *getMultiplicativeExpression() == *another.getMultiplicativeExpression() &&
               *getOp() == *another.getOp() && *getCastExpression() == *another.getCastExpression();
    }
    return false;
}

CastExpression *MultiplicativeExpression::getCastExpression() const {
    return castExpression;
}

MultiplicativeExpression *MultiplicativeExpression::getMultiplicativeExpression() const {
    return multiplicativeExpression;
}

string *MultiplicativeExpression::getOp() const {
    return op;
}

Type *MultiplicativeExpression::resolve_type(G_Context &context) {
    if (isCastExpr())
        return castExpression->resolve_type(context);

    Type *left_type = multiplicativeExpression->resolve_type(context);
    Type *right_type = castExpression->resolve_type(context);
    if (left_type != nullptr && right_type != nullptr) {
        if (*left_type == right_type)
            return left_type;
        if (*left_type < right_type)
            return right_type;
        if (*left_type > right_type)
            return left_type;
        return left_type;
    }
    return nullptr;
}


void IfStatement::setExpression(Expression *expression) {
    IfStatement::expression = expression;
}


void IfStatement::setT_case(CompoundStatement *t_case) {
    IfStatement::t_case = t_case;
}


void IfStatement::setF_case(CompoundStatement *f_case) {
    IfStatement::f_case = f_case;
}

string &IfStatement::toString() {
    string &ans = *(new string("if ( "));
    ans += expression->toString();
    ans += " ) \n";
    ans += t_case->toString();
    if (f_case != nullptr) {
        ans += "\n else \n";
        ans += f_case->toString();
    }
    return ans;
}

bool IfStatement::hasFCase() { return f_case != nullptr; }

Expression *IfStatement::getExpression() const {
    return expression;
}

CompoundStatement *IfStatement::getT_case() const {
    return t_case;
}

CompoundStatement *IfStatement::getF_case() const {
    return f_case;
}

string &ExpressionStatement::toString() {
    if (expression == nullptr)
        return *(new string(";"));
    else {
        string &ans = expression->toString();
        ans += " ;\n";
        return ans;
    }
}

Type::Type(string *val) : Node() {
    this->value = val;
}

string &Type::getValue() {
    if (this->value == nullptr) {
        return *(new string(""));
    }
    return *this->value;
}

int Type::get_priority() {
    int i = 0;
    ++i;
    if (auto builtin_void = dynamic_cast<BuiltinVoidType *>(this)) {
        return i;
    }
    ++i;
    if (auto builtin_bool = dynamic_cast<BuiltinBoolType *>(this)) {
        return i;
    }
    ++i;
    if (auto builtin_sizeof = dynamic_cast<BuiltinSizeofType *>(this)) {
        return i;
    }
    ++i;
    if (auto builtin_int = dynamic_cast<BuiltinIntType *>(this)) {
        return i;
    }
    ++i;
    if (auto builtin_tid = dynamic_cast<BuiltinThreadIdType *>(this)) {
        return i;
    }
    ++i;
    if (auto builtin_ptr = dynamic_cast<BuiltinPointerType *>(this)) {
        return i;
    }
    ++i;
    if (auto builtin_double = dynamic_cast<BuiltinDoubleType *>(this)) {
        return i;
    }
    ++i;
    if (auto builtin_string = dynamic_cast<BuiltinStringType *>(this)) {
        return i;
    }
    ++i;
    if (auto builtin_cpp = dynamic_cast<BuiltinCPPType *>(this)) {
        return i;
    }
    ++i;
    if (auto builtin_set = dynamic_cast<BuiltinSet *>(this)) {
        return i;
    }
    ++i;
    if (auto builtin_map = dynamic_cast<BuiltinMap *>(this)) {
        return i;
    }
    if (auto builtin_clazz = dynamic_cast<ClazzType *>(this)) {
        return -1;
    }
    if (auto builtin_efunc = dynamic_cast<ExternalFunctionType *>(this)) {
        return -2;
    }
    if (auto builtin_llvm = dynamic_cast<LLVMInstructionType *>(this)) {
        return -3;
    }
    if (auto builtin_func = dynamic_cast<FunctionType *>(this)) {
        return -4;
    }
    return 0;
}

BuiltinType::BuiltinType(string *value) : Type(value) {
}

string BuiltinType::typeName() {
    return string("BuiltinType");
}

string BuiltinElementType::typeName() {
    return string("BuiltinElementType");
}

BuiltinPointerType::BuiltinPointerType(string *value) : BuiltinElementType(value) {
    this->type = "pointer";
    this->real_cpp_Type = "void *";
}

BuiltinPointerType::BuiltinPointerType(string *value, bool sync) : BuiltinElementType(value) {
    this->type = "pointer";
    this->real_cpp_Type = "void *";
    this->sync = sync;
}

bool BuiltinPointerType::isSync() const {
    return sync;
}

string &CombinedStatements::toString() {
    string &ans = *(new string());
    for (auto stmt: statements) {
        ans += stmt->toString() + "\n";
    }
    return ans;
}

Node *Node::getParent() {
    return parent;
}

unordered_map<string, Type *> &ClazzType::getFields() {
    return fields;
}

void RootNode::setStatements(const vector<Node *> &statements) {
    RootNode::statements = statements;
}

unordered_map<string, Type *> &G_Context::getScalar_type() {
    return scalar_type;
}

void G_Context::setScalar_type(const unordered_map<string, Type *> &scalar_type) {
    G_Context::scalar_type = scalar_type;
}


Type *G_Context::query_scalar_type(string scalar) {
    if (scalar_type.count(scalar))
        return scalar_type[scalar];
    return nullptr;
}

Type *G_Context::query_method_type(string scalar) {
    if (scalar_type.count(scalar))
        return scalar_type[scalar];
    return nullptr;
}

G_Context::G_Context() {
    string &scope = *(new string("bottom"));
    this->scalar_type["set"] = new BuiltinSet(nullptr, scope, nullptr);
    this->scalar_type["size"] = new BuiltinIntType(nullptr);
    this->scalar_type["find"] = new BuiltinBoolType(nullptr);
    this->scalar_type["warning"] = new BuiltinVoidType(nullptr);
    this->scalar_type["remove"] = new BuiltinVoidType(nullptr);
    this->scalar_type["remove_range"] = new BuiltinVoidType(nullptr);
}

bool Type::operator==(Type *another) {
    int left = this->get_priority();
    int right = another->get_priority();
    if (left == right) {
        if (auto l = dynamic_cast<BuiltinSet *>(this)) {
            auto r = dynamic_cast<BuiltinSet *>(another);
            return l->getScope() == r->getScope() &&
                   (*l->getElementType()->getType()) == r->getElementType()->getType();
        }
        if (auto l = dynamic_cast<BuiltinMap *>(this)) {
            auto r = dynamic_cast<BuiltinMap *>(another);
            if (l->getScope() == r->getScope() && (*l->getKeyType()->getType()) == r->getKeyType()->getType()) {
                if (!(*l->getValueType()->getType() == r->getValueType()->getType())) {
                    if (auto clazz = dynamic_cast<ClazzType *>(l->getValueType()->getType())) {
                        for (auto &it : clazz->getFields()) {
                            if (*it.second == r->getValueType()->getType())
                                return true;
                        }
                    }
                    if (auto clazz = dynamic_cast<ClazzType *>(r->getValueType()->getType())) {
                        for (auto &it : clazz->getFields()) {
                            if (*it.second == l->getValueType()->getType())
                                return true;
                        }
                    }
                } else
                    return true;
            }
            return false;
        }
        if (auto l = dynamic_cast<BuiltinElementType *>(this)) {
            auto r = dynamic_cast<BuiltinElementType *>(another);
            return l->getValue() == r->getValue();
        }
        return true;
    }
    return false;
}

bool Type::operator<(Type *another) {
    return this->get_priority() < another->get_priority();
}

bool Type::operator>(Type *another) {
    return this->get_priority() > another->get_priority();
}

void BuiltinSet::setElementType(VariableDefinition *elementType) {
    BuiltinSet::elementType = elementType;
}

string &BuiltinSet::toString() {
    string &ans = *(new string(this->getScope()));
    ans += "::set<" + this->getElementType()->getType()->toString() + ">";
    return ans;
}

string &BuiltinMap::toString() {
    string &ans = *(new string(this->getScope()));
    ans += "::map<" + this->getKeyType()->getType()->toString() + ", " + this->getValueType()->getType()->toString() +
           ">";
    return ans;
}

string &ReturnStatement::toString() {
    string &ans = *(new string("return "));
    if (expression != nullptr)
        ans += expression->toString() + ";";
    else
        ans += ";";
    return ans;
}
