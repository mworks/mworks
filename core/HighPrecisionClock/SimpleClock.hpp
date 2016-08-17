//
//  SimpleClock.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/17/16.
//
//

#ifndef SimpleClock_hpp
#define SimpleClock_hpp


BEGIN_NAMESPACE_MW


class SimpleClock : public Component, public Clock, boost::noncopyable {
    
public:
    SimpleClock();
    
    MWTime getSystemBaseTimeNS() override;
    MWTime getSystemTimeNS() override;
    MWTime getCurrentTimeNS() override;
    
    void sleepNS(MWTime time) override;
    void yield() override;
    
protected:
    static bool logMachError(const char *functionName, mach_error_t error) {
        const bool failed = (error != ERR_SUCCESS);
        if (failed) {
            merror(M_SCHEDULER_MESSAGE_DOMAIN, "%s failed: %s (%d)", functionName, mach_error_string(error), error);
        }
        return failed;
    }
    
    MWTime absoluteToNanos(uint64_t absolute) const {
        return absolute * timebaseInfo.numer / timebaseInfo.denom;
    }
    
    uint64_t nanosToAbsolute(MWTime nanos) const {
        return nanos * timebaseInfo.denom / timebaseInfo.numer;
    }
    
private:
    static mach_timebase_info_data_t getTimebaseInfo();
    
    const mach_timebase_info_data_t timebaseInfo;
    const uint64_t systemBaseTimeAbsolute;
    
};


END_NAMESPACE_MW


#endif /* SimpleClock_hpp */


























