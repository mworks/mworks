//
//  MessageTestFixture.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 7/28/16.
//
//

#ifndef MessageTestFixture_hpp
#define MessageTestFixture_hpp

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "MWorksCore/EventConstants.h"
#include "MWorksCore/VariableNotification.h"


BEGIN_NAMESPACE_MW


class MessageTestFixture : public CppUnit::TestFixture {
    
public:
    void setUp() override;
    void tearDown() override;
    
protected:
    void assertMessage(MessageType type, const std::string &msg);
    void assertError(const std::string &msg) { assertMessage(M_ERROR_MESSAGE, msg); }
    
private:
    boost::shared_ptr<VariableNotification> messageNotification;
    Datum messageValue;
    
};


END_NAMESPACE_MW


#endif /* MessageTestFixture_hpp */


























