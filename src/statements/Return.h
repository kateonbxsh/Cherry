#include "../statement.h"
#include "../type.h"
#include "../lexer.h"

class Return : public Statement {

public:
    Return(): Statement(RETURN_STATEMENT) {};

    TokenList expression;
};
