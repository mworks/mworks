//
//  MWNotebook.m
//  New Client
//
//  Created by David Cox on 5/30/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "MWNotebook.h"


@implementation MWNotebook

@synthesize content;
@synthesize logFilePath;

- (id)init {
    content = [[NSMutableString alloc] init];

    logFilePath = [[NSMutableString alloc] init];
    
    [logFilePath appendString:@"/Library/MonkeyWorks/DataFiles/"];

    [logFilePath appendString:[[NSDate date] descriptionWithCalendarFormat:@"%Y%m%d_%H%M%S%F.log"
                                           timeZone:Nil
                                           locale:Nil]];
    
    NSLog(@"%@",logFilePath);

    [[NSFileManager defaultManager] createFileAtPath:logFilePath contents:Nil attributes:Nil];
    logFile = [NSFileHandle fileHandleForWritingAtPath:logFilePath];
    return self;
}

- (void) addEntry:(NSString *)entry{
    
    [self willChangeValueForKey:@"content"];
    
    NSMutableString *full_entry = [[NSMutableString alloc] init];
    
    NSString *time_string = [[NSDate date] descriptionWithCalendarFormat:@"<%H:%M:%S>: "
                                           timeZone:Nil
                                           locale:Nil];
    
    
    [full_entry appendString:time_string];
    [full_entry appendString:entry];
    [full_entry appendString:@"\n"];
    
    [content appendString:full_entry];
    [self didChangeValueForKey:@"content"];
    
    [logFile writeData:[full_entry dataUsingEncoding: NSASCIIStringEncoding]];
    [full_entry finalize];
}




@end
