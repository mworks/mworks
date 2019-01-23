//
//  DataFileReader.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 1/22/19.
//

#ifndef DataFileReader_hpp
#define DataFileReader_hpp

#include <unordered_set>

#include <boost/filesystem/path.hpp>

#include "Utilities.h"


BEGIN_NAMESPACE_MW


class DataFileReader {
    
public:
    static std::unique_ptr<DataFileReader> openDataFile(const boost::filesystem::path &filePath);
    
    virtual ~DataFileReader() { }
    
    virtual std::size_t getNumEvents() = 0;
    virtual MWTime getTimeMin() = 0;
    virtual MWTime getTimeMax() = 0;
    
    virtual void selectEvents(const std::unordered_set<int> &codes = {},
                              MWTime timeMin = MIN_MWORKS_TIME(),
                              MWTime timeMax = MAX_MWORKS_TIME()) = 0;
    virtual bool nextEvent(int &code, MWTime &time, Datum &data) = 0;
    
};


END_NAMESPACE_MW


#endif /* DataFileReader_hpp */
