//
//  SoundGroup.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 10/4/22.
//

#include "SoundGroup.hpp"

#include "ExpressionVariable.h"


BEGIN_NAMESPACE_MW


void SoundGroup::describeComponent(ComponentInfo &info) {
    Component::describeComponent(info);
    info.setSignature("sound_group");
}


SoundGroup::SoundGroup(const ParameterValueMap &parameters) :
    Component(parameters)
{ }


void SoundGroup::addChild(std::map<std::string, std::string> parameters,
                          ComponentRegistryPtr reg,
                          ComponentPtr child)
{
    if (auto sound = boost::dynamic_pointer_cast<Sound>(child)) {
        children.emplace_back(std::move(sound));
    } else if (auto subGroup = boost::dynamic_pointer_cast<SoundGroup>(child)) {
        children.emplace_back(std::move(subGroup));
    } else {
        throw SimpleException("Sound group can contain only sounds and other sound groups");
    }
}


SoundPtr SoundGroup::getSound(Datum index) const {
    if (auto child = getChild(index)) {
        if (auto sound = std::get_if<SoundPtr>(child)) {
            return *sound;
        }
        merror(M_SYSTEM_MESSAGE_DOMAIN, "Sound group element at index %lld is not a sound", index.getInteger());
    }
    return SoundPtr();
}


SoundGroupPtr SoundGroup::getSubGroup(Datum index) const {
    if (auto child = getChild(index)) {
        if (auto subGroup = std::get_if<SoundGroupPtr>(child)) {
            return *subGroup;
        }
        merror(M_SYSTEM_MESSAGE_DOMAIN, "Sound group element at index %lld is not a sub group", index.getInteger());
    }
    return SoundGroupPtr();
}


auto SoundGroup::getChild(Datum index) const -> const Child * {
    auto indexValue = index.getInteger();
    if (indexValue >= 0 && indexValue < children.size()) {
        return &(children.at(indexValue));
    }
    merror(M_SYSTEM_MESSAGE_DOMAIN, "Sound group index is out of bounds: %lld", indexValue);
    return nullptr;
}


SoundGroupReferenceSound::SoundGroupReferenceSound(const std::string &expr, ComponentRegistryPtr reg) {
    std::string groupName;
    indexExprs = ParsedExpressionVariable::parseVarnameWithSubscripts(expr, groupName);
    if (indexExprs.empty()) {
        throw SimpleException("Invalid sound group indexing expression", expr);
    }
    
    group = reg->getObject<SoundGroup>(groupName);
    if (!group) {
        throw SimpleException("Unknown sound group", groupName);
    }
    
    // Try evaluating the index expressions, hopefully catching any critical errors
    std::vector<Datum> indexes;
    ParsedExpressionVariable::evaluateParseTreeList(indexExprs, indexes);
}


void SoundGroupReferenceSound::load() {
    if (auto sound = getSound()) {
        sound->load();
    }
}


void SoundGroupReferenceSound::unload() {
    if (auto sound = getSound()) {
        sound->unload();
    }
}


void SoundGroupReferenceSound::play() {
    if (auto sound = getSound()) {
        sound->play();
    }
}


void SoundGroupReferenceSound::play(MWTime startTime) {
    if (auto sound = getSound()) {
        sound->play(startTime);
    }
}


void SoundGroupReferenceSound::pause() {
    if (auto sound = getSound()) {
        sound->pause();
    }
}


void SoundGroupReferenceSound::stop() {
    if (auto sound = getSound()) {
        sound->stop();
    }
}


SoundPtr SoundGroupReferenceSound::getSound() const {
    std::vector<Datum> indexes;
    ParsedExpressionVariable::evaluateParseTreeList(indexExprs, indexes);
    
    // All indexes prior to the last must refer to a group
    auto targetGroup = group;
    for (std::size_t i = 0; i < indexes.size() - 1; i++) {
        targetGroup = targetGroup->getSubGroup(indexes.at(i));
        if (!targetGroup) {
            return SoundPtr();
        }
    }
    
    // The last index must refer to a sound
    return targetGroup->getSound(indexes.back());
}


END_NAMESPACE_MW
