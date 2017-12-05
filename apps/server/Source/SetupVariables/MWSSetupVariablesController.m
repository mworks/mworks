//
//  MWSSetupVariablesController.m
//  MWServer
//
//  Created by Christopher Stawarz on 6/4/14.
//
//

#import "MWSSetupVariablesController.h"

#if TARGET_OS_OSX
#  import <AppKit/NSScreen.h>
#endif

#include <MWorksCore/ComponentRegistry.h>
#include <MWorksCore/PlatformDependentServices.h>
#include <MWorksCore/StandardVariables.h>
#include <MWorksCore/XMLVariableWriter.h>


@implementation MWSSetupVariablesController {
    dispatch_queue_t writeQueue;
}


- (instancetype)init {
    if ((self = [super init])) {
        writeQueue = dispatch_queue_create(NULL, DISPATCH_QUEUE_SERIAL);
        dispatch_set_target_queue(writeQueue, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW, 0));
        
        _serverName = @(mw::serverName->getValue().toString().c_str());
        
        {
            mw::Datum mdi = mw::mainDisplayInfo->getValue();
            if (mdi.isDictionary()) {
                _displayToUse = [[NSNumber alloc] initWithLong:(setDefaultAndGetValue(mdi, M_DISPLAY_TO_USE_KEY, -1).getInteger() + 1)];
                _displayWidth = @(mdi.getElement(M_DISPLAY_WIDTH_KEY).getFloat());
                _displayHeight = @(mdi.getElement(M_DISPLAY_HEIGHT_KEY).getFloat());
                _displayDistance = @(mdi.getElement(M_DISPLAY_DISTANCE_KEY).getFloat());
                _displayRefreshRateHz = @(mdi.getElement(M_REFRESH_RATE_KEY).getFloat());
                _alwaysDisplayMirrorWindow = setDefaultAndGetValue(mdi, M_ALWAYS_DISPLAY_MIRROR_WINDOW_KEY, false).getBool();
                _mirrorWindowBaseHeight = @(mdi.getElement(M_MIRROR_WINDOW_BASE_HEIGHT_KEY).getFloat());
                _announceIndividualStimuli = setDefaultAndGetValue(mdi, M_ANNOUNCE_INDIVIDUAL_STIMULI_KEY, true).getBool();
                _renderAtFullResolution = setDefaultAndGetValue(mdi, M_RENDER_AT_FULL_RESOLUTION, true).getBool();
                
                // Save any defaults we added.  Note that we need to serialize the changes, because
                // setup_variables.xml is re-read from disk when an experiment is loaded.
                [self updateVariable:mw::mainDisplayInfo value:mdi];
            }
        }
        
        _warnOnSkippedRefresh = (BOOL)(mw::warnOnSkippedRefresh->getValue().getBool());
        _stopOnError = (BOOL)(mw::stopOnError->getValue().getBool());
        _allowAltFailover = (BOOL)(mw::alt_failover->getValue().getBool());
    }
    
    return self;
}


static mw::Datum setDefaultAndGetValue(mw::Datum &dict, const char *key, const mw::Datum &defaultValue) {
    if (dict.hasKey(key)) {
        return dict.getElement(key);
    }
    dict.addElement(key, defaultValue);
    return defaultValue;
}


- (void)setServerName:(NSString *)serverName {
    _serverName = [serverName copy];
    [self updateVariable:mw::serverName value:serverName.UTF8String];
}


- (NSArray *)availableDisplays {
#if TARGET_OS_IPHONE
    return @[ @"Main display" ];
#else
    NSMutableArray *displays = [NSMutableArray arrayWithArray:@[ @"Mirror window only", @"Main display" ]];
    
    NSArray *screens = [NSScreen screens];
    // Index 0 is always the main display, so start iteration at index 1
    for (NSUInteger index = 1; index < screens.count; index++) {
        [displays addObject:[NSString stringWithFormat:@"Display %lu", (unsigned long)(index+1)]];
    }
    
    return displays;
#endif
}


- (void)setDisplayToUse:(NSNumber *)displayToUse {
    _displayToUse = displayToUse;
    [self updateVariable:mw::mainDisplayInfo
                     key:M_DISPLAY_TO_USE_KEY
                   value:(displayToUse.longValue - 1)];
}


- (void)setDisplayWidth:(NSNumber *)displayWidth {
    _displayWidth = displayWidth;
    [self updateVariable:mw::mainDisplayInfo
                     key:M_DISPLAY_WIDTH_KEY
                   value:displayWidth.doubleValue];
}


- (void)setDisplayHeight:(NSNumber *)displayHeight {
    _displayHeight = displayHeight;
    [self updateVariable:mw::mainDisplayInfo
                     key:M_DISPLAY_HEIGHT_KEY
                   value:displayHeight.doubleValue];
}


- (void)setDisplayDistance:(NSNumber *)displayDistance {
    _displayDistance = displayDistance;
    [self updateVariable:mw::mainDisplayInfo
                     key:M_DISPLAY_DISTANCE_KEY
                   value:displayDistance.doubleValue];
}


- (void)setDisplayRefreshRateHz:(NSNumber *)displayRefreshRateHz {
    _displayRefreshRateHz = displayRefreshRateHz;
    [self updateVariable:mw::mainDisplayInfo
                     key:M_REFRESH_RATE_KEY
                   value:displayRefreshRateHz.doubleValue];
}


- (void)setAlwaysDisplayMirrorWindow:(BOOL)alwaysDisplayMirrorWindow {
    _alwaysDisplayMirrorWindow = alwaysDisplayMirrorWindow;
    [self updateVariable:mw::mainDisplayInfo
                     key:M_ALWAYS_DISPLAY_MIRROR_WINDOW_KEY
                   value:bool(alwaysDisplayMirrorWindow)];
}


- (void)setMirrorWindowBaseHeight:(NSNumber *)mirrorWindowBaseHeight {
    _mirrorWindowBaseHeight = mirrorWindowBaseHeight;
    [self updateVariable:mw::mainDisplayInfo
                     key:M_MIRROR_WINDOW_BASE_HEIGHT_KEY
                   value:mirrorWindowBaseHeight.doubleValue];
}


- (void)setAnnounceIndividualStimuli:(BOOL)announceIndividualStimuli {
    _announceIndividualStimuli = announceIndividualStimuli;
    [self updateVariable:mw::mainDisplayInfo
                     key:M_ANNOUNCE_INDIVIDUAL_STIMULI_KEY
                   value:bool(announceIndividualStimuli)];
}


- (void)setRenderAtFullResolution:(BOOL)renderAtFullResolution {
    _renderAtFullResolution = renderAtFullResolution;
    [self updateVariable:mw::mainDisplayInfo
                     key:M_RENDER_AT_FULL_RESOLUTION
                   value:bool(renderAtFullResolution)];
}


- (void)setWarnOnSkippedRefresh:(BOOL)warnOnSkippedRefresh {
    _warnOnSkippedRefresh = warnOnSkippedRefresh;
    [self updateVariable:mw::warnOnSkippedRefresh value:bool(warnOnSkippedRefresh)];
}


- (void)setStopOnError:(BOOL)stopOnError {
    _stopOnError = stopOnError;
    [self updateVariable:mw::stopOnError value:bool(stopOnError)];
}


- (void)setAllowAltFailover:(BOOL)allowAltFailover {
    _allowAltFailover = allowAltFailover;
    [self updateVariable:mw::alt_failover value:bool(allowAltFailover)];
}


- (void)updateVariable:(const mw::VariablePtr &)var value:(const mw::Datum &)value {
    // Note:  We should always set and serialize the new value, even if it's equal to the
    // current value, because we don't know whether the current value was read from
    // setup_variables.xml or set at run time, and changes made via this interface are
    // always intended to be persistent.
    var->setValue(value);
    [self writeSetupVariables];
}


- (void)updateVariable:(const mw::VariablePtr &)var key:(const char *)key value:(const mw::Datum &)value {
    mw::Datum dict = var->getValue();
    if (!dict.isDictionary()) {
        dict = mw::Datum(mw::M_DICTIONARY, 1);
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
                mw::stopOnError,
                mw::alt_failover,
                mw::realtimeComponents,
            }, setupVariablesFile);
        } catch (const std::exception &e) {
            mw::merror(mw::M_SERVER_MESSAGE_DOMAIN, "Failed to save setup variables: %s", e.what());
        }
    });
}


@end

























