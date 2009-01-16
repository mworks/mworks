/**
 * StandardInterpreters.h
 *
 * Description: The standard MonkeyWorks interpreters.
 * PerlInterpreter: Evaluates commands or scripts written in Perl.  Scripts
 * rely on the MonkeyScript perl module that was created using SWIG.  Someone
 * should really document what is in the MonkeyScript module.  Variables 
 * defined are persistant through each invocation of the eval... commands.
 * To clear the context call the restart() or reset() functions.
 *
 * History:
 * Dave Cox on ??/??/?? - Created.
 * Paul Jankunas on 03/23/05 - Cleaned spacing.  Added Comments.
 * Paul Jankunas on 04/27/05 - Added restart() and reset().  Added description
 *
 * Copyright MIT 2005.  All rights reserved.
 */

#ifndef _STANDARD_INTERPRETERS_H
#define _STANDARD_INTERPRETERS_H


#if 0

#include "Interpreter.h"


#include "libperl++/interp.h"
#include "libperl++/scalar.h"
#include "libperl++/pattern.h"
#include "libperl++/array.h"
#include "libperl++/hash.h"
#include "libperl++/sub.h"



class PerlInterpreter : public Interpreter {
    protected:   
        wPerl *perl;
    public:
        /**
         * Default Constructor.  Just calls init()
         */
        PerlInterpreter();
        
        /**
         * Destructor.
         */
		virtual ~PerlInterpreter();
        
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
        virtual void evalString(const char *string);
        
        /**
         * Evaluates a perl file.
         */
        virtual void evalFile(const char *filename);
        
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

extern PerlInterpreter *GlobalPerlInterpreter;

#endif

#endif