#include "Exceptions.h"
#include "Component.h"
#include <boost/variant/visitor_ptr.hpp>
#include "Stimulus.h"
#include "GenericVariable.h"

using namespace mw;
using namespace boost;

namespace mw{
   
string to_string_visitor::operator()(shared_ptr<mw::Component> operand) const{
    string tag = operand->getTag(); // TODO: try/catch
    if(tag.empty()){
        tag = "<unknown component>";
    }
    
    return tag;
}

string to_string_visitor::operator()(shared_ptr<mw::Stimulus> operand)  const{
    return this->operator()(dynamic_pointer_cast<mw::Component, mw::Stimulus>(operand) );
}

string to_string_visitor::operator()(shared_ptr<mw::StimulusGroup> operand)  const{
    return this->operator()(dynamic_pointer_cast<mw::Component, mw::StimulusGroup>(operand) );
}

string to_string_visitor::operator()(shared_ptr<mw::Variable> operand)  const{
    return this->operator()(dynamic_pointer_cast<mw::Component, mw::Variable>(operand) );
}


#define CHECK_AND_REPORT_STRING_ERROR_INFORMATION(TOKEN, TYPE)                             \
    if( get_error_info< ERROR_INFO_OBJECT(TOKEN) >(e) != NULL ){                    \
        TYPE TOKEN ## _tmp = *(get_error_info< ERROR_INFO_OBJECT(TOKEN) >(e));  \
        merror(e.getDomain(), "\t%s: %s", #TOKEN, TOKEN ## _tmp.c_str());                            \
    }    


#define CHECK_AND_REPORT_ERROR_INFORMATION(TOKEN, TYPE)                                                 \
    if( get_error_info< ERROR_INFO_OBJECT(TOKEN) >(e) != NULL ){                                                \
        TYPE TOKEN ## _tmp = *(get_error_info< ERROR_INFO_OBJECT(TOKEN) >(e));                                  \
        string result = apply_visitor(to_string_visitor(), TOKEN ## _tmp);                                       \
        if(!result.empty()){                                                           \
            merror(e.getDomain(), "\t%s: %s", #TOKEN, result.c_str());                              \
        }                                                                               \
    } 


void display_extended_error_information(SimpleException& e){
    
    // Display the primary message
    merror(e.getDomain(), e.what());
    
    
    using namespace mw::error_info_types;
    
    CHECK_AND_REPORT_ERROR_INFORMATION( reason, ErrorString);
    CHECK_AND_REPORT_ERROR_INFORMATION( additional_msg, ErrorString);
    CHECK_AND_REPORT_ERROR_INFORMATION( object_type, ErrorString);
    CHECK_AND_REPORT_ERROR_INFORMATION( parent_scope, ErrorString);
    CHECK_AND_REPORT_ERROR_INFORMATION( ref_id, ErrorString);
    CHECK_AND_REPORT_ERROR_INFORMATION( parent_component, ComponentOrString );
    CHECK_AND_REPORT_ERROR_INFORMATION( child_component, ComponentOrString );
    CHECK_AND_REPORT_ERROR_INFORMATION( variable, VariableOrString );
    CHECK_AND_REPORT_ERROR_INFORMATION( parser_context, ErrorString );
    
}

}