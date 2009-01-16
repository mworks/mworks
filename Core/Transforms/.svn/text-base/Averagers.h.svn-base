/*
 *  Averagers.h
 *  MonkeyWorksCore
 *
 *  Created by dicarlo on 5/3/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _AVERAGERS
#define _AVERAGERS

#include "VariableTransformAdaptors.h"
#include "ComponentFactory.h"
#include "ComponentRegistry_new.h"

// the Averager class takes a single input and is a specific type of the 
//  standard streaming variable interface (VarTransformAdaptor) 
//  Currently the time stamps are ignored
namespace mw {
class Averager : public VarTransformAdaptor  {
	
private:
	long    n;
	double  sum;
	double  sum2;
    
protected:
	bool   running;
	Data  ave;
	Data  var;
	Data  stdDev;
	Data  SEM;
	MonkeyWorksTime aveTimeUS;
	void    computeAllValues(Data data, MonkeyWorksTime timeUS);
	
public:
	Averager(shared_ptr<Variable> _inputVar);
	virtual ~Averager();
	bool    isRunning();
	Data   getAverage();
	Data   getStdDeviation();
	Data   getSEM();
	
	void    reset();
	void    start();
	void    stop();
	
	
	// method overriden from base class
	virtual void newDataReceived(int inputIndex, const Data& data, 
								 MonkeyWorksTime timeUS);
};


// subclass of the averagers exposes to the user
// (allows thigs like calibrators to use a basic version -- e.g. without output vars)
// in this case, time is not ignored
class AveragerUser : public Averager, public ComponentFactory {
	
private:
	shared_ptr<Variable> outputVariable;
	
public:
	AveragerUser(shared_ptr<Variable> _inputVar, shared_ptr<Variable> _outputVar);
	virtual ~AveragerUser();
	virtual void newDataReceived(int inputIndex, const Data& data, 
								 MonkeyWorksTime timeUS);
};

class AveragerUserFactory : public ComponentFactory {
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};
}
#endif

