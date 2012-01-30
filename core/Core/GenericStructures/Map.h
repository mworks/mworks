/*
 *  Map.h
 *  MWorksCore
 *
 *  Created by Christopher Stawarz on 3/23/11.
 *  Copyright 2011 MIT. All rights reserved.
 *
 */

#ifndef _MW_MAP_H
#define _MW_MAP_H


#include <map>
#include <string>

#include "MWorksMacros.h"


BEGIN_NAMESPACE_MW


template<typename ValueType>
class Map : public std::map<std::string, ValueType> {
    
public:
    //
    // This allows use of operator[] with const maps
    //
    const ValueType& operator[](const std::string &key) const {
        return std::map<std::string, ValueType>::at(key);
    }
    
    //
    // Without this, the compiler will always use the const version of operator[], even with non-const objects
    //
    using std::map<std::string, ValueType>::operator[];
    
};


typedef Map<std::string> StringMap;


END_NAMESPACE_MW


#endif






















