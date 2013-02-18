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
#ifndef STRUCTARRAYOBJ_H_
#define STRUCTARRAYOBJ_H_

// Header files
#include "matrixobjs.h"
#include <map>

/****************************************************************************
 * ScalarStruct is the base element of a structarray.
****************************************************************************/
typedef std::unordered_map<
            std::string,
            DataObject*
            //std::hash<std::string>,
            //std::equal_to<std::string>,
            //gc_allocator<DataObject*>
        >ScalarStruct;
        
typedef ScalarStruct* ScalarStructPtr;
typedef MatrixObj<ScalarStruct*> StructArrayObj;
typedef StructArrayObj* StructPtr ;

inline ScalarStructPtr makeScalarStructPtr() {
    return new ScalarStruct() ;
}

inline void insertInScalarStruct(
        ScalarStructPtr scalar_struct,
        const char* string,
        DataObject* obj)
{
    std::pair<std::string,DataObject*> p(string,obj) ;
    scalar_struct->insert(p);
}

template <> inline DataObject::Type StructArrayObj::getClassType() 
{ return DataObject::Type::STRUCTARRAY; }

template <> DataObject* StructArrayObj::convert(DataObject::Type outType) const ;

#endif // #ifndef STRUCTOBJ_H_
