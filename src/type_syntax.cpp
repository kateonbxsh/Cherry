#include "type_syntax.h"
#include "runtime_exception.h"

namespace {

std::string typeSyntaxDisplayName(const TypeSyntaxExpression& syntax) {
    if (syntax.isUnion) {
        std::string out;
        for (size_t i = 0; i < syntax.unionMembers.size(); ++i) {
            if (i > 0) out += " | ";
            out += typeSyntaxDisplayName(syntax.unionMembers[i]);
        }
        return out.empty() ? std::string("<type union>") : out;
    }

    std::string out = syntax.name.value;
    if (!syntax.arguments.empty()) {
        out += "<";
        for (size_t i = 0; i < syntax.arguments.size(); ++i) {
            if (i > 0) out += ", ";
            out += typeSyntaxDisplayName(syntax.arguments[i]);
        }
        out += ">";
    }
    return out.empty() ? std::string("<type>") : out;
}

bool parseTypeSyntaxPrimary(Lexer& lexer, TypeSyntaxExpression& out, Statement& st) {
    Token base = lexer.nextToken();
    if (base.kind != IDENTIFIER && base.kind != TYPE) {
        st.valid = false;
        st.expected = {"type identifier"};
        st.lastToken = base;
        return false;
    }
    out.isUnion = false;
    out.unionMembers.clear();
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

}

bool parseTypeSyntaxExpression(Lexer& lexer, TypeSyntaxExpression& out, Statement& st) {
    TypeSyntaxExpression first;
    if (!parseTypeSyntaxPrimary(lexer, first, st)) {
        return false;
    }

    std::vector<TypeSyntaxExpression> members;
    members.push_back(std::move(first));
    while (lexer.expectToken(BITWISE_OR)) {
        TypeSyntaxExpression next;
        if (!parseTypeSyntaxPrimary(lexer, next, st)) {
            return false;
        }
        members.push_back(std::move(next));
    }

    if (members.size() == 1) {
        out = std::move(members.front());
        return true;
    }

    out = TypeSyntaxExpression{};
    out.isUnion = true;
    out.unionMembers = std::move(members);
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
    if (syntax.isUnion) {
        auto unionType = create_reference<Type>(Type(TypeKind::Dynamic));
        std::vector<reference<Type>> members;
        members.reserve(syntax.unionMembers.size());
        std::string unionName;

        for (const auto& memberSyntax : syntax.unionMembers) {
            Value member = resolveTypeSyntaxExpression(scope, memberSyntax, strictMissing, errorPrefix);
            if (member.thrownException != nullptr) return member;
            if (member.type != TypeType) {
                return makeThrown("TypeException", errorPrefix + "union member is not a type: " + typeSyntaxDisplayName(memberSyntax));
            }
            auto memberType = get<reference<Type>>(member.value);
            if (memberType == nullptr) {
                return makeThrown("TypeException", errorPrefix + "union member is null type: " + typeSyntaxDisplayName(memberSyntax));
            }
            members.push_back(memberType);
            if (!unionName.empty()) unionName += " | ";
            unionName += memberType->getName();
        }

        unionType->dynamicUnionTypes = std::move(members);
        unionType->setName(unionName);
        return Value(unionType);
    }

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
            return makeThrown("TypeException", errorPrefix + "type argument is not a type: " + typeSyntaxDisplayName(argExpr));
        }
        args.push_back(get<reference<Type>>(argType.value));
    }

    return instantiateGenericTypeSyntax(baseType, args, strictMissing, errorPrefix);
}

