/*
 *  XMLParser.h
 *  MWorksCore
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
#include "ComponentRegistry.h"

#include <boost/algorithm/string/case_conv.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#include <boost/lexical_cast.hpp>
#pragma clang diagnostic pop

using std::string;
using std::vector;

#define INSTANCE_PREFIX "instance:"
#define INSTANCE_STEM	"_"


BEGIN_NAMESPACE_MW


class XMLParser : boost::noncopyable {

	private:
    
		const std::string path;
		xmlParserCtxt *context;
		xmlDoc *xml_doc;
		
		vector<string> parser_errors;
		
		xsltStylesheetPtr simplification_transform;
	
        const boost::shared_ptr<ComponentRegistry> registry;
	
        void _addLocationAttributes(xmlNode *node);

        void addParserError(string error){
			parser_errors.push_back(error);
		}
		
        void _processNode(xmlNode *child);
        void _processRangeReplicator(xmlNode *node);
        void _generateRangeReplicatorValues(xmlNode *node, vector<string> &values);
        void _processListReplicator(xmlNode *node);
        void _generateListReplicatorValues(xmlNode *node, vector<string> &values);
        void _substituteAttributeStrings(xmlNode *node, string token, string replacement);
        void _substituteAttributeStrings(xmlNode *node, const string& form1, const string& form2, const string& replacement);
    
        void _substituteTagStrings(xmlNode *node, string token, string replacement);
        void _substituteTagStrings(xmlNode *node, const string& form1, const string& form2, const string& replacement);
  
        void _addVariableAssignment(xmlNode *node, const string& variable, const string& value);
	
		virtual void _processCreateDirective(xmlNode *node);
		virtual void _processAnonymousCreateDirective(xmlNode *node);
        void _processGenericCreateDirective(xmlNode *node, bool anon);
		
        virtual void _processInstanceDirective(xmlNode *node);
		virtual void _processConnectDirective(xmlNode *node);
        void _connectChildToParent(shared_ptr<mw::Component> parent,
                                   map<string, string> properties,
                                   xmlNode *child_node);
									   
		virtual void _processFinalizeDirective(xmlNode *node);
        string _attributeForName(xmlNode *node, string name);
        const char *_cStringAttributeForName(xmlNode *node, string name);
        void _setAttributeForName(xmlNode *node, string name, string value);
  
        string _generateInstanceTag(string tag, string reference_id, string instance_id) {
            string instance_tag = string(INSTANCE_PREFIX) + reference_id + string(INSTANCE_STEM) + instance_id; 
            return instance_tag;
        }
            
        void _processVariableAssignment(xmlNode *node);
		
		shared_ptr<mw::Component> _getConnectionChild(xmlNode *child, map<string, string> properties);
		
        map<string, string> _createPropertiesMap(xmlNode *node);
		
		// Parse a node which contains a numeric, or structured (e.g. list, 
		// dictionary) datum and return an Datum object
        Datum _parseDataValue(xmlNode *node);
		
		vector<xmlNode *> _getChildren(xmlNode *node);
		vector<xmlNode *> _getChildren(xmlNode *node, string tag);
	
		void _createAndAddReplicatedChildren(xmlNode *node, 
											 const string &variable, 
											 const vector<string> &values);
	
		void _createAndConnectReplicatedChildren(shared_ptr<mw::Component> parent, 
												 map<string, string> properties,
												 xmlNode *child_node,
												 const string &child_instance_id,
												 const string &variable, 
												 const vector<string> &values);
    
protected:
        static string _getContent(xmlNode *node);
        xmlXPathObjectPtr evaluateXPathExpression(const std::string &expr);
	
public:
	
        XMLParser(const boost::shared_ptr<ComponentRegistry> &_reg,
                  const std::string &_path,
                  const std::string &_simplification_transform_path="");
		explicit XMLParser(const std::string &_path, const std::string &_simplification_transform_path="");
		virtual ~XMLParser();
    
        static void error_func(void * _parser_context, const char * error, ...);
		
        void loadFile();
        void getDocumentData(std::vector<xmlChar> &data);
        virtual void parse(bool announce_progress = false);
		
		static string squashFileName(string name);
        std::string getWorkingPathString();
};


END_NAMESPACE_MW


#endif


