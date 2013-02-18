// =========================================================================== //
//                                                                             //
// Copyright 2013 Matthieu Dubet and McGill University.                        //
//                                                                             //
//   Licensed under the Apache License, Version 2.0 (the "License");           //
//   you may not use this file except in compliance with the License.          //
//   You may obtain a copy of the License at                                   //
//                                                                             //
//       http://www.apache.org/licenses/LICENSE-2.0                            //
//                                                                             // 
//   Unless required by applicable law or agreed to in writing, software       //
//   distributed under the License is distributed on an "AS IS" BASIS,         //
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  //
//   See the License for the specific language governing permissions and       //
//  limitations under the License.                                             //
//                                                                             //
// =========================================================================== //
#ifndef MCVM_ANALYSIS_FRAMEWORK_H
#define MCVM_ANALYSIS_FRAMEWORK_H

#include "functions.h"
#include "assignstmt.h"
#include "ifelsestmt.h"

namespace mcvm { namespace analysis {
    
template <typename Info>
class DataFlowAnalysisRunner
{
    using InfoMap = std::unordered_map<Statement*,Info> ;
    
    public:
    enum class Direction {
        Forward,
        Backward
    };

    DataFlowAnalysisRunner() = delete ;

    DataFlowAnalysisRunner(
            ProgFunction* pf,
            Direction d)
        :
            void_([](const Info& in, const Statement*){return in ;}),
            assign_(void_),
            function_to_analyze_(pf),
            direction_(d)
    {
    };

    void run() ;
    Info run(const Info&,StmtSequence*) ; //run on a statement sequence
    InfoMap get_result() { return data_ ; }  ;
    
    // The merging function
    std::function<Info(
            const Info&,
            const Info&
            )> merge_;

    // The type of a transfer function
    template <typename Stmt> 
        using TransferFn = std::function<Info (const Info&,Stmt)> ;

    // The void function (doesn't do anything)
    TransferFn<const Statement*> void_ ;

    // For each type of statement we have an std::function
    TransferFn<const AssignStmt*> assign_ ;

    private:

    const ProgFunction *function_to_analyze_;

    Direction direction_;
    //
    // The dataflow result
    InfoMap data_;

};


template <typename Info>
void DataFlowAnalysisRunner<Info>::run() {
    Info initial ;
    // Perform type inference on the function body
    run(
            initial,
            function_to_analyze_->getCurrentBody()
    );
}

template <typename Info>
Info DataFlowAnalysisRunner<Info>::run(
        const Info& in,
        StmtSequence* sequence
        )
{
    Info current ;
    for (auto st: sequence->getStatements() ) {
        switch (st->getStmtType()) {
            case Statement::ASSIGN:
                current = assign_(current,static_cast<AssignStmt*>(st) ) ;
                break;

            case Statement::IF_ELSE:
                {
                    auto stmt_if = static_cast<IfElseStmt*>(st)->getIfBlock() ;
                    auto stmt_else = static_cast<IfElseStmt*>(st)->getElseBlock() ;
                    auto info_if = 
                        run(current,stmt_if) ;
                    auto info_else = 
                        run(current,stmt_else) ;
                    current = merge_(info_if,info_else) ;
                }
                break;
            case Statement::SWITCH:
            case Statement::FOR:
            case Statement::WHILE:
            case Statement::LOOP:
            case Statement::COMPOUND_END:
            case Statement::BREAK:
            case Statement::CONTINUE:
            case Statement::RETURN:
            case Statement::EXPR:
                std::cout << "Error : " << st->toString() << std::endl ;;
                assert(false);
        }
        data_[st] = current ;
    }
    return current ;
}


}}

#endif //include guard

