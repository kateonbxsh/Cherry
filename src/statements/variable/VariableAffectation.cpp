#include "VariableAffectation.h"
#include "types/type.h"
#include "types/function.h"
#include "runtime_builtins.h"
#include "statements/Block.hpp"
#include <expressions.h>

static Value makeAssignmentError(const std::string& message) {
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

static Value invokeFunction(
    const Function& function,
    Scope& scope,
    const std::vector<Value>& args,
    const Value* implicitAssignedValue = nullptr
) {
    std::vector<Value> boundArgs;
    std::string bindError;
    if (!bindCallArguments(function, args, boundArgs, bindError)) {
        return makeAssignmentError(bindError);
    }

    if (function.kind == FunctionKind::Internal) {
        if (!function.internalHandler) return makeAssignmentError("internal function is missing implementation");
        return function.internalHandler(scope, boundArgs, function.__this);
    }

    Scope funcScope(function.closure);
    if (function.__this != nullptr) {
        funcScope.addVariable("this", *function.__this);
    }
    for (size_t i = 0; i < function.parameters.size(); ++i) {
        funcScope.addVariable(function.parameters[i].name, boundArgs[i]);
    }
    if (implicitAssignedValue != nullptr) {
        funcScope.addVariable("value", *implicitAssignedValue);
    }
    Value result = function.body->execute(funcScope);
    if (result.thrownException != nullptr) return result;

    if (function.__this != nullptr) {
        Value updatedThis = funcScope.getVariable("this");
        if (updatedThis.thrownException != nullptr) return updatedThis;
        *function.__this = updatedThis;
    }
    return result;
}

static Value setStaticMember(Value& owner, const std::string& name, const Value& newValue) {
    if (owner.type == TypeType) {
        auto typeRef = get<reference<Type>>(owner.value);
        auto applyToType = [&](reference<Type> current) {
            if (current->staticFieldValues.contains(name)) {
                *current->staticFieldValues[name] = newValue;
            } else {
                current->staticFieldValues[name] = create_reference<Value>(newValue);
            }
        };

        applyToType(typeRef);
        auto p = typeRef->parent;
        while (p != nullptr) {
            applyToType(p);
            p = p->parent;
        }
        return newValue;
    }

    return makeAssignmentError("dot assignment on type requires static field target");
}

static Value assignToTarget(Expression* target, Scope& scope, const Value& value);

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
    Method& outMethod,
    reference<Type>& outOwnerType
) {
    reference<Type> cursor = startType;
    while (cursor != nullptr) {
        if (cursor->methods.contains(name)) {
            outMethod = cursor->methods[name];
            outOwnerType = cursor;
            return true;
        }
        cursor = cursor->parent;
    }
    return false;
}

static Expression* unwrapAssignableTarget(Expression* target) {
    Expression* current = target;
    while (current != nullptr) {
        auto expr = dynamic_cast<Expression*>(current);
        if (expr == nullptr) break;
        if (expr->expressionOperator.kind != NONE) break;
        if (expr->conditional) break;
        if (expr->firstOperand == nullptr) break;
        current = expr->firstOperand.get();
    }
    return current;
}

static Value assignToIndex(IndexAccessExpression* idx, Scope& scope, const Value& value) {
    Value base = idx->target->execute(scope);
    if (base.thrownException != nullptr) return base;

    std::vector<Value> indexArgs;
    indexArgs.reserve(idx->arguments.size());
    for (auto& argExpr : idx->arguments) {
        Value argValue = argExpr->execute(scope);
        if (argValue.thrownException != nullptr) return argValue;
        indexArgs.push_back(argValue);
    }

    if (base.type != nullptr && base.type->kind == TypeKind::Class) {
        auto typeRef = base.type;
        Method setMethodMeta;
        reference<Type> methodOwner;
        if (!findMethodInHierarchy(typeRef, "set", setMethodMeta, methodOwner)) {
            return makeAssignmentError("type does not implement set[...] operator");
        }
        if (!canAccessMember(setMethodMeta.flags, methodOwner, scope)) {
            return makeAssignmentError("cannot access set[...] operator");
        }
        Function* selected = nullptr;
        bool selectedWithAssignedParameter = false;
        for (const auto& overload : setMethodMeta.overloads) {
            std::vector<Value> fullArgs = indexArgs;
            fullArgs.push_back(value);

            bool matchesIndexOnly = overload.validArguments(indexArgs);
            bool matchesWithAssigned = overload.validArguments(fullArgs);
            bool matches = matchesIndexOnly || matchesWithAssigned;
            if (!matches) continue;
            selected = const_cast<Function*>(&overload);
            selectedWithAssignedParameter = matchesWithAssigned && !matchesIndexOnly;
            break;
        }
        if (selected == nullptr) return makeAssignmentError("no matching set[...] overload");

        Function fn = *selected;
        fn.__this = create_reference<Value>(base);
        std::vector<Value> callArgs = indexArgs;
        if (fn.kind == FunctionKind::Internal || selectedWithAssignedParameter) {
            callArgs.push_back(value);
        }
        Value callResult = invokeFunction(fn, scope, callArgs, &value);
        if (callResult.thrownException != nullptr) return callResult;

        if (fn.__this != nullptr) {
            Value persistedOwner = assignToTarget(idx->target.get(), scope, *fn.__this);
            if (persistedOwner.thrownException != nullptr) return persistedOwner;
        }
        return callResult;
    }

    return makeAssignmentError("index assignment is not supported on this value");
}

static Value assignToTarget(Expression* target, Scope& scope, const Value& value) {
    target = unwrapAssignableTarget(target);

    if (auto id = dynamic_cast<ExpressionValue*>(target)) {
        Token t;
        if (!id->isPlainIdentifier(&t)) {
            return makeAssignmentError("assignment target must be assignable");
        }
        if (t.kind == THIS) {
            scope.setVariable("this", value);
        } else {
            scope.setVariable(t.value, value);
        }
        return value;
    }

    if (auto dot = dynamic_cast<DotAccessExpression*>(target)) {
        Value owner = dot->target->execute(scope);
        if (owner.thrownException != nullptr) return owner;
        if (owner.type == TypeType) {
            Field fieldMeta;
            reference<Type> fieldOwner;
            if (findFieldInHierarchy(get<reference<Type>>(owner.value), dot->member.value, true, fieldMeta, fieldOwner)) {
                if (!canAccessMember(fieldMeta.flags, fieldOwner, scope)) {
                    return makeAssignmentError("cannot assign static field: " + dot->member.value);
                }
                if ((fieldMeta.flags & MemberFlags::Readonly) != 0) {
                    return makeAssignmentError("cannot assign readonly static field: " + dot->member.value);
                }
            }
            return setStaticMember(owner, dot->member.value, value);
        }
        if (owner.type != nullptr && owner.type->kind == TypeKind::Class) {
            Field fieldMeta;
            reference<Type> fieldOwner;
            if (!findFieldInHierarchy(owner.type, dot->member.value, false, fieldMeta, fieldOwner)) {
                return makeAssignmentError("unknown field: " + dot->member.value);
            }
            if (!canAccessMember(fieldMeta.flags, fieldOwner, scope)) {
                return makeAssignmentError("cannot assign field: " + dot->member.value);
            }
            if ((fieldMeta.flags & MemberFlags::Readonly) != 0) {
                return makeAssignmentError("cannot assign readonly field: " + dot->member.value);
            }
            auto instance = get<ClassInstance>(owner.value);
            instance.fieldValues[dot->member.value] = value;
            owner.value = instance;

            Value persistedOwner = assignToTarget(dot->target.get(), scope, owner);
            if (persistedOwner.thrownException != nullptr) return persistedOwner;
            return value;
        }
        return makeAssignmentError("dot assignment requires class type or instance");
    }

    if (auto idx = dynamic_cast<IndexAccessExpression*>(target)) {
        return assignToIndex(idx, scope, value);
    }

    return makeAssignmentError("invalid assignment target");
}

uref<VariableAffectation> VariableAffectation::parse(Lexer &lexer) {
    lexer.savePosition();

    auto varAff = create_unique<VariableAffectation>();
    auto lhs = Expression::parse(lexer);
    if (!lhs->valid) {
        varAff->valid = false;
        varAff->expected = lhs->expected;
        varAff->lastToken = lhs->lastToken;
        lexer.rollPosition();
        return varAff;
    }

    varAff->selfOperation = NONE;
    Token op = lexer.nextToken();
    if (isBinaryOperator(op.kind)) {
        varAff->selfOperation = op.kind;
    } else {
        lexer.back();
    }

    if (!lexer.expectToken(EQUALS)) {
        varAff->valid = false;
        varAff->expected = tokenKindsToString({EQUALS});
        varAff->lastToken = lexer.nextToken();
        lexer.rollPosition();
        return varAff;
    }

    auto rhs = Expression::parse(lexer);
    if (!rhs->valid) {
        varAff->valid = false;
        varAff->expected = rhs->expected;
        varAff->lastToken = rhs->lastToken;
        lexer.rollPosition();
        return varAff;
    }

    if (!lexer.expectToken(SEMICOLON)) {
        varAff->valid = false;
        varAff->expected = tokenKindsToString({SEMICOLON});
        varAff->lastToken = lexer.nextToken();
        lexer.rollPosition();
        return varAff;
    }

    varAff->leftTarget = move(lhs);
    varAff->expression = move(rhs);
    varAff->valid = true;
    lexer.deletePosition();
    return varAff;
}

Value VariableAffectation::execute(Scope& scope) {
    Value value = expression->execute(scope);
    if (value.thrownException != nullptr) return value;

    if (selfOperation != NONE) {
        Value current = leftTarget->execute(scope);
        if (current.thrownException != nullptr) return current;
        value = performBinaryOperator(current, value, selfOperation);
        if (value.thrownException != nullptr) return value;
    }

    return assignToTarget(leftTarget.get(), scope, value);
}
