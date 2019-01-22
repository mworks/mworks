/*
 *  dfindex.h
 *  DataFileIndexer
 *
 *  Created by bkennedy on 2/20/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#ifndef dfindex_
#define dfindex_

#include <boost/filesystem/path.hpp>

#include "DataFileIndexer.h"
#include "DataFileReader.hpp"


BEGIN_NAMESPACE_MW
BEGIN_NAMESPACE(scarab)


class dfindex : public DataFileReader {
    
public:
    explicit dfindex(const boost::filesystem::path &data_file);
    
    std::size_t getNumEvents() override;
    MWTime getTimeMin() override;
    MWTime getTimeMax() override;
    
    void selectEvents(const std::unordered_set<int> &event_codes, MWTime lower_bound, MWTime upper_bound) override;
    bool nextEvent(int &code, MWTime &time, Datum &data) override;
    
private:
    static boost::filesystem::path prepareDataFile(const boost::filesystem::path &mwk_data_file);
    
    const boost::filesystem::path mwk_data_file;
    const boost::filesystem::path actual_mwk_file;
    const boost::filesystem::path index_file;
    
    DataFileIndexer dfi;
    std::unique_ptr<DataFileIndexer::EventsIterator> eventsIterator;
    
};


END_NAMESPACE(scarab)
END_NAMESPACE_MW


#endif //dfindex_
