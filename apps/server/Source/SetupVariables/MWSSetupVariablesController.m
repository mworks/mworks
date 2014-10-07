//
//  MWSSetupVariablesController.m
//  MWServer
//
//  Created by Christopher Stawarz on 6/4/14.
//
//

#import "MWSSetupVariablesController.h"

#import <AppKit/NSScreen.h>

#include <MWorksCore/ComponentRegistry.h>
#include <MWorksCore/PlatformDependentServices.h>
#include <MWorksCore/StandardVariables.h>
#include <MWorksCore/XMLVariableWriter.h>

using mw::Datum;


@implementation MWSSetupVariablesController

@synthesize serverName = _serverName;
@synthesize displayToUse = _displayToUse;
@synthesize displayWidth = _displayWidth;
@synthesize displayHeight = _displayHeight;
@synthesize displayDistance = _displayDistance;
@synthesize alwaysDisplayMirrorWindow = _alwaysDisplayMirrorWindow;
@synthesize mirrorWindowBaseHeight = _mirrorWindowBaseHeight;
@synthesize announceIndividualStimuli = _announceIndividualStimuli;
@synthesize warnOnSkippedRefresh = _warnOnSkippedRefresh;
@synthesize allowAltFailover = _allowAltFailover;
@synthesize useHighPrecisionClock = _useHighPrecisionClock;


- (instancetype)init {
    if ((self = [super init])) {
        writeQueue = dispatch_queue_create(NULL, DISPATCH_QUEUE_SERIAL);
        dispatch_set_target_queue(writeQueue, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW, 0));
        
        _serverName = [[NSString alloc] initWithUTF8String:(mw::serverName->getValue().toString().c_str())];
        
        {
            Datum mdi = mw::mainDisplayInfo->getValue();
            if (mdi.isDictionary()) {
                _displayToUse = [[NSNumber alloc] initWithLong:(mdi.getElement(M_DISPLAY_TO_USE_KEY).getInteger() + 1)];
                _displayWidth = [[NSNumber alloc] initWithDouble:(mdi.getElement(M_DISPLAY_WIDTH_KEY).getFloat())];
                _displayHeight = [[NSNumber alloc] initWithDouble:(mdi.getElement(M_DISPLAY_HEIGHT_KEY).getFloat())];
                _displayDistance = [[NSNumber alloc] initWithDouble:(mdi.getElement(M_DISPLAY_DISTANCE_KEY).getFloat())];
                _alwaysDisplayMirrorWindow = mdi.getElement(M_ALWAYS_DISPLAY_MIRROR_WINDOW_KEY).getBool();
                _mirrorWindowBaseHeight = [[NSNumber alloc] initWithDouble:(mdi.getElement(M_MIRROR_WINDOW_BASE_HEIGHT_KEY).getFloat())];
                _announceIndividualStimuli = mdi.getElement(M_ANNOUNCE_INDIVIDUAL_STIMULI_KEY).getBool();
            }
        }
        
        _warnOnSkippedRefresh = (BOOL)(mw::warnOnSkippedRefresh->getValue().getBool());
        _allowAltFailover = (BOOL)(mw::alt_failover->getValue().getBool());
        
        {
            Datum rtc = mw::realtimeComponents->getValue();
            if (rtc.isDictionary()) {
                Datum clockValue = rtc.getElement(M_REALTIME_CLOCK_KEY);
                if (clockValue.isString() && clockValue == "HighPrecisionClock") {
                    _useHighPrecisionClock = YES;
                }
            }
        }
    }
    
    return self;
}


- (void)dealloc {
    [_mirrorWindowBaseHeight release];
    [_displayDistance release];
    [_displayHeight release];
    [_displayWidth release];
    [_displayToUse release];
    [_serverName release];
    
    dispatch_release(writeQueue);
    
    [super dealloc];
}


- (void)setServerName:(NSString *)serverName {
    if (_serverName != serverName) {
        [_serverName release];
        _serverName = [serverName copy];
    }
    [self updateVariable:mw::serverName value:[serverName UTF8String]];
}


- (NSArray *)availableDisplays {
    NSMutableArray *displays = [NSMutableArray arrayWithArray:@[ @"Mirror window only", @"Main display" ]];
    
    NSArray *screens = [NSScreen screens];
    // Index 0 is always the main display, so start iteration at index 1
    for (NSUInteger index = 1; index < [screens count]; index++) {
        [displays addObject:[NSString stringWithFormat:@"Display %lu", (unsigned long)(index+1)]];
    }
    
    return displays;
}


- (void)setDisplayToUse:(NSNumber *)displayToUse {
    if (_displayToUse != displayToUse) {
        [_displayToUse release];
        _displayToUse = [displayToUse retain];
    }
    [self updateVariable:mw::mainDisplayInfo
                     key:M_DISPLAY_TO_USE_KEY
                   value:([displayToUse longValue] - 1)];
}


- (void)setDisplayWidth:(NSNumber *)displayWidth {
    if (_displayWidth != displayWidth) {
        [_displayWidth release];
        _displayWidth = [displayWidth retain];
    }
    [self updateVariable:mw::mainDisplayInfo
                     key:M_DISPLAY_WIDTH_KEY
                   value:[displayWidth doubleValue]];
}


- (void)setDisplayHeight:(NSNumber *)displayHeight {
    if (_displayHeight != displayHeight) {
        [_displayHeight release];
        _displayHeight = [displayHeight retain];
    }
    [self updateVariable:mw::mainDisplayInfo
                     key:M_DISPLAY_HEIGHT_KEY
                   value:[displayHeight doubleValue]];
}


- (void)setDisplayDistance:(NSNumber *)displayDistance {
    if (_displayDistance != displayDistance) {
        [_displayDistance release];
        _displayDistance = [displayDistance retain];
    }
    [self updateVariable:mw::mainDisplayInfo
                     key:M_DISPLAY_DISTANCE_KEY
                   value:[displayDistance doubleValue]];
}


- (void)setAlwaysDisplayMirrorWindow:(BOOL)alwaysDisplayMirrorWindow {
    _alwaysDisplayMirrorWindow = alwaysDisplayMirrorWindow;
    [self updateVariable:mw::mainDisplayInfo
                     key:M_ALWAYS_DISPLAY_MIRROR_WINDOW_KEY
                   value:bool(alwaysDisplayMirrorWindow)];
}


- (void)setMirrorWindowBaseHeight:(NSNumber *)mirrorWindowBaseHeight {
    if (_mirrorWindowBaseHeight != mirrorWindowBaseHeight) {
        [_mirrorWindowBaseHeight release];
        _mirrorWindowBaseHeight = [mirrorWindowBaseHeight retain];
    }
    [self updateVariable:mw::mainDisplayInfo
                     key:M_MIRROR_WINDOW_BASE_HEIGHT_KEY
                   value:[mirrorWindowBaseHeight doubleValue]];
}


- (void)setAnnounceIndividualStimuli:(BOOL)announceIndividualStimuli {
    _announceIndividualStimuli = announceIndividualStimuli;
    [self updateVariable:mw::mainDisplayInfo
                     key:M_ANNOUNCE_INDIVIDUAL_STIMULI_KEY
                   value:bool(announceIndividualStimuli)];
}


- (void)setWarnOnSkippedRefresh:(BOOL)warnOnSkippedRefresh {
    _warnOnSkippedRefresh = warnOnSkippedRefresh;
    [self updateVariable:mw::warnOnSkippedRefresh value:bool(warnOnSkippedRefresh)];
}


- (void)setAllowAltFailover:(BOOL)allowAltFailover {
    _allowAltFailover = allowAltFailover;
    [self updateVariable:mw::alt_failover value:bool(allowAltFailover)];
}


- (BOOL)highPrecisionClockAvailable {
    return mw::ComponentRegistry::getSharedRegistry()->hasFactory("HighPrecisionClock");
}


- (void)setUseHighPrecisionClock:(BOOL)useHighPrecisionClock {
    _useHighPrecisionClock = useHighPrecisionClock;
    [self updateVariable:mw::realtimeComponents
                     key:M_REALTIME_CLOCK_KEY
                   value:(useHighPrecisionClock ? "HighPrecisionClock" : "MachClock")];
}


- (void)updateVariable:(const mw::VariablePtr &)var value:(const Datum &)value {
    var->setValue(value);
    [self writeSetupVariables];
}


- (void)updateVariable:(const mw::VariablePtr &)var key:(const char *)key value:(const Datum &)value {
    Datum dict = var->getValue();
    if (!dict.isDictionary()) {
        dict = Datum(mw::M_DICTIONARY, 1);
    }
    dict.addElement(key, value);
    [self updateVariable:var value:dict];
}


- (void)writeSetupVariables {
    dispatch_async(writeQueue, ^{
        try {
            boost::filesystem::path setupVariablesFile = mw::userSetupVariablesFile();
            
            // Ensure that the parent directory exists
            boost::filesystem::create_directories(setupVariablesFile.parent_path());
            
            // Write the file
            mw::XMLVariableWriter::writeVariablesToFile({
                mw::serverName,
                mw::mainDisplayInfo,
                mw::warnOnSkippedRefresh,
                mw::alt_failover,
                mw::realtimeComponents,
            }, setupVariablesFile);
        } catch (const std::exception &e) {
            mw::merror(mw::M_SERVER_MESSAGE_DOMAIN, "Failed to save setup variables: %s", e.what());
        }
    });
}


@end

























