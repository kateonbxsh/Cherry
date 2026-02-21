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

    Token typeToken = lexer.nextToken();
    if (typeToken.kind != IDENTIFIER) {
        st.valid = false;
        st.expected = {"exception type"};
        st.lastToken = typeToken;
        return true;
    }
    out.typeName = typeToken;

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
            if (st->catches[i].typeName.value == st->catches[j].typeName.value) {
                st->valid = false;
                st->lastToken = st->catches[j].typeName;
                st->expected = {"non-duplicate catch type"};
                st->errorMessage = "duplicate compatible catch blocks for type " + st->catches[j].typeName.value;
                lexer.rollPosition();
                return st;
            }
        }
    }

    st->valid = true;
    lexer.deletePosition();
    return st;
}

Value TryCatchFinallyStatement::execute(Scope& scope) {
    std::vector<reference<Type>> catchTypes;
    catchTypes.reserve(catches.size());
    for (const auto& c : catches) {
        Value tv = scope.getVariable(c.typeName.value);
        if (tv.thrownException != nullptr) return tv;
        if (tv.type != TypeType) {
            return makeThrown("TypeException", "catch type is not a type: " + c.typeName.value, c.typeName.line, c.typeName.pos + 1);
        }
        catchTypes.push_back(get<reference<Type>>(tv.value));
    }

    for (size_t i = 0; i < catchTypes.size(); ++i) {
        for (size_t j = i + 1; j < catchTypes.size(); ++j) {
            auto aType = catchTypes[i];
            auto bType = catchTypes[j];
            auto aProbe = Value::Uninitialized(aType);
            auto bProbe = Value::Uninitialized(bType);
            aProbe.type = aType;
            bProbe.type = bType;
            if (aType->assignableFrom(bProbe) || bType->assignableFrom(aProbe)) {
                return makeThrown(
                    "TypeException",
                    "compatible catch blocks detected: " + catches[i].typeName.value + " and " + catches[j].typeName.value,
                    catches[j].typeName.line,
                    catches[j].typeName.pos + 1
                );
            }
        }
    }

    Value result = tryBlock->execute(scope);
    if (result.thrownException != nullptr) {
        auto ex = normalizeExceptionRef(result.thrownException);
        bool handled = false;
        for (size_t i = 0; i < catches.size(); ++i) {
            if (!catchTypes[i]->assignableFrom(*ex)) continue;
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
