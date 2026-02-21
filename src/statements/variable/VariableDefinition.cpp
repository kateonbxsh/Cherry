#include "VariableDefinition.h"
#include "statements/expression/Expression.h"
#include "types/type.h"

namespace {

Value makeVarDefError(const std::string& message) {
    Value err;
    err.thrownException = create_reference<Value>(Value(message));
    return err;
}

Value instantiateGenericType(reference<Type> baseType, const std::vector<reference<Type>>& args, bool strictMissing) {
    if (baseType == nullptr) return makeVarDefError("cannot instantiate null type");

    std::vector<size_t> undefinedIndices;
    for (size_t i = 0; i < baseType->typeParameters.size(); ++i) {
        if (baseType->typeParameters[i].value == nullptr) undefinedIndices.push_back(i);
    }

    if (args.size() > undefinedIndices.size()) {
        return makeVarDefError("too many type arguments for " + baseType->getName());
    }

    auto specialized = create_reference<Type>(*baseType);
    specialized->parent = baseType;
    specialized->typeBindings = baseType->typeBindings;

    size_t consumed = 0;
    for (size_t idx : undefinedIndices) {
        auto& tp = specialized->typeParameters[idx];
        if (consumed < args.size()) {
            auto candidate = args[consumed++];
            if (tp.hasConstraint && tp.constraint != nullptr) {
                auto probeType = candidate;
                Value probe = Value::Uninitialized(probeType);
                probe.type = candidate;
                if (!tp.constraint->assignableFrom(probe)) {
                    return makeVarDefError("type argument for " + tp.name + " does not satisfy extends constraint");
                }
            }
            tp.value = candidate;
            continue;
        }

        if (tp.hasDefault && tp.defaultValue != nullptr) {
            auto candidate = tp.defaultValue;
            if (tp.hasConstraint && tp.constraint != nullptr) {
                auto probeType = candidate;
                Value probe = Value::Uninitialized(probeType);
                probe.type = candidate;
                if (!tp.constraint->assignableFrom(probe)) {
                    return makeVarDefError("default type argument for " + tp.name + " does not satisfy extends constraint");
                }
            }
            tp.value = candidate;
            continue;
        }

        if (strictMissing) {
            return makeVarDefError("missing type argument for " + tp.name + " in " + baseType->getName());
        }
    }

    std::vector<TypeParameter> remaining;
    for (auto& tp : specialized->typeParameters) {
        specialized->typeBindings[tp.name] = tp.value;
        if (tp.value == nullptr) remaining.push_back(tp);
    }
    specialized->typeParameters = move(remaining);

    return Value(specialized);
}

}

uref<VariableDefinition> VariableDefinition::parse(Lexer &lexer) {

    lexer.savePosition();

    auto varDef = create_unique<VariableDefinition>();

    if (lexer.expectToken(INFER)) {
        varDef->inferred = true;
    } else {

        Token nextToken = lexer.nextToken();
        if (nextToken.kind == IDENTIFIER) {
            varDef->type = nextToken;

            if (lexer.expectToken(SMALLER_THAN)) {
                while (true) {
                    Token arg = lexer.nextToken();
                    if (arg.kind != IDENTIFIER) {
                        varDef->lastToken = arg;
                        varDef->expected = {"type argument"};
                        varDef->valid = false;
                        lexer.rollPosition();
                        return varDef;
                    }
                    varDef->typeArguments.push_back(arg);

                    Token sep = lexer.nextToken();
                    if (sep.kind == BIGGER_THAN) break;
                    if (sep.kind != COMMA) {
                        varDef->lastToken = sep;
                        varDef->expected = tokenKindsToString({COMMA, BIGGER_THAN});
                        varDef->valid = false;
                        lexer.rollPosition();
                        return varDef;
                    }
                }
            }
        } else {
            varDef->lastToken = nextToken;
            varDef->expected = {"type name", "class name", "interface name", "enum name", "primitive type"};
            varDef->valid = false;
            lexer.rollPosition();
            return varDef;
        }

    }

    Token nextToken = lexer.nextToken();
    if (nextToken.kind == IDENTIFIER) {
        varDef->name = nextToken;
    } else {
        varDef->lastToken = nextToken;
        varDef->expected = {"variable name"};
        varDef->valid = false;
        lexer.rollPosition();
        return varDef;
    }
    
    if (!varDef->inferred && lexer.expectToken(SEMICOLON)) {
        varDef->valid = true;
        varDef->expression = nullptr;
        lexer.deletePosition();
        return varDef;
    } else if (!lexer.expectToken(EQUALS)) {
        varDef->valid = false;
        varDef->expression = nullptr;
        varDef->expected = tokenKindsToString({EQUALS, SEMICOLON});
        varDef->lastToken = lexer.nextToken();
        lexer.rollPosition();
        return varDef;
    }

    auto parsedExpression = Expression::parse(lexer);

    if (!parsedExpression->valid) {
        varDef->lastToken = parsedExpression->lastToken;
        varDef->expected = parsedExpression->expected;
        varDef->valid = false;
        lexer.rollPosition();
        return varDef;
    }

    varDef->expression = move(parsedExpression);

    if (lexer.expectToken(SEMICOLON)) {
        varDef->valid = true;
        lexer.deletePosition();
        return varDef;
    }
    
    varDef->valid = false;
    varDef->lastToken = lexer.peekToken();
    varDef->expected = tokenKindsToString({SEMICOLON});
    lexer.rollPosition();
    return varDef;

}

Value VariableDefinition::execute(Scope& scope) {

    Value type = scope.getVariable(this->type.value);
    auto name = this->name.value;

    if (!inferred) {
        if (type.type != TypeType) { // wtf am i doing atp
            Value ret;
            ret.thrownException = create_reference<Value>(
                "type value is not a type"
            );
            return ret;
        }
        auto typeType = get<reference<Type>>(type.value);

        if (!typeArguments.empty()) {
            std::vector<reference<Type>> args;
            args.reserve(typeArguments.size());
            for (const auto& argToken : typeArguments) {
                Value argType = scope.getVariable(argToken.value);
                if (argType.thrownException != nullptr) return argType;
                if (argType.type != TypeType) {
                    Value ret;
                    ret.thrownException = create_reference<Value>(
                        "type argument is not a type: " + argToken.value
                    );
                    return ret;
                }
                args.push_back(get<reference<Type>>(argType.value));
            }

            Value specialized = instantiateGenericType(typeType, args, false);
            if (specialized.thrownException != nullptr) return specialized;
            typeType = get<reference<Type>>(specialized.value);
        }

        if (expression == nullptr) {
            auto value = Value::Uninitialized(typeType);
            scope.addVariable(name, value);
            return value;
        }

        auto value = this->expression->execute(scope);
        if (value.thrownException != nullptr) return value;
        if (!typeType->assignableFrom(value)) {
            Value ret;
            ret.thrownException = create_reference<Value>(
                "value is not assignable to type " + typeType->getName()
            );
            return ret;
        }
        scope.addVariable(name, value);
        return value;
    }

    if (this->expression != nullptr) {
        auto value = this->expression->execute(scope);
        scope.addVariable(name, value);
        return value;
    } else {
        
        return NullValue;
    }

}
