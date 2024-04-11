//
//  IOSAudioEngineManager.cpp
//  MWorksCore-iOS
//
//  Created by Christopher Stawarz on 4/11/24.
//

#include "IOSAudioEngineManager.hpp"

#include <AVFAudio/AVAudioSession.h>


BEGIN_NAMESPACE_MW


IOSAudioEngineManager::IOSAudioEngineManager() {
    @autoreleasepool {
        auto audioSession = [AVAudioSession sharedInstance];
        
        // We want the number of samples per buffer to be the smallest power of 2
        // that produces a buffer duration less than or equal to the target duration
        const auto samplesPerBuffer = std::pow(2.0, std::floor(std::log2(targetBufferDuration * audioSession.sampleRate)));
        const auto bufferDuration = samplesPerBuffer / audioSession.sampleRate;
        
        if (![audioSession setPreferredIOBufferDuration:bufferDuration error:nil]) {
            mwarning(M_SYSTEM_MESSAGE_DOMAIN,
                     "Cannot set preferred audio I/O buffer duration; sounds may start later than requested");
        }
    }
}


END_NAMESPACE_MW
