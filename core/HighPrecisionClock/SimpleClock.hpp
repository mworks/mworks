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
    const MachTimebase timebase;
    
private:
    const uint64_t systemBaseTimeAbsolute;
    
};


END_NAMESPACE_MW


#endif /* SimpleClock_hpp */


























