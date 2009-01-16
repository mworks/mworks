#ifndef COMMON_UI_DEFS_H
#define COMMON_UI_DEFS_H

#import "MonkeyWorksCore/MessagesController.h"
#import "MonkeyWorksCore/StimMirrorController.h"
#import "MonkeyWorksCore/ParadigmVariablesController.h"
#import "MonkeyWorksCore/NetworkLogDataSource.h"

//const CFStringRef kMainNibFileName = CFSTR("ExperimentalControl");
//const CFStringRef kStatAndMessWindowName = CFSTR("StatusAndMessages");
//const CFStringRef kMenuBarNibName  = CFSTR("MenuBar");
#define kApplicationSignature 'MCMD'
namespace mw {
extern MessagesController *GlobalMessageController;
extern ParadigmVariablesController *GlobalParadigmVariablesController;
// set to the data source so that it picks up log messages that get sent
// before the initial opening of the log window
extern NetworkLogDataSource * GlobalNetworkLog;
}
#endif