#include <data.h>
#include "statements/expression/Expression.h"
#include "statements/function/FunctionCall.hpp"
#include "statements/function/LambdaExpression.hpp"
#include "types/function.h"
#include <expressions.h>
#include <functional>

static bool parseTypeArgumentList(Lexer& lexer, std::vector<Token>& outArgs) {
    lexer.savePosition();
    if (!lexer.expectToken(SMALLER_THAN)) {
        lexer.rollPosition();
        return false;
    }

    while (true) {
        Token arg = lexer.nextToken();
        if (arg.kind != IDENTIFIER) {
            lexer.rollPosition();
            return false;
        }
        outArgs.push_back(arg);

        Token sep = lexer.nextToken();
        if (sep.kind == BIGGER_THAN) {
            lexer.deletePosition();
            return true;
        }
        if (sep.kind != COMMA) {
            lexer.rollPosition();
            return false;
        }
    }
}

static Value makeTypeInstantiationError(const std::string& message) {
    Value err;
    err.thrownException = create_reference<Value>(Value(message));
    return err;
}

static Value instantiateType(reference<Type> baseType, const std::vector<reference<Type>>& args, bool strictMissing) {
    if (baseType == nullptr) {
        return makeTypeInstantiationError("cannot instantiate null type");
    }

    std::vector<size_t> undefinedIndices;
    for (size_t i = 0; i < baseType->typeParameters.size(); ++i) {
        if (baseType->typeParameters[i].value == nullptr) {
            undefinedIndices.push_back(i);
        }
    }

    if (args.size() > undefinedIndices.size()) {
        return makeTypeInstantiationError("too many type arguments for " + baseType->getName());
    }

    auto specialized = create_reference<Type>(*baseType);
    specialized->parent = baseType;
    specialized->typeBindings = baseType->typeBindings;

    size_t consumed = 0;
    for (size_t idx : undefinedIndices) {
        if (consumed < args.size()) {
            specialized->typeParameters[idx].value = args[consumed++];
            continue;
        }

        if (specialized->typeParameters[idx].hasDefault && specialized->typeParameters[idx].defaultValue != nullptr) {
            specialized->typeParameters[idx].value = specialized->typeParameters[idx].defaultValue;
            continue;
        }

        if (strictMissing) {
            return makeTypeInstantiationError(
                "missing type argument for " + specialized->typeParameters[idx].name + " in " + baseType->getName()
            );
        }
    }

    std::vector<TypeParameter> remaining;
    for (auto& tp : specialized->typeParameters) {
        specialized->typeBindings[tp.name] = tp.value;
        if (tp.value == nullptr) {
            remaining.push_back(tp);
        }
    }
    specialized->typeParameters = move(remaining);

    return Value(specialized);
}

uref<Expression> Expression::parse(Lexer &lexer) {

    lexer.savePosition();

    auto expression = create_unique<Expression>();

    if (DEBUG) std::cout << DEBUG_PREFIX << "Entering Expression::parse\n";

    std::vector<uref<Statement>> invalids;
    bool foundFirstOperand = false;

    using ExprParser = std::function<uref<Expression>(Lexer&)>;
    std::vector<ExprParser> parsers = {
        [](Lexer& l) { return UnaryExpression::parse(l); },
        [](Lexer& l) { return ExpressionParenWrapped::parse(l); },
        [](Lexer& l) { return ConstructorExpression::parse(l); },
        [](Lexer& l) { return ExpressionValue::parse(l); },
        [](Lexer& l) { return LambdaDefinition::parse(l); },
    };

    if (DEBUG) std::cout << DEBUG_PREFIX << "Trying to parse first operand\n";

    for (auto& parser : parsers) {
        DEBUG_TABS++;
        auto expr = parser(lexer);
        DEBUG_TABS--;
        if (expr->valid) {
            if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "First operand parsed successfully\n";
            expression->firstOperand = move(expr);
            foundFirstOperand = true;
            break;
        } else {
            if (DEBUG) std::cout << DEBUG_WARNING_PREFIX << "Operand parser failed\n";
            invalids.push_back(move(expr));
        }
    }

    if (!foundFirstOperand) {
        if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "No valid first operand found\n";

        lexer.rollPosition();
        auto furthest = getFurthestInvalidStatement(invalids);
        expression->expected = furthest->expected;
        expression->lastToken = furthest->lastToken;
        expression->valid = false;
        return expression;
    }

    expression->expressionOperator = {NONE, "", 0, 0};

    while (true) {
        if (lexer.expectToken(LEFT_PARENTHESIS)) {

            // this is a function call
            if (DEBUG) std::cout << DEBUG_PREFIX << "Expression is a function call\n";
            auto call = create_unique<FunctionCall>();
            call->function = move(expression);

            expression = create_unique<Expression>();

            while (!lexer.expectToken(RIGHT_PARENTHESIS)) {
                auto arg = Expression::parse(lexer);
                if (!arg->valid) {
                    if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Invalid expression in function arguments\n";

                    expression->valid = false;
                    expression->expected = arg->expected;
                    expression->lastToken = arg->lastToken;
                    lexer.rollPosition();
                    return expression;
                }

                call->arguments.push_back(move(arg));

                if (DEBUG) {
                    std::cout << DEBUG_SUCCESS_PREFIX
                            << "Parsed argument #" << call->arguments.size() << "\n";
                }

                Token sep = lexer.nextToken();
                if (sep.kind == RIGHT_PARENTHESIS) {
                    if (DEBUG) std::cout << DEBUG_PREFIX << "End of argument list\n";
                    break;
                }

                if (sep.kind != COMMA) {
                    if (DEBUG) {
                        std::cout << DEBUG_ERROR_PREFIX
                                << "Expected ',' or ')', got\n";
                    }

                    expression->valid = false;
                    expression->expected = tokenKindsToString({COMMA, RIGHT_PARENTHESIS});
                    expression->lastToken = sep;
                    lexer.rollPosition();
                    return call;
                }

                if (DEBUG) std::cout << DEBUG_PREFIX << "Comma found, parsing next argument\n";
            }
            expression->firstOperand = move(call);
            continue;
        }

        if (lexer.expectToken(DOT)) {
            auto dot = create_unique<DotAccessExpression>();
            dot->target = move(expression);

            Token member = lexer.nextToken();
            if (member.kind != IDENTIFIER) {
                dot->valid = false;
                dot->expected = {"member name"};
                dot->lastToken = member;
                lexer.rollPosition();
                return dot;
            }

            dot->member = member;
            dot->valid = true;

            expression = create_unique<Expression>();
            expression->firstOperand = move(dot);
            continue;
        }

        break;
    }

    // conditional
    if (lexer.expectToken(IF) || lexer.expectToken(UNLESS)) {

        if (DEBUG) std::cout << DEBUG_PREFIX << "Conditional expression\n";
        expression->conditional = true;
        expression->unlessCondition = lexer.currentToken().kind == UNLESS;

        if (DEBUG) std::cout << DEBUG_PREFIX << "Parsing expression\n";
        DEBUG_TABS++;
        auto condition = Expression::parse(lexer);
        DEBUG_TABS--;
        if (!condition->valid) {
            lexer.rollPosition();
            expression->valid = false;
            expression->expected = condition->expected;
            expression->lastToken = condition->lastToken;
            return expression;
        }
        expression->condition = move(condition);

        expression->withElseValue = false;
        if (lexer.expectToken(ELSE)) {
            expression->withElseValue = true;
            if (DEBUG) std::cout << DEBUG_PREFIX << "Parsing elseValue\n";
            DEBUG_TABS++;
            auto elseValue = Expression::parse(lexer);
            DEBUG_TABS--;
            if (!elseValue->valid) {
                lexer.rollPosition();
                expression->valid = false;
                expression->expected = elseValue->expected;
                expression->lastToken = elseValue->lastToken;
                return expression;
            }
            expression->elseValue = move(elseValue);
        }
    }

    lexer.savePosition();
    auto potentialOperator = lexer.nextToken();

    if (isBinaryOperator(potentialOperator.kind)) {
        if (DEBUG) {
            std::cout << DEBUG_PREFIX
                      << "Binary operator found: " << potentialOperator.value
                      << "\n";
        }
        expression->expressionOperator = potentialOperator;
        lexer.deletePosition();
    } else {
        if (DEBUG) std::cout << DEBUG_PREFIX << "No operator found, single operand expression\n";

        lexer.rollPosition();
        lexer.deletePosition();
        expression->valid = true;
        return expression;
    }

    if (DEBUG) std::cout << DEBUG_PREFIX << "Parsing second operand\n";

    DEBUG_TABS++;
    auto potentialSecondOperand = Expression::parse(lexer);
    DEBUG_TABS--;
    
    if (potentialSecondOperand->valid) {

        expression->secondOperand = move(potentialSecondOperand);
        expression->valid = true;
        lexer.deletePosition();

        if (expression->secondOperand->expressionOperator.kind != NONE) {
            int myPrecedence = precedence(expression->expressionOperator);
            int childPrecedence = precedence(expression->secondOperand->expressionOperator);

            if (DEBUG) {
                std::cout << DEBUG_PREFIX
                          << "Checking precedence: parent=" << myPrecedence
                          << ", child=" << childPrecedence << "\n";
            }

            if (myPrecedence > childPrecedence) {
                if (DEBUG) std::cout << DEBUG_PREFIX << "Rotating expression tree\n";

                auto newRoot = move(expression->secondOperand);
                expression->secondOperand = move(newRoot->firstOperand);
                newRoot->firstOperand = move(expression);
                return newRoot;
            }
        }

        if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "Expression parsed successfully\n";
        return expression;
    }

    if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Invalid second operand\n";

    lexer.rollPosition();
    auto furthest = getFurthestInvalidStatement(invalids);
    expression->expected = furthest->expected;
    expression->lastToken = furthest->lastToken;
    expression->valid = false;
    return expression;
}


uref<Expression> ExpressionParenWrapped::parse(Lexer& lexer) {

    lexer.savePosition();

    if (DEBUG) std::cout << DEBUG_PREFIX << "Trying ExpressionParenWrapped\n";  

    if (!lexer.expectToken(LEFT_PARENTHESIS)) {
        if (DEBUG) std::cout << DEBUG_WARNING_PREFIX << "No '(' found\n";

        auto exp = create_unique<Expression>();
        exp->lastToken = lexer.nextToken();
        exp->expected = tokenKindsToString({LEFT_PARENTHESIS});
        exp->valid = false;
        lexer.rollPosition();
        return exp;
    }

    if (DEBUG) std::cout << DEBUG_PREFIX << "Parsing inner expression\n";

    auto expression = Expression::parse(lexer);
    if (!expression->valid) {
        if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Invalid expression inside parentheses\n";
        lexer.rollPosition();
        return expression;
    }

    if (!lexer.expectToken(RIGHT_PARENTHESIS)) {
        if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Missing ')'\n";

        expression->lastToken = lexer.nextToken();
        expression->expected = tokenKindsToString({RIGHT_PARENTHESIS});
        expression->valid = false;
        lexer.rollPosition();
        return expression;
    }

    if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "Parenthesized expression parsed\n";

    lexer.deletePosition();
    return expression;
}

uref<ExpressionValue> ExpressionValue::parse(Lexer& lexer) {

    lexer.savePosition();
    auto expression = create_unique<ExpressionValue>();

    if (DEBUG) std::cout << DEBUG_PREFIX << "Trying ExpressionValue\n";

    Token nextToken = lexer.nextToken();
    if (
        nextToken.kind == IDENTIFIER ||
        nextToken.kind == STRING ||
        nextToken.kind == INTEGER ||
        nextToken.kind == FLOAT ||
        nextToken.kind == THIS ||
        nextToken.kind == TRUE ||
        nextToken.kind == FALSE ||
        nextToken.kind == NULL_TOKEN
    ) {
        if (DEBUG) {
            std::cout << DEBUG_SUCCESS_PREFIX
                      << "ExpressionValue parsed: "
                      << nextToken.value << "\n";
        }

        lexer.deletePosition();
        expression->identifier = nextToken;
        if (nextToken.kind == IDENTIFIER) {
            std::vector<Token> parsedArgs;
            if (parseTypeArgumentList(lexer, parsedArgs)) {
                expression->hasTypeArguments = true;
                expression->typeArguments = move(parsedArgs);
            }
        }
        expression->valid = true;
        return expression;
    }

    if (DEBUG) {
        std::cout << DEBUG_WARNING_PREFIX
                  << "ExpressionValue failed, got "
                  << nextToken.value << "\n";
    }

    expression->valid = false;
    expression->lastToken = nextToken;
    expression->expected = tokenKindsToString({
        IDENTIFIER, STRING, INTEGER, FLOAT, THIS, TRUE, FALSE, NULL_TOKEN
    });
    lexer.rollPosition();

    return expression;
}

uref<Expression> UnaryExpression::parse(Lexer& lexer) {

    lexer.savePosition();
    auto unaryExpr = create_unique<UnaryExpression>();

    Token op = lexer.nextToken();

    if (!isUnaryOperator(op.kind)) {
        lexer.rollPosition();
        unaryExpr->expected = {"unary operator"};
        unaryExpr->valid = false;
        return unaryExpr;
    }

    auto expr = Expression::parse(lexer);
    if (!expr->valid) {
        unaryExpr->valid = false;
        unaryExpr->lastToken = expr->lastToken;
        unaryExpr->expected = expr->expected;
        lexer.rollPosition();
        return unaryExpr;
    }

    lexer.deletePosition();
    unaryExpr->firstOperand = move(expr);
    unaryExpr->expressionOperator = op;
    unaryExpr->valid = true;
    return unaryExpr;

}

Value Expression::execute(Scope& scope) {
    
    auto value1 = this->firstOperand->execute(scope);
    if (conditional) {
        auto cond = isTruthy(condition->execute(scope));
        if (unlessCondition) cond = !cond;
        if (!cond) {
            value1 = NullValue;
            if (withElseValue) value1 = elseValue->execute(scope); 
        }
    }
    if (value1.thrownException != nullptr) return value1;

    if (this->expressionOperator.kind != NONE) {
        TokenKind opKind = this->expressionOperator.kind;

        auto value2 = this->secondOperand->execute(scope);
        if (value2.thrownException != nullptr) return value2;

        return performBinaryOperator(value1, value2, opKind);

    }

    return value1;

}

Value ExpressionValue::execute(Scope &scope) {

    auto token = this->identifier;

    switch(token.kind) {
        case INTEGER:
        {
            return Value((integer) std::stol(token.value));
        }
        
        case FLOAT:
        {
            return Value((real) std::stod(token.value));
        }
            

        case NULL_TOKEN:
        {
            return NullValue;
        }

        case STRING:
        {;
            return Value(token.value);
        }
            

        case TRUE:
        case FALSE:
        {
            boolean v = (token.kind == TRUE);
            return Value(v);
        }
        
        case THIS:
        {
            return scope.getVariable("this");
        }

        default: 
        {
            Value base = scope.getVariable(token.value);
            if (base.thrownException != nullptr) return base;

            if (!hasTypeArguments) {
                return base;
            }

            if (base.type != TypeType) {
                return makeTypeInstantiationError("type arguments can only be used on types");
            }

            std::vector<reference<Type>> args;
            args.reserve(typeArguments.size());
            for (auto& argToken : typeArguments) {
                Value argValue = scope.getVariable(argToken.value);
                if (argValue.thrownException != nullptr) return argValue;
                if (argValue.type != TypeType) {
                    return makeTypeInstantiationError("type argument is not a type: " + argToken.value);
                }
                args.push_back(get<reference<Type>>(argValue.value));
            }

            auto baseType = get<reference<Type>>(base.value);
            return instantiateType(baseType, args, false);
        }
    }
}

Value UnaryExpression::execute(Scope &scope) {
    return performUnaryOperator(this->firstOperand->execute(scope), this->expressionOperator.kind);
}

uref<Expression> DotAccessExpression::parse(Lexer& lexer) {
    auto expr = create_unique<DotAccessExpression>();
    expr->valid = false;
    expr->expected = {"dot access"};
    expr->lastToken = lexer.peekToken();
    return expr;
}

Value DotAccessExpression::execute(Scope& scope) {
    Value base = target->execute(scope);
    if (base.thrownException != nullptr) return base;

    if (base.type == TypeType) {
        auto typeRef = get<reference<Type>>(base.value);

        if (typeRef->staticFieldValues.contains(member.value)) {
            return *typeRef->staticFieldValues[member.value];
        }

        if (typeRef->staticMethods.contains(member.value)) {
            const auto& method = typeRef->staticMethods[member.value];
            if (method.overloads.empty()) {
                Value err;
                err.thrownException = create_reference<Value>(
                    Value("static method has no overloads: " + member.value)
                );
                return err;
            }
            return Value(method.overloads.front());
        }

        Value err;
        err.thrownException = create_reference<Value>(
            Value("unknown static member: " + member.value)
        );
        return err;
    }

    if (base.type != nullptr && base.type->kind == TypeKind::Class) {
        auto instance = get<ClassInstance>(base.value);

        if (instance.fieldValues.contains(member.value)) {
            return instance.fieldValues[member.value];
        }

        if (base.type->methods.contains(member.value)) {
            const auto& method = base.type->methods[member.value];
            if (method.overloads.empty()) {
                Value err;
                err.thrownException = create_reference<Value>(
                    Value("method has no overloads: " + member.value)
                );
                return err;
            }
            Function boundMethod = method.overloads.front();
            boundMethod.__this = create_reference<Value>(base);
            return Value(boundMethod);
        }

        Value err;
        err.thrownException = create_reference<Value>(
            Value("unknown instance member: " + member.value)
        );
        return err;
    }

    Value err;
    err.thrownException = create_reference<Value>(
        Value("dot access requires class type or instance")
    );
    return err;
}

uref<Expression> ConstructorExpression::parse(Lexer& lexer) {
    lexer.savePosition();
    auto expr = create_unique<ConstructorExpression>();

    if (!lexer.expectToken(NEW)) {
        expr->valid = false;
        expr->expected = {"new"};
        expr->lastToken = lexer.peekToken();
        lexer.rollPosition();
        return expr;
    }

    Token typeToken = lexer.nextToken();
    if (typeToken.kind != IDENTIFIER) {
        expr->valid = false;
        expr->expected = {"class name"};
        expr->lastToken = typeToken;
        lexer.rollPosition();
        return expr;
    }
    expr->typeName = typeToken;
    parseTypeArgumentList(lexer, expr->typeArguments);

    if (!lexer.expectToken(LEFT_PARENTHESIS)) {
        expr->valid = false;
        expr->expected = tokenKindsToString({LEFT_PARENTHESIS});
        expr->lastToken = lexer.nextToken();
        lexer.rollPosition();
        return expr;
    }

    while (!lexer.expectToken(RIGHT_PARENTHESIS)) {
        auto arg = Expression::parse(lexer);
        if (!arg->valid) {
            expr->valid = false;
            expr->expected = arg->expected;
            expr->lastToken = arg->lastToken;
            lexer.rollPosition();
            return expr;
        }
        expr->arguments.push_back(move(arg));

        Token sep = lexer.nextToken();
        if (sep.kind == RIGHT_PARENTHESIS) {
            break;
        }
        if (sep.kind != COMMA) {
            expr->valid = false;
            expr->expected = tokenKindsToString({COMMA, RIGHT_PARENTHESIS});
            expr->lastToken = sep;
            lexer.rollPosition();
            return expr;
        }
    }

    expr->valid = true;
    lexer.deletePosition();
    return expr;
}

Value ConstructorExpression::execute(Scope& scope) {
    Value classValue = scope.getVariable(typeName.value);
    if (classValue.thrownException != nullptr) return classValue;
    if (classValue.type != TypeType) {
        Value err;
        err.thrownException = create_reference<Value>(
            Value("new target is not a type: " + typeName.value)
        );
        return err;
    }

    auto classType = get<reference<Type>>(classValue.value);
    if (!typeArguments.empty()) {
        std::vector<reference<Type>> args;
        args.reserve(typeArguments.size());
        for (auto& argToken : typeArguments) {
            Value argValue = scope.getVariable(argToken.value);
            if (argValue.thrownException != nullptr) return argValue;
            if (argValue.type != TypeType) {
                return makeTypeInstantiationError("type argument is not a type: " + argToken.value);
            }
            args.push_back(get<reference<Type>>(argValue.value));
        }

        Value specialized = instantiateType(classType, args, true);
        if (specialized.thrownException != nullptr) return specialized;
        classType = get<reference<Type>>(specialized.value);
    } else if (!classType->typeParameters.empty()) {
        Value specialized = instantiateType(classType, {}, true);
        if (specialized.thrownException != nullptr) return specialized;
        classType = get<reference<Type>>(specialized.value);
    }

    if (classType->kind != TypeKind::Class) {
        Value err;
        err.thrownException = create_reference<Value>(
            Value("new target is not a class: " + typeName.value)
        );
        return err;
    }

    ClassInstance instance;
    instance.classType = classType;
    Scope typeScope(scope);
    for (auto& [tpName, tpType] : classType->typeBindings) {
        if (tpType != nullptr) {
            typeScope.addVariable(tpName, Value(tpType));
        }
    }

    for (const auto& field : classType->fields) {
        Value fieldValue;
        if (field.hasDefaultValue) {
            fieldValue = field.value->execute(typeScope);
        } else {
            Value fieldTypeValue = field.type->execute(typeScope);
            if (fieldTypeValue.thrownException != nullptr) return fieldTypeValue;
            if (fieldTypeValue.type != TypeType) {
                Value err;
                err.thrownException = create_reference<Value>(
                    Value("field type is not a type: " + field.name)
                );
                return err;
            }
            auto fieldType = get<reference<Type>>(fieldTypeValue.value);
            fieldValue = Value::Uninitialized(fieldType);
        }

        if (fieldValue.thrownException != nullptr) return fieldValue;
        instance.fieldValues[field.name] = fieldValue;
    }

    std::vector<Value> argValues;
    argValues.reserve(arguments.size());
    for (auto& argExpr : arguments) {
        Value argValue = argExpr->execute(scope);
        if (argValue.thrownException != nullptr) return argValue;
        argValues.push_back(argValue);
    }

    if (!classType->constructor.overloads.empty()) {
        Function* selected = nullptr;
        for (auto& overload : classType->constructor.overloads) {
            if (overload.parameters.size() != argValues.size()) continue;
            bool compatible = true;
            for (size_t i = 0; i < argValues.size(); ++i) {
                if (argValues[i].type != overload.parameters[i].type) {
                    compatible = false;
                    break;
                }
            }
            if (compatible) {
                selected = &overload;
                break;
            }
        }

        if (selected == nullptr) {
            Value err;
            err.thrownException = create_reference<Value>(
                Value("no matching constructor overload for " + typeName.value)
            );
            return err;
        }

        Function ctorFunction = *selected;
        Value thisValue;
        thisValue.type = classType;
        thisValue.value = instance;
        ctorFunction.__this = create_reference<Value>(thisValue);

        Scope ctorScope(ctorFunction.closure);
        ctorScope.addVariable("this", *ctorFunction.__this);
        for (size_t i = 0; i < ctorFunction.parameters.size(); ++i) {
            ctorScope.addVariable(ctorFunction.parameters[i].name, argValues[i]);
        }

        Value ctorResult = ctorFunction.body->execute(ctorScope);
        if (ctorResult.thrownException != nullptr) return ctorResult;
        thisValue = ctorScope.getVariable("this");
        if (thisValue.thrownException != nullptr) return thisValue;
        instance = get<ClassInstance>(thisValue.value);
    } else if (!argValues.empty()) {
        Value err;
        err.thrownException = create_reference<Value>(
            Value("class has no constructor overloads: " + typeName.value)
        );
        return err;
    }

    Value ret;
    ret.type = classType;
    ret.value = instance;
    return ret;
}
