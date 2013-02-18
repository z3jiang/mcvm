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
#include "structobj.h"

template <> DataObject* MatrixObj< ScalarStruct* >::convert(DataObject::Type outType) const {
  assert(false) ;
}

template <> std::string StructArrayObj::toString() const {
  std::string res ;

  if (isScalar()) {
      auto s = getScalar() ;
      for ( auto it = s->begin(); it != s->end() ; ++it) {
          res += "\t" ;
          res += it->first ;
          res +=  ": " ;
          auto d = it->second ;
          if ( d == NULL ) {
              res += "[]" ;
          } else if ( d->getType() == DataObject::Type::STRUCTARRAY ) {
              res += "(struct)\n";
              res += indentText(d->toString());
          } else {
              res += d->toString()  ;
          }
          res += "\n" ;
      }
  } else {
      res = "\t" ;
      res +=  " struct array with fields:\n" ;
      for ( auto it = m_Fields.begin(); it != m_Fields.end() ; ++it) {
          res += "\t\t"  ;
          res += * it ;
          res += "\n" ;
      }
  }
  return res ;
}

template <> StructArrayObj* StructArrayObj::copy() const {
  StructArrayObj* res = new StructArrayObj() ;
  res->m_size = m_size ;
  res->m_Fields = m_Fields ;
  res->allocMatrix() ;
  //std::cout << m_numElements << std::endl ;
  for ( size_t i = 1 ; i <= m_numElements ; i++) {
    ScalarStruct* pSS = getElem1D(i) ;
    ScalarStruct* pCopySS = new ScalarStruct() ;

    if (pSS == NULL ) {
      res->setElem1D(i,NULL) ;
    } else {

      for ( ScalarStruct::const_iterator it = pSS->begin(); it != pSS->end() ; it++) {
        DataObject* pObject = it->second ;
        if ( pObject->getType() == Type::STRUCTARRAY ) {
          // Deep copy
          pObject = pObject->copy() ;
        }
        (*pCopySS)[it->first] = pObject ;
      }
      res->setElem1D(i,pCopySS) ;
    }
  }
  return res ;
}

