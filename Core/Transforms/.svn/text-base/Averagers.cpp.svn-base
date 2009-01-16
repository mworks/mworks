/*
 *  Averagers.cpp
 *  MonkeyWorksCore
 *
 *  Created by dicarlo on 5/3/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "Averagers.h"
using namespace mw;

#define VERBOSE_AVERAGER 0


Averager::Averager(shared_ptr<Variable> _inputVar) {
    
    this->registerInput(_inputVar);
    this->reset();
    if (VERBOSE_AVERAGER) mprintf(" ****   Averager constructor called");
   
}

Averager::~Averager() {
}
 
bool Averager::isRunning() {
    return (running);
}
 
     
void Averager::start() {
    lock();
    running = true;
    if (VERBOSE_AVERAGER) mprintf(" ****   Averager started");
    unlock();
}   
 
void Averager::stop() {
    lock();
    running = false;
    if (VERBOSE_AVERAGER) mprintf(" ****   Averager stopped");
    unlock();
}  

void Averager::reset() {
    lock();
    
    n = 0;
    sum = 0;
    sum2 = 0;
    
    Data zed = (Data)((double)0);
    ave = zed;
    var = zed;
    stdDev = zed;
    SEM = zed;
    
    running = false;
    
    unlock();
}

Data Averager::getAverage() {
    return ave;
}  

Data Averager::getStdDeviation() {
    return stdDev;
}  

Data Averager::getSEM() {
    return SEM;
}  
  

void Averager::computeAllValues(Data data, MonkeyWorksTime timeUS) {

    double dat = (double)data;    
                                        
    sum = sum + dat;
    sum2 = sum2 + (dat*dat);    
    n++;
    
    double nd = (double)n;
    ave = (Data)(sum/nd);    
    if (n>=2) {
        double v = (sum2 - ((sum*sum)/nd)) / (nd-1.0);
        double sd = (sqrt(v));
        double sm = sd / (sqrt(nd));
        var = (Data)v;
        stdDev = (Data)sd;
        SEM = (Data)sm;
    };
    
    
    // for now, use time of last value
    aveTimeUS = timeUS;
    
    // if (VERBOSE_AVERAGER) mprintf(" ****   Averager accumulated data point");

}


void Averager::newDataReceived(int inputIndex, const Data& data, 
                                                MonkeyWorksTime timeUS) {
    lock();                                            
    if (!running) {
        unlock();
        return;
    }
    if (inputIndex != 0) {  // error
        unlock();
        return;
    }
    
    this->computeAllValues(data, timeUS);
    unlock();   
}
    

// ====================================


AveragerUser::AveragerUser(shared_ptr<Variable> _inputVar, shared_ptr<Variable> _outputVar):Averager(_inputVar) {
    
    this->registerOutput(_outputVar);        // VarTransformAdaptor class method
    if (VERBOSE_AVERAGER) mprintf(" ****   AveragerUser constructor called");

}

AveragerUser::~AveragerUser() {};


void AveragerUser::newDataReceived(int inputIndex, const Data& data, 
                                                MonkeyWorksTime timeUS) {
    
                                                
    lock();  
                                                                                                                          
    if (!running) {
        unlock();
        return;
    }
    if (inputIndex != 0) {  // error
        unlock();
        return;
    }
    
    this->computeAllValues(data, timeUS);           // Averager base class method
    
    //if (VERBOSE_AVERAGER) mprintf(" ****   AveragerUser posting results");
    this->postResults(0, (Data)ave, aveTimeUS);    // VarTransformAdaptor class method

    unlock();

}

shared_ptr<mw::Component> AveragerUserFactory::createObject(std::map<std::string, std::string> parameters,
											mwComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters, "in1", "out1");
	
	shared_ptr<Variable> in1 = reg->getVariable(parameters.find("in1")->second);
	shared_ptr<Variable> out1 = reg->getVariable(parameters.find("out1")->second);
	
	
	checkAttribute(in1, parameters.find("reference_id")->second, "in1", parameters.find("in1")->second);
	
	
	checkAttribute(out1, parameters.find("reference_id")->second, "out1", parameters.find("out1")->second);
	
	
	shared_ptr <mw::Component> newAveragerUser = shared_ptr<mw::Component>(new AveragerUser(in1, out1));
	return newAveragerUser;		
}




