//
//  DataFileReader.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 1/22/19.
//

#include "DataFileReader.hpp"

#include <TargetConditionals.h>

#include <boost/algorithm/string/case_conv.hpp>

#if TARGET_OS_OSX
#  include "dfindex.h"
#endif
#include "MWK2File.hpp"


BEGIN_NAMESPACE_MW


std::unique_ptr<DataFileReader> DataFileReader::openDataFile(const boost::filesystem::path &filePath) {
#if TARGET_OS_OSX
    if (boost::algorithm::to_lower_copy(filePath.extension().string()) != ".mwk2") {
        return std::unique_ptr<DataFileReader>(new scarab::dfindex(filePath));
    }
#endif
    return std::unique_ptr<DataFileReader>(new MWK2Reader(filePath.string()));
}


END_NAMESPACE_MW
