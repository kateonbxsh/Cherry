#include "TryCatchFinallyStatement.hpp"
#include "runtime_exception.h"

namespace {

bool parseCatchClause(Lexer& lexer, CatchClause& out, Statement& st) {
    if (!lexer.expectToken(CATCH)) return false;

    if (!lexer.expectToken(LEFT_PARENTHESIS)) {
        st.valid = false;
        st.expected = tokenKindsToString({LEFT_PARENTHESIS});
        st.lastToken = lexer.nextToken();
        return true;
    }

    Token firstTypeToken = lexer.nextToken();
    if (firstTypeToken.kind != IDENTIFIER) {
        st.valid = false;
        st.expected = {"exception type"};
        st.lastToken = firstTypeToken;
        return true;
    }
    out.typeNames.push_back(firstTypeToken);

    while (lexer.expectToken(BITWISE_OR)) {
        Token nextTypeToken = lexer.nextToken();
        if (nextTypeToken.kind != IDENTIFIER) {
            st.valid = false;
            st.expected = {"exception type"};
            st.lastToken = nextTypeToken;
            return true;
        }
        out.typeNames.push_back(nextTypeToken);
    }

    Token maybeName = lexer.nextToken();
    if (maybeName.kind == IDENTIFIER) {
        out.variableName = maybeName;
    } else {
        lexer.back();
    }

    if (!lexer.expectToken(RIGHT_PARENTHESIS)) {
        st.valid = false;
        st.expected = tokenKindsToString({RIGHT_PARENTHESIS});
        st.lastToken = lexer.nextToken();
        return true;
    }

    auto body = Block::parse(lexer);
    if (!body->valid) {
        st.valid = false;
        st.expected = body->expected;
        st.lastToken = body->lastToken;
        return true;
    }
    out.body = move(body);
    return true;
}

}

uref<TryCatchFinallyStatement> TryCatchFinallyStatement::parse(Lexer& lexer) {
    lexer.savePosition();
    auto st = create_unique<TryCatchFinallyStatement>();

    Token t = lexer.nextToken();
    if (t.kind != TRY) {
        st->valid = false;
        st->expected = tokenKindsToString({TRY});
        st->lastToken = t;
        lexer.rollPosition();
        return st;
    }
    st->tryToken = t;

    auto tryBody = Block::parse(lexer);
    if (!tryBody->valid) {
        st->valid = false;
        st->expected = tryBody->expected;
        st->lastToken = tryBody->lastToken;
        lexer.rollPosition();
        return st;
    }
    st->tryBlock = move(tryBody);

    while (true) {
        CatchClause cc;
        size_t before = st->catches.size();
        if (!parseCatchClause(lexer, cc, *st)) break;
        if (!st->valid) {
            lexer.rollPosition();
            return st;
        }
        if (st->catches.size() == before) st->catches.push_back(move(cc));
    }

    if (lexer.expectToken(FINALLY)) {
        auto finallyBody = Block::parse(lexer);
        if (!finallyBody->valid) {
            st->valid = false;
            st->expected = finallyBody->expected;
            st->lastToken = finallyBody->lastToken;
            lexer.rollPosition();
            return st;
        }
        st->finallyBlock = move(finallyBody);
    }

    if (st->catches.empty() && st->finallyBlock == nullptr) {
        st->valid = false;
        st->expected = {"catch", "finally"};
        st->lastToken = lexer.peekToken();
        lexer.rollPosition();
        return st;
    }

    for (size_t i = 0; i < st->catches.size(); ++i) {
        for (size_t j = i + 1; j < st->catches.size(); ++j) {
            for (const auto& leftType : st->catches[i].typeNames) {
                for (const auto& rightType : st->catches[j].typeNames) {
                    if (leftType.value != rightType.value) continue;
                    st->valid = false;
                    st->lastToken = rightType;
                    st->expected = {"non-duplicate catch type"};
                    st->errorMessage = "duplicate compatible catch blocks for type " + rightType.value;
                    lexer.rollPosition();
                    return st;
                }
            }
        }
    }

    st->valid = true;
    lexer.deletePosition();
    return st;
}

Value TryCatchFinallyStatement::execute(Scope& scope) {
    std::vector<std::vector<reference<Type>>> catchTypes;
    catchTypes.reserve(catches.size());
    for (const auto& c : catches) {
        std::vector<reference<Type>> clauseTypes;
        clauseTypes.reserve(c.typeNames.size());
        for (const auto& typeName : c.typeNames) {
            Value tv = scope.getVariable(typeName.value);
            if (tv.thrownException != nullptr) return tv;
            if (tv.type != TypeType) {
                return makeThrown("TypeException", "catch type is not a type: " + typeName.value, typeName.line, typeName.pos + 1);
            }
            clauseTypes.push_back(get<reference<Type>>(tv.value));
        }
        catchTypes.push_back(std::move(clauseTypes));
    }

    auto typeCoversType = [](const reference<Type>& covering, const reference<Type>& target) -> bool {
        auto targetTypeCopy = target;
        auto targetProbe = Value::Uninitialized(targetTypeCopy);
        targetProbe.type = target;
        return covering->assignableFrom(targetProbe);
    };

    auto clauseCoversClause = [&typeCoversType](const std::vector<reference<Type>>& a, const std::vector<reference<Type>>& b) -> bool {
        for (const auto& bType : b) {
            bool covered = false;
            for (const auto& aType : a) {
                if (typeCoversType(aType, bType)) {
                    covered = true;
                    break;
                }
            }
            if (!covered) return false;
        }
        return true;
    };

    for (size_t i = 0; i < catchTypes.size(); ++i) {
        for (size_t j = i + 1; j < catchTypes.size(); ++j) {
            // Reject only unreachable ordering: an earlier clause fully covering a later one.
            if (clauseCoversClause(catchTypes[i], catchTypes[j])) {
                auto offending = catches[j].typeNames.empty() ? catches[j].variableName : catches[j].typeNames.front();
                return makeThrown(
                    "TypeException",
                    "unreachable catch block: catch clause is already covered by previous catch clause",
                    offending.line,
                    offending.pos + 1
                );
            }
        }
    }

    Value result = tryBlock->execute(scope);
    if (result.thrownException != nullptr) {
        auto ex = normalizeExceptionRef(result.thrownException);
        bool handled = false;
        for (size_t i = 0; i < catches.size(); ++i) {
            bool matches = false;
            for (const auto& catchType : catchTypes[i]) {
                if (catchType->assignableFrom(*ex)) {
                    matches = true;
                    break;
                }
            }
            if (!matches) continue;
            reference<Scope> parentRef(&scope, [](Scope*) {});
            Scope catchScope(parentRef);
            if (catches[i].variableName.kind == IDENTIFIER) {
                catchScope.addVariable(catches[i].variableName.value, *ex);
            }
            result = catches[i].body->execute(catchScope);
            handled = true;
            break;
        }
        if (!handled) {
            result.thrownException = ex;
        }
    }

    if (finallyBlock != nullptr) {
        Value f = finallyBlock->execute(scope);
        if (f.thrownException != nullptr) return f;
        if (f.returning) return f;
    }
    return result;
}
