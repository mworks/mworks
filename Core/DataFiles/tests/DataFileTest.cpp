/*
 *  DataFileTest.cpp
 *  MonkeyWorksCore
 *
 *  Created by Ben Kennedy 2006
 *  Copyright 2006 MIT. All rights reserved.
 *
 */

#include "DataFileTest.h"
#include "MonkeyWorksCore/EventBuffer.h"
#include "MonkeyWorksCore/Event.h"
#include "MonkeyWorksCore/GenericData.h"
#include "MonkeyWorksCore/DataFileManager.h"
#include "MonkeyWorksCore/ScarabServices.h"
#include <pthread.h>
using namespace mw;

#define USE_MACRO_EVENTS	0
#define MACRO_EVENT_HIGH_WATER_MARK	1
#define MAX_MACRO_EVENT_SIZE	1

#define USE_EXPLICIT_BUFFERING	1
#define BUFFER_HIGH_WATER_MARK	1
#define MAX_EVENTS_TO_BUFFER	10000

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( DataFileTestFixture, "Unit Test" );
//CPPUNIT_TEST_SUITE_REGISTRATION( DataFileTestFixture );

#define dft_CPPUNIT_ASSERT_TS(x)	dft_cppunit_lock->lock(); \
CPPUNIT_ASSERT(x); \
dft_cppunit_lock->unlock()

struct dftTestArgs {
	int testNumber;
	bool runningFlag;
};

const int dft_TIMEOUT_TIME = 2400;
const int dft_BUFF_SIZE = 50000;
const int dft_NUM_THREADS = 50;
const int dft_NUM_EVENTS_PER_THREAD = 999;

BufferManager *dftGlobalBufferManager;
Lockable *dft_cppunit_lock;
struct dftTestArgs *dft_ptrTestArgs;

void *dftMassEventDump(void *args);
void *dftMassEventRead(void *args);
void *dftTimeoutThread(void *args);



void DataFileTestFixture::setUp() {
	static int testNumber = 0;
	dft_ptrTestArgs = new struct dftTestArgs;
	dft_ptrTestArgs->testNumber = testNumber;
	dft_ptrTestArgs->runningFlag = true;
	testNumber++;
	dft_cppunit_lock = new Lockable();
	
	if(dftGlobalBufferManager)
		delete dftGlobalBufferManager;
	dftGlobalBufferManager = new BufferManager();
	
	pthread_t timeout;
	pthread_create(&timeout, NULL, &dftTimeoutThread, (void *)dft_ptrTestArgs);	
}

void *dftTimeoutThread(void *args) {
	struct dftTestArgs *ta = (struct dftTestArgs *)args;	
	sleep(dft_TIMEOUT_TIME);
	if(ta->runningFlag) {
		printf("DataFileTest %d timed out\n", ta->testNumber);

		bool timeout = false;
		dft_CPPUNIT_ASSERT_TS(timeout);
	} 
	
	pthread_detach(pthread_self());	
	delete ta;
	
	return 0;
}

void DataFileTestFixture::tearDown() {
	dft_ptrTestArgs->runningFlag = false;
	delete dft_cppunit_lock;
	delete dftGlobalBufferManager;
	dftGlobalBufferManager = 0;
}


void DataFileTestFixture::initializationTest() {
	fprintf(stderr, "mDataFileTestFixture::initializationTest()\n");
}

void DataFileTestFixture::dataFileWriterTest() {
	fprintf(stderr, "mDataFileTestFixture::dataFileWriterTest()\n");
//	// fill up an event queue
//	for (int i =0; i<dft_BUFF_SIZE-1; ++i) {
//		Data _i(M_INTEGER, i );
//		dftGlobalBufferManager->putEvent(new DataEvent(i, _i));
//	}
//
//	EventBufferReader *buffer_reader = dftGlobalBufferManager->
//												getNewDisplayBufferReader();
//
//	
//	char *uri = "ldobinary:file:///Library/MonkeyWorks/DataFiles/dft_test.mwk";
//    ScarabSession *pipe = scarab_session_connect(uri);
//	CPPUNIT_ASSERT(getScarabError(pipe) == 0);
//
//
//	delete buffer_reader;
}

void DataFileTestFixture::multiDataFileWriterTest() {
	fprintf(stderr, "mDataFileTestFixture::multiDataFileWriterTest()\n");
}
