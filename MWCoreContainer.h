//
//  MWCoreContainer.h
//  MonkeyWorksCocoa
//
//  Created by David Cox on 12/21/09.
//  Copyright 2009 Harvard University. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@protocol MWCoreContainer


-(shared_ptr<EventStreamInterface>)eventStreamInterface;

@end
