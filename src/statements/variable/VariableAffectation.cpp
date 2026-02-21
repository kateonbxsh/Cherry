#include "VariableAffectation.h"
#include "statements/expression/Expression.h"
#include "types/type.h"
#include <expressions.h>

static Value makeAssignmentError(const std::string& message) {
    Value err;
    err.thrownException = create_reference<Value>(Value(message));
    return err;
}

static Value readMember(const Value& owner, const std::string& name) {
    if (owner.type == TypeType) {
        auto typeRef = get<reference<Type>>(owner.value);
        if (typeRef->staticFieldValues.contains(name)) {
            return *typeRef->staticFieldValues[name];
        }
        return makeAssignmentError("unknown static member: " + name);
    }

    if (owner.type != nullptr && owner.type->kind == TypeKind::Class) {
        auto instance = get<ClassInstance>(owner.value);
        if (instance.fieldValues.contains(name)) {
            return instance.fieldValues[name];
        }
        return makeAssignmentError("unknown instance member: " + name);
    }

    return makeAssignmentError("dot access requires class type or instance");
}

static Value resolvePathValue(Scope& scope, const std::vector<Token>& path) {
    if (path.empty()) return makeAssignmentError("empty assignment path");

    Value current = scope.getVariable(path.front().value);
    if (current.thrownException != nullptr) return current;

    for (size_t i = 1; i < path.size(); ++i) {
        current = readMember(current, path[i].value);
        if (current.thrownException != nullptr) return current;
    }

    return current;
}

static Value setMember(Value& owner, const std::string& name, const Value& newValue) {
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

    if (owner.type != nullptr && owner.type->kind == TypeKind::Class) {
        auto instance = get<ClassInstance>(owner.value);
        instance.fieldValues[name] = newValue;
        owner.value = instance;
        return newValue;
    }

    return makeAssignmentError("dot assignment requires class type or instance");
}

uref<VariableAffectation> VariableAffectation::parse(Lexer &lexer) {

    lexer.savePosition();

    auto varAff = create_unique<VariableAffectation>();

    Token next = lexer.nextToken();
    if (next.kind == IDENTIFIER || next.kind == THIS) {
        varAff->path.push_back(next);
    } else {
        varAff->valid = false;
        varAff->expected = {"variable name"};
        varAff->lastToken = next;
        lexer.rollPosition();
        return varAff;
    }

    while (lexer.expectToken(DOT)) {
        Token member = lexer.nextToken();
        if (member.kind != IDENTIFIER) {
            varAff->valid = false;
            varAff->expected = {"member name"};
            varAff->lastToken = member;
            lexer.rollPosition();
            return varAff;
        }
        varAff->path.push_back(member);
    }

    varAff->selfOperation = NONE;

    next = lexer.nextToken();
    if (isBinaryOperator(next.kind)) {
        varAff->selfOperation = next.kind;
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

    auto potentialExpr = Expression::parse(lexer);
    
    if (!potentialExpr->valid) {
        varAff->valid = false;
        varAff->expected = potentialExpr->expected;
        varAff->lastToken = potentialExpr->lastToken;
        lexer.rollPosition();
        return varAff;
    }

    varAff->expression = move(potentialExpr);

    if (!lexer.expectToken(SEMICOLON)) {
        varAff->valid = false;
        varAff->expected = tokenKindsToString({SEMICOLON});
        varAff->lastToken = lexer.nextToken();
        lexer.rollPosition();
        return varAff;
    }

    varAff->valid = true;
    return varAff;

}

Value VariableAffectation::execute(Scope& scope) {

    if (path.empty()) return NullValue;

    if (!scope.hasVariable(path.front().value)) return NullValue;

    Value value = expression->execute(scope);
    if (value.thrownException != nullptr) return value;

    if (selfOperation != NONE) {
        Value current = resolvePathValue(scope, path);
        if (current.thrownException != nullptr) return current;
        value = performBinaryOperator(current, value, selfOperation);
        if (value.thrownException != nullptr) return value;
    }

    if (path.size() == 1) {
        scope.setVariable(path.front().value, value);
        return value;
    }

    // Build owner/value chain for nested assignment:
    // path: a.b.c = v  -> nodes: [a, a.b]
    std::vector<Value> nodes;
    nodes.reserve(path.size());

    Value root = scope.getVariable(path.front().value);
    if (root.thrownException != nullptr) return root;
    nodes.push_back(root);

    for (size_t i = 1; i + 1 < path.size(); ++i) {
        Value next = readMember(nodes.back(), path[i].value);
        if (next.thrownException != nullptr) return next;
        nodes.push_back(next);
    }

    // Assign leaf on the deepest owner
    Value writeResult = setMember(nodes.back(), path.back().value, value);
    if (writeResult.thrownException != nullptr) return writeResult;

    // Propagate updated nested objects back to root
    for (size_t i = nodes.size(); i > 1; --i) {
        Value child = nodes[i - 1];
        Value& parent = nodes[i - 2];
        Value linkResult = setMember(parent, path[i - 1].value, child);
        if (linkResult.thrownException != nullptr) return linkResult;
    }

    scope.setVariable(path.front().value, nodes.front());
    return value;

}
