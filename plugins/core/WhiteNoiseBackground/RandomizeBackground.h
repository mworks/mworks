/*
 *  RandomizeBackground.h
 *  WhiteNoiseBackground
 *
 *  Created by Christopher Stawarz on 12/17/10.
 *  Copyright 2010 MIT. All rights reserved.
 *
 */

#ifndef RandomizeBackground_H_
#define RandomizeBackground_H_


BEGIN_NAMESPACE_MW


class RandomizeBackground : public Action {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit RandomizeBackground(const ParameterValueMap &parameters);
    bool execute() override;
    
private:
    const boost::shared_ptr<StimulusNode> backgroundNode;
    
};


END_NAMESPACE_MW


#endif /* RandomizeBackground_H_ */
