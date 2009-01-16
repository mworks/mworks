/*
 *  XMLParser.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 11/25/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef	XML_PARSER_H_
#define XML_PARSER_H_
#include <iostream>
#include <string>
#include <map>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>

#include "PlatformDependentServices.h"
#include "ComponentRegistry_new.h"

#include <boost/algorithm/string/case_conv.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <boost/lexical_cast.hpp>

namespace mw {
	using namespace std;

void	error_func(void * _parser_context, const char * error, ...);

class XMLParser {

	protected:
		std::string path;
		xmlParserCtxt *context;
		xmlDoc *xml_doc;
		
		vector<string> parser_errors;
		
		xmlDoc *errors_doc;
		
		xsltStylesheetPtr simplification_transform;
	
		shared_ptr<mwComponentRegistry> registry;
	
		void setup(shared_ptr<mwComponentRegistry> _reg, std::string _path);
	
		void _createAndAddReplicatedChildren(xmlNode *node, 
											 const string &variable, 
											 const vector<string> &values);
	
		void _createAndConnectReplicatedChildren(shared_ptr<mw::Component> parent, 
												 map<string, string> properties,
												 xmlNode *child_node,
												 const string &child_instance_id,
												 const string &variable, 
												 const vector<string> &values);
	
public:
	
		XMLParser(shared_ptr<mwComponentRegistry> _reg, std::string _path);
		XMLParser(std::string _path);
		virtual ~XMLParser();
		
		virtual void validate();
		virtual void parse(bool announce_progress = false);
		
		virtual void addParserError(string error){
			parser_errors.push_back(error);
		}
		
		static string squashFileName(string name);
		virtual std::string getWorkingPathString();
		
		virtual void _processNode(xmlNode *child);
		virtual void _processRangeReplicator(xmlNode *node);
		virtual void _processListReplicator(xmlNode *node);
		virtual void _dumpNode(xmlNode *node);
		virtual void _substituteAttributeStrings(xmlNode *node, string token, string replacement);
		virtual void _addVariableAssignment(xmlNode *node, const string& variable, const string& value);
	
		virtual void _processCreateDirective(xmlNode *node);
		virtual void _processAnonymousCreateDirective(xmlNode *node);
		virtual void _processGenericCreateDirective(xmlNode *node, bool anon);
		
		virtual void _processInstanceDirective(xmlNode *node);
		virtual void _processConnectDirective(xmlNode *node);
		virtual void _connectChildToParent(shared_ptr<mw::Component> parent, 
									   map<string, string> properties, 
									   xmlNode *child_node);
									   
		virtual void _processFinalizeDirective(xmlNode *node);
		virtual string _attributeForName(xmlNode *node, string name);
		virtual void _setAttributeForName(xmlNode *node, string name, string value);
		
		virtual string _generateInstanceTag(string tag, string reference_id, string instance_id);
		
		virtual void _processVariableAssignment(xmlNode *node);
		
		shared_ptr<mw::Component> XMLParser::_getConnectionChild(xmlNode *child);
		
		virtual map<string, string> _createPropertiesMap(xmlNode *node);
		
		// Parse a node which contains a numeric, or structured (e.g. list, 
		// dictionary) datum and return an Data object
		virtual Data _parseDataValue(xmlNode *node);
		
		vector<xmlNode *> _getChildren(xmlNode *node);
		vector<xmlNode *> _getChildren(xmlNode *node, string tag);
		string _getContent(xmlNode *node);
};
}

#endif


