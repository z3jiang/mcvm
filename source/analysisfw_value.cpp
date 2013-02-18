#include "analysisfw_value.h"

#include "constexprs.h"

namespace mcvm { namespace analysis { namespace value {

Runner create(ProgFunction* pf) {

    Runner result(
            pf,
            Runner::Direction::Forward);

    result.assign_ = [] (const ValueInfo& in, const AssignStmt* stmt) {
        ValueInfo gen = in ;
        auto left = stmt->getLeftExprs() ;
        auto right = stmt->getRightExpr() ;

        auto variable = * left.begin() ;
        auto symbol = (SymbolExpr*)variable ;
        
        
        switch (right->getExprType()) {
            case Expression::ExprType::INT_CONST:
                {
                    auto intval = ((IntConstExpr*)right)->getValue() ;
                    gen[symbol] = intval;
                }
                break;

            default:
                assert(false);
        }
        
        return gen ;
    };

    result.merge_ = [] (const ValueInfo& a, const ValueInfo& b) {
        return a ;
    };

    return result;
}

}}}
