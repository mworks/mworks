//
//  StandardSoundFactory.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/3/22.
//

#ifndef StandardSoundFactory_hpp
#define StandardSoundFactory_hpp

#include "Sound.h"
#include "StandardComponentFactory.h"


BEGIN_NAMESPACE_MW


template<typename SoundType>
class StandardSoundFactory : public StandardComponentFactory<SoundType> {
    
public:
    boost::shared_ptr<Component> createObject(ComponentFactory::StdStringMap parameters,
                                              ComponentRegistryPtr reg) override
    {
        auto sound = boost::dynamic_pointer_cast<Sound>(StandardComponentFactory<SoundType>::createObject(parameters, reg));
        if (sound->shouldAutoload()) {
            sound->load();
        }
        return sound;
    }
    
};


END_NAMESPACE_MW


#endif /* StandardSoundFactory_hpp */
