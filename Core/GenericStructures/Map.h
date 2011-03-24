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


BEGIN_NAMESPACE(mw)


template<typename ValueType>
class Map : public std::map<std::string, ValueType> {
    
public:
    //
    // This looks redundant, but without it, the compiler always uses the
    // const version of operator[], even with non-const objects
    //
    ValueType& operator[](const std::string &key) {
        return std::map<std::string, ValueType>::operator[](key);
    }
    
    //
    // This allows use of operator[] with const maps
    //
    const ValueType& operator[](const std::string &key) const {
        return std::map<std::string, ValueType>::at(key);
    }
    
};


typedef Map<std::string> StringMap;


END_NAMESPACE(mw)


#endif






















