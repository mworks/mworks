//
//  MWNotebook.m
//  New Client
//
//  Created by David Cox on 5/30/08.
//  Copyright 2008 Harvard University. All rights reserved.
//

#import "MWNotebook.h"
#import "MworksCore/PlatformDependentServices.h"


@implementation MWNotebook

@synthesize content;
@synthesize logFilePath;

- (id)init {
    content = [[NSMutableString alloc] init];

    NSString *logFileName = [[NSDate date] descriptionWithCalendarFormat:@"%Y%m%d_%H%M%S%F.log"
                                                                timeZone:Nil
                                                                  locale:Nil];

    logFilePath = [[NSString alloc]
                   initWithCString:(mw::prependDataFilePath([logFileName UTF8String]).string().c_str())];
    
    NSLog(@"log file path: %@", logFilePath);

    if (![[NSFileManager defaultManager] createFileAtPath:logFilePath contents:Nil attributes:Nil]) {
        NSLog(@"log file creation failed");
    } else {
        logFile = [[NSFileHandle fileHandleForWritingAtPath:logFilePath] retain];
    }

    return self;
}

- (void) addEntry:(NSString *)entry{
    
    NSMutableString *full_entry = [[NSMutableString alloc] init];
    
    NSString *time_string = [[NSDate date] descriptionWithCalendarFormat:@"<%H:%M:%S>: "
                                           timeZone:Nil
                                           locale:Nil];
    
    
    [full_entry appendString:time_string];
    [full_entry appendString:entry];
    [full_entry appendString:@"\n"];
    
    [self willChangeValueForKey:@"content"];
    [content appendString:full_entry];
    [self didChangeValueForKey:@"content"];
    
    [logFile writeData:[full_entry dataUsingEncoding: NSASCIIStringEncoding]];
    [full_entry release];
}

- (void)dealloc {
    [logFile release];
    [logFilePath release];
    [content release];
    [super dealloc];
}


@end
