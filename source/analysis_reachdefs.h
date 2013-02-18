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
#ifndef ANALYSIS_REACHDEFS_H_
#define ANALYSIS_REACHDEFS_H_

// Header files
#include <map>
#include <set>
#include "poolalloc.h"
#include "analysismanager.h"

// Forward declarations
class ProgFunction;
class SymbolExpr;
class IfElseStmt;
class StmtSequence;
class LoopStmt;
class IIRNode;

// Variable definition set type definition
//typedef std::set<const IIRNode*> VarDefSet;
typedef std::set<const IIRNode*, std::less<const IIRNode*>, PoolAlloc<std::pair<const IIRNode*, const IIRNode*> > > VarDefSet;

// Variable definition map type definition
class VarDefMap : public std::unordered_map<const SymbolExpr*, VarDefSet > {} ;

// Reaching definitions map type definition
class ReachDefMap : public std::unordered_map<const IIRNode*, VarDefMap > {} ;

/***************************************************************
* Class   : ReachDefInfo
* Purpose : Store reaching definition analysis information
* Initial : Maxime Chevalier-Boisvert on May 5, 2009
****************************************************************
Revisions and bug fixes:
*/
class ReachDefInfo : public AnalysisInfo
{
public:
	
	// Reaching definition info map
	ReachDefMap reachDefMap;
	
	// Set of reaching definitions at exit points
	VarDefMap exitDefMap;
};

// Function to get the reaching definitions for a function body
AnalysisInfo* computeReachDefs(
	const ProgFunction* pFunction,
	const StmtSequence* pFuncBody,
	const TypeSetString& inArgTypes,
	bool returnBottom
);

// Function to get the reaching definitions for a statement sequence
void getReachDefs(
	const StmtSequence* pStmtSeq,
	const VarDefMap& startMap,
	VarDefMap& exitMap,
	VarDefMap& retMap,
	VarDefMap& breakMap, 
	VarDefMap& contMap,
	ReachDefMap& reachDefMap
);

// Function to get the reaching definitions for an if-else statement
void getReachDefs(
	const IfElseStmt* pIfStmt,
	const VarDefMap& startMap,
	VarDefMap& exitMap,
	VarDefMap& retMap,
	VarDefMap& breakMap, 
	VarDefMap& contMap,
	ReachDefMap& reachDefMap
);

// Function to get the reaching definitions for a loop statement
void getReachDefs(
	const LoopStmt* pLoopStmt,
	const VarDefMap& startMap,
	VarDefMap& exitMap,
	VarDefMap& retMap,
	ReachDefMap& reachDefMap
);

// Function to obtain the union of two variable definition maps
VarDefMap varDefMapUnion(
	const VarDefMap& mapA,
	const VarDefMap& mapB
);

#endif // #ifndef ANALYSIS_REACHDEFS_H_ 
