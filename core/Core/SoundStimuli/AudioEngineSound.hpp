//
//  AudioEngineSound.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/2/21.
//

#ifndef AudioEngineSound_hpp
#define AudioEngineSound_hpp

#include <boost/noncopyable.hpp>

#include "Sound.h"


BEGIN_NAMESPACE_MW


class AudioEngineSound : public Sound, boost::noncopyable {
    
public:
    explicit AudioEngineSound(const ParameterValueMap &parameters);
    
};


END_NAMESPACE_MW


#endif /* AudioEngineSound_hpp */
