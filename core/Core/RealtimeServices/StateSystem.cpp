#include "StateSystem.h"

#include "EventBuffer.h"
#include "SystemEventFactory.h" 


BEGIN_NAMESPACE_MW

 
void StateSystem::sendSystemStateEvent() {
    if (auto outgoingEventBuffer = global_outgoing_event_buffer) {
        outgoingEventBuffer->putEvent(SystemEventFactory::currentExperimentState());
    }
}


SINGLETON_INSTANCE_STATIC_DECLARATION(StateSystem)


END_NAMESPACE_MW
