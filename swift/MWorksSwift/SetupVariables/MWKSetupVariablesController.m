//
//  MWKSetupVariablesController.m
//  MWServer
//
//  Created by Christopher Stawarz on 6/4/14.
//
//

#import "MWKSetupVariablesController.h"

#if TARGET_OS_OSX
#  import <AppKit/NSScreen.h>
#endif


@implementation MWKSetupVariablesController {
    dispatch_queue_t writeQueue;
}


- (instancetype)init {
    if ((self = [super init])) {
        writeQueue = dispatch_queue_create(NULL, DISPATCH_QUEUE_SERIAL);
        dispatch_set_target_queue(writeQueue, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW, 0));
        
        _serverName = @(mw::serverName->getValue().toString().c_str());
        
        {
            // Use getDisplayConfiguration to get the display settings, because it will set defaults for keys missing
            // from #mainScreenInfo
            const auto config = mw::StimulusDisplay::getDisplayConfiguration(mw::mainDisplayInfo->getValue());
            _displayToUse = @(config.displayToUse + 1);
            _displayWidth = @(config.width);
            _displayHeight = @(config.height);
            _displayDistance = @(config.distance);
            _displayRefreshRateHz = @(config.refreshRateHz);
            _alwaysDisplayMirrorWindow = (BOOL)(config.alwaysDisplayMirrorWindow);
            _mirrorWindowBaseHeight = @(config.mirrorWindowBaseHeight);
            _useColorManagement = (BOOL)(config.useColorManagement);
            _setDisplayGamma = (BOOL)(config.setDisplayGamma);
            _redGamma = @(config.redGamma);
            _greenGamma = @(config.greenGamma);
            _blueGamma = @(config.blueGamma);
            _makeWindowOpaque = (BOOL)(config.makeWindowOpaque);
        }
        
        _warnOnSkippedRefresh = (BOOL)(mw::warnOnSkippedRefresh->getValue().getBool());
        _stopOnError = (BOOL)(mw::stopOnError->getValue().getBool());
        _allowAltFailover = (BOOL)(mw::alt_failover->getValue().getBool());
    }
    
    return self;
}


- (void)setServerName:(NSString *)serverName {
    _serverName = [serverName copy];
    [self updateVariable:mw::serverName value:serverName.UTF8String];
}


- (NSArray<NSString *> *)availableDisplays {
    NSMutableArray<NSString *> *displays = [NSMutableArray arrayWithObject:@"Mirror window only"];
    
#if !TARGET_OS_OSX
    [displays addObject:@"Main display"];
#else
    for (NSScreen *screen in NSScreen.screens) {
        [displays addObject:screen.localizedName];
    }
#endif
    
    return displays;
}


- (void)setDisplayToUse:(NSNumber *)displayToUse {
    _displayToUse = displayToUse;
    [self updateVariable:mw::mainDisplayInfo
                     key:M_DISPLAY_TO_USE_KEY
                   value:(displayToUse.intValue - 1)];
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


- (void)setUseColorManagement:(BOOL)useColorManagement {
    _useColorManagement = useColorManagement;
    [self updateVariable:mw::mainDisplayInfo
                     key:M_USE_COLOR_MANAGEMENT_KEY
                   value:bool(useColorManagement)];
}


- (void)setSetDisplayGamma:(BOOL)setDisplayGamma {
    _setDisplayGamma = setDisplayGamma;
    [self updateVariable:mw::mainDisplayInfo
                     key:M_SET_DISPLAY_GAMMA_KEY
                   value:bool(setDisplayGamma)];
}


- (void)setRedGamma:(NSNumber *)redGamma {
    _redGamma = redGamma;
    [self updateVariable:mw::mainDisplayInfo
                     key:M_DISPLAY_GAMMA_RED_KEY
                   value:redGamma.doubleValue];
}


- (void)setGreenGamma:(NSNumber *)greenGamma {
    _greenGamma = greenGamma;
    [self updateVariable:mw::mainDisplayInfo
                     key:M_DISPLAY_GAMMA_GREEN_KEY
                   value:greenGamma.doubleValue];
}


- (void)setBlueGamma:(NSNumber *)blueGamma {
    _blueGamma = blueGamma;
    [self updateVariable:mw::mainDisplayInfo
                     key:M_DISPLAY_GAMMA_BLUE_KEY
                   value:blueGamma.doubleValue];
}


- (void)setMakeWindowOpaque:(BOOL)makeWindowOpaque {
    _makeWindowOpaque = makeWindowOpaque;
    [self updateVariable:mw::mainDisplayInfo
                     key:M_MAKE_WINDOW_OPAQUE_KEY
                   value:bool(makeWindowOpaque)];
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
