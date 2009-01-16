/*
 *  EmbeddedPerlInterpreter.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 11/14/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "EmbeddedPerlInterpreter.h"

#include "EXTERN.h"
#include <string.h>
#include <perl.h>
#include "PlatformDependentServices.h"
#include "Utilities.h"
using namespace mw;

extern "C" void boot_DynaLoader(pTHX_ CV *cv);

static void libperl_xs_init(pTHX)
{
    char *file = __FILE__;

    newXS("DynaLoader::boot_DynaLoader", boot_DynaLoader, file);
}

static PerlInterpreter *my_perl;

typedef struct {
    char type;       
    void *pdata;
} Out_Param;


int
perl_eval_va (const char *str, ...)
{
    /* Evals a string, returns -1 if unsuccessful, else returns
     *  the number of return params
     *  char buf[10]; int a;
     *  perl_eval_va ("$a = 10; ($a, $a+1)",
     *                "i", &a,
     *                "s", buf,
     *                 NULL);
     */
       
    SV*       sv     = newSVpv(str,0);
    va_list   vl;
    char      *p     = NULL;  
    int       i      = 0; 
    int       nret   = 0;     /* number of return params expected*/
    int       result = 0;
    Out_Param op[20];
    int ii; double d;

    dSP;
    ENTER;
    SAVETMPS;
    PUSHMARK(sp);
    va_start (vl, str);

    while (p = va_arg(vl, char *)) {
        if ((*p != 's') && (*p != 'i') && (*p != 'd')) {
            fprintf (stderr, "perl_eval_va: Unknown option \'%c\'.\n"
                              "Did you forget a trailing NULL ?\n", *p);
            return -1;
        }
        op[nret].pdata = (void*) va_arg(vl, char *);
        op[nret++].type = *p;
    }
    va_end(vl);
    PUTBACK;
    result = perl_eval_sv(sv, (nret == 0) ? G_DISCARD :
                              (nret == 1) ? G_SCALAR  :
                                            G_ARRAY  );

    SPAGAIN;
   // if (SvTRUE(GvSV(errgv))) { /* errgv == $@ */
   //     fprintf (stderr, "Eval error: %s", SvPV(GvSV(errgv), na)) ;
  //      return -1;
  //  }
	
    SvREFCNT_dec(sv);
    /*printf ("nret: %d, result: %d\n", nret, result);*/
    if (nret > result)
        nret = result;

    for (i = --nret; i >= 0; i--) {
        switch (op[i].type) {
        case 's':
            str = POPp;
            /*printf ("String: %s\n", str);*/
            strcpy((char *)op[i].pdata, str);
            break;
        case 'i':
            ii = POPi;
            /*printf ("Int: %d\n", ii);*/
            *((int *)(op[i].pdata)) = ii;
            break;
        case 'd':
            d = POPn;
            /*printf ("Double: %f\n", d);*/
            *((double *) (op[i].pdata)) = d;
            break;
        }
   }
   FREETMPS ;
   LEAVE;
   return result;
}    

int
perl_call_va (char *subname, ...)
{
    char *p;
    char *str = NULL; int i = 0; double d = 0;
    int  nret = 0; /* number of return params expected*/
//    int  ax;
    int ii=0;
    Out_Param op[20];
    va_list vl;
    int out = 0;
    int result = 0;

    dSP;
    ENTER;
    SAVETMPS;
    PUSHMARK(sp);
    va_start (vl, subname);
 
    /*printf ("Entering perl_call %s\n", subname);*/
    while (p = va_arg(vl, char *)) {
        /*printf ("Type: %s\n", p);*/
        switch (*p)
        {
        case 's' :
            if (out) {
                op[nret].pdata = (void*) va_arg(vl, char *);
                op[nret++].type = 's';
            } else {
                str = va_arg(vl, char *);
         /*printf ("IN: String %s\n", str);*/
         ii = strlen(str);
                XPUSHs(sv_2mortal(newSVpv(str,ii)));
            }
            break;
        case 'i' :
            if (out) {
                op[nret].pdata = (void*) va_arg(vl, int *);
                op[nret++].type = 'i';
            } else {
                ii = va_arg(vl, int);
         /*printf ("IN: Int %d\n", ii);*/
                XPUSHs(sv_2mortal(newSViv(ii)));
            }
            break;
        case 'd' :
            if (out) {
                op[nret].pdata = (void*) va_arg(vl, double *);
                op[nret++].type = 'd';
            } else {
               d = va_arg(vl, double);
               /*printf ("IN: Double %f\n", d);*/
               XPUSHs(sv_2mortal(newSVnv(d)));
            }
            break;
        case 'O':
            out = 1;  /* Out parameters starting */
            break;
        default:
             fprintf (stderr, "perl_eval_va: Unknown option \'%c\'.\n"
                               "Did you forget a trailing NULL ?\n", *p);
            return 0;
        }
    }
   
    va_end(vl);
 
    PUTBACK;
    result = perl_call_pv(subname, (nret == 0) ? G_DISCARD :
                                   (nret == 1) ? G_SCALAR  :
                                                 G_ARRAY  );
 
    
 
    SPAGAIN;
    /*printf ("nret: %d, result: %d\n", nret, result);*/
    if (nret > result)
        nret = result;
 
    for (i = --nret; i >= 0; i--) {
        switch (op[i].type) {
        case 's':
            str = POPp;
            /*printf ("String: %s\n", str);*/
            strcpy((char *)op[i].pdata, str);
            break;
        case 'i':
            ii = POPi;
            /*printf ("Int: %d\n", ii);*/
            *((int *)(op[i].pdata)) = ii;
            break;
        case 'd':
            d = POPn;
            /*printf ("Double: %f\n", d);*/
            *((double *) (op[i].pdata)) = d;
            break;
        }
    }
   
    FREETMPS ;
    LEAVE ;
    return result;
}


int EmbeddedPerlInterpreter::n_interpreters_in_the_world = 0;


/**
 * Default Constructor.  Just calls init()
 */
EmbeddedPerlInterpreter::EmbeddedPerlInterpreter() : Interpreter(){
   init();
}

/**
 * Destructor.
 */
EmbeddedPerlInterpreter::~EmbeddedPerlInterpreter(){
	shutdown();
}

/**
 * This function is useless, it just returns true.
 */
bool EmbeddedPerlInterpreter::startup(){
	return true;
}

/**
 * Starts up the perl interpreter.
 */
bool EmbeddedPerlInterpreter::init(){
	
	static char *dummy_argv[] = {"","-e","0"}; 
//	int num;
	
	
	
	lock();
	
	//if(EmbeddedPerlInterpreter::n_interpreters_in_the_world == 0){
		
		//PL_perl_destruct_level = 1;
		
		if(EmbeddedPerlInterpreter::n_interpreters_in_the_world > 0){
			mwarning(M_PARSER_MESSAGE_DOMAIN,
				  "Attempt to instantiate more than one perl interpreter, which "
				  "is not allowed");
			shutdown(); // this could cause problems. TODO TODO TODO
		}
		
		my_perl = perl_alloc();
		perl_construct(my_perl);

		perl_parse(my_perl, libperl_xs_init, 3, dummy_argv, NULL);

		EmbeddedPerlInterpreter::n_interpreters_in_the_world++;

		

	//}
	unlock();
	return true;
}

/**
 * Evaluates a perl command.
 */
void EmbeddedPerlInterpreter::evalString(const std::string command){
	const std::string substitutionDescriptor("%s");
	
	lock();
	
	std::string evalString("use lib '%s';"
						 "use MonkeyScript;"
						 "use FileHandle;" 
						 "use English;"
						 "$command = qq(%s);"
						 "$command =~ s{=\\s+new\\s+m}{= new MonkeyScript\\:\\:m}sg;"
						 "eval($command);" 
						 "if($EVAL_ERROR){" 
						 "	mprintf(qq(Perl Interpreter Error: $EVAL_ERROR));"  
						 "  printf(qq(Perl Interpreter Error: $EVAL_ERROR));" 
						 "	mprintf(qq(Command was: $command)); }");
 
	evalString.replace(evalString.find(substitutionDescriptor), 
					   substitutionDescriptor.length(), 
					   scriptingPath().string());	
	evalString.replace(evalString.find(substitutionDescriptor), 
					   substitutionDescriptor.length(), command);	
	
	perl_eval_va(evalString.c_str(), NULL);
	
	unlock();
}

/**
 * Evaluates a perl file.
 */
void EmbeddedPerlInterpreter::evalFile(const boost::filesystem::path filename){
	const std::string substitutionDescriptor("%s");
 
	lock();
 
	
	std::string evalString("use lib '%s';" 
						 //"use MonkeyScript;" 
						 "use FileHandle;" 
						 "use English;"  
						 "$filename = qq(%s);" 
//						 "mprintf(qq(Executing file: $filename));"
						 "$file = new FileHandle($filename);" 
						 "if(!(-e $file)){" 
						 "	merror($MonkeyScript::M_PARSER_MESSAGE_DOMAIN, qq(Missing file: $filename));"
						 "};" 
						 "@lines = <$file>;" 
						 "if(scalar(@lines) == 0){" 
						 "	mprintf(qq(Error reading file... $filename));" 
						 "};" 
						 "$lines = join(qq(),@lines);" 
						 "$lines =~ s{=\\s+new\\s+m}{= new MonkeyScript\\:\\:m}sg;" 
						 "eval(qq(use MonkeyScript; $lines));" 
						 "if($EVAL_ERROR){" 
						 "	merror($MonkeyScript::M_PARSER_MESSAGE_DOMAIN, qq(Perl Error: $EVAL_ERROR));"
						 "print(qq(Perl Interpreter Error: $EVAL_ERROR)); }");
 
	evalString.replace(evalString.find(substitutionDescriptor), 
					   substitutionDescriptor.length(), 
					   scriptingPath().string());	
	evalString.replace(evalString.find(substitutionDescriptor), 
					   substitutionDescriptor.length(), filename.string());	
	
	perl_eval_va(evalString.c_str(), NULL);
	unlock();
	
}

/**
 * Deletes the perl interpreter.  Always returns true.
 */
bool EmbeddedPerlInterpreter::shutdown(){
	
	lock();
	
	PL_perl_destruct_level = 1;
	perl_destruct(my_perl);
	perl_free(my_perl);
	
	EmbeddedPerlInterpreter::n_interpreters_in_the_world--;
	
	unlock();
	return true;
}

/**
 * Reset function deletes the perl object and sets to NULL.  To use
 * the interpreter again call the init function.
 */
void EmbeddedPerlInterpreter::reset(){
	shutdown();
}

/**
 * Shutsdown the perl interpreter and then restarts it for you.  
 * Returns false if the perl object could not be restarted.
 */
bool EmbeddedPerlInterpreter::restart(){

	shutdown();
	init();
		
	return true;
}

Data *GlobalPerlReturn;

void setPerlReturn(Data *pr) {
	if(pr) {
		GlobalPerlReturn = pr;
	} else {
		mprintf("Attempt to set GlobalPerlReturn to null");
	}
}




