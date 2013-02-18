#include "analysis_framework.h"


class SymbolExpr;

namespace mcvm { namespace analysis { namespace value {

typedef std::unordered_map<const SymbolExpr*,int> ValueInfo ;

using Runner = DataFlowAnalysisRunner<ValueInfo> ;

Runner create(ProgFunction* pf);

}}}
