#include "TypeDefinition.hpp"
#include "runtime_exception.h"

namespace {

Value makeTypeDefinitionError(const std::string& msg) {
    return makeThrown("TypeException", msg);
}

bool parseTypeArgumentList(Lexer& lexer, std::vector<Token>& outArgs, Statement& st) {
    if (!lexer.expectToken(SMALLER_THAN)) return true;
    while (true) {
        Token arg = lexer.nextToken();
        if (arg.kind != IDENTIFIER) {
            st.valid = false;
            st.expected = {"type argument"};
            st.lastToken = arg;
            return false;
        }
        outArgs.push_back(arg);

        Token sep = lexer.nextToken();
        if (sep.kind == BIGGER_THAN) break;
        if (sep.kind != COMMA) {
            st.valid = false;
            st.expected = tokenKindsToString({COMMA, BIGGER_THAN});
            st.lastToken = sep;
            return false;
        }
    }
    return true;
}

Value instantiateGenericType(reference<Type> baseType, const std::vector<reference<Type>>& args, bool strictMissing) {
    if (baseType == nullptr) return makeTypeDefinitionError("cannot instantiate null type");

    std::vector<size_t> undefinedIndices;
    for (size_t i = 0; i < baseType->typeParameters.size(); ++i) {
        if (baseType->typeParameters[i].value == nullptr) undefinedIndices.push_back(i);
    }
    if (args.size() > undefinedIndices.size()) {
        return makeTypeDefinitionError("too many type arguments for " + baseType->getName());
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
                    return makeTypeDefinitionError("type argument for " + tp.name + " does not satisfy extends constraint");
                }
            }
            tp.value = candidate;
            continue;
        }

        auto& tp = specialized->typeParameters[idx];
        if (tp.hasDefault && tp.defaultValue != nullptr) {
            auto candidate = tp.defaultValue;
            if (tp.hasConstraint && tp.constraint != nullptr) {
                auto probeType = candidate;
                Value probe = Value::Uninitialized(probeType);
                probe.type = candidate;
                if (!tp.constraint->assignableFrom(probe)) {
                    return makeTypeDefinitionError("default type argument for " + tp.name + " does not satisfy extends constraint");
                }
            }
            tp.value = candidate;
            continue;
        }

        if (strictMissing) {
            return makeTypeDefinitionError("missing type argument for " + tp.name + " in " + baseType->getName());
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

bool parseTypeParameters(Lexer& lexer, std::vector<TypeDeclarationParameter>& outParams, Statement& st) {
    if (!lexer.expectToken(SMALLER_THAN)) return true;

    bool seenDefault = false;
    while (true) {
        Token paramName = lexer.nextToken();
        if (paramName.kind != IDENTIFIER) {
            st.valid = false;
            st.expected = {"type parameter"};
            st.lastToken = paramName;
            return false;
        }

        TypeDeclarationParameter tp;
        tp.name = paramName;

        if (lexer.expectToken(EXTENDS)) {
            Token constraint = lexer.nextToken();
            if (constraint.kind != IDENTIFIER) {
                st.valid = false;
                st.expected = {"type constraint"};
                st.lastToken = constraint;
                return false;
            }
            tp.withConstraint = true;
            tp.constraintType = constraint;
        }

        if (lexer.expectToken(EQUALS)) {
            Token def = lexer.nextToken();
            if (def.kind != IDENTIFIER) {
                st.valid = false;
                st.expected = {"type default"};
                st.lastToken = def;
                return false;
            }
            tp.withDefaultValue = true;
            tp.defaultValue = def;
            seenDefault = true;
        } else if (seenDefault) {
            st.valid = false;
            st.expected = {"type parameter with default value"};
            st.lastToken = paramName;
            st.errorMessage = "type parameters with default values must be at the end of the list";
            return false;
        }

        outParams.push_back(tp);
        Token sep = lexer.nextToken();
        if (sep.kind == BIGGER_THAN) break;
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

uref<TypeDefinition> TypeDefinition::parse(Lexer& lexer) {
    lexer.savePosition();
    auto td = create_unique<TypeDefinition>();

    if (!lexer.expectToken(TYPE)) {
        td->valid = false;
        lexer.rollPosition();
        return td;
    }

    Token aliasName = lexer.nextToken();
    if (aliasName.kind != IDENTIFIER) {
        td->valid = false;
        td->expected = {"type name"};
        td->lastToken = aliasName;
        lexer.rollPosition();
        return td;
    }
    td->name = aliasName.value;

    if (!parseTypeParameters(lexer, td->parameters, *td)) {
        lexer.rollPosition();
        return td;
    }

    if (!lexer.expectToken(EQUALS)) {
        td->valid = false;
        td->expected = tokenKindsToString({EQUALS});
        td->lastToken = lexer.nextToken();
        lexer.rollPosition();
        return td;
    }

    // Try dynamic predicate form: <type> <var> when <expr> default <expr>;
    lexer.savePosition();
    Token baseType = lexer.nextToken();
    Token varName = lexer.nextToken();
    if (baseType.kind == IDENTIFIER && varName.kind == IDENTIFIER && lexer.expectToken(WHEN)) {
        auto predicate = Expression::parse(lexer);
        if (!predicate->valid) {
            td->valid = false;
            td->expected = predicate->expected;
            td->lastToken = predicate->lastToken;
            lexer.rollPosition();
            return td;
        }
        if (!lexer.expectToken(DEFAULT)) {
            td->valid = false;
            td->expected = tokenKindsToString({DEFAULT});
            td->lastToken = lexer.nextToken();
            lexer.rollPosition();
            return td;
        }
        auto defExpr = Expression::parse(lexer);
        if (!defExpr->valid) {
            td->valid = false;
            td->expected = defExpr->expected;
            td->lastToken = defExpr->lastToken;
            lexer.rollPosition();
            return td;
        }
        if (!lexer.expectToken(SEMICOLON)) {
            td->valid = false;
            td->expected = tokenKindsToString({SEMICOLON});
            td->lastToken = lexer.nextToken();
            lexer.rollPosition();
            return td;
        }

        td->usesPredicateSyntax = true;
        td->predicateBaseType = baseType;
        td->predicateVariable = varName;
        td->predicateExpression = move(predicate);
        td->defaultExpression = move(defExpr);
        td->valid = true;
        lexer.deletePosition();
        lexer.deletePosition();
        return td;
    }
    lexer.rollPosition();

    while (true) {
        Token t = lexer.nextToken();
        if (t.kind == IDENTIFIER || t.kind == STRING || t.kind == INTEGER || t.kind == TRUE || t.kind == FALSE || t.kind == NULL_TOKEN) {
            TypeUnionTerm term;
            term.token = t;
            term.literal = (t.kind != IDENTIFIER);
            if (!term.literal) {
                if (!parseTypeArgumentList(lexer, term.typeArguments, *td)) {
                    lexer.rollPosition();
                    return td;
                }
            }
            td->unionTerms.push_back(term);
        } else {
            td->valid = false;
            td->expected = {"type identifier or literal"};
            td->lastToken = t;
            lexer.rollPosition();
            return td;
        }

        Token sep = lexer.nextToken();
        if (sep.kind == SEMICOLON) break;
        if (sep.kind != BITWISE_OR) {
            td->valid = false;
            td->expected = tokenKindsToString({BITWISE_OR, SEMICOLON});
            td->lastToken = sep;
            lexer.rollPosition();
            return td;
        }
    }

    td->valid = true;
    lexer.deletePosition();
    return td;
}

Value TypeDefinition::execute(Scope& scope) {
    auto typeRef = create_reference<Type>(TypeKind::Dynamic);
    typeRef->setName(name);

    Scope typeScope = scope.createChild();
    typeScope.addVariable(name, Value(typeRef));

    for (const auto& p : parameters) {
        TypeParameter tp;
        tp.name = p.name.value;
        tp.hasDefault = p.withDefaultValue;
        tp.hasConstraint = p.withConstraint;

        if (p.withConstraint) {
            Value c = typeScope.getVariable(p.constraintType.value);
            if (c.thrownException != nullptr) return c;
            if (c.type != TypeType) return makeTypeDefinitionError("type constraint is not a type");
            tp.constraint = get<reference<Type>>(c.value);
        }

        if (p.withDefaultValue) {
            Value d = typeScope.getVariable(p.defaultValue.value);
            if (d.thrownException != nullptr) return d;
            if (d.type != TypeType) return makeTypeDefinitionError("type parameter default is not a type");
            tp.defaultValue = get<reference<Type>>(d.value);
        }

        typeRef->typeParameters.push_back(tp);
        typeRef->typeBindings[tp.name] = nullptr;

        auto placeholder = create_reference<Type>(TypeKind::Dynamic);
        placeholder->setName(tp.name);
        if (tp.hasDefault && tp.defaultValue != nullptr) typeScope.addVariable(tp.name, Value(tp.defaultValue));
        else typeScope.addVariable(tp.name, Value(placeholder));
    }

    if (usesPredicateSyntax) {
        Value base = typeScope.getVariable(predicateBaseType.value);
        if (base.thrownException != nullptr) return base;
        if (base.type != TypeType) return makeTypeDefinitionError("dynamic type base is not a type");
        typeRef->dynamicBaseType = get<reference<Type>>(base.value);
        typeRef->dynamicVariableName = predicateVariable.value;
        typeRef->dynamicPredicate = predicateExpression;

        Value unboundDefault = defaultExpression->execute(typeScope);
        if (unboundDefault.thrownException != nullptr) return unboundDefault;
        typeRef->setDefaultValue(unboundDefault);
    } else {
        for (auto& term : unionTerms) {
            if (!term.literal) {
                Value t = typeScope.getVariable(term.token.value);
                if (t.thrownException != nullptr) return t;
                if (t.type != TypeType) return makeTypeDefinitionError("union member is not a type: " + term.token.value);
                auto baseType = get<reference<Type>>(t.value);
                if (term.typeArguments.empty()) {
                    typeRef->dynamicUnionTypes.push_back(baseType);
                } else {
                    std::vector<reference<Type>> args;
                    args.reserve(term.typeArguments.size());
                    for (const auto& argTok : term.typeArguments) {
                        Value argTypeValue = typeScope.getVariable(argTok.value);
                        if (argTypeValue.thrownException != nullptr) return argTypeValue;
                        if (argTypeValue.type != TypeType) {
                            return makeTypeDefinitionError("type argument is not a type: " + argTok.value);
                        }
                        args.push_back(get<reference<Type>>(argTypeValue.value));
                    }
                    Value specialized = instantiateGenericType(baseType, args, false);
                    if (specialized.thrownException != nullptr) return specialized;
                    typeRef->dynamicUnionTypes.push_back(get<reference<Type>>(specialized.value));
                }
                continue;
            }

            if (term.token.kind == STRING) typeRef->dynamicUnionLiterals.push_back(create_reference<Value>(Value(term.token.value)));
            else if (term.token.kind == INTEGER) typeRef->dynamicUnionLiterals.push_back(create_reference<Value>(Value((integer)std::stol(term.token.value))));
            else if (term.token.kind == TRUE) typeRef->dynamicUnionLiterals.push_back(create_reference<Value>(Value((boolean)true)));
            else if (term.token.kind == FALSE) typeRef->dynamicUnionLiterals.push_back(create_reference<Value>(Value((boolean)false)));
            else typeRef->dynamicUnionLiterals.push_back(create_reference<Value>(NullValue));
        }
    }

    scope.addVariable(name, Value(typeRef));
    return Value(typeRef);
}
