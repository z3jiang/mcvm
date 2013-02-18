// =========================================================================== //
//                                                                             //
// Copyright 2012 Matthieu Dubet and McGill University.                        //
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

// Header files
#include "dotexpr.h"

/***************************************************************
* Function: DotExpr::copy()
* Purpose : Copy this IIR node recursively
* Initial : Matthieu Dubet on March 12, 2012
****************************************************************
Revisions and bug fixes:
*/
DotExpr* DotExpr::copy() const
{
	// Create and return a copy of this node
	return new DotExpr(m_pExpr->copy(),m_Field);
}	

/***************************************************************
* Function: BinaryOpExpr::toString()
* Purpose : Generate a text representation of this IIR node
* Initial : Matthieu Dubet on March 12, 2012
****************************************************************
Revisions and bug fixes:
*/
std::string DotExpr::toString() const
{
	// Declare a variable for the output string
	std::string output;
	
	// Concatenate
	output = m_pExpr->toString() + "." + m_Field ;

	// Return the output string
	return output;
}


/***************************************************************
* Function: DotExpr::getSubExprs()
* Purpose : Access sub-expressions
* Initial : Matthieu Dubet on March 13, 2012
****************************************************************
Revisions and bug fixes:
*/
Expression::ExprVector DotExpr::getSubExprs() const
{
	// Create a list to store the sub-expression pointers
	ExprVector list;
	
	// Add the symbol to the list
	list.push_back(m_pExpr);
	
	// Return the list
	return list;
}


/***************************************************************
* Function: DotExpr::replaceSubExpr()
* Purpose : Replace a sub-expression
* Initial : Matthieu Dubet on March 13, 2012
****************************************************************
Revisions and bug fixes:
*/
void DotExpr::replaceSubExpr(size_t index, Expression* pNewExpr)
{
	if (index == 0)
	{
		m_pExpr = pNewExpr;
	} else {
		assert(false) ;
	}
	return;
}
/*
SymbolExpr* DotExpr::toSymExpr(std::string f) const  {
  std::string res ;
  res = toString() ;
  res += "." ;
  res += f ;
  const std::string v = res ;
  return SymbolExpr::getSymbol(v) ;
}
*/

SymbolExpr* DotExpr::getRootSymbol() {
  return m_pExpr->getRootSymbol() ;
}

