//
//  MWClientPluginViewController.m
//  MWorksCocoa
//
//  Created by Christopher Stawarz on 5/26/20.
//

#import "MWClientPluginViewController.h"


NS_ASSUME_NONNULL_BEGIN


@interface MWClientPluginViewControllerStoredPropertyInfo : NSObject

- (instancetype)initWithDefaultsKey:(NSString *)defaultsKey
                       workspaceKey:(NSString *)workspaceKey
                       handleEvents:(BOOL)handleEvents;

@property(nonatomic, readonly) NSString *defaultsKey;
@property(nonatomic, readonly) NSString *workspaceKey;
@property(nonatomic, readonly) BOOL handleEvents;

@end


NS_ASSUME_NONNULL_END


@implementation MWClientPluginViewControllerStoredPropertyInfo


- (instancetype)initWithDefaultsKey:(NSString *)defaultsKey
                       workspaceKey:(NSString *)workspaceKey
                       handleEvents:(BOOL)handleEvents
{
    self = [super init];
    if (self) {
        _defaultsKey = [defaultsKey copy];
        _workspaceKey = [workspaceKey copy];
        _handleEvents = handleEvents;
    }
    return self;
}


@end


@implementation MWClientPluginViewController {
    NSMutableDictionary<NSString *, MWClientPluginViewControllerStoredPropertyInfo *> *storedProperties;
}


+ (NSViewController *)viewControllerWithClient:(id<MWKClient>)client {
    return [[self alloc] initWithClient:client];
}


- (instancetype)initWithClient:(id<MWKClient>)client {
    Class class = [self class];
    self = [super initWithNibName:NSStringFromClass(class) bundle:[NSBundle bundleForClass:class]];
    if (self) {
        _client = client;
        storedProperties = [NSMutableDictionary dictionary];
    }
    return self;
}


- (id<NSObject>)registerStoredPropertyWithName:(NSString *)name
                                   defaultsKey:(NSString *)defaultsKey
{
    return [self registerStoredPropertyWithName:name
                                    defaultsKey:defaultsKey
                                   handleEvents:NO];
}


- (id<NSObject>)registerStoredPropertyWithName:(NSString *)name
                                   defaultsKey:(NSString *)defaultsKey
                                  workspaceKey:(NSString *)workspaceKey
{
    return [self registerStoredPropertyWithName:name
                                    defaultsKey:defaultsKey
                                   workspaceKey:workspaceKey
                                   handleEvents:NO];
}


- (id<NSObject>)registerStoredPropertyWithName:(NSString *)name
                                   defaultsKey:(NSString *)defaultsKey
                                  handleEvents:(BOOL)handleEvents
{
    return [self registerStoredPropertyWithName:name
                                    defaultsKey:defaultsKey
                                   workspaceKey:name
                                   handleEvents:handleEvents];
}


- (id<NSObject>)registerStoredPropertyWithName:(NSString *)name
                                   defaultsKey:(NSString *)defaultsKey
                                  workspaceKey:(NSString *)workspaceKey
                                  handleEvents:(BOOL)handleEvents
{
    id info = [[MWClientPluginViewControllerStoredPropertyInfo alloc] initWithDefaultsKey:defaultsKey
                                                                             workspaceKey:workspaceKey
                                                                             handleEvents:handleEvents];
    storedProperties[name] = info;
    return info;
}


- (NSString *)eventCallbackKeyWithLabel:(NSString *)label {
    return [NSString stringWithFormat:@"%@ %@ callback key", NSStringFromClass([self class]), label];
}


- (void)registerEventCallbacksForStoredProperties {
    for (NSString *name in storedProperties) {
        MWClientPluginViewControllerStoredPropertyInfo *info = storedProperties[name];
        if (info.handleEvents) {
            [self registerEventCallbackForStoredPropertyWithName:name
                                                           value:[self valueForKey:name]
                                                            info:info];
        }
    }
}


- (void)registerEventCallbackForStoredPropertyWithName:(NSString *)name
                                                 value:(nullable NSString *)value
                                                  info:(MWClientPluginViewControllerStoredPropertyInfo *)info
{
    NSString *key = [self eventCallbackKeyWithLabel:[NSString stringWithFormat:@"%@ property", name]];
    [self.client unregisterCallbacksWithKey:key];
    if (value && [value isKindOfClass:[NSString class]] && value.length > 0) {
        NSInteger code = [self.client codeForTag:value];
        if (code < 0) {
            [self postWarning:[NSString stringWithFormat:@"Can't find variable \"%@\"", value]];
        } else {
            [self.client registerCallbackWithKey:key
                                         forCode:code
                                        callback:^(MWKEvent *event) {
                [self handleEvent:event forStoredProperty:info];
            }];
        }
    }
}


- (void)handleEvent:(MWKEvent *)event forStoredProperty:(id)property {
    // Default implementation does nothing
}


static void * const storedPropertyContext = (void *)(&storedPropertyContext);


- (void)viewDidLoad {
    [super viewDidLoad];
    
    NSUserDefaults *defaults = NSUserDefaults.standardUserDefaults;
    for (NSString *name in storedProperties) {
        id defaultValue = [defaults objectForKey:storedProperties[name].defaultsKey];
        if (defaultValue) {
            [self setValue:defaultValue forKey:name];
        }
        [self addObserver:self forKeyPath:name options:NSKeyValueObservingOptionNew context:storedPropertyContext];
    }
    
    [self registerEventCallbacksForStoredProperties];
    [self.client registerCallbackWithKey:[self eventCallbackKeyWithLabel:@"codec"]
                                 forCode:MWKReservedEventCodeCodec
                                callback:^(MWKEvent *event) { [self registerEventCallbacksForStoredProperties]; }];
}


- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary<NSKeyValueChangeKey, id> *)change
                       context:(void *)context
{
    if (context != storedPropertyContext) {
        [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
        return;
    }
    
    NSString *name = keyPath;
    id value = change[NSKeyValueChangeNewKey];
    MWClientPluginViewControllerStoredPropertyInfo *info = storedProperties[name];
    
    [NSUserDefaults.standardUserDefaults setObject:value forKey:info.defaultsKey];
    if (info.handleEvents) {
        [self registerEventCallbackForStoredPropertyWithName:name value:value info:info];
    }
}


- (void)dealloc {
    for (NSString *name in storedProperties) {
        [self removeObserver:self forKeyPath:name context:storedPropertyContext];
    }
}


- (NSDictionary *)workspaceState {
    NSMutableDictionary *workspaceState = [NSMutableDictionary dictionary];
    for (NSString *name in storedProperties) {
        workspaceState[storedProperties[name].workspaceKey] = [self valueForKey:name];
    }
    return workspaceState;
}


- (void)setWorkspaceState:(NSDictionary *)workspaceState {
    for (NSString *name in storedProperties) {
        id value = workspaceState[storedProperties[name].workspaceKey];
        if (value) {
            [self setValue:value forKey:name];
        }
    }
}


- (void)postMessage:(NSString *)message {
    [self.client logMessage:[NSString stringWithFormat:@"%@: %@", self.title, message]];
}


- (void)postWarning:(NSString *)warning {
    [self.client logWarning:[NSString stringWithFormat:@"%@: %@", self.title, warning]];
}


- (void)postError:(NSString *)error {
    [self.client logError:[NSString stringWithFormat:@"%@: %@", self.title, error]];
}


@end
