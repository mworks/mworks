//
//  PythonFileResource.hpp
//  PythonPlugin
//
//  Created by Christopher Stawarz on 10/18/17.
//  Copyright © 2017 MWorks Project. All rights reserved.
//

#ifndef PythonFileResource_hpp
#define PythonFileResource_hpp


BEGIN_NAMESPACE_MW


class PythonFileResource : public Component {
    
public:
    static const std::string PATH;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit PythonFileResource(const ParameterValueMap &parameters);
    
private:
    const boost::filesystem::path path;
    
};


END_NAMESPACE_MW


#endif /* PythonFileResource_hpp */
