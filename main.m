//
//  main.m
//  New Client
//
//  Created by David Cox on 3/10/08.
//  Copyright __MyCompanyName__ 2008. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "MonkeyWorksCore/StandardClientCoreBuilder.h"
#import "MonkeyWorksCore/CoreBuilderForeman.h"
#import "MonkeyWorksCore/Exceptions.h"

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
