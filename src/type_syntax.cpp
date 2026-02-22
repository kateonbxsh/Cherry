#include "type_syntax.h"
#include "runtime_exception.h"

bool parseTypeSyntaxExpression(Lexer& lexer, TypeSyntaxExpression& out, Statement& st) {
    Token base = lexer.nextToken();
    if (base.kind != IDENTIFIER) {
        st.valid = false;
        st.expected = {"type identifier"};
        st.lastToken = base;
        return false;
    }
    out.name = base;
    out.arguments.clear();

    if (!lexer.expectToken(SMALLER_THAN)) {
        return true;
    }

    while (true) {
        TypeSyntaxExpression arg;
        if (!parseTypeSyntaxExpression(lexer, arg, st)) {
            return false;
        }
        out.arguments.push_back(std::move(arg));

        Token sep = lexer.nextToken();
        if (sep.kind == BIGGER_THAN) {
            break;
        }
        if (sep.kind != COMMA) {
            st.valid = false;
            st.expected = tokenKindsToString({COMMA, BIGGER_THAN});
            st.lastToken = sep;
            return false;
        }
    }

    return true;
}

Value instantiateGenericTypeSyntax(
    reference<Type> baseType,
    const std::vector<reference<Type>>& args,
    bool strictMissing,
    const std::string& errorPrefix
) {
    if (baseType == nullptr) {
        return makeThrown("TypeException", errorPrefix + "cannot instantiate null type");
    }

    std::vector<size_t> undefinedIndices;
    for (size_t i = 0; i < baseType->typeParameters.size(); ++i) {
        if (baseType->typeParameters[i].value == nullptr) undefinedIndices.push_back(i);
    }

    if (args.size() > undefinedIndices.size()) {
        return makeThrown("TypeException", errorPrefix + "too many type arguments for " + baseType->getName());
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
                    return makeThrown("TypeException", errorPrefix + "type argument for " + tp.name + " does not satisfy extends constraint");
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
                    return makeThrown("TypeException", errorPrefix + "default type argument for " + tp.name + " does not satisfy extends constraint");
                }
            }
            tp.value = candidate;
            continue;
        }

        if (strictMissing) {
            return makeThrown("TypeException", errorPrefix + "missing type argument for " + tp.name + " in " + baseType->getName());
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

Value resolveTypeSyntaxExpression(
    Scope& scope,
    const TypeSyntaxExpression& syntax,
    bool strictMissing,
    const std::string& errorPrefix
) {
    Value base = scope.getVariable(syntax.name.value);
    if (base.thrownException != nullptr) return base;
    if (base.type != TypeType) {
        return makeThrown("TypeException", errorPrefix + "type is not a type: " + syntax.name.value);
    }

    auto baseType = get<reference<Type>>(base.value);
    if (syntax.arguments.empty()) {
        return Value(baseType);
    }

    std::vector<reference<Type>> args;
    args.reserve(syntax.arguments.size());
    for (const auto& argExpr : syntax.arguments) {
        Value argType = resolveTypeSyntaxExpression(scope, argExpr, strictMissing, errorPrefix);
        if (argType.thrownException != nullptr) return argType;
        if (argType.type != TypeType) {
            return makeThrown("TypeException", errorPrefix + "type argument is not a type: " + argExpr.name.value);
        }
        args.push_back(get<reference<Type>>(argType.value));
    }

    return instantiateGenericTypeSyntax(baseType, args, strictMissing, errorPrefix);
}

