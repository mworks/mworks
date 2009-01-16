/**
 * Interpreter.h
 *
 * Description:
 * An Interpreter object is a simple wrapper around an interpreter for
 * a scripting language.  Through the use of Interpreter objects, one can 
 * interact with the MonkeyWorks Core via various non-compiled scripting 
 * languages.
 *
 * In principle, any scripting language supported by SWIG 
 * (Simple Wrapper Interface Generator, an open source project) should be 
 * trivial to add in. Currently, Perl is supported, and is used heavily in 
 * those arenas where Perl is superior to to hand-rolled C-code, particularly
 * in the parsing of XML files.  It may be desirable at some point to also 
 * allow users to extend their XML experiments with small snippets 
 * of interpreted code.
 *
 * History:
 * Dave Cox ??/??/?? - Created.
 * Paul Jankunas on 04/27/05 - Added reset() and restart() functions
 * Paul Jankunas on 1/24/06 - Adding virtual destructor.
 *
 * Copyright (c) 2005 MIT. All rights reserved. 
 */

#ifndef _INTERPRETER_H
#define _INTERPRETER_H
namespace mw {
class Interpreter {
    public:
        // These are all base classes. All bool functions return false.
        
        /**
         * Constructor.
         */
        Interpreter();
        
        virtual ~Interpreter();
        
        /**
         * Starts the interpreter.
         */
        virtual bool startup();
        
        /**
         * Initializes the interpreter.
         */
        virtual bool init();

        /**
         * Evaluates a string as a command in the interpreted language
         */
        virtual void evalString(char *string);
        
        /**
         * Evalutates a file as a series of commands in the
         * interpreted language
         */
        virtual void evalFile(char *filename);
        
        /**
         * Shutdowns the interpreter.
         */
        virtual bool shutdown();
        
        /**
         * Resets the interpreter.
         */
        virtual void reset();
        
        /**
         * Restarts the interpreter.
         */
        virtual bool restart();
};
}
#endif
