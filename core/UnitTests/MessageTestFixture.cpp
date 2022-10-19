//
//  MessageTestFixture.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 7/28/16.
//
//

#include "MessageTestFixture.hpp"

#include "MWorksCore/StandardVariables.h"


BEGIN_NAMESPACE_MW


void MessageTestFixture::setUp() {
    if (!GlobalMessageVariable) {
        VariableProperties props(Datum(0),
                                 ANNOUNCE_MESSAGE_VAR_TAGNAME,
                                 M_WHEN_CHANGED,
                                 false);
        GlobalMessageVariable = boost::make_shared<GlobalVariable>(props);
    }
    
    messageNotification = boost::make_shared<VariableCallbackNotification>([this](const Datum& value, MWTime time) {
        if (value.isDictionary()) {
            messageValue = value;
        }
    });
    
    GlobalMessageVariable->addNotification(messageNotification);
}


void MessageTestFixture::tearDown() {
    messageNotification->remove();
}


void MessageTestFixture::assertMessage(MessageType type, const std::string &msg) {
    CPPUNIT_ASSERT( messageValue.isDictionary() );
    
    {
        CPPUNIT_ASSERT( messageValue.hasKey(M_MESSAGE_TYPE) );
        Datum actualType = messageValue.getElement(M_MESSAGE_TYPE);
        CPPUNIT_ASSERT( actualType.isInteger() );
        CPPUNIT_ASSERT_EQUAL( type, MessageType(actualType.getInteger()) );
    }
    
    {
        CPPUNIT_ASSERT( messageValue.hasKey(M_MESSAGE) );
        Datum actualMsg = messageValue.getElement(M_MESSAGE);
        CPPUNIT_ASSERT( actualMsg.isString() );
        CPPUNIT_ASSERT_EQUAL( msg, std::string(actualMsg) );
    }
}


END_NAMESPACE_MW


























