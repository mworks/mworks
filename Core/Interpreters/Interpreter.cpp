/**
 * Interpreter.cpp
 *
 * History:
 * Dave Cox ??/??/?? - Created.
 * Paul Jankunas on 04/27/05 - Added reset() and restart() functions
 *
 * Copyright (c) 2005 MIT. All rights reserved. 
 */
#include "Interpreter.h"
using namespace mw;

Interpreter::Interpreter() { }
Interpreter::~Interpreter() { }
bool Interpreter::startup() { return false; }        
bool Interpreter::init() { return false; }
void Interpreter::evalString(char *string) { }
void Interpreter::evalFile(char *filename) { }
bool Interpreter::shutdown(){ return false; }
void Interpreter::reset() { }
bool Interpreter::restart() { return false; }