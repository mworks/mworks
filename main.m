//
//  main.m
//  New Client
//
//  Created by David Cox on 3/10/08.
//  Copyright Harvard University 2008. All rights reserved.
//


#import "MWorksCore/StandardClientCoreBuilder.h"
#import "MWorksCore/CoreBuilderForeman.h"
#import "MWorksCore/Exceptions.h"

#import <Cocoa/Cocoa.h>


int main(int argc, char *argv[])
{

    // -----------------------------
    // Initialize the core
    // -----------------------------
    try{
        mw::CoreBuilderForeman::constructCoreStandardOrder(new mw::StandardClientCoreBuilder());
    } catch(mw::SimpleException& e){
        // do nothing
    }
    
    
    return NSApplicationMain(argc,  (const char **) argv);
    
}
