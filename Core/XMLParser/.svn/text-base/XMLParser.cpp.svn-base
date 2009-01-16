/*
 *  XMLParser.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 11/25/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "XMLParser.h"
#include "States.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#define INSTANCE_PREFIX "instance:"
#define INSTANCE_STEM	"_"

#include <boost/spirit/core.hpp>
#include <boost/spirit/utility/confix.hpp>
#include <boost/spirit/utility/lists.hpp>
#include <boost/spirit/utility/escape_char.hpp>
using namespace mw;

using namespace boost::spirit;

namespace mw {
	void	error_func(void * _parser_context, const char * error, ...){
		
		va_list ap;
		va_start(ap, error);
		
		//mXMLParser *parser = (XMLParser *)_parser;
		//parser->addParserError(error);
#define ERROR_MSG_BUFFER_SIZE	512
		char buffer[ERROR_MSG_BUFFER_SIZE];// = { '\0' };
		vsnprintf(buffer, ERROR_MSG_BUFFER_SIZE-1, error, ap);
		va_end(ap);
		
		xmlParserCtxt *context = (xmlParserCtxt *)_parser_context;
		XMLParser *parser = (XMLParser *)context->_private;
		string error_string((char *)buffer);
		parser->addParserError(error_string);
		cerr << buffer << endl;
	}
}


void XMLParser::setup(shared_ptr<mwComponentRegistry> _reg, std::string _path){
	path = _path;
	registry = _reg;
	
	LIBXML_TEST_VERSION
	
	context = xmlNewParserCtxt();	
	if(context == NULL){
		throw SimpleException("Failed to create XML Parser Context");
	}
	
	context->_private = (void *)this;
	xmlSetGenericErrorFunc(context, &error_func);
	
	// parse the XSLT simplification transform
	std::string simplification_path = (prependResourcePath(std::string("MWParserTransformation.xsl"))).string();
	simplification_transform = xsltParseStylesheetFile((const xmlChar *)(simplification_path.c_str()));
	
	// parse the file and get the DOM 
	xml_doc = xmlCtxtReadFile(context, path.c_str(), NULL, 0);
	
	errors_doc = NULL;
}

XMLParser::XMLParser(shared_ptr<mwComponentRegistry> _reg, std::string _path){
	
	setup(_reg, _path);
}

XMLParser::XMLParser(std::string _path){
	shared_ptr<mwComponentRegistry> dummy(new mwComponentRegistry());
	
	setup(dummy, _path);
}




XMLParser::~XMLParser() {
	if(xml_doc != NULL){
		xmlFreeDoc(xml_doc);
	}
	
	if(errors_doc != NULL){
		xmlFreeDoc(errors_doc);
	}
	
	if(context != NULL){
		xmlFreeParserCtxt(context);
	}
}


void XMLParser::validate(){ }


void XMLParser::parse(bool announce_progress){ 
	
	if(xml_doc == NULL){
		string complete_message = "The following parser errors were encountered: \n\n";
		
		vector<string>::iterator i;
		for(i = parser_errors.begin(); i != parser_errors.end(); i++){
			complete_message += *i;
		}
		throw MalformedXMLException(complete_message);
	}
	
	
	// TODO: destroy exisiting resources
	
	xmlDoc *simplified = xsltApplyStylesheet(simplification_transform, xml_doc, NULL);
	
	
	//xmlDocDump(stderr, simplified);
	
	xmlNode *root_element = xmlDocGetRootElement(simplified);
	
	// TODO: check the root node to make sure it is okay
	// TODO: pull out the "passthrough" node and send it out as an event
	//		 any errors will be reported with reference id's that are contained
	//		 in this passthrough document
	
	xmlNode *child = root_element->children;
	
	// quickly count the number of nodes
	int n_nodes = 0;
	xmlNode *_child = child;
	while(_child != NULL){
		_child = _child->next;
		n_nodes++;
	}
	
	int counter = 0;
	while(child != NULL){
		experimentLoadProgress->setValue((double)counter / (double)n_nodes);
		//mprintf("progress: %g", (double)(*experimentLoadProgress));
		_processNode(child);
		counter++;
		child = child->next;
	}
	
}


string XMLParser::squashFileName(string name){
	boost::algorithm::replace_all(name, "/", "_");
	boost::algorithm::replace_all(name, "~", "_");
	boost::algorithm::replace_all(name, " ", "__");
	
	return name;
}


void XMLParser::_processNode(xmlNode *child){
	string name((const char *)(child->name));
	
	if(name == "mw_create"){
		_processCreateDirective(child);
	} else if(name == "mw_anonymous_create"){
		_processAnonymousCreateDirective(child);
	} else if(name == "mw_instance"){
		_processInstanceDirective(child);
	} else if(name == "mw_connect"){
		_processConnectDirective(child);
	} else if(name == "mw_range_replicator"){
		_processRangeReplicator(child);
	} else if(name == "mw_list_replicator"){
		_processListReplicator(child);
	} else if(name == "mw_finalize"){
		_processFinalizeDirective(child);
	} else if(name == "variable_assignment"){
		_processVariableAssignment(child);
	} else if(name == "mw_passthrough"){
		
	} else if(name == "text"){
		// do nothing
	}
}


//void XMLParser::_processRangeReplicator(xmlNode *node){
//	
//	string variable(_attributeForName(node, "variable"));
//	string from_string(_attributeForName(node, "from"));
//	string to_string(_attributeForName(node, "to"));
//	string step_string(_attributeForName(node, "step"));
//	
//	double from = boost::lexical_cast<double>(from_string);
//	double to = boost::lexical_cast<double>(to_string);
//	double step = boost::lexical_cast<double>(step_string);
//	
//	int instance_number=0, i = 0;
//	for(double v = from; v <= to; v += step){
//		
//		instance_number = i++;
//		string current_level_instance_id((boost::format("%d")%instance_number).str());
//		
//		char v_char_buffer[1024];
//		sprintf(v_char_buffer, "%g", v);
//		string v_string(v_char_buffer);
//		
//		xmlNode *child = node->children;
//		while(child){
//			string name((const char *)(child->name));
//			if(xmlNodeIsText(child)){
//				child = child->next;
//				continue;
//			}
//			
//			xmlNode *child_copy = xmlCopyNode(child, 1);
//			_substituteAttributeStrings(child_copy, variable, v_string);
//			
//			string reference_id = _attributeForName(child_copy, "reference_id");
//			
//			string instance_id;
//			string node_instance_id(_attributeForName(node, "instance_id"));
//			if(node_instance_id.empty()){
//				instance_id = current_level_instance_id;
//			} else {
//				instance_id = node_instance_id + string(INSTANCE_STEM) + current_level_instance_id;
//			}
//			_setAttributeForName(child_copy, "instance_id", instance_id); 
//			
//			
//			if(name == "mw_instance"){
//				// add variable assignments
//				xmlNode *assignment_node = xmlNewChild(child_copy, NULL, (const xmlChar*)"variable_assignment", (const xmlChar*)v_string.c_str());
//				_setAttributeForName(assignment_node, "variable", variable.c_str());
//				_processNode(child_copy);
//			} else {
//				_processNode(child_copy);
//			}
//			
//			xmlFreeNode(child_copy);
//			child = child->next;
//		}
//		
//	}
//	
//}

void XMLParser::_processRangeReplicator(xmlNode *node){
	
	string variable(_attributeForName(node, "variable"));
	string from_string(_attributeForName(node, "from"));
	string to_string(_attributeForName(node, "to"));
	string step_string(_attributeForName(node, "step"));
	
	double from = boost::lexical_cast<double>(from_string);
	double to = boost::lexical_cast<double>(to_string);
	double step = boost::lexical_cast<double>(step_string);
	
	
	vector<string> values;
	for(double v = from; v <= to; v += step){
		values.push_back(boost::lexical_cast<string>(v));
	}
	
	_createAndAddReplicatedChildren(node, variable, values);
}	

void XMLParser::_processListReplicator(xmlNode *node){
	string variable(_attributeForName(node, "variable"));
	string values_string(_attributeForName(node, "values"));
	
	// following code came from:
	// http://www.boost.org/doc/libs/1_35_0/libs/spirit/example/fundamental/list_parser.cpp
	
	// modified from boost.org to use our strings, etc.)
	std::vector<std::string>    vec_list;
	char const *plist_csv = values_string.c_str();
	parse_info<> result;
	
	// BEGIN from boost.org (I changed nothing)	
    rule<> list_csv, list_csv_item;
    std::vector<std::string> vec_item;
	
    vec_list.clear();
	
    list_csv_item =
	!(
	  confix_p('\"', *c_escape_ch_p, '\"')
	  |   longest_d[real_p | int_p]
	  );
	
    list_csv =
	list_p(
		   list_csv_item[push_back_a(vec_item)],
		   ','
		   )[push_back_a(vec_list)]
	;
	
	// END from boost.org
    
	// modified to contain the correct namespace
	result = boost::spirit::parse (plist_csv, list_csv);	
	
	_createAndAddReplicatedChildren(node, variable, vec_item);
}

void XMLParser::_createAndAddReplicatedChildren(xmlNode *node, 
												const string &variable, 
												const vector<string> &values) {
	int instance_number=0;
	int i = 0;
	for(vector<string>::const_iterator value = values.begin();
		value != values.end();
		++value) {
		
		instance_number = i++;
		string current_level_instance_id((boost::format("%d")%instance_number).str());
		
		xmlNode *child = node->children;
		while(child){
			string name((const char *)(child->name));
			if(xmlNodeIsText(child)){
				child = child->next;
				continue;
			}
			
			xmlNode *child_copy = xmlCopyNode(child, 1);
			_substituteAttributeStrings(child_copy, variable, *value);
			
			string reference_id = _attributeForName(child_copy, "reference_id");
			
			string instance_id;
			string node_instance_id(_attributeForName(node, "instance_id"));
			if(node_instance_id.empty()){
				instance_id = current_level_instance_id;
			} else {
				instance_id = node_instance_id + string(INSTANCE_STEM) + current_level_instance_id;
			}
			_setAttributeForName(child_copy, "instance_id", instance_id); 
			
			
			if(name == "mw_instance" || name == "mw_range_replicator"){  // or range rep?
				// add variable assignments
				_addVariableAssignment(child_copy, variable, *value); 
				_processNode(child_copy);
			} else {
				_processNode(child_copy);
			}
			
			xmlFreeNode(child_copy);
			child = child->next;
		}		
	}
}

void XMLParser::_addVariableAssignment(xmlNode *node, const string& variable, const string& value){
	
	string name((const char *)(node->name));
	if(name == "mw_range_replicator"){
		xmlNode *child = node->children;
		while(child != NULL){
			_addVariableAssignment(child, variable, value);
			child = child->next;
		}
	} else {
		xmlNode *assignment_node = xmlNewChild(node, NULL, (const xmlChar*)"variable_assignment", (const xmlChar*)value.c_str());
		_setAttributeForName(assignment_node, "variable", variable.c_str());
	}
}

void XMLParser::_dumpNode(xmlNode *node){
	
	_xmlAttr *att = node->properties;
	
	while( att != NULL){
		cerr << "\t" << att->name << ":" 
		<< att->children->content << endl;
		
		att = att->next;
	}
}

void XMLParser::_substituteAttributeStrings(xmlNode *node, string token, string replacement){
	
	string prefix("$");
	
	if(xmlNodeIsText(node)){
		string form1 = prefix + token;
		string form2 = prefix + string("{") + token + string("}");
		string content((const char *)xmlNodeGetContent(node));
		boost::replace_all(content, form1, replacement);
		boost::replace_all(content, form2, replacement);
		xmlNodeSetContent(node, (const xmlChar *)(content.c_str()));
		return;
	} else {
		
		xmlNode *child = node->children;
		
		while( child != NULL ){
			
			_substituteAttributeStrings(child, token, replacement);
			child = child->next;
		}
	}
	
}


std::string XMLParser::getWorkingPathString(){
	
	try{
		std::string escaped_path_string = path;
		//boost::replace_all(escaped_path_string, " ", "\\ ");
		boost::filesystem::path the_path(escaped_path_string, boost::filesystem::native);
		boost::filesystem::path the_branch = the_path.branch_path();
		std::string branch_string = the_branch.string();
		std::cerr << "The working path: " << branch_string << endl; 
		
		return branch_string;
		
	} catch (std::exception& e) {
		
		cerr << "Path no good:" << path << ":" << e.what() << endl;
	}
	
	return "";
}

void XMLParser::_processFinalizeDirective(xmlNode *node){
	
	string tag(_attributeForName(node, "object"));
	string reference_id(_attributeForName(node, "reference_id"));
	string instance_id(_attributeForName(node, "instance_id"));
	
	if(instance_id.empty()){
		instance_id = "0";
	}
	
	map<string, string> properties = _createPropertiesMap(node);
	
	shared_ptr<mw::Component> object; 
	
	// Try to lookup by instance
	string instance_tag = _generateInstanceTag(tag, reference_id, instance_id);
	object = registry->getObject<mw::Component>(instance_tag);
	string used_tag = instance_tag;
	
	// Try to lookup by tag
	if(object == NULL){
		object = registry->getObject<mw::Component>(tag);
		used_tag = tag;
	}
	
	// If that fails, try by reference id
	if(object == NULL){
		object = registry->getObject<mw::Component>(reference_id);
		used_tag = reference_id;
	}
	
	
	if(object == NULL && properties.find("alt") != properties.end()){
		object = registry->getObject<mw::Component>(properties["alt"]);
		if(object != NULL){
			registry->registerObject(tag, object, true);
		}
	}
	
	if(object != NULL){
		object->finalize(properties, registry.get());
		//mprintf(M_PARSER_MESSAGE_DOMAIN,
		//				"Finalized object \"%s\"", used_tag.c_str());
	} else {
		// TODO: throw
		throw InvalidXMLException(reference_id, 
								  "Failed to finalize object", tag);
	}
	
	
}


void XMLParser::_processAnonymousCreateDirective(xmlNode *node){
	_processGenericCreateDirective(node,1);
}

void XMLParser::_processCreateDirective(xmlNode *node){
	_processGenericCreateDirective(node, 0);
}

void XMLParser::_processGenericCreateDirective(xmlNode *node, bool anon){
	string object_type(_attributeForName(node, "object"));
	string reference_id(_attributeForName(node, "reference_id"));
    string parent_scope(_attributeForName(node, "parent_scope"));
	
	// Create an STL map containing all properties contained under this
	// node
	map<string, string> properties = _createPropertiesMap(node);
	
    string tag;
    if(parent_scope.empty()){
        tag = properties["tag"];
    } else {
        tag = parent_scope + "/" + properties["tag"];
        properties["parent_scope"] = parent_scope;
    }
	
	// Add XML document information to this properties map
	// (for instance, if you need to know the working path...)
	properties["xml_document_path"] = path;
	
	std::string branch_string = getWorkingPathString();
	properties["working_path"] = branch_string.c_str();
	
	
	// Concatenate a type identifier to the object type string
	// e.g. if type = "itc18", then object_type would be iodevice/itc18
	if(!(properties["type"].empty())){
		object_type += "/" + boost::to_lower_copy(properties["type"]);
	}
	
	
	shared_ptr<mw::Component> component;
	
	try {
		// Create a new object and register it using it's reference id.
		// this id is generated by the xslt transformation, and is
		// guaranteed to be unique within the XML document
		component = registry->registerNewObject(reference_id, object_type, properties);
		
		// Also register the object using it's tag.  These are usually unique,
		// but are sometimes missing or duplicated (e.g. task system states can
		// have the same names in different task systems, while remaining valid
		// If the create directive is "anonymous," then ignore the tag.  See 
		// comments for _procressAnonymousCreateDirective.
		if(!anon && !tag.empty()){
			registry->registerObject(tag, component);
		}
		
	} catch(SimpleException& e){
		
		// Objects are allowed to indicate an alternative object to use if 
		// the create directive fails (this is common with IO Devices, which 
		// will fail to create if the device is not currently connected.
		// Alt substitution is the responsibility of the object class in question,
		// and is usually handled in a "finalize" directive
		if(properties.find("alt") != properties.end()){
			mwarning(M_PARSER_MESSAGE_DOMAIN,
					 "Failed to create object \"%s\" of type %s (but alt object is specified)",
					 tag.c_str(), object_type.c_str());
		} else {
			merror(M_PARSER_MESSAGE_DOMAIN, 
				   "Failed to create object \"%s\" of type %s (error was:\"%s\")", tag.c_str(), object_type.c_str(), e.what());
			throw InvalidXMLException(reference_id, (const SimpleException&)e);
		}
	}
	
	if(component == NULL){
		mwarning(M_PARSER_MESSAGE_DOMAIN, 
				 "Failed to create object \"%s\" of type %s", tag.c_str(), object_type.c_str());
		// TODO: throw
	} else {
		component->setReferenceID(reference_id);
		//mprintf(M_PARSER_MESSAGE_DOMAIN,
		//				"Created object \"%s\" of type %s", tag.c_str(), object_type.c_str());
	}
}



string XMLParser::_generateInstanceTag(string tag, string reference_id, string instance_id){
	string instance_tag = string(INSTANCE_PREFIX) + reference_id + string(INSTANCE_STEM) + instance_id; 
	return instance_tag;
}


void XMLParser::_processInstanceDirective(xmlNode *node){
	string tag(_attributeForName(node, "object"));
	string reference_id(_attributeForName(node, "reference_id"));
	
	// "Instances" have an id associated with them. When a replicator
	string instance_id(_attributeForName(node, "instance_id"));
	if(instance_id.empty()){
		instance_id = "0";
	}
	
	// Try to look up the object by the provided tag
	shared_ptr<mw::Component> object = registry->getObject<mw::Component>(tag);
	
	// If that fails, look it up by its reference_id
	if(object == NULL){
		// "Anonymous" style name
		object = registry->getObject<mw::Component>(reference_id);
	}
	
	// If the object is still unfound, something is very wrong
	if(object == NULL){
		throw  InvalidXMLException(reference_id,
								   "Unknown object during aliasing phase",tag);
	}
	
	// Create an "instance" of the object.  What this means will vary from
	// object to object.  In the case of paradigm components (e.g. block, trial)
	// this produces a shallow copy with it's own separate variable context and
	// selection machinery, but with children, etc. shared with the original
	shared_ptr<mw::Component> alias = object->createInstanceObject();
	string instance_tag = _generateInstanceTag(tag, reference_id, instance_id);
	
	// If the object is a valid variable environment, we'll apply variable
	// assignments if there are any
	// Look for variable assignments if appropriate
	shared_ptr<ScopedVariableEnvironment> env = dynamic_pointer_cast<ScopedVariableEnvironment, mw::Component>(alias);
	if(env != NULL){
		
		xmlNode *alias_child = node->children;
		
		while(alias_child != NULL){
			string child_name((const char *)alias_child->name);
			
			if(child_name == "variable_assignment"){
				string variable_name = _attributeForName(alias_child, "variable");
				shared_ptr<Variable> var = registry->getVariable(variable_name);
				string content((const char *)xmlNodeGetContent(alias_child));
				Data value = registry->getNumber(content);
				
				if(var == NULL){
					// TODO: better throw
					throw InvalidXMLException(reference_id,
											  "Invalid variable", variable_name);
				}
				
				if(value.isUndefined()){
					// TODO: better throw
					throw InvalidXMLException(reference_id,
											  "Invalid value", content);
				} 
				
				shared_ptr<ScopedVariable> svar = dynamic_pointer_cast<ScopedVariable, Variable>(var);
				
				if(svar == NULL){
					// TODO: better throw
					throw InvalidXMLException(reference_id,
											  "Cannot assign a non-local variable", variable_name);
				} 
				
				env->setValue(svar->getContextIndex(), value);
				//mprintf(M_PARSER_MESSAGE_DOMAIN, "Assigned variable %s to value %s in the context of %s",
				//						variable_name.c_str(), content.c_str(), instance_tag.c_str());
				
			}
			
			alias_child = alias_child->next;
		}
		
	}
	
	registry->registerObject(instance_tag, alias);
	
	// Check to be sure that the object was registered properly
	shared_ptr<mw::Component> test = registry->getObject<mw::Component>(instance_tag);
	
	if(alias.get() != test.get()){
		throw  InvalidXMLException(reference_id,
								   "Failed to register component", instance_tag);
		return;
	}
	
#define VERBOSE_PARSER	1
#ifdef VERBOSE_PARSER
	//mprintf(M_PARSER_MESSAGE_DOMAIN,
	//			"Created an alias (%s) of object %s", instance_tag.c_str(), tag.c_str());
#endif
}


shared_ptr<mw::Component> XMLParser::_getConnectionChild(xmlNode *child){
	
	//string child_name((const char *)child->name);
	string child_tag(_attributeForName(child, "tag"));
	string child_reference_id(_attributeForName(child, "reference_id"));
	string child_instance_id(_attributeForName(child, "instance_id"));
	if(child_instance_id.empty()){
		child_instance_id = "0";
	}
	
	// First look up if there is an "instanced" version of this object
	// as these versions always have priority if they exist
	string child_instance_tag = _generateInstanceTag(child_tag, child_reference_id, child_instance_id);
	
	shared_ptr<mw::Component> child_component;
	child_component = registry->getObject<mw::Component>(child_instance_tag);
	
	// if it exists, return it
	if(child_component != NULL){
		//mprintf(M_PARSER_MESSAGE_DOMAIN, "\tretrieving child %s", child_instance_tag.c_str());
		return child_component;
	}
	
	//mprintf("failed to find child %s", child_instance_tag.c_str());
	
	// Second, try to look up the object using its tag
	child_component = registry->getObject<mw::Component>(child_tag);
	
	if(child_component != NULL && !(child_component->isAmbiguous())){
		//mprintf(M_PARSER_MESSAGE_DOMAIN, "\tretrieving child %s", child_tag.c_str());
		return child_component;
	}
	
	
	// Finally, try to look up the object using its reference_id
	child_component = registry->getObject<mw::Component>(child_reference_id);
	//mprintf(M_PARSER_MESSAGE_DOMAIN, "\tretrieving child %s", child_reference_id.c_str());
	
	if(child_component == NULL){
		// do nothing
	}
	
	return child_component;
}


void XMLParser::_connectChildToParent(shared_ptr<mw::Component> parent, 
									  map<string, string> properties, 
									  xmlNode *child_node){
	
	string child_name((const char *)child_node->name);
	string child_reference_id(_attributeForName(child_node, "reference_id"));
	string child_instance_id(_attributeForName(child_node, "instance_id"));
	string child_tag(_attributeForName(child_node, "tag"));
	string parent_tag = properties["tag"];
	
	// if the node is a replicator, we'll recursively call this method
	// on each of the replicator's children
	if(child_name == "mw_range_replicator" || child_name == "mw_list_replicator"){
		string variable(_attributeForName(child_node, "variable"));
		
		if(child_name == "mw_range_replicator") {
			string from_string(_attributeForName(child_node, "from"));
			string to_string(_attributeForName(child_node, "to"));
			string step_string(_attributeForName(child_node, "step"));
			
			double from = boost::lexical_cast<double>(from_string);
			double to = boost::lexical_cast<double>(to_string);
			double step = boost::lexical_cast<double>(step_string);
			
			vector<string> values;
			for(double v = from; v <= to; v += step){
				values.push_back(boost::lexical_cast<string>(v));
			}
			
			_createAndConnectReplicatedChildren(parent, 
												properties,
												child_node,
												child_instance_id,
												variable,
												values);
		} else { // "mw_list_replicator"
			string values_string(_attributeForName(child_node, "values"));
			
			// following code came from:
			// http://www.boost.org/doc/libs/1_35_0/libs/spirit/example/fundamental/list_parser.cpp
			
			// modified from boost.org to use our strings, etc.)
			std::vector<std::string>    vec_list;
			char const *plist_csv = values_string.c_str();
			parse_info<> result;
			
			// BEGIN from boost.org (I changed nothing)	
			rule<> list_csv, list_csv_item;
			std::vector<std::string> vec_item;
			
			vec_list.clear();
			
			list_csv_item =
			!(
			  confix_p('\"', *c_escape_ch_p, '\"')
			  |   longest_d[real_p | int_p]
			  );
			
			list_csv =
			list_p(
				   list_csv_item[push_back_a(vec_item)],
				   ','
				   )[push_back_a(vec_list)]
			;
			
			// END from boost.org
			
			// modified to contain the correct namespace
			result = boost::spirit::parse (plist_csv, list_csv);	
			
			_createAndConnectReplicatedChildren(parent, 
												properties,
												child_node,
												child_instance_id,
												variable,
												vec_item);
		}
		// Otherwise, just connect the child to parent
	} else {
		
		shared_ptr<mw::Component> child_component = _getConnectionChild(child_node);
		
		if(child_component != NULL){
			parent->addChild(properties, registry.get(), child_component);
			//mprintf(M_PARSER_MESSAGE_DOMAIN,
			//			"Connected child (%s / %s) to parent %s", child_tag.c_str(), 
			//										child_reference_id.c_str(),
			//										parent_tag.c_str());
		} else {
			string message((boost::format("Could not find child (%s) to connect to parent (%s)") % child_tag % parent_tag).str());
			//throw InvalidXMLException(child_reference_id, message);
			
			// is this warning necessary?
			// mwarning(M_PARSER_MESSAGE_DOMAIN, message.c_str());
		}
	}
}


void XMLParser::_createAndConnectReplicatedChildren(shared_ptr<mw::Component> parent, 
													map<string, string> properties,
													xmlNode *child_node,
													const string &child_instance_id,
													const string &variable, 
													const vector<string> &values) {
	int i = 0;
	for(vector<string>::const_iterator value = values.begin();
		value != values.end();
		++value) {
		
		int instance_number = i++;
		string current_level_instance_id((boost::format("%d") % instance_number).str());
		
		string subchild_instance_id;
		if(child_instance_id.empty()){
			subchild_instance_id = current_level_instance_id;
		} else {
			subchild_instance_id = child_instance_id;
			subchild_instance_id += string(INSTANCE_STEM) + current_level_instance_id;
		}
		
		xmlNode *subchild = child_node->children;
		
		while(subchild != NULL){
			
			xmlNode *subchild_copy = xmlCopyNode(subchild,1);
			
			// perform substitution in the variable's tag, if necessary
			string prefix("$");
			string form1 = prefix + variable;
			string form2 = prefix + string("{") + variable + string("}");
			string content = _attributeForName(subchild, "tag");
			boost::replace_all(content, form1, *value);
			boost::replace_all(content, form2, *value);
			_setAttributeForName(subchild_copy, "tag", content);
			
			_setAttributeForName(subchild_copy, "instance_id", subchild_instance_id);
			
			_connectChildToParent(parent, properties, subchild_copy);
			
			xmlFreeNode(subchild_copy);
			subchild = subchild->next;
		}
	}
}

void XMLParser::_processConnectDirective(xmlNode *node){
	
	string parent_tag(_attributeForName(node, "parent"));
	string reference_id(_attributeForName(node, "reference_id"));
    string parent_scope(_attributeForName(node, "parent_scope"));
    
	map<string, string> properties;
	
    if(parent_scope.empty()){
        properties["parent_tag"] = parent_tag;
    } else {
        properties["parent_tag"] = parent_scope + "/" + parent_tag;
    }
	properties["parent_reference_id"] = reference_id;
	
	xmlNode *child = node->children;
	
	shared_ptr<mw::Component> parent_component;
	
	// Try to look up object with its tag
    // DDC swapped: try the ref id first
	parent_component = registry->getObject<mw::Component>(parent_tag);
	//parent_component = registry->getObject<mw::Component>(reference_id);
    
	// If that didn't work, try using the reference id
	if(parent_component == NULL){
		parent_component = registry->getObject<mw::Component>(reference_id);
        //parent_component = registry->getObject<mw::Component>(parent_tag);
	}
	
	if(parent_component == NULL){
		// error (should eventually throw)
		//		throw SimpleException("Unknown object during connection phase",
		//														parent_tag);
		return;
	}
	
	while(child != NULL){
		
		_connectChildToParent(parent_component, properties, child);		
		child = child->next;
	}
	
}





string XMLParser::_attributeForName(xmlNode *node, string name){
	// Parse the attributes
	_xmlAttr *att = node->properties;
	
	while( att != NULL){
		
		if(name == (const char *)(att->name)){
			return string((const char *)(att->children->content));
		}			
		att = att->next;
	}
	
	return string();
}

void XMLParser::_setAttributeForName(xmlNode *node, string name, string value){
	
	//	_xmlAttr *att = node->properties;
	
	xmlSetProp(node, (const xmlChar *)name.c_str(), (const xmlChar *)value.c_str());
}



map<string, string> XMLParser::_createPropertiesMap(xmlNode *node){
	
	map<string, string> returnmap;
	
	xmlNode *child = node->children;
	
	while(child != NULL){
		string name((const char *)child->name);
		if(!xmlNodeIsText(child)){
			returnmap[name] = (const char *)(child->children->content);
		}
		child = child->next;
	}
	
	return returnmap;
}


vector<xmlNode *> XMLParser::_getChildren(xmlNode *node){
	string empty("");
	return _getChildren(node, empty);
}

vector<xmlNode *> XMLParser::_getChildren(xmlNode *node, string tag){
	
	vector<xmlNode *> children;
	
	xmlNode *child = node->children;
	
	while(child != NULL){
		// TODO: check to only take nodes, not text and stuff
		
		if( !xmlNodeIsText(child) &&
		   (tag.empty() || tag == (const char *)child->name)){
			children.push_back(child);
		}
		child = child->next;
	}
	
	return children;
}

string XMLParser::_getContent(xmlNode *node){
	
	string returnval((const char *)xmlNodeGetContent(node));
	return returnval;
}


Data XMLParser::_parseDataValue(xmlNode *node){
	
	// if node has "value" field, just use that
	// call getValue() to convert the string into a datum
	string value_field_contents = _attributeForName(node, "value");
	string type_string = _attributeForName(node, "type");
	string reference_id = _attributeForName(node, "reference_id");
	if(reference_id.empty()){
		reference_id = "unknown";
	}
	
	GenericDataType type = M_UNDEFINED;
	
	// TODO standardize this
	if(type_string == "integer"){
		type = M_INTEGER;
	} else if(type_string == "float"){
		type = M_FLOAT;
	} else if(type_string == "boolean"){
		type = M_BOOLEAN;
	} else if (type_string == "string"){
		type = M_STRING;
	}
	
	if(value_field_contents.empty() == false){
		
		if(type == M_STRING){
			return Data(value_field_contents);
		} else if(type != M_UNDEFINED){
			return registry->getNumber(value_field_contents, type);
		} else {
			return Data(value_field_contents);
		}
	}
	
	// otherwise, look into the contents of the node
	vector<xmlNode *> children = _getChildren(node);
	
	if(children.size() == 0){
		if(type != M_UNDEFINED && type != M_STRING){
			return registry->getNumber(_getContent(node), type);
		} else {
			return Data(_getContent(node));
		}
	}
	
	if(children.size() > 1){
		// TODO: better throw
		throw InvalidXMLException(reference_id, "Invalid variable assignment");
	}
	
	xmlNode *child = children[0];
	
	string child_name((const char *)child->name);
	
	// if there is a <dictionary> node inside
	if(child_name == "dictionary"){
		
		vector<xmlNode *> elements = _getChildren(child, "dictionary_element");
		
		
		vector<Data> keys;
		vector<Data> values;
		
		for(vector<xmlNode *>::iterator el = elements.begin();
			el != elements.end(); 
			++el) {
			vector<xmlNode *> key_nodes = _getChildren(*el, "key");
			if(key_nodes.size() != 1){
				// TODO: better throw
				throw InvalidXMLException(reference_id,
										  "Cannot have more than one key in a dictionary element");
			}
			
			xmlNode *key_node = key_nodes[0];
			
			vector<xmlNode *> value_nodes = _getChildren(*el, "value");
			if(value_nodes.size() != 1){
				throw InvalidXMLException(reference_id,
										  "Cannot have more than one value in a dictionary element");
			}
			
			xmlNode *value_node = value_nodes[0];
			Data value_data = _parseDataValue(value_node);
			values.push_back(value_data);
			Data key_data = _parseDataValue(key_node);
			keys.push_back(key_data);
		}
		
		Data dictionary(M_DICTIONARY, (int)keys.size());
		
		vector<Data>::iterator k = keys.begin();
		vector<Data>::iterator v = values.begin();
		while(k != keys.end() && v != values.end()){
			dictionary.addElement(*k, *v);
			k++; v++;
		}
		
		return dictionary;
		
		// if there is a <list> node inside
	} else if(child_name == "list_data"){
		
		vector<xmlNode *> elements = _getChildren(child, "list_element");
		
		vector<Data> values;
		
		
		for(vector<xmlNode *>::iterator el = elements.begin(); 
			el != elements.end(); 
			++el ){
			vector<xmlNode *> value_nodes = _getChildren(*el, "value");
			if(value_nodes.size() != 1){
				throw InvalidXMLException(reference_id,
										  "Cannot have more than one value in a list element");
			}
			
			xmlNode *value_node = value_nodes[0];
			Data value_data = _parseDataValue(value_node);
			values.push_back(value_data);
		}
		
		Data list(M_LIST, (int)values.size());
		
		vector<Data>::iterator v = values.begin();
		while(v != values.end()){
			list.addElement(*v);
			v++;
		}
		
		return list;
		
		// if there is an <integer> node inside
	} else if(child_name == "integer"){
		Data value = registry->getNumber(_getContent(node));
		
		Data transformed(value.getInteger());
		
		return transformed;
		
	} else if(child_name == "float"){
		
		Data value = registry->getNumber(_getContent(node));
		
		Data transformed(value.getFloat());
		
		return transformed;
		
	} else {
		// TODO: better throw
		throw InvalidXMLException(reference_id, "Unknown data type", child_name);
	}
}


void XMLParser::_processVariableAssignment(xmlNode *node){
	
	string variable_name(_attributeForName(node, "variable"));
	
	shared_ptr<Variable> variable = registry->getVariable(variable_name);
	
	if(variable == NULL){
		// TODO: better throw
		if(variable_name.empty()){
			throw SimpleException("Variable missing tag attribute in variable assignment");
		} else {
			throw SimpleException("Unknown variable during variable assignment", variable_name);
		}
	}
	
	Data value = _parseDataValue(node);
	
	variable->setValue(value);
}

