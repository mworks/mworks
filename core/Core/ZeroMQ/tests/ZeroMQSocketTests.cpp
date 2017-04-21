//
//  ZeroMQSocketTests.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/2/16.
//
//

#include "ZeroMQSocketTests.hpp"

#include <boost/endian/arithmetic.hpp>

#include "MWorksCore/MessagePackAdaptors.hpp"
#include "MWorksCore/ZeroMQSocket.hpp"


BEGIN_NAMESPACE_MW


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( ZeroMQSocketTests, "Unit Test" );


void ZeroMQSocketTests::testBadType() {
    CPPUNIT_ASSERT_THROW( ZeroMQSocket(123456), SimpleException );
}


void ZeroMQSocketTests::testGetLastEndpoint() {
    ZeroMQSocket s(ZMQ_PUB);
    std::string lastEndpoint;
    
    // No endpoint
    CPPUNIT_ASSERT( s.getLastEndpoint(lastEndpoint) );
    CPPUNIT_ASSERT_EQUAL( std::string(), lastEndpoint );
    
    // Endpoint
    {
        const std::string endpoint("tcp://127.0.0.1:19992");
        CPPUNIT_ASSERT( s.bind(endpoint) );
        CPPUNIT_ASSERT( s.getLastEndpoint(lastEndpoint) );
        CPPUNIT_ASSERT_EQUAL( endpoint, lastEndpoint );
        CPPUNIT_ASSERT( s.unbind(endpoint) );
    }
}


void ZeroMQSocketTests::testSetOptionFailure() {
    ZeroMQSocket s(ZMQ_PAIR);
    CPPUNIT_ASSERT( !s.setOption(123456, 0) );
    assertError("ERROR: Cannot set option on ZeroMQ socket: Invalid argument");
}


void ZeroMQSocketTests::testBindFailure() {
    ZeroMQSocket s(ZMQ_PUB);
    CPPUNIT_ASSERT( !s.bind("pct://*:*") );
    assertError("ERROR: Cannot bind ZeroMQ socket: Protocol not supported");
}


void ZeroMQSocketTests::testUnbindFailure() {
    ZeroMQSocket s(ZMQ_PUB);
    CPPUNIT_ASSERT( !s.unbind("pct://*:*") );
    assertError("ERROR: Cannot unbind ZeroMQ socket: Protocol not supported");
}


void ZeroMQSocketTests::testConnectFailure() {
    ZeroMQSocket s(ZMQ_SUB);
    CPPUNIT_ASSERT( !s.connect("pct://*:*") );
    assertError("ERROR: Cannot connect ZeroMQ socket: Protocol not supported");
}


void ZeroMQSocketTests::testDisconnectFailure() {
    ZeroMQSocket s(ZMQ_SUB);
    CPPUNIT_ASSERT( !s.disconnect("pct://*:*") );
    assertError("ERROR: Cannot disconnect ZeroMQ socket: Protocol not supported");
}


void ZeroMQSocketTests::testBasicSendReceive() {
    const std::string endpoint("tcp://127.0.0.1:19992");
    const std::string outgoingMessage("Hello, world!");
    ZeroMQSocket::Message incomingMessage;
    
    ZeroMQSocket sender(ZMQ_PUSH);
    CPPUNIT_ASSERT( sender.setOption(ZMQ_IMMEDIATE, 1) );
    
    ZeroMQSocket receiver(ZMQ_PULL);
    
    // Send failure
    CPPUNIT_ASSERT( ZeroMQSocket::Result::error == receiver.send(outgoingMessage.data(), outgoingMessage.size()) );
    assertError("ERROR: Cannot send on ZeroMQ socket: Operation not supported");
    
    // Receive failure
    CPPUNIT_ASSERT( ZeroMQSocket::Result::error == incomingMessage.recv(sender) );
    assertError("ERROR: Cannot receive on ZeroMQ socket: Operation not supported");
    
    // Send timeout
    CPPUNIT_ASSERT( ZeroMQSocket::Result::timeout == sender.send(outgoingMessage.data(),
                                                                 outgoingMessage.size(),
                                                                 ZMQ_DONTWAIT) );  // Flag
    CPPUNIT_ASSERT( sender.setOption(ZMQ_SNDTIMEO, 100) );  // Option
    CPPUNIT_ASSERT( sender.connect(endpoint) );
    CPPUNIT_ASSERT( ZeroMQSocket::Result::timeout == sender.send(outgoingMessage.data(), outgoingMessage.size()) );
    
    // Receive timeout
    CPPUNIT_ASSERT( ZeroMQSocket::Result::timeout == incomingMessage.recv(receiver, ZMQ_DONTWAIT) );  // Flag
    CPPUNIT_ASSERT( receiver.setOption(ZMQ_RCVTIMEO, 100) );  // Option
    CPPUNIT_ASSERT( receiver.bind(endpoint) );
    CPPUNIT_ASSERT( ZeroMQSocket::Result::timeout == incomingMessage.recv(receiver) );
    
    // Success
    CPPUNIT_ASSERT( ZeroMQSocket::Result::ok == sender.send(outgoingMessage.data(), outgoingMessage.size()) );
    CPPUNIT_ASSERT( ZeroMQSocket::Result::ok == incomingMessage.recv(receiver) );
    CPPUNIT_ASSERT_EQUAL( outgoingMessage, std::string(static_cast<const char *>(incomingMessage.getData()),
                                                       incomingMessage.getSize()) );
    
    CPPUNIT_ASSERT( sender.disconnect(endpoint) );
    CPPUNIT_ASSERT( receiver.unbind(endpoint) );
}


void ZeroMQSocketTests::testMultipartSendReceive() {
    const std::string endpoint("tcp://127.0.0.1:19992");
    const std::string part1("ab");
    const std::string part2("cde");
    const std::string part3("fghi");
    ZeroMQSocket::Message message;
    
    ZeroMQSocket sender(ZMQ_PUSH);
    CPPUNIT_ASSERT( sender.setOption(ZMQ_IMMEDIATE, 1) );
    CPPUNIT_ASSERT( sender.connect(endpoint) );
    
    ZeroMQSocket receiver(ZMQ_PULL);
    CPPUNIT_ASSERT( receiver.bind(endpoint) );
    
    // Send
    CPPUNIT_ASSERT( ZeroMQSocket::Result::ok == sender.send(part1.data(), part1.size(), ZMQ_SNDMORE) );
    CPPUNIT_ASSERT( ZeroMQSocket::Result::ok == sender.send(part2.data(), part2.size(), ZMQ_SNDMORE) );
    CPPUNIT_ASSERT( ZeroMQSocket::Result::ok == sender.send(part3.data(), part3.size()) );
    
    // Receive part 1
    CPPUNIT_ASSERT( ZeroMQSocket::Result::ok == message.recv(receiver) );
    CPPUNIT_ASSERT_EQUAL( part1, std::string(static_cast<const char *>(message.getData()), message.getSize()) );
    CPPUNIT_ASSERT( !message.isComplete() );
    
    // Receive part 2
    CPPUNIT_ASSERT( ZeroMQSocket::Result::ok == message.recv(receiver) );
    CPPUNIT_ASSERT_EQUAL( part2, std::string(static_cast<const char *>(message.getData()), message.getSize()) );
    CPPUNIT_ASSERT( !message.isComplete() );
    
    // Receive part 3
    CPPUNIT_ASSERT( ZeroMQSocket::Result::ok == message.recv(receiver) );
    CPPUNIT_ASSERT_EQUAL( part3, std::string(static_cast<const char *>(message.getData()), message.getSize()) );
    CPPUNIT_ASSERT( message.isComplete() );
    
    CPPUNIT_ASSERT( sender.disconnect(endpoint) );
    CPPUNIT_ASSERT( receiver.unbind(endpoint) );
}


void ZeroMQSocketTests::testEventSendReceive() {
    const std::string endpoint("tcp://127.0.0.1:19992");
    const boost::endian::big_uint16_t eventCode = 512;
    const MWTime eventTime = 1234567890;
    const Datum eventData(Datum::list_value_type { Datum(true), Datum(2.5), Datum("foo") });
    const auto outgoingEvent = boost::make_shared<Event>(eventCode, eventTime, eventData);
    boost::shared_ptr<Event> incomingEvent;
    
    ZeroMQSocket sender(ZMQ_PUSH);
    CPPUNIT_ASSERT( sender.setOption(ZMQ_IMMEDIATE, 1) );
    CPPUNIT_ASSERT( sender.setOption(ZMQ_SNDTIMEO, 100) );
    
    ZeroMQSocket receiver(ZMQ_PULL);
    CPPUNIT_ASSERT( receiver.setOption(ZMQ_RCVTIMEO, 100) );
    
    // Send failure
    CPPUNIT_ASSERT( ZeroMQSocket::Result::error == receiver.send(outgoingEvent) );
    assertError("ERROR: Cannot send on ZeroMQ socket: Operation not supported");
    
    // Receive failure
    CPPUNIT_ASSERT( ZeroMQSocket::Result::error == sender.recv(incomingEvent) );
    assertError("ERROR: Cannot receive on ZeroMQ socket: Operation not supported");
    
    // Send timeout
    CPPUNIT_ASSERT( sender.connect(endpoint) );
    CPPUNIT_ASSERT( ZeroMQSocket::Result::timeout == sender.send(outgoingEvent) );
    
    // Receive timeout
    CPPUNIT_ASSERT( receiver.bind(endpoint) );
    CPPUNIT_ASSERT( ZeroMQSocket::Result::timeout == receiver.recv(incomingEvent) );
    
    // Negative event code
    CPPUNIT_ASSERT( ZeroMQSocket::Result::error == sender.send(boost::make_shared<Event>(-1, eventTime, eventData)) );
    assertError("ERROR: Cannot send event: event code (-1) is out of range");
    
    // Out-of-bounds event code
    CPPUNIT_ASSERT( ZeroMQSocket::Result::error == sender.send(boost::make_shared<Event>(int(std::numeric_limits<std::uint16_t>::max()) + 1,
                                                                                         eventTime,
                                                                                         eventData)) );
    assertError("ERROR: Cannot send event: event code (65536) is out of range");
    
    // Wrong header size
    CPPUNIT_ASSERT( ZeroMQSocket::Result::ok == sender.send(&eventTime, sizeof(eventTime)) );
    CPPUNIT_ASSERT( ZeroMQSocket::Result::error == receiver.recv(incomingEvent) );
    assertError("ERROR: Received invalid event: header has wrong size (expected 2 bytes, got 8 bytes)");
    
    // No body
    CPPUNIT_ASSERT( ZeroMQSocket::Result::ok == sender.send(&eventCode, sizeof(eventCode)) );
    CPPUNIT_ASSERT( ZeroMQSocket::Result::error == receiver.recv(incomingEvent) );
    assertError("ERROR: Received invalid event: body is missing");
    
    // Invalid body
    CPPUNIT_ASSERT( ZeroMQSocket::Result::ok == sender.send(&eventCode, sizeof(eventCode), ZMQ_SNDMORE) );
    {
        std::ostringstream buffer;
        msgpack::pack(buffer, eventTime);
        msgpack::pack(buffer, eventData);
        auto data = buffer.str();
        CPPUNIT_ASSERT( ZeroMQSocket::Result::ok == sender.send(data.data(), data.size() - 1) );  // Not enough data
    }
    CPPUNIT_ASSERT( ZeroMQSocket::Result::error == receiver.recv(incomingEvent) );
    assertError("ERROR: Received invalid event: body is invalid or corrupt");
    
    // Success
    CPPUNIT_ASSERT( ZeroMQSocket::Result::ok == sender.send(outgoingEvent) );
    incomingEvent.reset();
    CPPUNIT_ASSERT( ZeroMQSocket::Result::ok == receiver.recv(incomingEvent) );
    CPPUNIT_ASSERT_EQUAL( int(eventCode), incomingEvent->getEventCode() );
    CPPUNIT_ASSERT_EQUAL( eventTime, incomingEvent->getTime() );
    CPPUNIT_ASSERT( eventData == incomingEvent->getData() );
    
    // Extra data
    CPPUNIT_ASSERT( ZeroMQSocket::Result::ok == sender.send(&eventCode, sizeof(eventCode), ZMQ_SNDMORE) );
    {
        std::ostringstream buffer;
        msgpack::pack(buffer, eventTime);
        msgpack::pack(buffer, eventData);
        msgpack::pack(buffer, 512);
        auto data = buffer.str();
        CPPUNIT_ASSERT( ZeroMQSocket::Result::ok == sender.send(data.data(), data.size()) );
    }
    incomingEvent.reset();
    CPPUNIT_ASSERT( ZeroMQSocket::Result::ok == receiver.recv(incomingEvent) );
    assertMessage(M_WARNING_MESSAGE, "WARNING: Received event body contains 3 bytes of extra data that will be ignored");
    CPPUNIT_ASSERT_EQUAL( int(eventCode), incomingEvent->getEventCode() );
    CPPUNIT_ASSERT_EQUAL( eventTime, incomingEvent->getTime() );
    CPPUNIT_ASSERT( eventData == incomingEvent->getData() );
    
    CPPUNIT_ASSERT( sender.disconnect(endpoint) );
    CPPUNIT_ASSERT( receiver.unbind(endpoint) );
}


END_NAMESPACE_MW



























