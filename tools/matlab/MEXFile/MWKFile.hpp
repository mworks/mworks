//
//  MWKFile.hpp
//  MEX File
//
//  Created by Christopher Stawarz on 2/7/23.
//  Copyright © 2023 MWorks Project. All rights reserved.
//

#ifndef MWKFile_hpp
#define MWKFile_hpp

#include "MEXInputs.h"
#include "MEXOutputs.h"


BEGIN_NAMESPACE_MW_MATLAB


class MWKFile {
    
public:
    static void open(MEXInputs &inputs, MEXOutputs &outputs);
    static void close(MEXInputs &inputs, MEXOutputs &outputs);
    static void getNumEvents(MEXInputs &inputs, MEXOutputs &outputs);
    static void getMinTime(MEXInputs &inputs, MEXOutputs &outputs);
    static void getMaxTime(MEXInputs &inputs, MEXOutputs &outputs);
    static void selectEvents(MEXInputs &inputs, MEXOutputs &outputs);
    static void nextEvent(MEXInputs &inputs, MEXOutputs &outputs);
    static void getEvents(MEXInputs &inputs, MEXOutputs &outputs);
    
private:
    using Handle = mxUint64;
    // Handle must be large enough to hold a pointer
    static_assert(sizeof(Handle) >= sizeof(void *));
    using ReaderMap = std::map<Handle, std::unique_ptr<DataFileReader>>;
    
    static void throwError(const std::string &msg);
    static ReaderMap & getReaderMap();
    static DataFileReader & getReader(MEXInputs &inputs);
    
};


END_NAMESPACE_MW_MATLAB


#endif /* MWKFile_hpp */
