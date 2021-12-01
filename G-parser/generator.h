//
// Created by cxworks on 19-6-9.
//

#ifndef G_PARSER_GENERATOR_H
#define G_PARSER_GENERATOR_H


#include "statement.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <fstream>
#include <stdlib.h>
using namespace std;
using namespace boost::property_tree;

static int shift = 3;
ptree parseFunctionDef(FunctionDefinition* funcDef){
    ptree ans;
    ans.put("name", funcDef->getScalarName());

    vector<FunctionParamDefinition *> params_vector = funcDef->getParameters();
    ptree params;
    bool most_once = false;
    for (auto it  = params_vector.begin(); it != params_vector.end() ; ++it) {
        Type* type = (*it)->getType();
        if(auto primitive = dynamic_cast<BuiltinCPPType*>(type)){
            ptree param;
            param.put("name", (*it)->getScalarName());
            param.put("type", primitive->getType());
            param.put("cpp_type", primitive->get_real_cpp_type());
            param.put("alias", (*it)->getAlias());
            param.put("sync", primitive->isSync());
            param.put("size", primitive->getLength());
            params.push_back(make_pair("", param));
        }
    }
    ans.push_back(make_pair("params", params));
    return ans;
}

void parseAST2opt(RootNode* root){
    vector<Node*> statements = root->getChildren();
    ptree opt_plugin;
    for (auto it = statements.begin(); it != statements.end(); it++){
        Node* node  = *it;
        if(auto * insertDef = dynamic_cast<InsertStatement*>(node)) {
            ptree insert_node;
            insert_node.put("shift", insertDef->getShift());
            insert_node.put("location", insertDef->getLocation()->getScalarName());
            insert_node.add_child("function", parseFunctionDef(insertDef->getFunctionDef()));
            if (insertDef->getCalledParams() != nullptr) {
                ptree params;
                for (string *pname:*(insertDef->getCalledParams())) {
                    ptree param;
                    param.put("name", *pname);
                    params.push_back(make_pair("", param));
                }
                insert_node.add_child("params", params);
            }
            if (insertDef->getReturnDef() != nullptr) {
                ptree return_info;
                return_info.put("def", *insertDef->getReturnDef());
                return_info.put("type", insertDef->getFunctionDef()->getReturnValue()->getType()->toString());
                insert_node.add_child("return_info", return_info);
            }
            opt_plugin.push_back(make_pair("", insert_node));
        }

    }
    ptree output;
    output.add_child("instr", opt_plugin);
    output.put("shift", shift);
//    write_json(cout, output, true);
    ofstream out;
    out.open("opt.json", ios::in|ios::out|ios::trunc);
    write_json(out, output, true);
}

ptree resolve_type(Type* builtinType){
    ptree ans;
    if(auto builtinElement = dynamic_cast<BuiltinElementType*>(builtinType)) {
        ans.put("type", "element");
        ans.put("type_name", builtinElement->typeName());
        ans.put("real_cpp_type", builtinElement->get_real_cpp_type());
        if (auto lockid = dynamic_cast<BuiltinLockIdType *>(builtinElement)) {
            ans.put("size", lockid->getLength());
        } else if (auto tid = dynamic_cast<BuiltinThreadIdType *>(builtinElement)) {
            ans.put("size", tid->getLength());
        } else if (auto ptr = dynamic_cast<BuiltinPointerType *>(builtinElement)) {
            ans.put("sync", ptr->isSync());
        } else if (auto otid = dynamic_cast<BuiltinOThreadIdType *>(builtinElement)) {
            ans.put("size", otid->getLength());
        } else if (auto bcpp = dynamic_cast<BuiltinCPPType *>(builtinElement)) {
            if (bcpp->getLength() > 0)
            ans.put("size", bcpp->getLength());
            if(bcpp->isSync())
                ans.put("sync", bcpp->isSync());
        }
    } else if(auto builtInCollection = dynamic_cast<BuiltinCollectionType*>(builtinType)){
        if(auto builtinMap = dynamic_cast<BuiltinMap*>(builtInCollection)){
            ans.put("type", "map");
            ans.put("scope", builtinMap->getScope());
            ans.put("type_name", builtinMap->typeName());
            ptree k = resolve_type(builtinMap->getKeyType()->getType());

            k.put("scalar", builtinMap->getKeyType()->getScalarName());
            ans.add_child("key", k);
            ptree v = resolve_type(builtinMap->getValueType()->getType());
            v.put("scalar", builtinMap->getValueType()->getScalarName());
            ans.add_child("value", v);
        } else if(auto builtinSet = dynamic_cast<BuiltinSet*>(builtInCollection)){
            ans.put("type", "set");
            ans.put("type_name", builtinSet->typeName());
            ans.put("scope", builtinSet->getScope());
            ptree e = resolve_type(builtinSet->getElementType()->getType());
            e.put("scalar", builtinSet->getElementType()->getScalarName());
            ans.add_child("element", e);
        }

    } else if (auto builtinClazz = dynamic_cast<ClazzType *>(builtinType)) {
        ans.put("type", "clazz");
        ans.put("type_name", "ClazzType");
        ans.put("clazz_name", builtinClazz->getClazzName());
        ans.put("scalar", builtinClazz->getValue());
        ptree fields;
        for (auto &it : builtinClazz->getFields()) {
            ptree k = resolve_type(it.second);
            k.put("scalar", it.first);
            fields.push_back(make_pair("", k));
        }
        ans.add_child("fields", fields);
    }
    return ans;
}

InsertStatement* find_insert_point(string& funcname, RootNode *root){
    vector<Node *> statements = root->getChildren();

    for (auto it = statements.begin(); it != statements.end(); it++) {
        Node* node = *it;
        if (auto insert = dynamic_cast<InsertStatement *>(node)) {
            if(insert->getFunctionDef()->getScalarName() == funcname){
                return insert;
            }
        }
    }
    return nullptr;
}

void parseAST2cpp(RootNode *root, int shift = 0) {
    vector<Node *> statements = root->getChildren();
    ptree info;
    for (auto it = statements.begin(); it != statements.end(); it++) {
        Node *node = *it;
        if (auto varDef = dynamic_cast<VariableDefinition *>(node)) {
            if (auto builtInType = dynamic_cast<BuiltinType *>(varDef->getType())) {
                ptree type = resolve_type(builtInType);
                type.put("scalar", varDef->getScalarName());
                type.put("type_def", varDef->isTypeDef);
                info.push_back(make_pair(varDef->getScalarName(), type));
            }

        } else if(auto * funcDef = dynamic_cast<FunctionDefinition*>(node)) {
            ptree func;
            func.put("type", "func");
            func.put("name", funcDef->getScalarName());
            func.put("sync", funcDef->isSync());
            if (funcDef->getReturnValue() != nullptr) {
                func.put("return_value", funcDef->getReturnValue()->getScalarName());
                func.put("return_type", funcDef->getReturnValue()->getType()->typeName());
            } else {
                func.put("return_value", "void");
            }

            //bodies
            func.put("body", funcDef->getBody()->toString());
            //params
            ptree params;
            InsertStatement* insert = find_insert_point( funcDef->getScalarName(), root);
            int counter = 0;
            for (auto it = funcDef->getParameters().begin(); it != funcDef->getParameters().end(); ++it) {
                if (auto builtInType = dynamic_cast<BuiltinType *>((*it)->getType())) {
                    ptree type = resolve_type(builtInType);
                    type.put("scalar", (*it)->getScalarName());
                    type.put("alias", (*it)->getAlias());
                    if(insert!= nullptr && insert->getCalledParams()!= nullptr && counter < insert->getCalledParams()->size()){
                        type.put("insert", *(*(insert->getCalledParams()->begin()+counter)));
                    }
                    params.push_back(make_pair("", type));
                }
                counter++;
            }
            func.add_child("params", params);
            if (funcDef->is_var_arg()) {
                func.put("var_arg", "true");
            } else
                func.put("var_arg", "false");
            for(auto it2 = statements.begin(); it2 != statements.end(); it2++){
                Node* node  = *it2;
                if(auto * insertDef = dynamic_cast<InsertStatement*>(node)) {
                    if(insertDef->getFunctionDef()->getScalarName() == funcDef->getScalarName()){
                        func.put("location", insertDef->getLocation()->getScalarName());
                    }
                }
            }
            info.push_back(make_pair(funcDef->getScalarName(), func));
        }
    }
    info.put("shift", shift);
//    write_json(cout, info, true);
    ofstream out;
    out.open("cxx.json", ios::in|ios::out|ios::trunc);
    write_json(out, info, true);
};

template<class T>
void iterate_all(FunctionBodyExpression *funcBody, vector<T *> &ans) {
    if (funcBody == nullptr)
        return;
    if (auto tar = dynamic_cast<T *>(funcBody)) {
        ans.push_back(tar);
    }
    if (auto expr = dynamic_cast<Expression *>(funcBody)) {
        for (auto assign : expr->getExprs()) {
            iterate_all<T>(assign, ans);
        }
    } else if (auto assignExpr = dynamic_cast<AssignmentExpression *>(funcBody)) {
        if (assignExpr->isAssignExpr()) {
            iterate_all(assignExpr->getUnaryExpression(), ans);
            iterate_all(assignExpr->getNext(), ans);
        } else if (assignExpr->isConditionalExpr()) {
            iterate_all(assignExpr->getConditionalExpression(), ans);
        }
    } else if (auto castExpr = dynamic_cast<CastExpression *>(funcBody)) {
        if (castExpr->isUnaryExpr()) {
            iterate_all(castExpr->getUnaryExpression(), ans);
        } else if (castExpr->isCastExpr()) {
            iterate_all(castExpr->getCastExpression(), ans);
        }
    } else if (auto unaryExpr = dynamic_cast<UnaryExpression *>(funcBody)) {
        if (unaryExpr->isCastExpr()) {
            iterate_all(unaryExpr->getCastExpression(), ans);
        } else if (unaryExpr->isUnaryExpr()) {
            iterate_all(unaryExpr->getUnaryExpression(), ans);
        } else if (unaryExpr->isPostfixExpr()) {
            iterate_all(unaryExpr->getPostfixExpression(), ans);
        }
    } else if (auto args = dynamic_cast<ArgumentExpressionList *>(funcBody)) {
        for (auto arg : args->getStatements()) {
            iterate_all(arg, ans);
        }
    } else if (auto postfixExpr = dynamic_cast<PostfixExpression *>(funcBody)) {
        if (postfixExpr->isPrimaryExpr()) {
            iterate_all(postfixExpr->getPrimaryExpression(), ans);
        } else if (postfixExpr->isArrayLookup()) {
            iterate_all(postfixExpr->getPostfixExpression(), ans);
            iterate_all(postfixExpr->getExpression(), ans);
        } else if (postfixExpr->isArgMethodCall()) {
            iterate_all(postfixExpr->getPostfixExpression(), ans);
            iterate_all(postfixExpr->getArgumentExpressionList(), ans);
        } else if (postfixExpr->isNoArgMethodCall()) {
            iterate_all(postfixExpr->getPostfixExpression(), ans);
        } else if (postfixExpr->isFieldAccess()) {
            iterate_all(postfixExpr->getPostfixExpression(), ans);
        } else if (postfixExpr->isSelfOp()) {
            iterate_all(postfixExpr->getPostfixExpression(), ans);
        }
    } else if (auto primaryExpr = dynamic_cast<PrimaryExpression *>(funcBody)) {
        if (primaryExpr->isExpression())
            iterate_all(primaryExpr->getExpr(), ans);
    } else if (auto condExpr = dynamic_cast<ConditionalExpression *>(funcBody)) {
        if (condExpr->isLogicalOrExpr())
            iterate_all(condExpr->getLogicalOrExpression(), ans);
        else if (condExpr->is3OpCondition()) {
            iterate_all(condExpr->getLogicalOrExpression(), ans);
            iterate_all(condExpr->getExpression(), ans);
            iterate_all(condExpr->getConditionalExpression(), ans);
        }
    } else if (auto logicalOrExpr = dynamic_cast<LogicalOrExpression *>(funcBody)) {
        if (logicalOrExpr->isLogicalAndExpr())
            iterate_all(logicalOrExpr->getLogicalAndExpression(), ans);
        else if (logicalOrExpr->isLogicalOrExpr()) {
            iterate_all(logicalOrExpr->getLogicalOrExpression(), ans);
            iterate_all(logicalOrExpr->getLogicalAndExpression(), ans);
        }
    } else if (auto logicalAndExpr = dynamic_cast<LogicalAndExpression *>(funcBody)) {
        if (logicalAndExpr->isInclusiveOrExpr())
            iterate_all(logicalAndExpr->getInclusiveOrExpression(), ans);
        else if (logicalAndExpr->isLogicalAndExpr()) {
            iterate_all(logicalAndExpr->getLogicalAndExpression(), ans);
            iterate_all(logicalAndExpr->getInclusiveOrExpression(), ans);
        }
    } else if (auto inclusiveOrExpr = dynamic_cast<InclusiveOrExpression *>(funcBody)) {
        if (inclusiveOrExpr->isExclusiveOrExpr()) {
            iterate_all(inclusiveOrExpr->getExclusiveOrExpression(), ans);
        } else if (inclusiveOrExpr->isInclusiveOrExpr()) {
            iterate_all(inclusiveOrExpr->getInclusiveOrExpression(), ans);
            iterate_all(inclusiveOrExpr->getExclusiveOrExpression(), ans);
        }
    } else if (auto exclusiveExpr = dynamic_cast<ExclusiveOrExpression *>(funcBody)) {
        if (exclusiveExpr->isAndExpr())
            iterate_all(exclusiveExpr->getAndExpression(), ans);
        else if (exclusiveExpr->isExclusiveOrExpr()) {
            iterate_all(exclusiveExpr->getExclusiveOrExpression(), ans);
            iterate_all(exclusiveExpr->getAndExpression(), ans);
        }
    } else if (auto andExpr = dynamic_cast<AndExpression *>(funcBody)) {
        if (andExpr->isEqualityExpr()) {
            iterate_all(andExpr->getEqualityExpression(), ans);
        } else if (andExpr->isAndExpr()) {
            iterate_all(andExpr->getAndExpression(), ans);
            iterate_all(andExpr->getEqualityExpression(), ans);
        }
    } else if (auto equalityExpr = dynamic_cast<EqualityExpression *>(funcBody)) {
        if (equalityExpr->isRelationExpr()) {
            iterate_all(equalityExpr->getRelationalExpression(), ans);
        } else if (equalityExpr->isEqualityExpr()) {
            iterate_all(equalityExpr->getEqualityExpression(), ans);
            iterate_all(equalityExpr->getRelationalExpression(), ans);
        }
    } else if (auto relationExpr = dynamic_cast<RelationalExpression *>(funcBody)) {
        if (relationExpr->isShiftExpr()) {
            iterate_all(relationExpr->getShiftExpression(), ans);
        } else if (relationExpr->isRelationExpr()) {
            iterate_all(relationExpr->getRelationalExpression(), ans);
            iterate_all(relationExpr->getShiftExpression(), ans);
        }
    } else if (auto shiftExpr = dynamic_cast<ShiftExpression *>(funcBody)) {
        if (shiftExpr->isAdditiveExpr())
            iterate_all(shiftExpr->getAdditiveExpression(), ans);
        else if (shiftExpr->isShiftExpr()) {
            iterate_all(shiftExpr->getShiftExpression(), ans);
            iterate_all(shiftExpr->getAdditiveExpression(), ans);
        }
    } else if (auto addExpr = dynamic_cast<AdditiveExpression *>(funcBody)) {
        if (addExpr->isMultiExpr()) {
            iterate_all(addExpr->getMultiplicativeExpression(), ans);
        } else if (addExpr->isAdditiveExpr()) {
            iterate_all(addExpr->getAdditiveExpression(), ans);
            iterate_all(addExpr->getMultiplicativeExpression(), ans);
        }
    } else if (auto multiExpr = dynamic_cast<MultiplicativeExpression *>(funcBody)) {
        if (multiExpr->isCastExpr())
            iterate_all(multiExpr->getCastExpression(), ans);
        else if (multiExpr->isMultiExpr()) {
            iterate_all(multiExpr->getMultiplicativeExpression(), ans);
            iterate_all(multiExpr->getCastExpression(), ans);
        }

    } else
        throw funcBody->toString();
}

template<class T>
void iterate_all(FunctionBodyStatement *stmt, vector<T *> &ans) {
    if (auto combined = dynamic_cast<CombinedStatements *>(stmt)) {
        for (auto st : combined->getStatements()) {
            iterate_all(st, ans);
        }
    } else if (auto comp = dynamic_cast<CompoundStatement *>(stmt)) {
        for (auto st : comp->getStatements()) {
            iterate_all(st, ans);
        }
    } else if (auto ifst = dynamic_cast<IfStatement *>(stmt)) {
        iterate_all(ifst->getExpression(), ans);
        iterate_all(ifst->getT_case(), ans);
        if (ifst->hasFCase())
            iterate_all(ifst->getF_case(), ans);
    } else if (auto exprst = dynamic_cast<ExpressionStatement *>(stmt)) {
        iterate_all(exprst->getExpression(), ans);
    } else if (auto retst = dynamic_cast<ReturnStatement *>(stmt)) {
        iterate_all(retst->getExpression(), ans);
    }
}

Node *trace_nearest_binary_operator(Node *child) {
    Node *parent = child->getParent();
    if (auto multiExpr = dynamic_cast<MultiplicativeExpression *>(parent)) {
        if (multiExpr->isCastExpr()) {
            return trace_nearest_binary_operator(multiExpr);
        } else if (multiExpr->isMultiExpr()) {
            return child;
        }
    } else if (auto addExpr = dynamic_cast<AdditiveExpression *>(parent)) {
        if (addExpr->isMultiExpr()) {
            return trace_nearest_binary_operator(addExpr);
        } else if (addExpr->isAdditiveExpr())
            return child;
    } else if (auto shiftExpr = dynamic_cast<ShiftExpression *>(parent)) {
        if (shiftExpr->isAdditiveExpr())
            return trace_nearest_binary_operator(shiftExpr);
        else if (shiftExpr->isShiftExpr())
            return child;
    } else if (auto relatExpr = dynamic_cast<RelationalExpression *>(parent)) {
        if (relatExpr->isShiftExpr())
            return trace_nearest_binary_operator(relatExpr);
        else if (relatExpr->isRelationExpr())
            return child;
    } else if (auto equalityExpr = dynamic_cast<EqualityExpression *>(parent)) {
        if (equalityExpr->isRelationExpr())
            return trace_nearest_binary_operator(equalityExpr);
        else if (equalityExpr->isEqualityExpr())
            return child;
    } else if (auto andExpr = dynamic_cast<AndExpression *>(parent)) {
        if (andExpr->isEqualityExpr()) {
            return trace_nearest_binary_operator(andExpr);
        } else if (andExpr->isAndExpr())
            return child;
    } else if (auto exclusiveOrExpr = dynamic_cast<ExclusiveOrExpression *>(parent)) {
        if (exclusiveOrExpr->isExclusiveOrExpr()) {
            return child;
        } else if (exclusiveOrExpr->isAndExpr())
            return trace_nearest_binary_operator(exclusiveOrExpr);
    } else if (auto inclusiveOrExpr = dynamic_cast<InclusiveOrExpression *>(parent)) {
        if (inclusiveOrExpr->isInclusiveOrExpr()) {
            return child;
        } else if (inclusiveOrExpr->isExclusiveOrExpr())
            return trace_nearest_binary_operator(inclusiveOrExpr);
    } else if (auto logicalAndExpr = dynamic_cast<LogicalAndExpression *>(parent)) {
        if (logicalAndExpr->isInclusiveOrExpr())
            return trace_nearest_binary_operator(logicalAndExpr->getParent());
        else
            return child;
    } else if (auto logicalOrExpr = dynamic_cast<LogicalOrExpression *>(parent)) {
        if (logicalOrExpr->isLogicalAndExpr())
            return trace_nearest_binary_operator(logicalOrExpr);
        else if (logicalOrExpr->isLogicalOrExpr())
            return child;
    } else if (auto unaryExpr = dynamic_cast<UnaryExpression *>(parent)) {
        if (unaryExpr->isPostfixExpr())
            return trace_nearest_binary_operator(parent);
        else
            return child;
    } else if (auto primaryExpr = dynamic_cast<PrimaryExpression *>(parent)) {
        if (primaryExpr->isExpression()) {
            return trace_nearest_binary_operator(primaryExpr);
        } else
            return child;
    } else if (auto castExpr = dynamic_cast<CastExpression *>(parent)) {
        if (castExpr->isUnaryExpr())
            return trace_nearest_binary_operator(parent);
        else
            return child;
    }
    return nullptr;
}



//void trace_operator(vector<Node*> v, int idx, vector<string>& left_ans, vector<string>& right_ans){
//    if(idx>=v.size())
//        return;
//    Node* current = v[idx];
//    Node* child = v[idx - 1];
//    if(auto multiExpr =  dynamic_cast<MultiplicativeExpression*>(current)){
//        if(multiExpr->isMultiExpr()){
//            if(auto right = dynamic_cast<CastExpression*>(child)){
//                left_ans.push_back(*multiExpr->getOp());
//            } else if(auto left = dynamic_cast<MultiplicativeExpression*>(child)){
//                right_ans.push_back(*multiExpr->getOp());
//            }
//        }
//    } else if(auto addExpr = dynamic_cast<AdditiveExpression*>(current)){
//        if(addExpr->isAdditiveExpr()){
//            if(auto right = dynamic_cast<MultiplicativeExpression*>(child))
//                left_ans.push_back(*addExpr->getOp());
//            else if(auto left = dynamic_cast<AdditiveExpression*>(child)){
//                right_ans.push_back(*addExpr->getOp());
//            }
//        }
//    } else if(auto andExpr = dynamic_cast<AndExpression*>(current)){
//        if(andExpr->isAndExpr()){
//            if(auto right = dynamic_cast<EqualityExpression*>(child))
//                left_ans.emplace_back("&");
//            else if(auto left = dynamic_cast<AndExpression*>(child)){
//                right_ans.emplace_back("&");
//            }
//        }
//    } else if(auto exclusiveOrExpr = dynamic_cast<ExclusiveOrExpression*>(current)){
//        if(exclusiveOrExpr->isExclusiveOrExpr()){
//            if(auto right = dynamic_cast<AndExpression*>(child))
//                left_ans.emplace_back("^");
//            else if(auto left = dynamic_cast<ExclusiveOrExpression*>(child)){
//                right_ans.emplace_back("^");
//            }
//        }
//    }else if(auto inclusiveOrExpr = dynamic_cast<InclusiveOrExpression*>(current)){
//        if(inclusiveOrExpr->isInclusiveOrExpr()){
//            if(auto right = dynamic_cast<ExclusiveOrExpression*>(child))
//                left_ans.emplace_back("|");
//            else if(auto left = dynamic_cast<InclusiveOrExpression*>(child)){
//                right_ans.emplace_back("|");
//            }
//        }
//    }else if(auto primaryExpr = dynamic_cast<PrimaryExpression*>(current)){
//        if(primaryExpr->isExpression()){
//            if(auto right = dynamic_cast<Expression*>(child))
//                left_ans.emplace_back("(");
//            else if(auto left = dynamic_cast<PrimaryExpression*>(child)){
//                right_ans.emplace_back(")");
//            }
//        }
//    }
//    trace_operator(v, idx + 1, left_ans, right_ans);
//}

bool is_swap_operator(string &op) {
    return op == "+" || op == "*" || op == "&" || op == "|" || op == "^";
}

BuiltinCollectionType *check_is_builtin_collection(PostfixExpression *postfixExpression,
                                                   unordered_map<string, BuiltinCollectionType *> &global_vars) {
    if (postfixExpression->isPrimaryExpr() && postfixExpression->getPrimaryExpression()->isIdentifier() &&
        global_vars.count(*postfixExpression->getPrimaryExpression()->getIdentifier()))
        return global_vars[*postfixExpression->getPrimaryExpression()->getIdentifier()];
    if (postfixExpression->isArrayLookup()) {
        BuiltinCollectionType *inner_type = check_is_builtin_collection(postfixExpression->getPostfixExpression(),
                                                                        global_vars);
        if (inner_type != nullptr) {
            if (auto builtin_map = dynamic_cast<BuiltinMap *>(inner_type)) {
                if (auto v_type = dynamic_cast<VariableDefinition *>(builtin_map->getValueType())) {
                    if (auto built_in_v = dynamic_cast<BuiltinCollectionType *>(v_type->getType()))
                        return built_in_v;
                }
            }
        }
    }
    return nullptr;
}

int check_could_be_extracted(Node *child) {
    Node *parent = child->getParent();
    if (auto multiExpr = dynamic_cast<MultiplicativeExpression *>(parent)) {
        if (multiExpr->isMultiExpr() && is_swap_operator(*multiExpr->getOp())) {
            if (auto child_ptr = dynamic_cast<CastExpression *>(child))
                return 1;
            else
                return -1;
        }
        if (multiExpr->isMultiExpr()) {
            if (auto child_ptr = dynamic_cast<MultiplicativeExpression *>(child))
                return -1;
        }
    } else if (auto addExpr = dynamic_cast<AdditiveExpression *>(parent)) {
        if (addExpr->isAdditiveExpr() && is_swap_operator(*addExpr->getOp())) {
            if (auto child_ptr = dynamic_cast<MultiplicativeExpression *>(child))
                return 1;
            else
                return -1;
        }
        if (addExpr->isAdditiveExpr()) {
            if (auto child_ptr = dynamic_cast<AdditiveExpression *>(child))
                return -1;
        }
    } else if (auto andExpr = dynamic_cast<AndExpression *>(parent)) {
        if (andExpr->isAndExpr()) {
            if (auto child_ptr = dynamic_cast<EqualityExpression *>(child))
                return 1;
            else
                return -1;
        }
    } else if (auto exclusiveOrExpr = dynamic_cast<ExclusiveOrExpression *>(parent)) {
        if (exclusiveOrExpr->isExclusiveOrExpr()) {
            if (auto child_ptr = dynamic_cast<AndExpression *>(child))
                return 1;
            else
                return -1;

        }
    } else if (auto inclusiveOrExpr = dynamic_cast<InclusiveOrExpression *>(parent)) {
        if (inclusiveOrExpr->isInclusiveOrExpr()) {
            if (auto child_ptr = dynamic_cast<ExclusiveOrExpression *>(child))
                return 1;
            else
                return -1;
        }
    }
    return 0;
}

void do_extraction(AssignmentExpression *root, Node *binary_operator, int flag) {
    if (auto multiExpr = dynamic_cast<MultiplicativeExpression *>(binary_operator)) {
        auto *new_op = new string();
        *new_op += *multiExpr->getOp() + *root->getOp();
        root->setOp(new_op);
        if (flag == 1) {
            auto father = dynamic_cast<AdditiveExpression *>(multiExpr->getParent());
            father->setMultiplicativeExpression(multiExpr->getMultiplicativeExpression());
            father->getMultiplicativeExpression()->setParent(father);
        } else {
            multiExpr->setOp(nullptr);
            multiExpr->setMultiplicativeExpression(nullptr);
        }
    } else if (auto addExpr = dynamic_cast<AdditiveExpression *>(binary_operator)) {
        auto *new_op = new string();
        *new_op += *addExpr->getOp() + *root->getOp();
        root->setOp(new_op);
        if (flag == 1) {
            auto father = dynamic_cast<ShiftExpression *>(addExpr->getParent());
            father->setAdditiveExpression(addExpr->getAdditiveExpression());
            father->getAdditiveExpression()->setParent(father);
        } else {
            addExpr->setOp(nullptr);
            addExpr->setAdditiveExpression(nullptr);
        }
    } else if (auto andExpr = dynamic_cast<AndExpression *>(binary_operator)) {
        auto *new_op = new string();
        *new_op += "&" + *root->getOp();
        root->setOp(new_op);
        if (flag == 1) {
            auto father = dynamic_cast<ExclusiveOrExpression *>(andExpr->getParent());
            father->setAndExpression(andExpr->getAndExpression());
            father->getAndExpression()->setParent(father);
        } else {
            andExpr->setAndExpression(nullptr);
        }
    } else if (auto exclusiveOrExpr = dynamic_cast<ExclusiveOrExpression *>(binary_operator)) {
        auto *new_op = new string();
        *new_op += "^" + *root->getOp();
        root->setOp(new_op);
        if (flag == 1) {
            auto father = dynamic_cast<InclusiveOrExpression *>(exclusiveOrExpr->getParent());
            father->setExclusiveOrExpression(exclusiveOrExpr->getExclusiveOrExpression());
            father->getExclusiveOrExpression()->setParent(father);
        } else {
            exclusiveOrExpr->setExclusiveOrExpression(nullptr);
        }
    } else if (auto inclusiveOrExpr = dynamic_cast<InclusiveOrExpression *>(binary_operator)) {
        auto *new_op = new string();
        *new_op += "|" + *root->getOp();
        root->setOp(new_op);
        if (flag == 1) {
            auto father = dynamic_cast<LogicalAndExpression *>(inclusiveOrExpr->getParent());
            father->setInclusiveOrExpression(inclusiveOrExpr->getInclusiveOrExpression());
            father->getInclusiveOrExpression()->setParent(father);
        } else {
            inclusiveOrExpr->setInclusiveOrExpression(nullptr);
        }
    }
}

void change_commutative_order(Node *child) {
    Node *parent = child->getParent();
    if (auto multiExpr = dynamic_cast<MultiplicativeExpression *>(parent)) {
        if (multiExpr->isMultiExpr() && is_swap_operator(*multiExpr->getOp())) {
            if (auto multi_child = dynamic_cast<MultiplicativeExpression *>(child)) {
                CastExpression *tmp = multiExpr->getCastExpression();
                multiExpr->setCastExpression(multi_child->getCastExpression());
                multiExpr->getCastExpression()->setParent(multiExpr);
                multi_child->setCastExpression(tmp);
                tmp->setParent(multi_child);
            }
            while (auto multi_parent = dynamic_cast<MultiplicativeExpression *>(parent->getParent())) {
                if (multi_parent->isMultiExpr() && is_swap_operator(*multi_parent->getOp())) {
                    CastExpression *tmp = multi_parent->getCastExpression();
                    multi_parent->setCastExpression(multiExpr->getCastExpression());
                    multi_parent->getCastExpression()->setParent(multi_parent);
                    multiExpr->setCastExpression(tmp);
                    tmp->setParent(multiExpr);
                    parent = multi_parent;
                } else
                    break;
            }
        }
    } else if (auto addExpr = dynamic_cast<AdditiveExpression *>(parent)) {
        if (addExpr->isAdditiveExpr() && is_swap_operator(*addExpr->getOp())) {
            if (auto add_child = dynamic_cast<AdditiveExpression *>(child)) {
                MultiplicativeExpression *tmp = addExpr->getMultiplicativeExpression();
                addExpr->setMultiplicativeExpression(add_child->getMultiplicativeExpression());
                addExpr->getMultiplicativeExpression()->setParent(addExpr);
                add_child->setMultiplicativeExpression(tmp);
                tmp->setParent(add_child);
            }
            while (auto add_parent = dynamic_cast<AdditiveExpression *>(parent->getParent())) {
                if (add_parent->isAdditiveExpr() && is_swap_operator(*add_parent->getOp())) {
                    MultiplicativeExpression *tmp = add_parent->getMultiplicativeExpression();
                    add_parent->setMultiplicativeExpression(addExpr->getMultiplicativeExpression());
                    add_parent->getMultiplicativeExpression()->setParent(add_parent);
                    addExpr->setMultiplicativeExpression(tmp);
                    tmp->setParent(addExpr);
                    parent = add_parent;
                } else
                    break;
            }
        }
    } else if (auto andExpr = dynamic_cast<AndExpression *>(parent)) {
        if (andExpr->isAndExpr()) {
            if (auto and_child = dynamic_cast<AndExpression *>(child)) {
                EqualityExpression *tmp = andExpr->getEqualityExpression();
                andExpr->setEqualityExpression(and_child->getEqualityExpression());
                andExpr->getEqualityExpression()->setParent(andExpr);
                and_child->setEqualityExpression(tmp);
                tmp->setParent(and_child);
            }
            while (auto and_parent = dynamic_cast<AndExpression *>(parent->getParent())) {
                if (and_parent->isAndExpr()) {
                    EqualityExpression *tmp = and_parent->getEqualityExpression();
                    and_parent->setEqualityExpression(andExpr->getEqualityExpression());
                    and_parent->getEqualityExpression()->setParent(and_parent);
                    andExpr->setEqualityExpression(tmp);
                    tmp->setParent(andExpr);
                    parent = and_parent;
                } else
                    break;
            }
        }
    } else if (auto exclusiveOrExpr = dynamic_cast<ExclusiveOrExpression *>(parent)) {
        if (exclusiveOrExpr->isExclusiveOrExpr()) {
            if (auto exclusive_or_child = dynamic_cast<ExclusiveOrExpression *>(child)) {
                AndExpression *tmp = exclusiveOrExpr->getAndExpression();
                exclusiveOrExpr->setAndExpression(exclusive_or_child->getAndExpression());
                exclusiveOrExpr->getAndExpression()->setParent(exclusiveOrExpr);
                exclusive_or_child->setAndExpression(tmp);
                tmp->setParent(exclusive_or_child);
            }
            while (auto exclusive_or_parent = dynamic_cast<ExclusiveOrExpression *>(parent->getParent())) {
                if (exclusive_or_parent->isExclusiveOrExpr()) {
                    AndExpression *tmp = exclusive_or_parent->getAndExpression();
                    exclusive_or_parent->setAndExpression(exclusiveOrExpr->getAndExpression());
                    exclusive_or_parent->getAndExpression()->setParent(exclusive_or_parent);
                    exclusiveOrExpr->setAndExpression(tmp);
                    tmp->setParent(andExpr);
                    parent = exclusive_or_parent;
                } else
                    break;
            }
        }
    } else if (auto inclusiveOrExpr = dynamic_cast<InclusiveOrExpression *>(parent)) {
        if (inclusiveOrExpr->isInclusiveOrExpr()) {
            if (auto inclusive_or_child = dynamic_cast<InclusiveOrExpression *>(child)) {
                ExclusiveOrExpression *tmp = inclusiveOrExpr->getExclusiveOrExpression();
                inclusiveOrExpr->setExclusiveOrExpression(inclusive_or_child->getExclusiveOrExpression());
                inclusiveOrExpr->getExclusiveOrExpression()->setParent(inclusiveOrExpr);
                inclusive_or_child->setExclusiveOrExpression(tmp);
                tmp->setParent(inclusive_or_child);
            }
            while (auto inclusive_or_parent = dynamic_cast<InclusiveOrExpression *>(parent->getParent())) {
                if (inclusive_or_parent->isInclusiveOrExpr()) {
                    ExclusiveOrExpression *tmp = inclusive_or_parent->getExclusiveOrExpression();
                    inclusive_or_parent->setExclusiveOrExpression(inclusiveOrExpr->getExclusiveOrExpression());
                    inclusive_or_parent->getExclusiveOrExpression()->setParent(inclusive_or_parent);
                    inclusiveOrExpr->setExclusiveOrExpression(tmp);
                    tmp->setParent(inclusiveOrExpr);
                    parent = inclusive_or_parent;
                } else
                    break;
            }
        }
    }
}
//the following part do the static optimization

void recursive_resolve_copy_constructor(Statement *stmt, unordered_map<string, BuiltinCollectionType *> &global_vars) {
    if (auto comp_stmt = dynamic_cast<CompoundStatement *>(stmt)) {
        for (auto n_stmt:comp_stmt->getStatements()) {
            recursive_resolve_copy_constructor(n_stmt, global_vars);
        }
    } else if (auto if_stmt = dynamic_cast<IfStatement *>(stmt)) {
        recursive_resolve_copy_constructor(if_stmt->getT_case(), global_vars);
        if (if_stmt->hasFCase())
            recursive_resolve_copy_constructor(if_stmt->getF_case(), global_vars);
    } else if (auto expr_stmt = dynamic_cast<ExpressionStatement *>(stmt)) {
        for (auto assign_expr : expr_stmt->getExpression()->getExprs()) {
            if (assign_expr->isAssignExpr() && *assign_expr->getOp() == "=" &&
                assign_expr->getNext()->isConditionalExpr() && assign_expr->getUnaryExpression()->isPostfixExpr()) {
                PostfixExpression *left = assign_expr->getUnaryExpression()->getPostfixExpression();
                ConditionalExpression *right = assign_expr->getNext()->getConditionalExpression();
                if (check_is_builtin_collection(left, global_vars) != nullptr) {
                    vector<PostfixExpression *> right_side;
                    iterate_all<PostfixExpression>(right, right_side);
                    for (auto candidate_expr:right_side) {
                        if (*left == *candidate_expr) {
                            auto first_binary = trace_nearest_binary_operator(candidate_expr);
                            if (first_binary != nullptr) {
                                change_commutative_order(first_binary);
                                first_binary = trace_nearest_binary_operator(candidate_expr);
                                if (trace_nearest_binary_operator(first_binary->getParent()) == nullptr) {
                                    int flag = check_could_be_extracted(first_binary);
                                    if (flag != 0) {
                                        // do extraction
                                        do_extraction(assign_expr, first_binary->getParent(), flag);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
};


void parse_eliminate_copy_constructor(RootNode *root) {
    vector<Node *> statements = root->getChildren();
    unordered_map<string, BuiltinCollectionType *> global_vars;
    for (auto node : statements) {
        if (auto *varDef = dynamic_cast<VariableDefinition *>(node)) {
            if (auto collection = dynamic_cast<BuiltinCollectionType *>(varDef->getType())) {
                global_vars[varDef->getScalarName()] = collection;
            }
        }
    }
    for (auto node : statements) {
        if (auto *funcDef = dynamic_cast<FunctionDefinition *>(node)) {
            if (funcDef->getParameters().size() > 0) {
                for (FunctionParamDefinition *functionParamDefinition:funcDef->getParameters()) {
                    if (auto collection = dynamic_cast<BuiltinCollectionType *>(funcDef->getType())) {
                        global_vars[functionParamDefinition->getAlias()] = collection;
                    }
                }
            }
            CombinedStatements *stmts = funcDef->getBody();
            for (auto stmt : stmts->getStatements()) {
                recursive_resolve_copy_constructor(stmt, global_vars);
            }
        }
    }
};

void do_ast_replacement(vector<Node *> stmts, vector<VariableDefinition *> &to_replace, ClazzType *target) {
    G_Context context;
    for (auto &statement : stmts) {
        if (auto var_def = dynamic_cast<VariableDefinition *>(statement)) {
            context.getScalar_type()[var_def->getScalarName()] = var_def->getType();
        }
    }
    for (auto &statement : stmts) {
        if (auto func_def = dynamic_cast<FunctionDefinition *>(statement)) {
            if (func_def->getParameters().size() > 0) {
                for (FunctionParamDefinition *functionParamDefinition:func_def->getParameters()) {
                    context.getScalar_type()[functionParamDefinition->getAlias()] = functionParamDefinition->getType();
                }
            }
            vector<PostfixExpression *> postexprs;
            iterate_all<PostfixExpression>(func_def->getBody(), postexprs);
            for (auto postexpr : postexprs) {
                Type *left = postexpr->resolve_type(context);
                if (left != nullptr) {
                    for (auto var : to_replace) {
                        if (auto map_type = dynamic_cast<BuiltinMap *>(var->getType())) {
                            if (*left == map_type->getValueType()->getType() &&
                                var->getScalarName() == postexpr->resolve_inner_scalar()) {
                                auto *n_post = new PostfixExpression();
                                n_post->setOp(new string("."));
                                n_post->setPostfixExpression(postexpr);
                                n_post->setParent(postexpr->getParent());
                                postexpr->setParent(n_post);
                                if (auto unary = dynamic_cast<UnaryExpression *>(n_post->getParent())) {
                                    unary->setPostfixExpression(n_post);
                                } else if (auto p_post = dynamic_cast<PostfixExpression *>(n_post->getParent())) {
                                    p_post->setPostfixExpression(n_post);
                                } else
                                    throw "Unexpected";
                                for (auto it = target->getFields().begin(); it != target->getFields().end(); it++) {
                                    if (*it->second == map_type->getValueType()->getType() &&
                                        it->first == map_type->getValue()) {

                                        n_post->setIdentifier(new string(it->first));
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}


vector<VariableDefinition *>
recursive_resolve_map_fusion(vector<VariableDefinition *> &search_field, int &global_sequence, vector<Node *> &stmts) {
    bool once = false;
    vector<VariableDefinition *> ans;
    vector<vector<VariableDefinition *>> match_list;
    for (auto varDef : search_field) {
        bool matched = false;
        if (auto builtin_map = dynamic_cast<BuiltinMap *>(varDef->getType())) {

            for (vector<VariableDefinition *> &v: match_list) {
                VariableDefinition *another = v[0];
                if (auto another_map = dynamic_cast<BuiltinMap *>(another->getType())) {
                    if (another_map->getScope() == builtin_map->getScope() &&
                        another_map->getKeyType()->getScalarName() ==
                        builtin_map->getKeyType()->getScalarName()) {
                        matched = true;
                        v.push_back(varDef);
                        break;
                    }
                }
            }

        }
        if (!matched) {
            vector<VariableDefinition *> v;
            v.push_back(varDef);
            match_list.push_back(v);
        }
    }

    for (vector<VariableDefinition *> &v: match_list) {

        if (v.size() > 1) {
            cout << "here" << endl;
            vector<VariableDefinition *> inner_vars, fields;
            string scope("bottom");
            VariableDefinition *key = nullptr;
            for (auto inner_var : v) {
                if (auto map_def = dynamic_cast<BuiltinMap *>(inner_var->getType())) {
                    scope = map_def->getScope();
                    key = map_def->getKeyType();
                    if (auto inner_map_def = dynamic_cast<BuiltinMap *>(map_def->getValueType()->getType())) {
                        inner_vars.push_back(map_def->getValueType());
                    } else
                        fields.push_back(map_def->getValueType());
                } else
                    throw "Unexpected Type";
            }
            for (auto field : recursive_resolve_map_fusion(inner_vars, global_sequence, stmts)) {
                fields.push_back(field);
            }
            string *clazz_name = new string("Mapfusion_");
            *clazz_name += to_string(++global_sequence);
            string *scalar_name = new string("mapfusion_");
            *scalar_name += to_string(global_sequence);
            auto *clazzType = new ClazzType(clazz_name);
            for (VariableDefinition *map_var : fields) {
                if (auto coll = dynamic_cast<BuiltinCollectionType *>(map_var->getType()))
                    clazzType->getFields()[map_var->getScalarName()] = map_var->getType();
                else {
                    VariableDefinition *parent = nullptr;
                    for (VariableDefinition *vd : v) {
                        auto map_def = dynamic_cast<BuiltinMap *>(vd->getType());
                        if (map_def->getValueType() == map_var && !clazzType->getFields().count(vd->getScalarName())) {
                            parent = vd;
                            break;
                        }
                    }

                    clazzType->getFields()[parent->getScalarName()] = map_var->getType();

                }

            }

            auto map_fusion = new VariableDefinition(scalar_name, clazzType);

            auto builtin_map = new BuiltinMap(scalar_name, scope, key, map_fusion);
            auto map_def = new VariableDefinition(scalar_name, builtin_map);
            ans.push_back(map_def);
            // do AST replacement
            do_ast_replacement(stmts, v, clazzType);
            once = true;
        } else
            ans.push_back(v[0]);
    }
    return ans;
}


void map_fusion(RootNode *root) {
    vector<Node *> &statements = root->getChildren();
    vector<Node *> n_statements;
    vector<VariableDefinition *> var_list;
    for (auto &statement : statements) {
        if (auto var_def = dynamic_cast<VariableDefinition *>(statement)) {
            var_list.push_back(var_def);
        }
    }
    int a = 0;
    vector<VariableDefinition *> vars = recursive_resolve_map_fusion(var_list, a, statements);
    n_statements.reserve(statements.size());
    for (auto &var : vars) {
        n_statements.push_back(var);
    }
    G_Context context;
    for (auto &var_def : var_list) {
        context.getScalar_type()[var_def->getScalarName()] = var_def->getType();
    }

    for (auto statement : statements) {
        if (auto var_def = dynamic_cast<VariableDefinition *>(statement)) {
        } else {
            n_statements.push_back(statement);
            if (auto funcDef = dynamic_cast<FunctionDefinition *>(statement)) {
                if (funcDef->getParameters().size() > 0) {
                    for (FunctionParamDefinition *functionParamDefinition:funcDef->getParameters()) {
                        context.getScalar_type()[functionParamDefinition->getAlias()] = functionParamDefinition->getType();
                    }
                }
                vector<PostfixExpression *> v;
                iterate_all<PostfixExpression>(funcDef->getBody(), v);
                for (auto &var : vars) {
                    if (auto map_type = dynamic_cast<BuiltinMap *>(var->getType())) {
                        if (auto clazz_type = dynamic_cast<ClazzType *>(map_type->getValueType()->getType())) {
                            for (auto post_expr : v) {
                                Type *post_expr_type = post_expr->resolve_type(context);
                                if (post_expr_type != nullptr && *post_expr_type == var->getType()) {

                                    PostfixExpression *p = post_expr;
                                    while (!p->isPrimaryExpr()) {
                                        p = p->getPostfixExpression();
                                    }

                                    if (auto parent = dynamic_cast<PostfixExpression *>(p->getParent()->getParent())) {
                                        if (parent->isArgMethodCall()) {
                                            string *func = parent->getPostfixExpression()->getIdentifier();
                                            if (func != nullptr && (*func).find("_range") != string::npos) {
                                                ClazzType *clazzType = reinterpret_cast<ClazzType *>(map_type->getValueType());
//                                                cout<<<<" "<<parent->getPostfixExpression()->getPostfixExpression()->toString()<< " "<<*func<<endl;
                                                string *arg1 = new string();
                                                *arg1 += "offsetof(" + clazz_type->getClazzName() + ", " +
                                                         p->toString() + ")";
                                                string *arg2 = new string();
                                                *arg2 += "sizeof(" + clazz_type->getClazzName() + "::" + p->toString() +
                                                         ")";
                                                parent->getArgumentExpressionList()->addStatements(arg1);
                                                parent->getArgumentExpressionList()->addStatements(arg2);
                                            }
                                        }
                                    }

                                    p->getPrimaryExpression()->setIdentifier(new string(var->getScalarName()));
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    root->getChildren().swap(n_statements);
}

PostfixExpression *find_map_lookup_inside_expr(Type *var, Expression *expr, G_Context &context) {
    vector<PostfixExpression *> v;
    iterate_all<PostfixExpression>(expr, v);
    for (auto post_expr : v) {
        if (post_expr->isArrayLookup()) {
            Type *type = post_expr->getPostfixExpression()->resolve_type(context);
            if (type != nullptr && *(type) == var) {
                return post_expr;
            }
        }
    }
    return nullptr;
}


string *recursive_map_lookup_elimination(FunctionBodyStatement *stmt, G_Context &context, BuiltinMap *target_map,
                                         int &counter) {
    vector<Statement *> inner_stmts;
    if (auto combined = dynamic_cast<CombinedStatements *>(stmt)) {
        inner_stmts = combined->getStatements();
    } else if (auto compound = dynamic_cast<CompoundStatement *>(stmt)) {
        inner_stmts = compound->getStatements();
    } else
        throw "Unexpected type";
    int& current_idx = counter;
    //find phase
    auto it = inner_stmts.begin();
    auto an_it = inner_stmts.begin();
    PostfixExpression *target_expr = nullptr;
    map<ExpressionStatement *, string> ref_offset;
    for (; it != inner_stmts.end(); it++, an_it++) {
        auto st = *it;
        if (auto expr_st = dynamic_cast<ExpressionStatement *>(st)) {

            if (expr_st->getExpression()->getExprs().size() > 0) {
                if (expr_st->getExpression()->getExprs()[0]->getUnaryExpression() != nullptr &&
                    expr_st->getExpression()->getExprs()[0]->getUnaryExpression()->resolve_type(context)) {
                    if (expr_st->getExpression()->getExprs()[0]->getUnaryExpression()->resolve_type(
                            context)->getValue() == target_map->getKeyType()->getScalarName()) {

                        if (target_map->getScope() == "universe") {
                            vector<PrimaryExpression *> ans;
                            iterate_all<PrimaryExpression>(expr_st->getExpression()->getExprs()[0]->getNext(), ans);
                            if (ans[0]->toString() == "ptr_offset") {
                                ref_offset[expr_st] = ans[2]->toString() + ">>" + to_string(shift);
                            }
                        }
                    }
                }
            }
        }
    }
    it = inner_stmts.begin();
    an_it = inner_stmts.begin();

    target_expr = nullptr;
    for (; it != inner_stmts.end(); it++, an_it++) {
        auto st = *it;
        if (auto expr_st = dynamic_cast<ExpressionStatement *>(st)) {
            target_expr = find_map_lookup_inside_expr(target_map, expr_st->getExpression(), context);
            if (target_expr)
                break;
        } else if (auto if_st = dynamic_cast<IfStatement *>(st)) {
            target_expr = find_map_lookup_inside_expr(target_map, if_st->getExpression(), context);
            if (target_expr)
                break;
            recursive_map_lookup_elimination(if_st->getT_case(), context, target_map, ++counter);
            if (if_st->hasFCase())
                recursive_map_lookup_elimination(if_st->getF_case(), context, target_map, ++counter);
        } else if (auto compound_st = dynamic_cast<CompoundStatement *>(st)) {
            recursive_map_lookup_elimination(compound_st, context, target_map, ++counter);
        } else if (auto ret_st = dynamic_cast<ReturnStatement *>(st)) {
            target_expr = find_map_lookup_inside_expr(target_map, ret_st->getExpression(), context);
            if (target_expr)
                break;
        }
    }

    //
    if (it != inner_stmts.end() && target_expr != nullptr) {
        vector<PostfixExpression *> can_be_replaced;
        for (; an_it != inner_stmts.end(); an_it++) {
            vector<PostfixExpression *> v;
            iterate_all<PostfixExpression>(*an_it, v);
            for (auto post:v) {
                if (*post == *target_expr)
                    can_be_replaced.push_back(post);
            }
        }
        if (can_be_replaced.size() > 1) {
            //found, we do insert first
            AssignmentExpression *assignmentExpression = new AssignmentExpression();
            assignmentExpression->setScalar_type(
                    new string("auto")); //&target_map->getValueType()->getType()->toString()
            string *scalar_name = new string("scalar_");
            *scalar_name += to_string(++current_idx);
            assignmentExpression->setScalar_name(scalar_name);
            assignmentExpression->setPostfixExpression(target_expr);

            //add location for locking
            AssignmentExpression *lockExpression = new AssignmentExpression();
            lockExpression->setScalar_type(new string(""));
            lockExpression->setScalar_name(new string(""));
            PrimaryExpression *pe = new PrimaryExpression();
            pe->setIdentifier(new string("PTHREAD_LOCK"));
            PostfixExpression *postE = new PostfixExpression();
            postE->setPrimaryExpression(pe);
            lockExpression->setPostfixExpression(postE);
            postE->setParent(lockExpression);
            pe->setParent(postE);


            Expression *expression = new Expression();
            if (target_map->getScope() == "universe") {
                expression->addExprs(assignmentExpression);
                expression->addExprs(lockExpression);
            } else {
                expression->addExprs(lockExpression);
                expression->addExprs(assignmentExpression);
            }
            assignmentExpression->setParent(expression);
            ExpressionStatement *expressionStatement = new ExpressionStatement();
            expressionStatement->setExpression(expression);
            expression->setParent(expressionStatement);
            inner_stmts.insert(it, expressionStatement);
//
            map<Statement *, string *> refoffset_map;
            if (ref_offset.size() > 0) {
                for (auto mit = ref_offset.begin(); mit != ref_offset.end(); mit++) {
                    AssignmentExpression *nassign = new AssignmentExpression();
                    nassign->setScalar_type(new string("auto"));
                    string *off_name = new string("scalar_");
                    *off_name += to_string(++current_idx);
                    nassign->setScalar_name(off_name);
                    nassign->setOp(new string("="));
                    PrimaryExpression *method_name = new PrimaryExpression();
                    method_name->setIdentifier(new string("ref_offset"));

                    PostfixExpression *method = new PostfixExpression();
                    method->setPrimaryExpression(method_name);
                    ArgumentExpressionList *argumentExpressionList = new ArgumentExpressionList();
                    argumentExpressionList->addStatements(assignmentExpression->getScalar_name());
                    argumentExpressionList->addStatements(new string(mit->second));
                    PostfixExpression *postfixExpression = new PostfixExpression();
                    postfixExpression->setPostfixExpression(method);
                    postfixExpression->setArgumentExpressionList(argumentExpressionList);
                    nassign->setPostfixExpression(postfixExpression);

                    Expression *expression = new Expression();
                    expression->addExprs(nassign);
                    nassign->setParent(expression);
                    ExpressionStatement *exst = new ExpressionStatement();
                    exst->setExpression(expression);
                    expression->setParent(exst);
                    auto to_find = inner_stmts.begin();
                    for (; to_find != inner_stmts.end(); to_find++) {
                        if (*to_find == mit->first)
                            break;
                    }
                    inner_stmts.insert(to_find, exst);
                    refoffset_map[exst] = off_name;
                }
            }

            //do replacement


//            for (auto post : can_be_replaced) {
//
//                PrimaryExpression *primaryExpression = new PrimaryExpression();
//                primaryExpression->setIdentifier(assignmentExpression->getScalar_name());
//                PostfixExpression *postfixExpression = new PostfixExpression();
//                postfixExpression->setPrimaryExpression(primaryExpression);
//                primaryExpression->setParent(postfixExpression);
//                if (auto f_post = dynamic_cast<PostfixExpression *>(post->getParent())) {
//                    f_post->setPostfixExpression(postfixExpression);
//                    postfixExpression->setParent(f_post);
//                } else if (auto f_unary = dynamic_cast<UnaryExpression *>(post->getParent())) {
//                    f_unary->setPostfixExpression(postfixExpression);
//                    postfixExpression->setParent(f_unary);
//                } else
//                    throw "Impossible";
//            }
            string *current_name = assignmentExpression->getScalar_name();
            for (auto last_it = inner_stmts.begin(); last_it != inner_stmts.end(); last_it++) {
                vector<PostfixExpression *> v;
                iterate_all<PostfixExpression>(*last_it, v);
                for (auto post:v) {
                    if (*post == *target_expr) {
                        PrimaryExpression *primaryExpression = new PrimaryExpression();
                        primaryExpression->setIdentifier(current_name);
                        PostfixExpression *postfixExpression = new PostfixExpression();
                        postfixExpression->setPrimaryExpression(primaryExpression);
                        primaryExpression->setParent(postfixExpression);
                        if (auto f_post = dynamic_cast<PostfixExpression *>(post->getParent())) {
                            f_post->setPostfixExpression(postfixExpression);
                            postfixExpression->setParent(f_post);
                        } else if (auto f_unary = dynamic_cast<UnaryExpression *>(post->getParent())) {
                            f_unary->setPostfixExpression(postfixExpression);
                            postfixExpression->setParent(f_unary);
                        } else
                            throw "Impossible";
                    }

                }
                if (refoffset_map.count((*last_it))) {
                    current_name = refoffset_map[*last_it];
                }
            }
            //insert unlock
            AssignmentExpression *unlockExpression = new AssignmentExpression();
            unlockExpression->setScalar_type(new string(""));
            unlockExpression->setScalar_name(new string(""));
            PrimaryExpression *unpe = new PrimaryExpression();
            unpe->setIdentifier(new string("PTHREAD_UNLOCK"));
            PostfixExpression *unpostE = new PostfixExpression();
            unpostE->setPrimaryExpression(unpe);
            unlockExpression->setPostfixExpression(unpostE);
            unpostE->setParent(lockExpression);
            unpe->setParent(unpostE);
            Expression *unexpression = new Expression();
            unexpression->addExprs(unlockExpression);
            unlockExpression->setParent(unexpression);
            ExpressionStatement *unexpressionStatement = new ExpressionStatement();
            unexpressionStatement->setExpression(unexpression);
            unexpression->setParent(unexpressionStatement);
            inner_stmts.push_back(unexpressionStatement);


            if (auto combined = dynamic_cast<CombinedStatements *>(stmt)) {
                combined->getStatements().swap(inner_stmts);
            } else if (auto compound = dynamic_cast<CompoundStatement *>(stmt)) {
                compound->getStatements().swap(inner_stmts);
            }


            target_expr->setParent(assignmentExpression);
            return assignmentExpression->getScalar_name();
        }

    }
    return nullptr;
}

void extract_map_type(Type *ty, vector<BuiltinMap *> &v) {
    if (auto map_ty = dynamic_cast<BuiltinMap *>(ty)) {
        v.push_back(map_ty);
        extract_map_type(map_ty->getValueType()->getType(), v);
    } else if (auto clazz_ty = dynamic_cast<ClazzType *>(ty)) {
        for (auto it = clazz_ty->getFields().begin(); it != clazz_ty->getFields().end(); it++) {
            if (auto clazz_map_ty = dynamic_cast<BuiltinMap *>(it->second)) {
                v.push_back(clazz_map_ty);
                extract_map_type(clazz_map_ty, v);
            } else if (auto clazz_clazz_ty = dynamic_cast<ClazzType *>(it->second)) {
                extract_map_type(clazz_clazz_ty, v);
            }

        }
    }
}


void map_lookup_elimination(RootNode *root) {
    vector<VariableDefinition *> var_list;
    vector<Node *> &statements = root->getChildren();
    queue<BuiltinMap *> q;
    for (auto &statement : statements) {
        if (auto var_def = dynamic_cast<VariableDefinition *>(statement)) {
            var_list.push_back(var_def);
            if (auto map_ty = dynamic_cast<BuiltinMap *>(var_def->getType())) {
                q.push(map_ty);
            }
        }
    }
    G_Context context;
    for (auto &var_def : var_list) {
        context.getScalar_type()[var_def->getScalarName()] = var_def->getType();

    }
    for (auto &statement : statements) {
        if (auto func_def = dynamic_cast<FunctionDefinition *>(statement)) {
            func_def->getBody();
            queue<BuiltinMap *> cq = q;
            G_Context session = context;
            for (FunctionParamDefinition *param:func_def->getParameters()) {
                session.getScalar_type()[param->getAlias()] = session.getScalar_type()[param->getScalarName()];
            }
            int counter = 0;
            while (!cq.empty()) {
                BuiltinMap *cvar = cq.front();
                cq.pop();
                string *scalar = recursive_map_lookup_elimination(func_def->getBody(), session, cvar, counter);
                while (scalar != nullptr &&
                       recursive_map_lookup_elimination(func_def->getBody(), session, cvar, counter)) {}
                if (scalar != nullptr)
                    session.getScalar_type()[*scalar] = cvar->getValueType()->getType();
                vector<BuiltinMap *> vs;
                extract_map_type(cvar->getValueType()->getType(), vs);
                for (auto e: vs) {
                    cq.push(e);
                }
//                cout << "dead" << cq.size() << endl;
            }
        }
    }
}


#endif //G_PARSER_GENERATOR_H
