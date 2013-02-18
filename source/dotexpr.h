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

// Include guards
#ifndef DOTEXPR_H_
#define DOTEXPR_H_

// Header files
#include <string>
#include "expressions.h"
#include "symbolexpr.h"

/***************************************************************
* Class   : DotExpr
* Purpose : Represent a dot expression (struct,oop)
* Initial : Matthieu Dubet on March 12, 2012
****************************************************************
Revisions and bug fixes:
*/
class DotExpr : public Expression
{
public:
	
	// Constructor
	DotExpr(Expression* pExpr, std::string field)
	: m_pExpr(pExpr), m_Field(field)
	{ m_exprType = Expression::ExprType::DOT ; }
	
	// Method to recursively copy this node
	DotExpr* copy() const;
	
	// Method to access sub-expressions
	ExprVector getSubExprs() const;
	
  SymbolExpr* getRootSymbol() ;
	// Method to replace a sub-expression
	void replaceSubExpr(size_t index, Expression* pNewExpr);
	
	// Method to obtain a string representation of this node
	virtual std::string toString() const;
	
	std::string getField() const { return m_Field; }
	
	// Accessor to get the arguments
	Expression* getExpr() const { return m_pExpr; }

  // Tranform a field to a symbolexpr
  //SymbolExpr* toSymExpr () const ;
	
protected:
	
	// Expression
	Expression* m_pExpr;

	// Symbol expression
	std::string m_Field;
	
};

#endif // #ifndef DOTEXPR_H_
