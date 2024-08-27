/*
 *  FakeMonkey.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 6/13/05.
 *  Copyright 2005 MIT. All rights reserved.
 *
 */

#include "FakeMonkey.h"
#include "FakeMonkeyStatus.h"
#include "FakeMonkeySpikeChannel.h"
#include "FakeMonkeyEyeMovementChannel.h"
#include "FakeMonkeyJuiceChannel.h"
#include "FakeMonkeyJuiceNotification.h"
#include "boost/bind.hpp"
#include <boost/random/variate_generator.hpp>
#include <boost/random/exponential_distribution.hpp>
using namespace mw;

// a schedulable saccade function
void *fake_monkey_saccade(const shared_ptr<mFakeMonkey> &monkey){
	monkey->startSaccading();
	return 0;
}


void *fake_monkey_fixate(const shared_ptr<mFakeMonkey> &monkey){
	monkey->startFixating();
	return 0;
}

void *fake_monkey_spike(const shared_ptr<mFakeMonkey> &monkey){
	monkey->spike();
	return 0;
}

void *update_fake_monkey_channel(const shared_ptr<mFakeMonkeyEyeMovementChannel> &eye_movement_chan){
	eye_movement_chan->update();
	return 0;
}



mFakeMonkey::mFakeMonkey(const boost::shared_ptr<Scheduler> &a_scheduler,
						 const shared_ptr<Variable> &spike_rate_) {
	scheduler = a_scheduler;
	spike_rate = spike_rate_;
    
    clock = Clock::instance();
	
	// Initialize the rng
    rng.seed((int)clock->getSystemTimeNS());
	
	monkey_lock = shared_ptr<boost::mutex>(new boost::mutex());
	
	status = shared_ptr<mFakeMonkeyStatus>(new mFakeMonkeyStatus(M_FAKE_MONKEY_FIXATING));
	
	saccade_start_time = shared_ptr<MWorksTime>(new MWorksTime(0));
	saccade_duration = shared_ptr<MWorksTime>(new MWorksTime(FAKE_MONKEY_MEAN_SACCADE_DURATION_US));
	fixation_duration = FAKE_MONKEY_MEAN_FIXATION_DURATION_US;
	
	saccade_start_h = shared_ptr<float>(new float(0));
	saccade_start_v = shared_ptr<float>(new float(0));
	saccade_target_h = shared_ptr<float>(new float(0));
	saccade_target_v = shared_ptr<float>(new float(0));
    
    command_pending = false;
}

//mFakeMonkey::mFakeMonkey(const mFakeMonkey& copy);
mFakeMonkey::~mFakeMonkey() {
	this->stopDeviceIO();
}

// Not clear what to do with these right now
//virtual ExpandableList<IODeviceReference> *getAvailableDevices(); 
bool mFakeMonkey::attachPhysicalDevice(){                                      // attach next avaialble device to this object
	
	// bypass all of this bullshit for now--
	attached_device = new IOPhysicalDeviceReference(0, "monkey1");
	
	return true;
}

void mFakeMonkey::addChild(std::map<std::string, std::string> parameters,
						   ComponentRegistry *reg,
						   shared_ptr<mw::Component> child) {
	shared_ptr<mFakeMonkeyEyeMovementChannel> eye_movement_channel = boost::dynamic_pointer_cast<mFakeMonkeyEyeMovementChannel,mw::Component>(child);
	if(eye_movement_channel != 0) {
		eye_movement_channel->setChannelParams(clock,
											   monkey_lock,
											   status,
											   saccade_start_time,
											   saccade_duration,
											   saccade_start_h,
											   saccade_start_v,
											   saccade_target_h,
											   saccade_target_v);
		movement_channels.push_back(eye_movement_channel);
	}
	
	shared_ptr<mFakeMonkeySpikeChannel> spike_channel = boost::dynamic_pointer_cast<mFakeMonkeySpikeChannel,mw::Component>(child);
	if(spike_channel != 0) {
		shared_ptr<Variable> spike_variable = spike_channel->getVariable();
		spike_variables.push_back(spike_variable);
	}
	
	shared_ptr<mFakeMonkeyJuiceChannel> juice_channel = boost::dynamic_pointer_cast<mFakeMonkeyJuiceChannel,mw::Component>(child);
	if(juice_channel != 0) {
		shared_ptr<VariableNotification> notif = shared_ptr<VariableNotification>(new mFakeMonkeyJuiceNotification());			
		juice_channel->getVariable()->addNotification(notif);	
	}	
}

bool mFakeMonkey::initializeChannels(){
	//	for(int i = 0; i < channels->getNElements(); i++){
	//		shared_ptr<IOChannel> chan = channels->getElement(i);
	//		IOCapability *cap = chan->getCapability();
	//		if(cap->getIdentifier() == M_FAKE_MONKEY_JUICE){
	//			shared_ptr<VariableNotification> notif = shared_ptr<VariableNotification>(new mFakeMonkeyJuiceNotification());			
	//			chan->getVariable()->addNotification(notif);
	//		}
	//		
	//		if(cap->getIdentifier() == M_FAKE_MONKEY_SPIKE){
	//			spike_var = chan->getVariable();
	//		}
	//	}
	return true;
}


bool mFakeMonkey::startDeviceIO(){
	boost::mutex::scoped_lock lock(*monkey_lock);
	
	// schedule updates for each movement channel
	for(vector<shared_ptr<mFakeMonkeyEyeMovementChannel> >::const_iterator movement_channel_iterator = movement_channels.begin(); 
		movement_channel_iterator != movement_channels.end();
		++movement_channel_iterator) {
		
		shared_ptr<ScheduleTask> node = scheduler->scheduleUS(std::string(FILELINE ": eye movement channel"),
															   0, 
															   (*movement_channel_iterator)->getUpdatePeriod(), 
															   M_REPEAT_INDEFINITELY, 
															   boost::bind(update_fake_monkey_channel, *movement_channel_iterator), 
															   M_DEFAULT_IODEVICE_PRIORITY,
															   M_DEFAULT_IODEVICE_WARN_SLOP_US,
															   M_DEFAULT_IODEVICE_FAIL_SLOP_US,
															   M_MISSED_EXECUTION_DROP);
		
		schedule_nodes_lock.lock();
		schedule_nodes.push_back(node);       
		schedule_nodes_lock.unlock();
	}
	
	shared_ptr <mFakeMonkey> this_one = shared_from_this();
	
	// schedule the first saccade
	movement_node = scheduler->scheduleUS(FILELINE,
										  0, 
										  0,
										  1,
										  boost::bind(fake_monkey_saccade, this_one),
										  M_DEFAULT_IODEVICE_PRIORITY, 
										  M_DEFAULT_IODEVICE_WARN_SLOP_US,
										  M_DEFAULT_IODEVICE_FAIL_SLOP_US,
										  M_MISSED_EXECUTION_CATCH_UP);
	
	// schedule the first spike
	spike_node = scheduler->scheduleUS(FILELINE,
									   0, 
									   0,
									   1,
									   boost::bind(fake_monkey_spike, this_one),
									   M_DEFAULT_IODEVICE_PRIORITY, 
									   M_DEFAULT_IODEVICE_WARN_SLOP_US,
									   M_DEFAULT_IODEVICE_FAIL_SLOP_US,
									   M_MISSED_EXECUTION_CATCH_UP);
	
	return true;
}



void mFakeMonkey::spike(){
	boost::mutex::scoped_lock lock(*monkey_lock);
	boost::exponential_distribution<double> dist = boost::exponential_distribution<double>(spike_rate->getValue().getFloat() / 1000000); 	
	boost::variate_generator<boost::mt19937&, boost::exponential_distribution<double> > sampler = boost::variate_generator<boost::mt19937&, boost::exponential_distribution<double> >(rng,dist);
	
	if(spike_node){
		spike_node->cancel();
	}
	
	MWorksTime spike_time = clock->getCurrentTimeUS();
	
	for(vector<shared_ptr<Variable> >::const_iterator spike_var_iterator = spike_variables.begin();
		spike_var_iterator != spike_variables.end();
		++spike_var_iterator) {
		(*spike_var_iterator)->setValue(Datum(1L), spike_time);
	}
	
	float delay = sampler();
	
	shared_ptr <mFakeMonkey> this_one = shared_from_this();
	spike_node = scheduler->scheduleUS(FILELINE,
									   delay,
									   0,
									   1, 
									   boost::bind(fake_monkey_spike, this_one),
									   M_DEFAULT_IODEVICE_PRIORITY,
									   M_DEFAULT_IODEVICE_WARN_SLOP_US,
									   M_DEFAULT_IODEVICE_FAIL_SLOP_US,
									   M_MISSED_EXECUTION_CATCH_UP);
}



void mFakeMonkey::startSaccading(){
	boost::mutex::scoped_lock lock(*monkey_lock);
	
	//mprintf("Saccading...");
	
	// reset where the next saccade will start from (i.e. here)
	*saccade_start_h = *saccade_target_h;
	*saccade_start_v = *saccade_target_v;
	
	if(command_pending){
		*saccade_target_h = saccade_next_target_h;
		*saccade_target_v = saccade_next_target_v;
		command_pending = false;
		//mprintf("saccade h: %g, saccade v: %g (pending command)",saccade_target_h, saccade_target_v);
	} else {
		// can make fancier later...
		*saccade_target_h = 80 * ((double)rand() / (double)RAND_MAX) - 40;
		*saccade_target_v = 80 * ((double)rand() / (double)RAND_MAX) - 40; 
		//mprintf("saccade h: %g, saccade v: %g",saccade_target_h, saccade_target_v);
	}
	*saccade_start_time = clock->getCurrentTimeUS();
	
	*status = M_FAKE_MONKEY_SACCADING;
	
	
	if(movement_node){
		movement_node->cancel();
	}
	
	shared_ptr <mFakeMonkey> this_one = shared_from_this();

	movement_node = scheduler->scheduleUS(FILELINE,
										  *saccade_duration,
										  0,
										  1, 
										  boost::bind(fake_monkey_fixate, this_one),
										  M_DEFAULT_IODEVICE_PRIORITY,
										  M_DEFAULT_IODEVICE_WARN_SLOP_US,
										  M_DEFAULT_IODEVICE_FAIL_SLOP_US,
										  M_MISSED_EXECUTION_CATCH_UP);
}


void mFakeMonkey::startFixating(){
	boost::mutex::scoped_lock lock(*monkey_lock);
	
	//mprintf("Fixating...");
	
	
	*status = M_FAKE_MONKEY_FIXATING;
	
	if(movement_node){
		movement_node->cancel();
	}
	
	shared_ptr <mFakeMonkey> this_one = shared_from_this();

	movement_node = scheduler->scheduleUS(FILELINE,
										  fixation_duration,
										  0,
										  1, 
										  boost::bind(fake_monkey_saccade, this_one),
										  M_DEFAULT_IODEVICE_PRIORITY,
										  M_DEFAULT_IODEVICE_WARN_SLOP_US,
										  M_DEFAULT_IODEVICE_FAIL_SLOP_US,
										  M_MISSED_EXECUTION_CATCH_UP);
	
	fixation_duration = FAKE_MONKEY_MEAN_FIXATION_DURATION_US;
}



bool mFakeMonkey::stopDeviceIO(){
	boost::mutex::scoped_lock lock(*monkey_lock);
	
	*status = M_FAKE_MONKEY_FIXATING;
	if(movement_node){
		movement_node->cancel();
        movement_node.reset();
	}
	if(spike_node){
		spike_node->cancel();
        spike_node.reset();
	}	
	
	return LegacyIODevice::stopDeviceIO();
}


bool mFakeMonkey::updateChannel(int index){
	//	
	//	channels_lock.lock();
	//	
	//	shared_ptr<IOChannel> chan = channels->getElement(index);
	//	IOCapability* cap = chan->getCapability();
	//	
	//	boost::mutex::scoped_lock lock(*monkey_lock);
	//	double percent_done = (double)(scheduler->getClock()->getCurrentTimeUS() - saccade_start_time_us) / (double)saccade_duration_us;
	//	percent_done = MAX(MIN(1.0, percent_done),0);
	//	
	//	double val = 0;
	//	
	//	switch (cap->getIdentifier()){
	//		case M_FAKE_MONKEY_H_GAZE:
	//		{
	//			if(status == M_FAKE_MONKEY_SACCADING){
	//				val = (1.0-percent_done) * saccade_start_h + percent_done * saccade_target_h + 0.05*(double)((double)rand()/(double)RAND_MAX);				
	//			} else {
	//				// fixating, hang out at the target
	//				val = saccade_target_h + 0.05*(double)((double)rand()/(double)RAND_MAX);
	//			}
	//			
	//		}			
	//			break;
	//		case M_FAKE_MONKEY_V_GAZE:
	//			
	//		{
	//			if(status == M_FAKE_MONKEY_SACCADING){
	//				val = (1.0 - percent_done) * saccade_start_v + percent_done * saccade_target_v + 0.05*(double)((double)rand()/(double)RAND_MAX);				
	//			} else { 
	//				// fixating
	//				val = saccade_target_v + 0.05*(double)((double)rand()/(double)RAND_MAX);				
	//			}
	//		}			
	//			
	//			break;
	//		default:
	//			break;
	//			
	//	}
	//	
	//	val = MAX(MIN(val, 90), -90);
	//	(chan->getVariable())->setValue(val);
	//	channels_lock.unlock();
	return true;
}		

void mFakeMonkey::saccadeTo(const double x, 
							const double y, 
							const MWorksTime time_to_fixate_at_end_of_saccade) {	
	{
		boost::mutex::scoped_lock lock(*monkey_lock);
		command_pending = true;
		saccade_next_target_h = x;
		saccade_next_target_v = y;
		fixation_duration = time_to_fixate_at_end_of_saccade;
	}
	startSaccading();
}

void mFakeMonkey::fixate(const MWorksTime duration){
	{
		boost::mutex::scoped_lock lock(*monkey_lock);
		fixation_duration = duration;
	}
	startFixating();
}



