#include "statement.h"
#include "lexer.h"

NotAStatement::NotAStatement(Token lastToken, std::vector<TokenKind> expected) {

    this->valid = false;
    this->lastToken = lastToken;
    this->expected = expected;

}
