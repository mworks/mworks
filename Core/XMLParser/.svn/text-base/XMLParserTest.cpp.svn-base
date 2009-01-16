/*
 *  XMLParserTest.cpp
 *  MonkeyWorksCore
 *
 *  Created by bkennedy on 12/21/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "XMLParserTest.h"
#include "MonkeyWorksCore/Clock.h"
#include <sstream>
#include "boost/filesystem/operations.hpp" 
#include "boost/filesystem/fstream.hpp"   
#include "MonkeyWorksCore/VariableLoad.h"
#include "MonkeyWorksCore/VariableSave.h"


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( mw::XMLParserTestFixture, "Unit Test" );

void mw::XMLParserTestFixture::setUp() {
	mw::FullCoreEnvironmentTestFixture::setUp();
	
	mw::Data default_value(0L);
	testVar = mw::GlobalVariableRegistry->createGlobalVariable( new mw::VariableProperties(&default_value, 
																						   "testVar",
																						   "testVar",
																						   "testVar",
																						   M_NEVER, 
																						   M_WHEN_CHANGED,
																						   true, 
																						   true,
																						   M_CONTINUOUS_INFINITE,
																						   ""));		
	
	CPPUNIT_ASSERT(testVar->getValue().getInteger() == 0);	
	
	ostringstream oss;
	oss << "/tmp/XMLParserTest" << rand() << ".xml";
	temp_xml_file_path = boost::filesystem::path(oss.str(), boost::filesystem::native);
	
}

void mw::XMLParserTestFixture::tearDown() {
	boost::filesystem::remove(temp_xml_file_path);
	mw::FullCoreEnvironmentTestFixture::tearDown();
}

void mw::XMLParserTestFixture::testLoadBool_1() {
	testVar->setValue(mw::Data(M_BOOLEAN, false));
	CPPUNIT_ASSERT(testVar->getValue().getBool() == false);		
	
	
	std::string xml_text("<?xml version=\"1.0\"?>\n<monkeyml version=\"1.1\"><variable_assignments><variable_assignment variable=\"testVar\" type=\"boolean\">1</variable_assignment></variable_assignments></monkeyml>");
	writeToFile(xml_text);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	
	CPPUNIT_ASSERT(testVar->getValue().getBool() == true);			
}

void mw::XMLParserTestFixture::testLoadBool_0() {
	testVar->setValue(mw::Data(M_BOOLEAN, true));
	CPPUNIT_ASSERT(testVar->getValue().getBool() == true);		
	
	
	std::string xml_text("<?xml version=\"1.0\"?>\n<monkeyml version=\"1.1\"><variable_assignments><variable_assignment variable=\"testVar\" type=\"boolean\">0</variable_assignment></variable_assignments></monkeyml>");
	writeToFile(xml_text);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	
	CPPUNIT_ASSERT(testVar->getValue().getBool() == false);			
}

void mw::XMLParserTestFixture::testLoadBool_true() {
	testVar->setValue(mw::Data(M_BOOLEAN, false));
	CPPUNIT_ASSERT(testVar->getValue().getBool() == false);		
	
	
	std::string xml_text("<?xml version=\"1.0\"?>\n<monkeyml version=\"1.1\"><variable_assignments><variable_assignment variable=\"testVar\" type=\"boolean\">true</variable_assignment></variable_assignments></monkeyml>");
	writeToFile(xml_text);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	
	CPPUNIT_ASSERT(testVar->getValue().getBool() == true);			
}

void mw::XMLParserTestFixture::testLoadBool_false() {
	testVar->setValue(mw::Data(M_BOOLEAN, true));
	CPPUNIT_ASSERT(testVar->getValue().getBool() == true);		
	
	
	std::string xml_text("<?xml version=\"1.0\"?>\n<monkeyml version=\"1.1\"><variable_assignments><variable_assignment variable=\"testVar\" type=\"boolean\">false</variable_assignment></variable_assignments></monkeyml>");
	writeToFile(xml_text);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	
	CPPUNIT_ASSERT(testVar->getValue().getBool() == false);			
}

void mw::XMLParserTestFixture::testLoadBool_TRUE() {
	testVar->setValue(mw::Data(M_BOOLEAN, false));
	CPPUNIT_ASSERT(testVar->getValue().getBool() == false);		
	
	
	std::string xml_text("<?xml version=\"1.0\"?>\n<monkeyml version=\"1.1\"><variable_assignments><variable_assignment variable=\"testVar\" type=\"boolean\">TRUE</variable_assignment></variable_assignments></monkeyml>");
	writeToFile(xml_text);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	
	CPPUNIT_ASSERT(testVar->getValue().getBool() == true);			
}

void mw::XMLParserTestFixture::testLoadBool_FALSE() {
	testVar->setValue(mw::Data(M_BOOLEAN, true));
	CPPUNIT_ASSERT(testVar->getValue().getBool() == true);		
	
	
	std::string xml_text("<?xml version=\"1.0\"?>\n<monkeyml version=\"1.1\"><variable_assignments><variable_assignment variable=\"testVar\" type=\"boolean\">FALSE</variable_assignment></variable_assignments></monkeyml>");
	writeToFile(xml_text);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	
	CPPUNIT_ASSERT(testVar->getValue().getBool() == false);			
}

void mw::XMLParserTestFixture::testLoadBool_YES() {
	testVar->setValue(mw::Data(M_BOOLEAN, false));
	CPPUNIT_ASSERT(testVar->getValue().getBool() == false);		
	
	
	std::string xml_text("<?xml version=\"1.0\"?>\n<monkeyml version=\"1.1\"><variable_assignments><variable_assignment variable=\"testVar\" type=\"boolean\">YES</variable_assignment></variable_assignments></monkeyml>");
	writeToFile(xml_text);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	
	CPPUNIT_ASSERT(testVar->getValue().getBool() == true);			
}

void mw::XMLParserTestFixture::testLoadBool_NO() {
	testVar->setValue(mw::Data(M_BOOLEAN, true));
	CPPUNIT_ASSERT(testVar->getValue().getBool() == true);		
	
	
	std::string xml_text("<?xml version=\"1.0\"?>\n<monkeyml version=\"1.1\"><variable_assignments><variable_assignment variable=\"testVar\" type=\"boolean\">NO</variable_assignment></variable_assignments></monkeyml>");
	writeToFile(xml_text);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	
	CPPUNIT_ASSERT(testVar->getValue().getBool() == false);			
}

void mw::XMLParserTestFixture::testLoadBool_yes() {
	testVar->setValue(mw::Data(M_BOOLEAN, false));
	CPPUNIT_ASSERT(testVar->getValue().getBool() == false);		
	
	
	std::string xml_text("<?xml version=\"1.0\"?>\n<monkeyml version=\"1.1\"><variable_assignments><variable_assignment variable=\"testVar\" type=\"boolean\">yes</variable_assignment></variable_assignments></monkeyml>");
	writeToFile(xml_text);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	
	CPPUNIT_ASSERT(testVar->getValue().getBool() == true);			
}

void mw::XMLParserTestFixture::testLoadBool_no() {
	testVar->setValue(mw::Data(M_BOOLEAN, true));
	CPPUNIT_ASSERT(testVar->getValue().getBool() == true);		
	
	
	std::string xml_text("<?xml version=\"1.0\"?>\n<monkeyml version=\"1.1\"><variable_assignments><variable_assignment variable=\"testVar\" type=\"boolean\">no</variable_assignment></variable_assignments></monkeyml>");
	writeToFile(xml_text);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	
	CPPUNIT_ASSERT(testVar->getValue().getBool() == false);			
}

void mw::XMLParserTestFixture::testLoadInt() {
	CPPUNIT_ASSERT(testVar->getValue().getInteger() == 0);		
	testVar->setValue(mw::Data(1L));
	CPPUNIT_ASSERT(testVar->getValue().getInteger() == 1);		
	
	
	std::string xml_text("<?xml version=\"1.0\"?>\n<monkeyml version=\"1.1\"><variable_assignments><variable_assignment variable=\"testVar\" type=\"integer\">2</variable_assignment></variable_assignments></monkeyml>");
	writeToFile(xml_text);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	
	CPPUNIT_ASSERT(testVar->getValue().getInteger() == 2);		
}

void mw::XMLParserTestFixture::testLoadIntNegative() {
	CPPUNIT_ASSERT(testVar->getValue().getInteger() == 0);		
	testVar->setValue(mw::Data(1L));
	CPPUNIT_ASSERT(testVar->getValue().getInteger() == 1);		
	
	
	std::string xml_text("<?xml version=\"1.0\"?>\n<monkeyml version=\"1.1\"><variable_assignments><variable_assignment variable=\"testVar\" type=\"integer\">-2</variable_assignment></variable_assignments></monkeyml>");
	writeToFile(xml_text);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	
	CPPUNIT_ASSERT(testVar->getValue().getInteger() == -2);		
}

void mw::XMLParserTestFixture::testLoadIntExpression() {
	CPPUNIT_ASSERT(testVar->getValue().getInteger() == 0);		
	testVar->setValue(mw::Data(1L));
	CPPUNIT_ASSERT(testVar->getValue().getInteger() == 1);		
	
	
	std::string xml_text("<?xml version=\"1.0\"?>\n<monkeyml version=\"1.1\"><variable_assignments><variable_assignment variable=\"testVar\" type=\"integer\">1+1</variable_assignment></variable_assignments></monkeyml>");
	writeToFile(xml_text);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	
	CPPUNIT_ASSERT(testVar->getValue().getInteger() == 2);		
}

void mw::XMLParserTestFixture::testLoadIntSpaces() {
	CPPUNIT_ASSERT(testVar->getValue().getInteger() == 0);		
	testVar->setValue(mw::Data(1L));
	CPPUNIT_ASSERT(testVar->getValue().getInteger() == 1);		
	
	
	std::string xml_text("<?xml version=\"1.0\"?>\n<monkeyml version=\"1.1\"><variable_assignments><variable_assignment variable=\"testVar\" type=\"integer\"> 2 </variable_assignment></variable_assignments></monkeyml>");
	writeToFile(xml_text);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	
	CPPUNIT_ASSERT(testVar->getValue().getInteger() == 2);		
}

void mw::XMLParserTestFixture::testLoadIntAttribute() {
	CPPUNIT_ASSERT(testVar->getValue().getInteger() == 0);		
	testVar->setValue(mw::Data(1L));
	CPPUNIT_ASSERT(testVar->getValue().getInteger() == 1);		
	
	
	std::string xml_text("<?xml version=\"1.0\"?>\n<monkeyml version=\"1.1\"><variable_assignments><variable_assignment variable=\"testVar\" type=\"integer\" value=\"2\"/></variable_assignments></monkeyml>");
	writeToFile(xml_text);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	
	CPPUNIT_ASSERT(testVar->getValue().getInteger() == 2);		
}

void mw::XMLParserTestFixture::testLoadIntAttributeClosingTag() {
	CPPUNIT_ASSERT(testVar->getValue().getInteger() == 0);		
	testVar->setValue(mw::Data(1L));
	CPPUNIT_ASSERT(testVar->getValue().getInteger() == 1);		
	
	
	std::string xml_text("<?xml version=\"1.0\"?>\n<monkeyml version=\"1.1\"><variable_assignments><variable_assignment variable=\"testVar\" type=\"integer\" value=\"2\"></variable_assignment></variable_assignments></monkeyml>");
	writeToFile(xml_text);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	
	CPPUNIT_ASSERT(testVar->getValue().getInteger() == 2);		
}

void mw::XMLParserTestFixture::testLoadFloat() {
	CPPUNIT_ASSERT(testVar->getValue().getFloat() == 0.0);		
	testVar->setValue(mw::Data(1.5));
	CPPUNIT_ASSERT(testVar->getValue().getFloat() == 1.5);		
	
	
	std::string xml_text("<?xml version=\"1.0\"?>\n<monkeyml version=\"1.1\"><variable_assignments><variable_assignment variable=\"testVar\" type=\"float\">2.5</variable_assignment></variable_assignments></monkeyml>");
	writeToFile(xml_text);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	
	CPPUNIT_ASSERT(testVar->getValue().getFloat() == 2.5);		
}

void mw::XMLParserTestFixture::testLoadString() {
	CPPUNIT_ASSERT(testVar->getValue().getFloat() == 0.0);		
	mw::Data test_string = "test string";
	testVar->setValue(test_string);
	CPPUNIT_ASSERT(testVar->getValue() == test_string);		
	CPPUNIT_ASSERT(testVar->getValue().getString() == std::string("test string"));		
	
	
	std::string xml_text("<?xml version=\"1.0\"?>\n<monkeyml version=\"1.1\"><variable_assignments><variable_assignment variable=\"testVar\" type=\"string\">new test string</variable_assignment></variable_assignments></monkeyml>");
	writeToFile(xml_text);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	
	CPPUNIT_ASSERT(testVar->getValue().getString() == std::string("new test string"));		
}

void mw::XMLParserTestFixture::testLoadStringSpaces() {
	CPPUNIT_ASSERT(testVar->getValue().getFloat() == 0.0);		
	mw::Data test_string = "test string";
	testVar->setValue(test_string);
	CPPUNIT_ASSERT(testVar->getValue() == test_string);		
	CPPUNIT_ASSERT(testVar->getValue().getString() == std::string("test string"));		
	
	
	std::string xml_text("<?xml version=\"1.0\"?>\n<monkeyml version=\"1.1\"><variable_assignments><variable_assignment variable=\"testVar\" type=\"string\">  new test string  </variable_assignment></variable_assignments></monkeyml>");
	writeToFile(xml_text);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	
	CPPUNIT_ASSERT(testVar->getValue().getString() == std::string("  new test string  "));		
}

void mw::XMLParserTestFixture::testLoadStringAttribute() {
	CPPUNIT_ASSERT(testVar->getValue().getFloat() == 0.0);		
	mw::Data test_string = "test string";
	testVar->setValue(test_string);
	CPPUNIT_ASSERT(testVar->getValue() == test_string);		
	CPPUNIT_ASSERT(testVar->getValue().getString() == std::string("test string"));		
	
	
	std::string xml_text("<?xml version=\"1.0\"?>\n<monkeyml version=\"1.1\"><variable_assignments><variable_assignment variable=\"testVar\" type=\"string\" value=\"new test string\"/></variable_assignments></monkeyml>");
	writeToFile(xml_text);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	
	CPPUNIT_ASSERT(testVar->getValue().getString() == std::string("new test string"));		
}

void mw::XMLParserTestFixture::testLoadDictionary() {
	CPPUNIT_ASSERT(testVar->getValue().getFloat() == 0.0);		
	
	const char *xml_text =
	"<?xml version=\"1.0\"?>"
	"<monkeyml version=\"1.1\">"
	"<variable_assignments>"
	"<variable_assignment variable=\"testVar\">"
	"<dictionary>"
	"<dictionary_element>"
	"<key>four</key>"
	"<value type=\"integer\">4</value>"
	"</dictionary_element>"
	"<dictionary_element>"
	"<key>six</key>"
	"<value type=\"integer\">6</value>"
	"</dictionary_element>"
	"</dictionary>"
	"</variable_assignment>"
	"</variable_assignments>"
	"</monkeyml>";
	writeToFile(xml_text);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	
	mw::Data test_dict(M_DICTIONARY, 2);
	test_dict.addElement("four", mw::Data(4L));
	test_dict.addElement("six", mw::Data(6L));
	
	CPPUNIT_ASSERT(testVar->getValue() == test_dict);		
	
	mw::Data test_dict2(M_DICTIONARY, 2);
	test_dict2.addElement("six", mw::Data(6L));
	test_dict2.addElement("four", mw::Data(4L));
	
	CPPUNIT_ASSERT(testVar->getValue() == test_dict2);		
	
	mw::Data test_dict3(M_DICTIONARY, 3);
	test_dict2.addElement("six", mw::Data(6L));
	test_dict2.addElement("four", mw::Data(4L));
	
	CPPUNIT_ASSERT(testVar->getValue() == test_dict2);		
}

void mw::XMLParserTestFixture::testLoadDictionaryInDictionary() {
	CPPUNIT_ASSERT(testVar->getValue().getFloat() == 0.0);		
	
	const char *xml_text =
	"<?xml version=\"1.0\"?>"
	"<monkeyml version=\"1.1\">"
	"  <variable_assignments>"
	"    <variable_assignment variable=\"testVar\">"
	"      <dictionary>"
	"       <dictionary_element>"
	"         <key>one</key>"
	"         <value>"
	"         <dictionary>"
	"           <dictionary_element>"
	"             <key>four</key>"
	"             <value type=\"integer\">14</value>"
	"           </dictionary_element>"
	"           <dictionary_element>"
	"             <key>six</key>"
	"             <value type=\"integer\">16</value>"
	"           </dictionary_element>"
	"         </dictionary>"
	"         </value>"
	"       </dictionary_element>"
	"       <dictionary_element>"
	"         <key>two</key>"
	"         <value>"
	"         <dictionary>"
	"           <dictionary_element>"
	"             <key>four</key>"
	"             <value type=\"integer\">24</value>"
	"           </dictionary_element>"
	"           <dictionary_element>"
	"             <key>six</key>"
	"             <value type=\"integer\">26</value>"
	"           </dictionary_element>"
	"         </dictionary>"
	"         </value>"
	"       </dictionary_element>"
	"      </dictionary>"
	"    </variable_assignment>"
	"  </variable_assignments>"
	"</monkeyml>";
	writeToFile(xml_text);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	
	mw::Data sub_dict1(M_DICTIONARY, 2);
	sub_dict1.addElement("four", mw::Data(14L));
	sub_dict1.addElement("six", mw::Data(16L));
	
	mw::Data sub_dict2(M_DICTIONARY, 2);
	sub_dict2.addElement("four", mw::Data(24L));
	sub_dict2.addElement("six", mw::Data(26L));
	
	mw::Data main_dict(M_DICTIONARY, 2);
	main_dict.addElement("one", sub_dict1);
	main_dict.addElement("two", sub_dict2);
	
	CPPUNIT_ASSERT(testVar->getValue() == main_dict);		
}

void mw::XMLParserTestFixture::testLoadList() {
	CPPUNIT_ASSERT(testVar->getValue().getFloat() == 0.0);		
	
	const char *xml_text =
	"<?xml version=\"1.0\"?>"
	"<monkeyml version=\"1.1\">"
	"<variable_assignments>"
	"<variable_assignment variable=\"testVar\">"
	"<list_data>"
	"<list_element>"
	"<value type=\"integer\">4</value>"
	"</list_element>"
	"<list_element>"
	"<value type=\"integer\">6</value>"
	"</list_element>"
	"</list_data>"
	"</variable_assignment>"
	"</variable_assignments>"
	"</monkeyml>";
	writeToFile(xml_text);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	
	mw::Data test_list(M_LIST, 2);
	test_list.setElement(0, mw::Data(4L));
	test_list.setElement(1, mw::Data(6L));
	
	CPPUNIT_ASSERT(testVar->getValue() == test_list);			
}

void mw::XMLParserTestFixture::testLoadListWithinList() {
	CPPUNIT_ASSERT(testVar->getValue().getFloat() == 0.0);		
	
	const char *xml_text =
	"<?xml version=\"1.0\"?>"
	"<monkeyml version=\"1.1\">"
	"  <variable_assignments>"
	"    <variable_assignment variable=\"testVar\">"
	"      <list_data>"
	"       <list_element>"
	"         <value>"
	"         <list_data>"
	"           <list_element>"
	"             <value type=\"integer\">1</value>"
	"           </list_element>"
	"           <list_element>"
	"             <value type=\"integer\">2</value>"
	"           </list_element>"
	"         </list_data>"
	"         </value>"
	"       </list_element>"
	"       <list_element>"
	"         <value>"
	"         <list_data>"
	"           <list_element>"
	"             <value type=\"integer\">3</value>"
	"           </list_element>"
	"           <list_element>"
	"             <value type=\"integer\">4</value>"
	"           </list_element>"
	"         </list_data>"
	"         </value>"
	"       </list_element>"
	"      </list_data>"
	"    </variable_assignment>"
	"  </variable_assignments>"
	"</monkeyml>";
	
	writeToFile(xml_text);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	
	mw::Data sublist1(M_LIST, 2);
	sublist1.setElement(0, mw::Data(1L));
	sublist1.setElement(1, mw::Data(2L));
	
	mw::Data sublist2(M_LIST, 2);
	sublist2.setElement(0, mw::Data(3L));
	sublist2.setElement(1, mw::Data(4L));
	
	mw::Data test_list(M_LIST, 2);
	test_list.setElement(0, sublist1);
	test_list.setElement(1, sublist2);
	
	
	CPPUNIT_ASSERT(testVar->getValue() == test_list);			
}

void mw::XMLParserTestFixture::testLoadDictionaryInList() {
	CPPUNIT_ASSERT(testVar->getValue().getFloat() == 0.0);		
	
	const char *xml_text =
	"<?xml version=\"1.0\"?>"
	"<monkeyml version=\"1.1\">"
	"  <variable_assignments>"
	"    <variable_assignment variable=\"testVar\">"
	"      <list_data>"
	"       <list_element>"
	"         <value>"
	"           <dictionary>"
	"             <dictionary_element>"
	"               <value type=\"integer\">11</value>"
	"               <key>one</key>"
	"             </dictionary_element>"
	"             <dictionary_element>"
	"               <value type=\"integer\">12</value>"
	"               <key>two</key>"
	"             </dictionary_element>"
	"           </dictionary>"
	"         </value>"
	"       </list_element>"
	"       <list_element>"
	"         <value>"
	"           <dictionary>"
	"             <dictionary_element>"
	"               <key>one</key>"
	"               <value type=\"integer\">21</value>"
	"             </dictionary_element>"
	"             <dictionary_element>"
	"               <key>two</key>"
	"               <value type=\"integer\">22</value>"
	"             </dictionary_element>"
	"           </dictionary>"
	"         </value>"
	"       </list_element>"
	"      </list_data>"
	"    </variable_assignment>"
	"  </variable_assignments>"
	"</monkeyml>";
	
	writeToFile(xml_text);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	
	mw::Data subdict1(M_DICTIONARY, 2);
	subdict1.addElement("one", mw::Data(11L));
	subdict1.addElement("two", mw::Data(12L));
	
	mw::Data subdict2(M_DICTIONARY, 2);
	subdict2.addElement("one", mw::Data(21L));
	subdict2.addElement("two", mw::Data(22L));
	
	mw::Data test_list(M_LIST, 2);
	test_list.setElement(0, subdict1);
	test_list.setElement(1, subdict2);
	
	CPPUNIT_ASSERT(testVar->getValue() == test_list);			
}

void mw::XMLParserTestFixture::testLoadListInDictionary() {
	CPPUNIT_ASSERT(testVar->getValue().getFloat() == 0.0);		
	
	const char *xml_text =
	"<?xml version=\"1.0\"?>"
	"<monkeyml version=\"1.1\">"
	"  <variable_assignments>"
	"    <variable_assignment variable=\"testVar\">"
	"      <dictionary>"
	"       <dictionary_element>"
	"         <key>one</key>"
	"         <value>"
	"           <list_data>"
	"             <list_element>"
	"               <value type=\"integer\">11</value>"
	"             </list_element>"
	"             <list_element>"
	"               <value type=\"integer\">12</value>"
	"             </list_element>"
	"           </list_data>"
	"         </value>"
	"       </dictionary_element>"
	"       <dictionary_element>"
	"         <key>two</key>"
	"         <value>"
	"           <list_data>"
	"             <list_element>"
	"               <value type=\"integer\">21</value>"
	"             </list_element>"
	"             <list_element>"
	"               <value type=\"integer\">22</value>"
	"             </list_element>"
	"           </list_data>"
	"         </value>"
	"       </dictionary_element>"
	"      </dictionary>"
	"    </variable_assignment>"
	"  </variable_assignments>"
	"</monkeyml>";
	
	writeToFile(xml_text);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	
	mw::Data sublist1(M_LIST, 2);
	sublist1.setElement(0, mw::Data(11L));
	sublist1.setElement(1, mw::Data(12L));
	
	mw::Data sublist2(M_LIST, 2);
	sublist2.setElement(0, mw::Data(21L));
	sublist2.setElement(1, mw::Data(22L));
	
	mw::Data test_dict(M_DICTIONARY, 2);
	test_dict.addElement("one", sublist1);
	test_dict.addElement("two", sublist2);
	
	
	CPPUNIT_ASSERT(testVar->getValue() == test_dict);			
}

void mw::XMLParserTestFixture::testSaveThenLoadBool() {
	testVar->setValue(mw::Data(M_BOOLEAN, true));
	mw::VariableSave::saveExperimentwideVariables(temp_xml_file_path);
	
	testVar->setValue(mw::Data(M_BOOLEAN, false));
	CPPUNIT_ASSERT(testVar->getValue().getBool() == false);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	CPPUNIT_ASSERT(testVar->getValue().getBool() == true);
}

void mw::XMLParserTestFixture::testSaveThenLoadInteger() {
	testVar->setValue(42L);
	mw::VariableSave::saveExperimentwideVariables(temp_xml_file_path);
	
	testVar->setValue(35L);
	CPPUNIT_ASSERT(testVar->getValue().getInteger() == 35);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	CPPUNIT_ASSERT(testVar->getValue().getInteger() == 42);
}

void mw::XMLParserTestFixture::testSaveThenLoadFloat() {
	testVar->setValue(0.5);
	mw::VariableSave::saveExperimentwideVariables(temp_xml_file_path);
	
	testVar->setValue(7.25);
	CPPUNIT_ASSERT(testVar->getValue().getFloat() == 7.25);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	CPPUNIT_ASSERT(testVar->getValue().getFloat() == 0.5);
}

void mw::XMLParserTestFixture::testSaveThenLoadString() {
	mw::Data first_string = "first string";
	testVar->setValue(first_string);
	mw::VariableSave::saveExperimentwideVariables(temp_xml_file_path);
	
	mw::Data second_string = "second string";
	testVar->setValue(second_string);
	CPPUNIT_ASSERT(testVar->getValue() == "second string");
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	CPPUNIT_ASSERT(testVar->getValue() == "first string");
}

void mw::XMLParserTestFixture::testSaveThenLoadDict() {
	mw::Data test_dict1(M_DICTIONARY, 2);
	test_dict1.addElement("four", mw::Data(14L));
	test_dict1.addElement("six", mw::Data(16L));
	
	testVar->setValue(test_dict1);
	mw::VariableSave::saveExperimentwideVariables(temp_xml_file_path);
	
	mw::Data test_dict2(M_DICTIONARY, 2);
	test_dict2.addElement("four", mw::Data(24L));
	test_dict2.addElement("six", mw::Data(26L));
	testVar->setValue(test_dict2);
	CPPUNIT_ASSERT(testVar->getValue() == test_dict2);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	CPPUNIT_ASSERT(testVar->getValue() == test_dict1);
}

void mw::XMLParserTestFixture::testSaveThenLoadList() {
	mw::Data test_list1(M_LIST, 2);
	test_list1.setElement(0, mw::Data(14L));
	test_list1.setElement(1, mw::Data(16L));
	
	testVar->setValue(test_list1);
	mw::VariableSave::saveExperimentwideVariables(temp_xml_file_path);
	
	mw::Data test_list2(M_LIST, 2);
	test_list2.setElement(0, mw::Data(24L));
	test_list2.setElement(1, mw::Data(16L));
	testVar->setValue(test_list2);
	CPPUNIT_ASSERT(testVar->getValue() == test_list2);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	CPPUNIT_ASSERT(testVar->getValue() == test_list1);
}

void mw::XMLParserTestFixture::testSaveThenLoadListInList() {
	mw::Data sublist1(M_LIST, 2);
	sublist1.setElement(0, mw::Data(1L));
	sublist1.setElement(1, mw::Data(2L));
	
	mw::Data sublist2(M_LIST, 2);
	sublist2.setElement(0, mw::Data(3L));
	sublist2.setElement(1, mw::Data(4L));
	
	mw::Data test_list(M_LIST, 2);
	test_list.setElement(0, sublist1);
	test_list.setElement(1, sublist2);
	
	testVar->setValue(test_list);
	mw::VariableSave::saveExperimentwideVariables(temp_xml_file_path);
	
	mw::Data test_data(5L);
	testVar->setValue(test_data);
	CPPUNIT_ASSERT(testVar->getValue().getInteger() == 5);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	CPPUNIT_ASSERT(testVar->getValue() == test_list);
}

void mw::XMLParserTestFixture::testSaveThenLoadDictInDict() {
	mw::Data sub_dict1(M_DICTIONARY, 2);
	sub_dict1.addElement("four", mw::Data(14L));
	sub_dict1.addElement("six", mw::Data(16L));
	
	mw::Data sub_dict2(M_DICTIONARY, 2);
	sub_dict2.addElement("four", mw::Data(24L));
	sub_dict2.addElement("six", mw::Data(26L));
	
	mw::Data main_dict(M_DICTIONARY, 2);
	main_dict.addElement("one", sub_dict1);
	main_dict.addElement("two", sub_dict2);
	
	testVar->setValue(main_dict);
	mw::VariableSave::saveExperimentwideVariables(temp_xml_file_path);
	
	mw::Data test_data(5L);
	testVar->setValue(test_data);
	CPPUNIT_ASSERT(testVar->getValue().getInteger() == 5);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	CPPUNIT_ASSERT(testVar->getValue() == main_dict);
}

void mw::XMLParserTestFixture::testSaveThenLoadDictInList() {
	mw::Data sub_dict1(M_DICTIONARY, 2);
	sub_dict1.addElement("four", mw::Data(14L));
	sub_dict1.addElement("six", mw::Data(16L));
	
	mw::Data sub_dict2(M_DICTIONARY, 2);
	sub_dict2.addElement("four", mw::Data(24L));
	sub_dict2.addElement("six", mw::Data(26L));
		
	mw::Data test_list(M_LIST, 2);
	test_list.setElement(0, sub_dict1);
	test_list.setElement(1, sub_dict2);
	
	testVar->setValue(test_list);
	mw::VariableSave::saveExperimentwideVariables(temp_xml_file_path);
	
	mw::Data test_data(5L);
	testVar->setValue(test_data);
	CPPUNIT_ASSERT(testVar->getValue().getInteger() == 5);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	CPPUNIT_ASSERT(testVar->getValue() == test_list);
}

void mw::XMLParserTestFixture::testSaveThenLoadListInDict() {
	mw::Data sublist1(M_LIST, 2);
	sublist1.setElement(0, mw::Data(1L));
	sublist1.setElement(1, mw::Data(2L));
	
	mw::Data sublist2(M_LIST, 2);
	sublist2.setElement(0, mw::Data(3L));
	sublist2.setElement(1, mw::Data(4L));
	
	mw::Data main_dict(M_DICTIONARY, 2);
	main_dict.addElement("one", sublist1);
	main_dict.addElement("two", sublist2);
	
	testVar->setValue(main_dict);
	mw::VariableSave::saveExperimentwideVariables(temp_xml_file_path);
	
	mw::Data test_data(5L);
	testVar->setValue(test_data);
	CPPUNIT_ASSERT(testVar->getValue().getInteger() == 5);
	
	mw::VariableLoad::loadExperimentwideVariables(temp_xml_file_path);
	CPPUNIT_ASSERT(testVar->getValue() == main_dict);
}

void mw::XMLParserTestFixture::writeToFile(const std::string &text) {
	boost::filesystem::ofstream file(temp_xml_file_path);
	file << text;
	file.close();	
}
