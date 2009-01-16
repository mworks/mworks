/*
 *  PlatformDependentServicesTest.cpp
 *  MonkeyWorksCore
 *
 *  Created by Ben Kennedy 2006
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "PlatformDependentServicesTest.h"
#include "PlatformDependentServices.h"
#include <string>
using namespace mw;


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( PlatformDependentServicesTestFixture, "Unit Test" );
//CPPUNIT_TEST_SUITE_REGISTRATION( PlatformDependentServicesTestFixture );


void PlatformDependentServicesTestFixture::setUp(){
}

void PlatformDependentServicesTestFixture::tearDown(){
}

#define PlatformDependentServicesTestFixture_LOTS 50000
#define NUM_CONST_STRINGS 5

bool PlatformDependentServicesTestFixture::bejesus_beater(std::string saved, 
														   const char * charPtr,
														   int counter) {
	const int big_ugly_chunk_of_data = 2112;
	char *dummy = new char[big_ugly_chunk_of_data];
	memset(dummy,78,big_ugly_chunk_of_data);
	
	if(counter >= PlatformDependentServicesTestFixture_LOTS) {
		delete [] dummy;
		return saved==charPtr;
	}
	
	delete [] dummy;
	return bejesus_beater(saved, charPtr, counter+1);
}

bool PlatformDependentServicesTestFixture::bejesus_beater(
									std::string *savedArray, 
								    std::string *charPtrArray,
									int counter,
									int index) {
	const int big_ugly_chunk_of_data = 666;
	
	char *dummy = new char[big_ugly_chunk_of_data];
	memset(dummy, 13, big_ugly_chunk_of_data);
	
	if(counter >= PlatformDependentServicesTestFixture_LOTS) {
		delete [] dummy;
		return savedArray[index]==charPtrArray[index];
	}
	
	delete [] dummy;
	return bejesus_beater(savedArray, 
						  charPtrArray, 
						  counter+1, 
						  (index+1)%NUM_CONST_STRINGS);
}

void PlatformDependentServicesTestFixture::testConstStrings() {
  fprintf(stderr, "Running PlatformDependentServicesTestFixture::testConstStrings()\n");
  using namespace std;
	
  const int big_ugly_chunk_of_data = 1978;
  char *dummy[12];
	
  string string_pluginPath(pluginPath().string());
  dummy[0] = new char[big_ugly_chunk_of_data];
  memset(dummy[0],42,big_ugly_chunk_of_data);
  string string_scriptingPath(scriptingPath().string());
  dummy[1] = new char[big_ugly_chunk_of_data];
  memset(dummy[1],42,big_ugly_chunk_of_data);
  string string_localPath(localPath().string());
  dummy[2] = new char[big_ugly_chunk_of_data];
  memset(dummy[2],42,big_ugly_chunk_of_data);
  string string_experimentInstallPath(experimentInstallPath().string());
  dummy[3] = new char[big_ugly_chunk_of_data];
  memset(dummy[3],42,big_ugly_chunk_of_data);
  
  std::string charPtr_pluginPath = pluginPath().string();
  dummy[4] = new char[big_ugly_chunk_of_data];
  memset(dummy[4],42,big_ugly_chunk_of_data);
  std::string charPtr_scriptingPath = scriptingPath().string();
  dummy[5] = new char[big_ugly_chunk_of_data];
  memset(dummy[5],42,big_ugly_chunk_of_data);
  std::string charPtr_localPath = localPath().string();
  dummy[6] = new char[big_ugly_chunk_of_data];
  memset(dummy[6],42,big_ugly_chunk_of_data);
  std::string charPtr_experimentInstallPath = experimentInstallPath().string();
  dummy[7] = new char[big_ugly_chunk_of_data];
  memset(dummy[7],42,big_ugly_chunk_of_data);

  CPPUNIT_ASSERT(string_pluginPath == charPtr_pluginPath);
  dummy[8] = new char[big_ugly_chunk_of_data];
  memset(dummy[8],42,big_ugly_chunk_of_data);
  CPPUNIT_ASSERT(string_scriptingPath == charPtr_scriptingPath);
  dummy[9] = new char[big_ugly_chunk_of_data];
  memset(dummy[9],42,big_ugly_chunk_of_data);
  CPPUNIT_ASSERT(string_localPath == charPtr_localPath);
  dummy[10] = new char[big_ugly_chunk_of_data];
  memset(dummy[10],42,big_ugly_chunk_of_data);
  CPPUNIT_ASSERT(string_experimentInstallPath == 
		 charPtr_experimentInstallPath);
  dummy[11] = new char[big_ugly_chunk_of_data];
  memset(dummy[11],42,big_ugly_chunk_of_data);

  for (int i=0; i<12; ++i) {
	  delete [] dummy[i];
  }
  
  // OK now time to beat the bejesus out of the stack to see if that's going
  // to cause any trouble
  
  CPPUNIT_ASSERT(bejesus_beater(string_pluginPath,
				charPtr_pluginPath.c_str(),
				0)
		 );
  dummy[0] = new char[big_ugly_chunk_of_data];
  memset(dummy[0],42,big_ugly_chunk_of_data);
  CPPUNIT_ASSERT(bejesus_beater(string_scriptingPath,
				charPtr_scriptingPath.c_str(),
				0)
		 );

  dummy[1] = new char[big_ugly_chunk_of_data];
  memset(dummy[1],42,big_ugly_chunk_of_data);
  CPPUNIT_ASSERT(bejesus_beater(string_localPath,
				charPtr_localPath.c_str(),
				0)
		 );
  dummy[2] = new char[big_ugly_chunk_of_data];
  memset(dummy[2],42,big_ugly_chunk_of_data);
  CPPUNIT_ASSERT(bejesus_beater(string_experimentInstallPath,
				charPtr_experimentInstallPath.c_str(),
				0)
		 );
  dummy[3] = new char[big_ugly_chunk_of_data];
  memset(dummy[3],42,big_ugly_chunk_of_data);
  
  CPPUNIT_ASSERT(string_pluginPath == charPtr_pluginPath);
  dummy[4] = new char[big_ugly_chunk_of_data];
  memset(dummy[4],42,big_ugly_chunk_of_data);
  CPPUNIT_ASSERT(string_scriptingPath == charPtr_scriptingPath);
  dummy[5] = new char[big_ugly_chunk_of_data];
  memset(dummy[5],42,big_ugly_chunk_of_data);
  CPPUNIT_ASSERT(string_localPath == charPtr_localPath);
  dummy[6] = new char[big_ugly_chunk_of_data];
  memset(dummy[6],42,big_ugly_chunk_of_data);
  CPPUNIT_ASSERT(string_experimentInstallPath == 
		 charPtr_experimentInstallPath);
  dummy[7] = new char[big_ugly_chunk_of_data];
  memset(dummy[7],42,big_ugly_chunk_of_data);

  for (int i=0; i<8; ++i) {
	  delete [] dummy[i];
  }
  
  
  
  string stringArray[NUM_CONST_STRINGS];
  std::string charPtrArray[NUM_CONST_STRINGS];
  stringArray[0] = pluginPath().string();
  dummy[0] = new char[big_ugly_chunk_of_data];
  memset(dummy[0],42,big_ugly_chunk_of_data);
  stringArray[1] = scriptingPath().string();
  dummy[1] = new char[big_ugly_chunk_of_data];
  memset(dummy[1],42,big_ugly_chunk_of_data);
  stringArray[2] = localPath().string();
  dummy[2] = new char[big_ugly_chunk_of_data];
  memset(dummy[2],42,big_ugly_chunk_of_data);
  stringArray[3] = experimentInstallPath().string();
  dummy[3] = new char[big_ugly_chunk_of_data];
  memset(dummy[3],42,big_ugly_chunk_of_data);
  stringArray[4] = experimentStorageDirectoryName().string();
  dummy[4] = new char[big_ugly_chunk_of_data];
  memset(dummy[4],42,big_ugly_chunk_of_data);
  
  charPtrArray[0] = pluginPath().string();
  dummy[5] = new char[big_ugly_chunk_of_data];
  memset(dummy[5],42,big_ugly_chunk_of_data);
  charPtrArray[1] = scriptingPath().string();
  dummy[6] = new char[big_ugly_chunk_of_data];
  memset(dummy[6],42,big_ugly_chunk_of_data);
  charPtrArray[2] = localPath().string();
  dummy[7] = new char[big_ugly_chunk_of_data];
  memset(dummy[7],42,big_ugly_chunk_of_data);
  charPtrArray[3] = experimentInstallPath().string();
  dummy[8] = new char[big_ugly_chunk_of_data];
  memset(dummy[8],42,big_ugly_chunk_of_data);
  charPtrArray[4] = experimentStorageDirectoryName().string();
  dummy[9] = new char[big_ugly_chunk_of_data];
  memset(dummy[9],42,big_ugly_chunk_of_data);
  
  for (int i=0; i<NUM_CONST_STRINGS; ++i) {
	  dummy[10] = new char[big_ugly_chunk_of_data];
	  memset(dummy[10],42,big_ugly_chunk_of_data);
	  
	  CPPUNIT_ASSERT(bejesus_beater(stringArray,
									charPtrArray,
									0,
									i)
					 );
	  delete [] dummy[10];	
  }
  
  for (int i=0; i<10; ++i) {
	  delete [] dummy[i];
  }
}	      

