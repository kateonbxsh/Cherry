#include "executor.h"
#include "statements/VariableDefinition.h"
#include "statements/VariableAffectation.h"
#include "statements/Return.h"
#include "statements/IfStatement.h"
#include "expression.h"
#include "statements/FunctionCall.h"

Value Executor::execute(Block& block) {

    Scope scope;
    Value result = executeInternal(block, scope);
    return result;

}

Value Executor::executeInternal(Block block, Scope& scope) {

    std::vector<Statement*> statements = block.statements;

    for(Statement* ptrStatement : statements) {

        switch(ptrStatement->getType()) {

            case NOT_A_STATEMENT:
                continue;
            case FUNCTION_CALL: {
                auto fCall = dynamic_cast<FunctionCall *>(ptrStatement);
                if (fCall == nullptr) continue;
                evaluateExpression(fCall->expression, scope);
                break;
            }
            case VARIABLE_DEFINITION: {
                auto pDefinition = dynamic_cast<VariableDefinition *>(ptrStatement);
                if (pDefinition == nullptr) continue;
                Value result = evaluateExpression(pDefinition->expression, scope);
                scope.setVariable(pDefinition->name, result);
                break;
            }
            case VARIABLE_AFFECTATION: {
                auto pAffectation = dynamic_cast<VariableAffectation *>(ptrStatement);
                if (pAffectation == nullptr) continue;
                Value result = evaluateExpression(pAffectation->expression, scope);
                scope.setVariable(pAffectation->name, result);
                break;
            }
            case IF_STATEMENT: {
                auto pIfStatement = dynamic_cast<IfStatement *>(ptrStatement);
                if (pIfStatement == nullptr) continue;
                for(IfStatement::SubIfStatement* sub : pIfStatement->subStatements) {
                    if (sub->isElse && !sub->elseHasExpression) {
                        executeInternal(sub->block, scope);
                        break;
                    }
                    Value result = evaluateExpression(sub->expression, scope);
                    if (isTruthy(result) ^ sub->isUnless) {
                        executeInternal(sub->block, scope);
                        break;
                    }
                }
                continue;
            }
            case RETURN_STATEMENT: {
                auto pReturn = dynamic_cast<Return*>(ptrStatement);
                if (pReturn == nullptr) continue;
                Value result = evaluateExpression(pReturn->expression, scope);
                return result;
            }
        }

    }

    return {"null", 0};

}
