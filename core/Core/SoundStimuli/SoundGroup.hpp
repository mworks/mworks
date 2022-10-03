//
//  SoundGroup.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 10/4/22.
//

#ifndef SoundGroup_hpp
#define SoundGroup_hpp

#include <variant>

#include "ExpressionParser.h"
#include "Sound.h"


BEGIN_NAMESPACE_MW


class SoundGroup;
using SoundGroupPtr = boost::shared_ptr<SoundGroup>;


class SoundGroup : public Component {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit SoundGroup(const ParameterValueMap &parameters);
    
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistryPtr reg,
                  ComponentPtr child) override;
    
    SoundPtr getSound(Datum index) const;
    SoundGroupPtr getSubGroup(Datum index) const;
    
private:
    using Child = std::variant<SoundPtr, SoundGroupPtr>;
    
    const Child * getChild(Datum index) const;
    
    std::vector<Child> children;
    
};


class SoundGroupReferenceSound : public Sound {
    
public:
    SoundGroupReferenceSound(const std::string &expr, ComponentRegistryPtr reg);
    
    void load() override;
    void unload() override;
    
    void play() override;
    void play(MWTime startTime) override;
    void pause() override;
    void stop() override;
    
private:
    SoundPtr getSound() const;
    
    SoundGroupPtr group;
    stx::ParseTreeList indexExprs;
    
};


END_NAMESPACE_MW


#endif /* SoundGroup_hpp */
