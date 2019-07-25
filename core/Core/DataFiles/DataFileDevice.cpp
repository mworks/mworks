//
//  DataFileDevice.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 7/25/19.
//

#include "DataFileDevice.hpp"

#include "DataFileManager.h"


BEGIN_NAMESPACE_MW


const std::string DataFileDevice::FILENAME("filename");


void DataFileDevice::describeComponent(ComponentInfo &info) {
    IODevice::describeComponent(info);
    info.setSignature("iodevice/data_file");
    info.addParameter(FILENAME);
}


DataFileDevice::DataFileDevice(const ParameterValueMap &parameters) :
    IODevice(parameters)
{
    DataFileManager::instance()->setAutoOpenFilename(parsedText(parameters[FILENAME]));
}


END_NAMESPACE_MW
