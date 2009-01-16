/*
 *  EmbeddedPerlInterpreter.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 11/14/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */
 
 #ifndef EMBEDDED_PERL_INTERPRETER_H_
 #define EMBEDDED_PERL_INTERPRETER_H_

#include "Interpreter.h"
#include "Lockable.h"
#include "GenericData.h"
#include <string>
#include "boost/filesystem/path.hpp"
namespace mw {
extern void setPerlReturn(Data *pr);
extern Data *GlobalPerlReturn;

class EmbeddedPerlInterpreter : public Interpreter, public Lockable {
    protected:
		
		static int n_interpreters_in_the_world;
		//PerlInterpreter *perl_interpreter;
		//EP	*perl;
        //EPI *perl_interpreter;
    public:
        /**
         * Default Constructor.  Just calls init()
         */
        EmbeddedPerlInterpreter();
        
        /**
         * Destructor.
         */
		virtual ~EmbeddedPerlInterpreter();
        
        /**
         * This function is useless, it just returns true.
         */
        virtual bool startup();
        
        /**
         * Starts up the perl interpreter.
         */
        virtual bool init();
        
        /**
         * Evaluates a perl command.
         */
        virtual void evalString(const std::string command);
        
        /**
         * Evaluates a perl file.
         */
        virtual void evalFile(const boost::filesystem::path filename);
        
        /**
         * Deletes the perl interpreter.  Always returns true.
         */
        virtual bool shutdown();
        
        /**
         * Reset function deletes the perl object and sets to NULL.  To use
         * the interpreter again call the init function.
         */
        virtual void reset();
        
        /**
         * Shutsdown the perl interpreter and then restarts it for you.  
         * Returns false if the perl object could not be restarted.
         */
        virtual bool restart();

};
}
#endif


