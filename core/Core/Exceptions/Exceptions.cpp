#include "Exceptions.h"
#include "Component.h"
#include <boost/variant/visitor_ptr.hpp>
#include "Stimulus.h"
#include "GenericVariable.h"


BEGIN_NAMESPACE_MW


AmbiguousComponentReferenceException::AmbiguousComponentReferenceException(shared_ptr<AmbiguousComponentReference> ref) :
    SimpleException(M_PARSER_MESSAGE_DOMAIN,
                    "An error has occurred because two objects are referenced by the same name.  "
                    "Please ensure that all object tag names are unique."),
    component_reference(ref){ 
    
    std::stringstream extended_info;
    extended_info << std::endl;
    
    const vector< shared_ptr<Component> > &components = component_reference->getAmbiguousComponents();
    vector< shared_ptr<Component> >::const_iterator i;
    extended_info << "Conflicts: " << std::endl; 
    for(i = components.begin(); i != components.end(); i++){
        if(*i == NULL){
            extended_info << "NULL object (this is a sign of a serious bug)";
            continue;
        }
        
        extended_info << "tag = " << (*i)->getTag() << ", ";
        
        extended_info << "reference_id = ";
        string reference_id = (*i)->getReferenceID();
        if(!reference_id.empty()){
            extended_info << reference_id << ", ";
        } else {
            extended_info << "<unknown> ";
        }
        
        extended_info << "object signature = ";
        string object_signature = (*i)->getObjectSignature();
        if(!object_signature.empty()){
            extended_info << object_signature;
        } else {
            extended_info << "<not set>";
        }
        
        extended_info << std::endl;
    }
    
    message += extended_info.str();
}
    


   
string to_string_visitor::operator()(shared_ptr<mw::Component> operand) const{
    string tag;
    
    if(operand != NULL){
       tag = operand->getTag(); // TODO: try/catch
    }
    
    if(tag.empty()){
        tag = "<unknown component>";
    }
    
    return tag;
}

string to_string_visitor::operator()(shared_ptr<mw::Stimulus> operand)  const{
    return this->operator()(boost::dynamic_pointer_cast<mw::Component, mw::Stimulus>(operand) );
}

string to_string_visitor::operator()(shared_ptr<mw::StimulusGroup> operand)  const{
    return this->operator()(boost::dynamic_pointer_cast<mw::Component, mw::StimulusGroup>(operand) );
}

string to_string_visitor::operator()(shared_ptr<mw::Variable> operand)  const{
    return this->operator()(boost::dynamic_pointer_cast<mw::Component, mw::Variable>(operand) );
}


#define CHECK_AND_REPORT_STRING_ERROR_INFORMATION(TOKEN, TYPE)                             \
    if( boost::get_error_info< ERROR_INFO_OBJECT(TOKEN) >(e) != NULL ){                    \
        TYPE TOKEN ## _tmp = *(boost::get_error_info< ERROR_INFO_OBJECT(TOKEN) >(e));  \
        merror(e.getDomain(), "\t%s: %s", #TOKEN, TOKEN ## _tmp.c_str());                            \
    }    


#define CHECK_AND_REPORT_ERROR_INFORMATION(TOKEN, TYPE)                                                 \
    if( boost::get_error_info< ERROR_INFO_OBJECT(TOKEN) >(e) != NULL ){                                                \
        TYPE TOKEN ## _tmp = *(boost::get_error_info< ERROR_INFO_OBJECT(TOKEN) >(e));                                  \
        string result = apply_visitor(to_string_visitor(), TOKEN ## _tmp);                                       \
        if(!result.empty()){                                                           \
            extended_info << "\t" << #TOKEN << ": " << result << endl;                              \
        }                                                                               \
    } 


void display_extended_error_information(SimpleException& e){
    
    // Display the primary message
    
    std::stringstream extended_info;
    extended_info << e.what() << endl;
        
    using namespace mw::error_info_types;
    
    extended_info << "Extended information:" << endl;
    CHECK_AND_REPORT_ERROR_INFORMATION( reason, ErrorString);
    CHECK_AND_REPORT_ERROR_INFORMATION( additional_msg, ErrorString);
    CHECK_AND_REPORT_ERROR_INFORMATION( object_type, ErrorString);
    CHECK_AND_REPORT_ERROR_INFORMATION( parent_scope, ErrorString);
    CHECK_AND_REPORT_ERROR_INFORMATION( ref_id, ErrorString);
    CHECK_AND_REPORT_ERROR_INFORMATION( component, ComponentOrString );
    CHECK_AND_REPORT_ERROR_INFORMATION( parent_component, ComponentOrString );
    CHECK_AND_REPORT_ERROR_INFORMATION( child_component, ComponentOrString );
    CHECK_AND_REPORT_ERROR_INFORMATION( variable, VariableOrString );
    CHECK_AND_REPORT_ERROR_INFORMATION( parser_context, ErrorString );
    
    merror(e.getDomain(), extended_info.str());

}


END_NAMESPACE_MW
