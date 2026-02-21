#include <data.h>
#include "statements/expression/Expression.h"
#include "statements/function/FunctionCall.hpp"
#include "statements/function/LambdaExpression.hpp"
#include "types/function.h"
#include <expressions.h>
#include <functional>
#include "runtime_builtins.h"

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

static Value makeExpressionError(const std::string& message) {
    Value err;
    err.thrownException = create_reference<Value>(Value(message));
    return err;
}

static bool bindCallArguments(
    const Function& function,
    const std::vector<Value>& args,
    std::vector<Value>& bound,
    std::string& error
) {
    if (!function.validArguments(args)) {
        error = "arguments are not compatible with function signature";
        return false;
    }

    size_t fixedCount = function.parameters.size();
    bool variadic = !function.parameters.empty() && function.parameters.back().variadic;
    if (variadic) fixedCount--;

    bound.clear();
    bound.reserve(function.parameters.size());
    for (size_t i = 0; i < fixedCount; ++i) bound.push_back(args[i]);

    if (variadic) {
        std::vector<Value> tail;
        for (size_t i = fixedCount; i < args.size(); ++i) tail.push_back(args[i]);
        Value packed = makeArrayFromValues(tail);
        if (packed.thrownException != nullptr) {
            error = stringify(*packed.thrownException);
            return false;
        }
        bound.push_back(packed);
    }

    return true;
}

static Value invokeFunction(const Function& function, Scope& scope, const std::vector<Value>& args) {
    std::vector<Value> boundArgs;
    std::string bindError;
    if (!bindCallArguments(function, args, boundArgs, bindError)) {
        return makeExpressionError(bindError);
    }

    if (function.kind == FunctionKind::Internal) {
        if (!function.internalHandler) return makeExpressionError("internal function is missing implementation");
        return function.internalHandler(scope, boundArgs, function.__this);
    }

    Scope funcScope(function.closure);
    if (function.__this != nullptr) {
        funcScope.addVariable("this", *function.__this);
    }
    for (size_t i = 0; i < function.parameters.size(); ++i) {
        funcScope.addVariable(function.parameters[i].name, boundArgs[i]);
    }
    return function.body->execute(funcScope);
}

static reference<Type> resolveParameterType(const reference<Type>& ownerType, const reference<Type>& parameterType) {
    if (ownerType == nullptr || parameterType == nullptr) return parameterType;
    if (parameterType->kind != TypeKind::Dynamic) return parameterType;
    auto paramName = parameterType->getName();
    reference<Type> cursor = ownerType;
    while (cursor != nullptr) {
        if (cursor->typeBindings.contains(paramName)) {
            auto bound = cursor->typeBindings.at(paramName);
            if (bound != nullptr) return bound;
            break;
        }
        cursor = cursor->parent;
    }
    return parameterType;
}

static bool acceptsParamType(const reference<Type>& targetType, const reference<Type>& sourceType) {
    if (targetType == nullptr || sourceType == nullptr) return false;
    auto sourceCopy = sourceType;
    Value probe = Value::Uninitialized(sourceCopy);
    probe.type = sourceType;
    return targetType->assignableFrom(probe);
}

static bool overloadDirectionallyCovers(const reference<Type>& ownerType, const Function& a, const Function& b) {
    const bool aVariadic = !a.parameters.empty() && a.parameters.back().variadic;
    const bool bVariadic = !b.parameters.empty() && b.parameters.back().variadic;
    if (aVariadic != bVariadic) return false;

    size_t aFixed = a.parameters.size() - (aVariadic ? 1 : 0);
    size_t bFixed = b.parameters.size() - (bVariadic ? 1 : 0);
    if (aFixed != bFixed) return false;

    for (size_t i = 0; i < aFixed; ++i) {
        auto aType = resolveParameterType(ownerType, a.parameters[i].type);
        auto bType = resolveParameterType(ownerType, b.parameters[i].type);
        if (aType != nullptr && aType == a.parameters[i].type && aType->kind == TypeKind::Dynamic) return false;
        if (bType != nullptr && bType == b.parameters[i].type && bType->kind == TypeKind::Dynamic) return false;
        if (!acceptsParamType(aType, bType)) return false;
    }

    if (aVariadic) {
        auto aType = resolveParameterType(ownerType, a.parameters.back().type);
        auto bType = resolveParameterType(ownerType, b.parameters.back().type);
        if (aType != nullptr && aType == a.parameters.back().type && aType->kind == TypeKind::Dynamic) return false;
        if (bType != nullptr && bType == b.parameters.back().type && bType->kind == TypeKind::Dynamic) return false;
        if (!acceptsParamType(aType, bType)) return false;
    }

    return true;
}

static Value ensureNoCompatibleOverloads(const reference<Type>& ownerType, const Method& method, const string& ownerName, const string& methodName) {
    for (size_t i = 0; i < method.overloads.size(); ++i) {
        for (size_t j = i + 1; j < method.overloads.size(); ++j) {
            const auto& a = method.overloads[i];
            const auto& b = method.overloads[j];
            if (overloadDirectionallyCovers(ownerType, a, b) || overloadDirectionallyCovers(ownerType, b, a)) {
                return makeTypeInstantiationError(
                    "ambiguous overloads for " + ownerName + "." + methodName + " after specialization"
                );
            }
        }
    }
    return NullValue;
}

static bool bindArgumentsForFunction(
    const Function& fn,
    const std::vector<Value>& args,
    std::vector<Value>& bound,
    Value& error
) {
    if (!fn.validArguments(args)) {
        error = makeTypeInstantiationError("constructor arguments are not compatible with signature");
        return false;
    }

    size_t fixedCount = fn.parameters.size();
    bool variadic = !fn.parameters.empty() && fn.parameters.back().variadic;
    if (variadic) fixedCount--;

    bound.clear();
    bound.reserve(fn.parameters.size());
    for (size_t i = 0; i < fixedCount; ++i) bound.push_back(args[i]);

    if (variadic) {
        std::vector<Value> tail;
        for (size_t i = fixedCount; i < args.size(); ++i) {
            tail.push_back(args[i]);
        }
        Value packed = makeArrayFromValues(tail);
        if (packed.thrownException != nullptr) {
            error = packed;
            return false;
        }
        bound.push_back(packed);
    }

    return true;
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
            auto candidate = args[consumed++];
            auto& tp = specialized->typeParameters[idx];
            if (tp.hasConstraint && tp.constraint != nullptr) {
                auto probeType = candidate;
                Value probe = Value::Uninitialized(probeType);
                probe.type = candidate;
                if (!tp.constraint->assignableFrom(probe)) {
                    return makeTypeInstantiationError(
                        "type argument for " + tp.name + " does not satisfy extends constraint"
                    );
                }
            }
            tp.value = candidate;
            continue;
        }

        if (specialized->typeParameters[idx].hasDefault && specialized->typeParameters[idx].defaultValue != nullptr) {
            auto& tp = specialized->typeParameters[idx];
            auto candidate = tp.defaultValue;
            if (tp.hasConstraint && tp.constraint != nullptr) {
                auto probeType = candidate;
                Value probe = Value::Uninitialized(probeType);
                probe.type = candidate;
                if (!tp.constraint->assignableFrom(probe)) {
                    return makeTypeInstantiationError(
                        "default type argument for " + tp.name + " does not satisfy extends constraint"
                    );
                }
            }
            tp.value = candidate;
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

    if (!specialized->constructor.overloads.empty()) {
        Value check = ensureNoCompatibleOverloads(specialized, specialized->constructor, specialized->getName(), specialized->getName());
        if (check.thrownException != nullptr) return check;
    }
    for (const auto& [methodName, method] : specialized->methods) {
        Value check = ensureNoCompatibleOverloads(specialized, method, specialized->getName(), methodName);
        if (check.thrownException != nullptr) return check;
    }
    for (const auto& [methodName, method] : specialized->staticMethods) {
        Value check = ensureNoCompatibleOverloads(specialized, method, specialized->getName(), methodName);
        if (check.thrownException != nullptr) return check;
    }

    return Value(specialized);
}

static bool isSameOrDerivedType(const reference<Type>& maybeChild, const reference<Type>& maybeAncestor) {
    if (maybeChild == nullptr || maybeAncestor == nullptr) return false;
    reference<Type> cursor = maybeChild;
    while (cursor != nullptr) {
        if (cursor == maybeAncestor) return true;
        if (!cursor->getName().empty() && cursor->getName() == maybeAncestor->getName()) return true;
        cursor = cursor->parent;
    }
    return false;
}

static bool hasThisAccess(Scope& scope, const reference<Type>& ownerType, bool allowDerived) {
    if (!scope.hasVariable("this")) return false;
    Value thisValue = scope.getVariable("this");
    if (thisValue.thrownException != nullptr || thisValue.type == nullptr || thisValue.type->kind != TypeKind::Class) {
        return false;
    }
    if (allowDerived) return isSameOrDerivedType(thisValue.type, ownerType);
    return thisValue.type == ownerType || thisValue.type->getName() == ownerType->getName();
}

static bool canAccessMember(unsigned int flags, const reference<Type>& ownerType, Scope& scope) {
    if ((flags & MemberFlags::Public) != 0) return true;
    if ((flags & MemberFlags::Private) != 0) return hasThisAccess(scope, ownerType, false);
    if ((flags & MemberFlags::Protected) != 0) return hasThisAccess(scope, ownerType, true);
    return hasThisAccess(scope, ownerType, false);
}

static bool findFieldInHierarchy(
    const reference<Type>& startType,
    const std::string& name,
    bool staticField,
    Field& outField,
    reference<Type>& outOwnerType
) {
    reference<Type> cursor = startType;
    while (cursor != nullptr) {
        const auto& fields = staticField ? cursor->staticFields : cursor->fields;
        for (const auto& f : fields) {
            if (f.name == name) {
                outField = f;
                outOwnerType = cursor;
                return true;
            }
        }
        cursor = cursor->parent;
    }
    return false;
}

static bool findMethodInHierarchy(
    const reference<Type>& startType,
    const std::string& name,
    bool staticMethod,
    Method& outMethod,
    reference<Type>& outOwnerType
) {
    reference<Type> cursor = startType;
    while (cursor != nullptr) {
        const auto& methods = staticMethod ? cursor->staticMethods : cursor->methods;
        if (methods.contains(name)) {
            outMethod = methods.at(name);
            outOwnerType = cursor;
            return true;
        }
        cursor = cursor->parent;
    }
    return false;
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

        if (lexer.expectToken(LEFT_BRACKET)) {
            auto idx = create_unique<IndexAccessExpression>();
            idx->target = move(expression);

            while (!lexer.expectToken(RIGHT_BRACKET)) {
                auto arg = Expression::parse(lexer);
                if (!arg->valid) {
                    idx->valid = false;
                    idx->expected = arg->expected;
                    idx->lastToken = arg->lastToken;
                    lexer.rollPosition();
                    return idx;
                }
                idx->arguments.push_back(move(arg));

                Token sep = lexer.nextToken();
                if (sep.kind == RIGHT_BRACKET) break;
                if (sep.kind != COMMA) {
                    idx->valid = false;
                    idx->expected = tokenKindsToString({COMMA, RIGHT_BRACKET});
                    idx->lastToken = sep;
                    lexer.rollPosition();
                    return idx;
                }
            }

            idx->valid = true;
            expression = create_unique<Expression>();
            expression->firstOperand = move(idx);
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

bool ExpressionValue::isPlainIdentifier(Token* outIdentifier) const {
    if (identifier.kind != IDENTIFIER && identifier.kind != THIS) return false;
    if (hasTypeArguments) return false;
    if (outIdentifier != nullptr) *outIdentifier = identifier;
    return true;
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

uref<Expression> IndexAccessExpression::parse(Lexer& lexer) {
    auto expr = create_unique<IndexAccessExpression>();
    expr->valid = false;
    expr->expected = {"index access"};
    expr->lastToken = lexer.peekToken();
    return expr;
}

Value DotAccessExpression::execute(Scope& scope) {
    Value base = target->execute(scope);
    if (base.thrownException != nullptr) return base;

    if (base.type == StringType) {
        if (member.value == "length") {
            Function fn;
            fn.kind = FunctionKind::Internal;
            fn.internalHandler = [](Scope&, const std::vector<Value>& args, const reference<Value>& self) -> Value {
                if (!args.empty()) {
                    return makeExpressionError("string.length expects no arguments");
                }
                if (self == nullptr || self->type != StringType) {
                    return makeExpressionError("string.length receiver is not a string");
                }
                return Value((integer)get<string>(self->value).size());
            };
            fn.__this = create_reference<Value>(base);
            return Value(fn);
        }

        return makeExpressionError("unknown string member: " + member.value);
    }

    if (base.type == TypeType) {
        auto typeRef = get<reference<Type>>(base.value);

        Field fieldMeta;
        reference<Type> ownerType;
        if (findFieldInHierarchy(typeRef, member.value, true, fieldMeta, ownerType)) {
            if (!canAccessMember(fieldMeta.flags, ownerType, scope)) {
                return makeExpressionError("cannot access static field: " + member.value);
            }
            if (ownerType->staticFieldValues.contains(member.value)) {
                return *ownerType->staticFieldValues[member.value];
            }
        }

        Method method;
        if (findMethodInHierarchy(typeRef, member.value, true, method, ownerType)) {
            if (!canAccessMember(method.flags, ownerType, scope)) {
                return makeExpressionError("cannot access static method: " + member.value);
            }
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

        Field fieldMeta;
        reference<Type> ownerType;
        if (findFieldInHierarchy(base.type, member.value, false, fieldMeta, ownerType)) {
            if (!canAccessMember(fieldMeta.flags, ownerType, scope)) {
                return makeExpressionError("cannot access field: " + member.value);
            }
            if (instance.fieldValues.contains(member.value)) {
                return instance.fieldValues[member.value];
            }
            return NullValue;
        }

        Method method;
        if (findMethodInHierarchy(base.type, member.value, false, method, ownerType)) {
            if (!canAccessMember(method.flags, ownerType, scope)) {
                return makeExpressionError("cannot access method: " + member.value);
            }
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

Value IndexAccessExpression::execute(Scope& scope) {
    Value base = target->execute(scope);
    if (base.thrownException != nullptr) return base;

    std::vector<Value> args;
    args.reserve(arguments.size());
    for (auto& argExpr : arguments) {
        Value argValue = argExpr->execute(scope);
        if (argValue.thrownException != nullptr) return argValue;
        args.push_back(argValue);
    }

    if (base.type == StringType) {
        if (args.size() != 1 || args[0].type != IntegerType) {
            return makeExpressionError("string indexing expects one integer index");
        }
        const auto& s = get<string>(base.value);
        integer idx = get<integer>(args[0].value);
        if (idx < 0 || (size_t)idx >= s.size()) {
            return makeExpressionError("string index out of range");
        }
        return Value(string(1, s[(size_t)idx]));
    }

    if (base.type == TypeType) {
        return makeExpressionError("cannot index type value");
    }

    if (base.type != nullptr && base.type->kind == TypeKind::Class) {
        auto typeRef = base.type;
        Method getMethod;
        reference<Type> ownerType;
        if (!findMethodInHierarchy(typeRef, "get", false, getMethod, ownerType)) {
            return makeExpressionError("type does not implement get[...] operator");
        }
        if (!canAccessMember(getMethod.flags, ownerType, scope)) {
            return makeExpressionError("cannot access get[...] operator");
        }

        const auto& overloads = getMethod.overloads;
        Function* selected = nullptr;
        for (const auto& overload : overloads) {
            if (!overload.validArguments(args)) continue;
            selected = const_cast<Function*>(&overload);
            break;
        }
        if (selected == nullptr) {
            return makeExpressionError("no matching get[...] overload");
        }

        Function fn = *selected;
        fn.__this = create_reference<Value>(base);
        return invokeFunction(fn, scope, args);
    }

    return makeExpressionError("index access is not supported on this value");
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
    initializeBuiltinInstance(instance);
    Scope typeScope(scope);
    for (auto& [tpName, tpType] : classType->typeBindings) {
        if (tpType != nullptr) {
            typeScope.addVariable(tpName, Value(tpType));
        }
    }

    std::vector<Field> allFields;
    std::function<void(reference<Type>)> collectFields = [&](reference<Type> t) {
        if (t == nullptr) return;
        collectFields(t->parent);
        for (const auto& f : t->fields) allFields.push_back(f);
    };
    collectFields(classType);

    for (const auto& field : allFields) {
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
        std::vector<Value> boundCtorArgs;
        for (auto& overload : classType->constructor.overloads) {
            Value bindErr;
            std::vector<Value> candidate;
            if (bindArgumentsForFunction(overload, argValues, candidate, bindErr)) {
                selected = &overload;
                boundCtorArgs = std::move(candidate);
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

        if (ctorFunction.kind == FunctionKind::Internal) {
            Value ctorResult = ctorFunction.internalHandler(scope, boundCtorArgs, ctorFunction.__this);
            if (ctorResult.thrownException != nullptr) return ctorResult;
            if (ctorFunction.__this != nullptr) {
                thisValue = *ctorFunction.__this;
                instance = get<ClassInstance>(thisValue.value);
            }
        } else {
            Scope ctorScope(ctorFunction.closure);
            ctorScope.addVariable("this", *ctorFunction.__this);
            for (size_t i = 0; i < ctorFunction.parameters.size(); ++i) {
                ctorScope.addVariable(ctorFunction.parameters[i].name, boundCtorArgs[i]);
            }

            Value ctorResult = ctorFunction.body->execute(ctorScope);
            if (ctorResult.thrownException != nullptr) return ctorResult;
            thisValue = ctorScope.getVariable("this");
            if (thisValue.thrownException != nullptr) return thisValue;
            instance = get<ClassInstance>(thisValue.value);
        }
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
