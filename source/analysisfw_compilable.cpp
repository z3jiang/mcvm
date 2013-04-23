/*! \file New value analysis */ 
#include "analysis_framework.h"
#include "typeinfer.h"
#include "analysis_typeinfer.h"

class SymbolExpr;

namespace mcvm { namespace analysis { namespace compilable {

typedef std::unordered_map<const SymbolExpr*,TypeInfoPtr> TypeCompilable ;

using Runner = DataFlowAnalysisRunner<TypeCompilable> ;

/**
 * \fn getf 
 * dfgdfg
 *
 * dfgdfgdg
 * dfg
 */
Runner create(ProgFunction* pf, TypeInferInfo typeinfered) {

    Runner result(
            pf,
            Runner::Direction::Backward);

    /*! \fn sfdsfsd
     * sdfsf
     *
     * sdfsdf
     *
     *
     *
     */
    result.assign_ = [] (const TypeCompilable& in, const AssignStmt* stmt) {
        TypeCompilable ret ;
        //std::cout << stmt->toString() ;
        return ret;
    };


    /**   \fn Merge function
     *  \brief sfdfsdfsdf
     *
     * sdfsdfsdfsdf
     * sdfsdfsdfsdf
     * sdfsdfsdfsdf
     */
    result.merge_ = [] (const TypeCompilable& a, const TypeCompilable& b) {
        return a ;
    };

    return result;
}

}}}
