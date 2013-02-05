#ifndef COMMON_UI_DEFS_H
#define COMMON_UI_DEFS_H

#import "MWorksCore/MessagesController.h"
#import "MWorksCore/StimMirrorController.h"
#import "MWorksCore/ParadigmVariablesController.h"
#import "MWorksCore/NetworkLogDataSource.h"

//const CFStringRef kMainNibFileName = CFSTR("ExperimentalControl");
//const CFStringRef kStatAndMessWindowName = CFSTR("StatusAndMessages");
//const CFStringRef kMenuBarNibName  = CFSTR("MenuBar");
#define kApplicationSignature 'MCMD'


BEGIN_NAMESPACE_MW


extern MessagesController *GlobalMessageController;
extern ParadigmVariablesController *GlobalParadigmVariablesController;
// set to the data source so that it picks up log messages that get sent
// before the initial opening of the log window
extern NetworkLogDataSource * GlobalNetworkLog;


END_NAMESPACE_MW


#endif