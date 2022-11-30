/*
 *  StandardStimulusFactory.h
 *  MWorksCore
 *
 *  Created by Christopher Stawarz on 3/24/11.
 *  Copyright 2011 MIT. All rights reserved.
 *
 */

#ifndef _MW_STANDARD_STIMULUS_FACTORY_H
#define _MW_STANDARD_STIMULUS_FACTORY_H

#include "StandardComponentFactory.h"
#include "StimulusNode.h"
#include "StimulusDisplay.h"


BEGIN_NAMESPACE_MW


template<typename StimulusType>
class StandardStimulusFactory : public StandardComponentFactory<StimulusType> {
    
public:
    virtual boost::shared_ptr<mw::Component> createObject(ComponentFactory::StdStringMap parameters,
                                                          ComponentRegistryPtr reg)
    {
        auto stim = boost::dynamic_pointer_cast<StimulusType>(StandardComponentFactory<StimulusType>::createObject(parameters, reg));
        
        if (stim->getDeferred() == Stimulus::nondeferred_load) {
            stim->load(StimulusDisplay::getDefaultStimulusDisplay());
        }
        
        reg->registerStimulusNode(stim->getTag(), boost::make_shared<StimulusNode>(stim));
        
        return stim;
    }
    
};


END_NAMESPACE_MW


#endif





















