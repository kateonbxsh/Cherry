
#include "lexer.h"
#include "statement.h"

class GlobalBlock : public Statement {

public:

    static GlobalBlock* parse(Lexer& lexer);
    void execute() override;

private:

    std::vector<Statement*> children;

};