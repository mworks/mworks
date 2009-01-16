/*
 *  XMLVariableWriter.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 5/30/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "XMLVariableWriter.h"
using namespace mw;

namespace mw {
	void	variable_writer_error_func(void * _parser_context, const char * error, ...){
		
		va_list ap;
		va_start(ap, error);
		
		//mXMLParser *parser = (XMLParser *)_parser;
		//parser->addParserError(error);
#define ERROR_MSG_BUFFER_SIZE	512
		char buffer[ERROR_MSG_BUFFER_SIZE];// = { '\0' };
		vsnprintf(buffer, ERROR_MSG_BUFFER_SIZE-1, error, ap);
		va_end(ap);
		
		//xmlParserCtxt *context = (xmlParserCtxt *)_parser_context;
		//string error_string((char *)buffer);
		cerr << buffer << endl;
		merror(M_PARSER_MESSAGE_DOMAIN, buffer);
	}
}

void XMLVariableWriter::writeVariablesToFile(vector< shared_ptr<Variable> > variables, 
											 filesystem::path file){
    
    xmlParserCtxt *context = xmlNewParserCtxt();	
	
    xmlSetGenericErrorFunc(context, &variable_writer_error_func);
    
    xmlDocPtr doc = xmlNewDoc((const xmlChar *)"1.0");
    
    xmlNodePtr mml_root = xmlNewDocNode(doc, NULL, (const xmlChar *)"monkeyml", NULL);
    xmlDocSetRootElement(doc, mml_root);
    
    xmlNewProp(mml_root, (const xmlChar *)"version", (const xmlChar *)"1.1");
    xmlNodePtr assignments_node = xmlNewChild(mml_root, NULL, (const xmlChar *)"variable_assignments", NULL);
    
    vector< shared_ptr<Variable> >::iterator i;
    for(i = variables.begin(); i != variables.end(); i++){
        shared_ptr<Variable> var = *i;
        xmlNodePtr variable_node = variableToXML(var);
        xmlAddChild(assignments_node, variable_node);
    }
    
    string path_str = file.string();
    xmlSaveFile(path_str.c_str(), doc);
}


xmlNodePtr XMLVariableWriter::variableToXML(shared_ptr<Variable> var){
    
    xmlNodePtr new_node = xmlNewNode(NULL, (const xmlChar *)"variable_assignment");
    xmlNewProp(new_node, (const xmlChar *)"variable", (const xmlChar *)((var->getVariableName()).c_str()));
    
	Data value = var->getValue();
	
    GenericDataType type = value.getDataType();
    if(type == M_INTEGER){
        xmlNewProp(new_node, (const xmlChar *)"type", (const xmlChar *)"integer");
		xmlNewProp(new_node, (const xmlChar *)"value", (const xmlChar *)(value.toString().c_str()));
    } else if(type == M_FLOAT){
        xmlNewProp(new_node, (const xmlChar *)"type", (const xmlChar *)"float");
		xmlNewProp(new_node, (const xmlChar *)"value", (const xmlChar *)(value.toString().c_str()));
    } else if(type == M_BOOLEAN){
        xmlNewProp(new_node, (const xmlChar *)"type", (const xmlChar *)"boolean");
		xmlNewProp(new_node, (const xmlChar *)"value", (const xmlChar *)(value.toString().c_str()));
    } else if(type == M_STRING) {
        xmlNewProp(new_node, (const xmlChar *)"type", (const xmlChar *)"string");
		xmlNewProp(new_node, (const xmlChar *)"value", (const xmlChar *)(value.toString().c_str()));
    } else if(type == M_DICTIONARY) {
		xmlAddChild(new_node, recursiveValueToXML(value));
    } else if(type == M_LIST) {
		xmlAddChild(new_node, recursiveValueToXML(value));
    } else {
		throw SimpleException("Trying to save variables with bad type");
	}
	
    
    return new_node;
}

xmlNodePtr XMLVariableWriter::recursiveValueToXML(const Data &value) {
    xmlNodePtr value_node;
	
	if(value.getDataType() == M_DICTIONARY) {
		value_node = xmlNewNode(NULL, (const xmlChar *)"dictionary");
		std::vector<Data> keys = value.getKeys();
		for(std::vector<Data>::const_iterator i = keys.begin();
			i != keys.end();
			++i) {
			if(value.hasKey(*i)) {
				xmlNodePtr dictionary_element_node = xmlNewNode(NULL, (const xmlChar *)"dictionary_element");
				xmlNodePtr key_node = xmlNewNode(NULL, (const xmlChar *)"key");
				xmlNodeSetContent(key_node, (const xmlChar *)i->toString().c_str());
				xmlNodePtr subvalue_node = xmlNewNode(NULL, (const xmlChar *)"value");
				
				
				Data subvalue = value.getElement(*i);
				if(subvalue.getDataType() == M_DICTIONARY || subvalue.getDataType() == M_LIST) {
					xmlAddChild(subvalue_node, recursiveValueToXML(subvalue));	
				} else {
					GenericDataType type = subvalue.getDataType();
					if(type == M_INTEGER){
						xmlNewProp(subvalue_node, (const xmlChar *)"type", (const xmlChar *)"integer");
						xmlNewProp(subvalue_node, (const xmlChar *)"value", (const xmlChar *)(subvalue.toString().c_str()));
					} else if(type == M_FLOAT){
						xmlNewProp(subvalue_node, (const xmlChar *)"type", (const xmlChar *)"float");
						xmlNewProp(subvalue_node, (const xmlChar *)"value", (const xmlChar *)(subvalue.toString().c_str()));
					} else if(type == M_BOOLEAN){
						xmlNewProp(subvalue_node, (const xmlChar *)"type", (const xmlChar *)"boolean");
						xmlNewProp(subvalue_node, (const xmlChar *)"value", (const xmlChar *)(subvalue.toString().c_str()));
					} else if(type == M_STRING) {
						xmlNewProp(subvalue_node, (const xmlChar *)"type", (const xmlChar *)"string");
						xmlNewProp(subvalue_node, (const xmlChar *)"value", (const xmlChar *)(subvalue.toString().c_str()));
					} else {
						throw SimpleException("Trying to find the value of a variable with a bad type");
					}					
				}
				
				xmlAddChild(dictionary_element_node, key_node);
				xmlAddChild(dictionary_element_node, subvalue_node);
				xmlAddChild(value_node, dictionary_element_node);
			}
		}
	} else if(value.getDataType() == M_LIST) {
		value_node = xmlNewNode(NULL, (const xmlChar *)"list_data");
		std::vector<Data> elements = value.getElements();
		for(std::vector<Data>::const_iterator i = elements.begin();
			i != elements.end();
			++i) {
			xmlNodePtr list_element_node = xmlNewNode(NULL, (const xmlChar *)"list_element");
			xmlNodePtr subvalue_node = xmlNewNode(NULL, (const xmlChar *)"value");
			
			Data subvalue(*i);
			if(subvalue.getDataType() == M_DICTIONARY || subvalue.getDataType() == M_LIST) {
				xmlAddChild(subvalue_node, recursiveValueToXML(subvalue));	
			} else {
				GenericDataType type = subvalue.getDataType();
				if(type == M_INTEGER){
					xmlNewProp(subvalue_node, (const xmlChar *)"type", (const xmlChar *)"integer");
					xmlNewProp(subvalue_node, (const xmlChar *)"value", (const xmlChar *)(subvalue.toString().c_str()));
				} else if(type == M_FLOAT){
					xmlNewProp(subvalue_node, (const xmlChar *)"type", (const xmlChar *)"float");
					xmlNewProp(subvalue_node, (const xmlChar *)"value", (const xmlChar *)(subvalue.toString().c_str()));
				} else if(type == M_BOOLEAN){
					xmlNewProp(subvalue_node, (const xmlChar *)"type", (const xmlChar *)"boolean");
					xmlNewProp(subvalue_node, (const xmlChar *)"value", (const xmlChar *)(subvalue.toString().c_str()));
				} else if(type == M_STRING) {
					xmlNewProp(subvalue_node, (const xmlChar *)"type", (const xmlChar *)"string");
					xmlNewProp(subvalue_node, (const xmlChar *)"value", (const xmlChar *)(subvalue.toString().c_str()));
				} else {
					throw SimpleException("Trying to find the value of a variable with a bad type");
				}									
			}
			xmlAddChild(list_element_node, subvalue_node);
			xmlAddChild(value_node, list_element_node);
		}		
	} else {
		throw SimpleException("Trying to save container data (M_LIST or M_DICT) and it's constructed incorrectly");		
	}
	
	return value_node;
}

