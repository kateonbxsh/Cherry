#pragma once

#include "../expression/Expression.h"

struct ForStatement : public Statement {
    
    uref<Statement> init;
    uref<Expression> condition;
    uref<Expression> step;

    uref<Block> body;

    static uref<Statement> parse(Lexer& lexer);
    Value execute(Scope& scope) override;
};

struct WhileStatement : public Statement {
    uref<Expression> condition;
    uref<Block> body;

    static uref<Statement> parse(Lexer&);
    Value execute(Scope&) override;
};

struct DoWhileStatement : public Statement {
    uref<Block> body;
    uref<Expression> condition;

    static uref<Statement> parse(Lexer&);
    Value execute(Scope&) override;
};

struct RepeatUntilStatement : public Statement {
    uref<Block> body;
    uref<Expression> condition;

    static uref<Statement> parse(Lexer&);
    Value execute(Scope&) override;
};

struct RepeatTimesStatement : public Statement {
    uref<Expression> count;
    uref<Block> body;

    static uref<Statement> parse(Lexer&);
    Value execute(Scope&) override;
};