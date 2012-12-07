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


class dfindex {
    
public:
    dfindex(const boost::filesystem::path &data_file);
    
    std::string getFilePath() const {
        return mwk_data_file.string();
    }
    
    unsigned int getNEvents() const {
        return dfi.getNEvents();
    }
    
    MWTime getMinimumTime() const {
        return dfi.getMinimumTime();
    }
    
    MWTime getMaximumTime() const {
        return dfi.getMaximumTime();
    }
    
    void getEvents(std::vector<EventWrapper> &events,
                   const std::set<unsigned int> &event_codes,
                   MWTime lower_bound,
                   MWTime upper_bound) const
    {
        dfi.getEvents(events, event_codes, lower_bound, upper_bound);
    }
    
    DataFileIndexer::EventsIterator getEventsIterator(const std::set<unsigned int> &event_codes,
                                                      MWTime lower_bound,
                                                      MWTime upper_bound) const
    {
        return dfi.getEventsIterator(event_codes, lower_bound, upper_bound);
    }
    
private:
    static boost::filesystem::path prepareDataFile(const boost::filesystem::path &mwk_data_file);
    
    const boost::filesystem::path mwk_data_file;
    const boost::filesystem::path actual_mwk_file;
    const boost::filesystem::path index_file;
    DataFileIndexer dfi;
    
};


#endif //dfindex_


























