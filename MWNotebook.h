//
//  MWNotebook.h
//  New Client
//
//  Created by David Cox on 5/30/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface MWNotebook : NSObject {

    NSMutableString *content;
    
    // log file
    NSMutableString *logFilePath;
    NSFileHandle *logFile;
}

@property(readonly) NSString *content;
@property(readonly) NSString *logFilePath;


- (id)init;
- (void) addEntry:(NSString *)entry;

@end
