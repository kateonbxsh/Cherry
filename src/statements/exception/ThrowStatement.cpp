#include "ThrowStatement.hpp"
#include "runtime_exception.h"
#include "expressions.h"

uref<ThrowStatement> ThrowStatement::parse(Lexer& lexer) {
    lexer.savePosition();
    auto st = create_unique<ThrowStatement>();

    Token t = lexer.nextToken();
    if (t.kind != THROW) {
        st->valid = false;
        st->expected = tokenKindsToString({THROW});
        st->lastToken = t;
        lexer.rollPosition();
        return st;
    }
    st->throwToken = t;

    auto expr = Expression::parse(lexer);
    if (!expr->valid) {
        st->valid = false;
        st->expected = expr->expected;
        st->lastToken = expr->lastToken;
        lexer.rollPosition();
        return st;
    }
    st->expression = move(expr);

    if (!lexer.expectToken(SEMICOLON)) {
        st->valid = false;
        st->expected = tokenKindsToString({SEMICOLON});
        st->lastToken = lexer.nextToken();
        lexer.rollPosition();
        return st;
    }

    st->valid = true;
    lexer.deletePosition();
    return st;
}

Value ThrowStatement::execute(Scope& scope) {
    Value v = expression->execute(scope);
    if (v.thrownException != nullptr) return v;

    Value out;
    if (isExceptionInstance(v)) {
        out.thrownException = create_reference<Value>(v);
        return out;
    }

    if (v.type == StringType) {
        out.thrownException = makeExceptionRef(
            "Exception",
            get<string>(v.value),
            throwToken.line,
            throwToken.pos + 1
        );
        return out;
    }

    out.thrownException = makeExceptionRef(
        "ValueException",
        stringify(v),
        throwToken.line,
        throwToken.pos + 1
    );
    return out;
}
