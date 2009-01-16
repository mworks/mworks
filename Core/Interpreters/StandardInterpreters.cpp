/**
 * StandardInterpreters.cpp
 *
 * History:
 * Dave Cox on ??/??/?? - Created.
 * Paul Jankunas on 03/23/05 - Cleaned spacing. 
 * Paul Jankunas on 04/27/05 - Added error checking to functions, added
 *                      reset() and restart() functions.  Fixed perl pathing.
 *
 * Copyright MIT 2005.  All rights reserved.
 */


#if 0

/*************************************************************
 *              evalString Program
**************************************************************
use lib '%s';   # gets the scriptingPath() return value, for monkeyscript
use lib '/sw/lib/perl5';
use lib '/sw/lib/perl5/darwin';
use lib '/sw/lib/perl/5.8.1/darwin-thread-multi-2level'; # Twig stuff
use MonkeyScript;
use FileHandle; 
use English; 

$command = qq(%s); # double quote the command
# replace all new SomeClas with new MonkeyScript::SomeClas
$command =~ s{=\\s+new\\s+m}{= new MonkeyScript\:\:m}sg;
# evaluates the command
eval($command); 
# error clause
if($EVAL_ERROR){ 
    mprintf(qq(Perl Interpreter Error: $EVAL_ERROR));
    printf(qq(Perl Interpreter Error: $EVAL_ERROR));
    mprintf(qq(Command was: $command)); 
}";
*/

/*************************************************************
 *             evalFile Program
**************************************************************
use lib '%s'; # gets the scriptingPath() return value, for monkeyscript
use lib '/sw/lib/perl5';
use lib '/sw/lib/perl5/darwin';
use lib '/sw/lib/perl/5.8.1/darwin-thread-multi-2level'; # Twig stuff
use MonkeyScript;
use FileHandle;
use English;

# double quote the file name we want to open
$filename = qq(%s);
mprintf(qq(Executing file... $filename));
# loads the file and puts the lines into an array
$file = new FileHandle($filename); 
@lines = <$file>; 

if(length(@lines) == 0) {
    mprintf(qq(Error reading file...));
};

# removes white space
$lines = join(qq(),@lines);
# changes all new SomeClass to new MonkeyScript::SomeClass
$lines =~ s{=\\s+new\\s+m}{= new MonkeyScript\:\:m}sg;

# evaluates the commands in the file using the MonkeyScript module
eval(qq(use MonkeyScript; $lines));

if($EVAL_ERROR) {
    mprintf(qq(Perl Interpreter Error: $EVAL_ERROR));
    printf(qq(Perl Interpreter Error: $EVAL_ERROR));
};
*/

#include "StandardInterpreters.h"
#include "Utilities.h"
#include "PlatformDependentServices.h"
#include <stdio.h>
#include <stdlib.h>

PerlInterpreter *GlobalPerlInterpreter;

PerlInterpreter::PerlInterpreter() {
    perl = NULL;
	init();
}
		
PerlInterpreter::~PerlInterpreter() {
    shutdown();
}
        
bool PerlInterpreter::startup() {
    return true;
}        
        
bool PerlInterpreter::init() {

    if(perl != NULL){
		delete perl;
	}
	
	perl = wPerl::start();
    if(perl == NULL) {
        return false;
    }
    return true;       
}

void PerlInterpreter::evalString(const char *string) {
    // this format string is long and tedious
    // a decription of what it does is at the top of the file.
//    char *formatstring = "use lib '%s'; use lib '/sw/lib/perl5'; use lib '/sw/lib/perl5/darwin'; use lib '/sw/lib/perl/5.8.1/darwin-thread-multi-2level'; use MonkeyScript; use FileHandle; use English; $command = qq(%s); $command =~ s{=\\s+new\\s+m}{= new MonkeyScript\:\:m}sg; eval($command); if($EVAL_ERROR){ mprintf(qq(Perl Interpreter Error: $EVAL_ERROR));  printf(qq(Perl Interpreter Error: $EVAL_ERROR)); mprintf(qq(Command was: $command)); }";
    #if __darwin__
	    char *formatstring = "use lib '%s'; use lib '/Library/MonkeyWorks'; use lib '/Library/MonkeyWorks/lib; use lib '/Library/MonkeyWorks/lib/perl5/5.8.8/darwin-thread-multi-2level/'; use lib '/Library/MonkeyWorks/lib/perl5/site_perl/5.8.8/darwin-thread-multi-2level/'; use lib '/Library/MonkeyWorks/lib/perl5/site_perl/5.8.8/darwin-thread-multi-2level/auto'; use MonkeyScript; use FileHandle; use English; $command = qq(%s); $command =~ s{=\\s+new\\s+m}{= new MonkeyScript\:\:m}sg; eval($command); if($EVAL_ERROR){ mprintf(qq(Perl Interpreter Error: $EVAL_ERROR));  printf(qq(Perl Interpreter Error: $EVAL_ERROR)); mprintf(qq(Command was: $command)); }";
    #else
    		char *formatstring = "use lib '%s'; use MonkeyScript; use FileHandle; use English; $command = qq(%s); $command =~ s{=\\s+new\\s+m}{= new MonkeyScript\:\:m}sg; eval($command); if($EVAL_ERROR){ mprintf(qq(Perl Interpreter Error: $EVAL_ERROR));  printf(qq(Perl Interpreter Error: $EVAL_ERROR)); mprintf(qq(Command was: $command)); }";
    #endif
    
    char *evalstring = (char *)calloc(strlen(formatstring) + 
                                            strlen(string) +
                                            strlen(scriptingPath()) + 1,
                                                               sizeof(char));
    sprintf(evalstring, formatstring, scriptingPath(), string);
    perl->eval(evalstring);
	
	delete evalstring;
}

void PerlInterpreter::evalFile(const char *filename) {
    // this format string is long and tedious
    // a decription of what it does is at the top of the file.
	// use lib '/Library/MonkeyWorks/Scripting/Perl/perl5/darwin'; 
	// 
//    char *formatstring = "use lib '%s'; use lib '/Library/MonkeyWorks/Scripting/Perl/perl5'; use MonkeyScript; use FileHandle; use English;  $filename = qq(%s); mprintf(qq(Executing file... $filename));$file = new FileHandle($filename); @lines = <$file>; if(length(@lines) == 0){ mprintf(qq(Error reading file...)); }; $lines = join(qq(),@lines); $lines =~ s{=\\s+new\\s+m}{= new MonkeyScript\:\:m}sg; eval(qq(use MonkeyScript; $lines)); if($EVAL_ERROR){ mprintf(qq(Perl Interpreter Error: $EVAL_ERROR)); printf(qq(Perl Interpreter Error: $EVAL_ERROR)); }";
    
    #if	__darwin__
    		char *formatstring = "use lib '%s';  use lib '/Library/MonkeyWorks'; use lib '/Library/MonkeyWorks/lib; use lib '/Library/MonkeyWorks/lib/perl5/5.8.8/darwin-thread-multi-2level/'; use lib '/Library/MonkeyWorks/lib/perl5/site_perl/5.8.8/darwin-thread-multi-2level/'; use lib '/Library/MonkeyWorks/lib/perl5/site_perl/5.8.8/darwin-thread-multi-2level/auto'; use MonkeyScript; use FileHandle; use English;  $filename = qq(%s);$file = new FileHandle($filename); @lines = <$file>; if(scalar(@lines) == 0){ mprintf(qq(Error reading file...)); }; $lines = join(qq(),@lines); $lines =~ s{=\\s+new\\s+m}{= new MonkeyScript\:\:m}sg; eval(qq(use MonkeyScript; $lines)); if($EVAL_ERROR){ merror(M_PARSER_MESSAGE_DOMAIN, qq(Perl Interpreter Error: $EVAL_ERROR)); printf(qq(Perl Interpreter Error: $EVAL_ERROR)); }";
    # else
    		char *formatstring = "use lib '%s'; use MonkeyScript; use FileHandle; use English;  $filename = qq(%s); $file = new FileHandle($filename); if(!(-e $file)){ merror($MonkeyScript::M_PARSER_MESSAGE_DOMAIN, qq(Missing file: <$file>)); }; @lines = <$file>; if(scalar(@lines) == 0){ mprintf(qq(Error reading file... $filename)); }; $lines = join(qq(),@lines); $lines =~ s{=\\s+new\\s+m}{= new MonkeyScript\:\:m}sg; eval(qq(use MonkeyScript; $lines)); if($EVAL_ERROR){ merror($MonkeyScript::M_PARSER_MESSAGE_DOMAIN, qq(Perl Error: $EVAL_ERROR)); print(qq(Perl Interpreter Error: $EVAL_ERROR)); }";
    #endif
    
    
    const char *script_path = scriptingPath();
    char *string = (char *)calloc(strlen(script_path) + strlen(formatstring) + 
                                        strlen(filename) + 1, sizeof(char));
    sprintf(string, formatstring, script_path, filename);
    
   
    perl->eval(string);
	
	delete string;
}
        
bool PerlInterpreter::shutdown() {
    if(perl) {
        delete(perl); 
        perl = NULL;
    }
    return true;
}

void PerlInterpreter::reset() {
    shutdown();
}

bool PerlInterpreter::restart() {
    shutdown();
    if(!init()) {
        return false;
    }
    return true;
}


#endif