// =========================================================================== //
//                                                                             //
// Copyright 2009 Maxime Chevalier-Boisvert and McGill University.             //
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

// Include guards
#ifndef ANALYSIS_LIVEVARS_H_
#define ANALYSIS_LIVEVARS_H_

// Header files
#include <map>
#include <unordered_map>
#include "analysismanager.h"
#include "expressions.h"

class SymbolExpr;
class LoopStmt;
class IfElseStmt;
class StmtSequence;
class IIRNode;

// Live variable map type definition
class LiveVarMap : public std::unordered_map<const IIRNode*, Expression::SymbolSet> {} ;

/***************************************************************
* Class   : LiveVarInfo
* Purpose : Store live variable analysis information
* Initial : Maxime Chevalier-Boisvert on May 4, 2009
****************************************************************
Revisions and bug fixes:
*/
class LiveVarInfo : public AnalysisInfo
{
public:
	
	// Live variable info map
	LiveVarMap liveVarMap;
	
	// Set of life variables at the entry point
	Expression::SymbolSet entryLiveSet;
};

// Function to get the live variables for a function body
AnalysisInfo* computeLiveVars(
	const ProgFunction* pFunction,
	const StmtSequence* pFuncBody,
	const TypeSetString& inArgTypes,
	bool returnBottom
);

// Function to get the live variable sets for a statement sequence
void getLiveVars(
	const StmtSequence* pStmtSeq,
	Expression::SymbolSet& startSet,
	const Expression::SymbolSet& exitSet,
	const Expression::SymbolSet& retSet,
	const Expression::SymbolSet* pBreakSet, 
	const Expression::SymbolSet* pContSet,
	LiveVarMap& liveVarMap
);

// Function to get the live variable sets for an if-else statement
void getLiveVars(
	const IfElseStmt* pIfElseStmt,
	Expression::SymbolSet& startSet,
	const Expression::SymbolSet& exitSet,
	const Expression::SymbolSet& retSet,
	const Expression::SymbolSet* pBreakSet, 
	const Expression::SymbolSet* pContSet,
	LiveVarMap& liveVarMap
);

// Function to get the live variable sets for a loop statement
void getLiveVars(
	const LoopStmt* pLoopStmt,
	Expression::SymbolSet& startSet,
	const Expression::SymbolSet& exitSet,
	const Expression::SymbolSet& retSet,
	LiveVarMap& liveVarMap
);

#endif // #ifndef ANALYSIS_LIVEVARS_H_ 
