/*
 *  ___PROJECTNAMEASIDENTIFIER___.h
 *  ___PROJECTNAME___
 *
 *  Created by ___FULLUSERNAME___ on ___DATE___.
 *  Copyright ___YEAR___ ___ORGANIZATIONNAME___. All rights reserved.
 *
 */

#ifndef ___PROJECTNAMEASIDENTIFIER____H_
#define ___PROJECTNAMEASIDENTIFIER____H_

#include <MWorksCore/Component.h>
#include <MWorksCore/GenericVariable.h>

using namespace mw;


class ___PROJECTNAMEASIDENTIFIER___ : public mw::Component {

public:
    ___PROJECTNAMEASIDENTIFIER___(const std::string &tag, shared_ptr<Variable> anotherAttribute);

    virtual ~___PROJECTNAMEASIDENTIFIER___();
    
private:
    ___PROJECTNAMEASIDENTIFIER___(const ___PROJECTNAMEASIDENTIFIER___ &other);
    
    shared_ptr<Variable> anotherAttribute;

};


#endif
