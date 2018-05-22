//
//  ViewController.m
//  MWServer-iOS
//
//  Created by Christopher Stawarz on 2/17/17.
//
//

#import "ViewController.h"

@import MobileCoreServices;

#import "AppDelegate.h"


@implementation ViewController


- (void)viewWillAppear:(BOOL)animated {
    self.listeningAddress.text = APP_DELEGATE.listeningAddress;
    self.listeningPort.text = APP_DELEGATE.listeningPort.stringValue;
    
    [super viewWillAppear:animated];
    
    [self updateSetupVariablesForSize:self.view.bounds.size];
}


- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    
    if (APP_DELEGATE.alert) {
        [self presentViewController:APP_DELEGATE.alert animated:YES completion:nil];
    }
}


- (void)viewWillTransitionToSize:(CGSize)size
       withTransitionCoordinator:(id<UIViewControllerTransitionCoordinator>)coordinator
{
    [super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];
    [self updateSetupVariablesForSize:size];
}


- (void)updateSetupVariablesForSize:(CGSize)size {
    //
    // If needed, swap display width and height to match the current device orientation
    //
    
    MWSSetupVariablesController *setupVariablesController = APP_DELEGATE.setupVariablesController;
    double width = setupVariablesController.displayWidth.doubleValue;
    double height = setupVariablesController.displayHeight.doubleValue;
    
    if ((width > height && size.width < size.height) ||
        (width < height && size.width > size.height))
    {
        setupVariablesController.displayWidth = @(height);
        setupVariablesController.displayHeight = @(width);
    }
}


- (IBAction)openExperimentChooser:(id)sender {
    UIDocumentPickerViewController *controller =
    [[UIDocumentPickerViewController alloc] initWithDocumentTypes:@[(NSString *)kUTTypeItem]
                                                           inMode:UIDocumentPickerModeImport];
    
    controller.delegate = self;
    controller.allowsMultipleSelection = YES;
    [self presentViewController:controller animated:YES completion:nil];
}


- (void)documentPicker:(UIDocumentPickerViewController *)controller didPickDocumentsAtURLs:(NSArray<NSURL *> *)urls {
    NSIndexSet *xmlIndexes = [urls indexesOfObjectsPassingTest:^BOOL(NSURL *url, NSUInteger index, BOOL *stop) {
        return [url.pathExtension isEqualToString:@"xml"];
    }];
    if (xmlIndexes.count == 0) {
        [self presentExperimentChooserAlertWithTitle:@"No XML file found"
                                             message:@"Please include an MWorks experiment XML file in your selection"];
        return;
    }
    if (xmlIndexes.count > 1) {
        [self presentExperimentChooserAlertWithTitle:@"Multiple XML files found"
                                             message:@"Please include only one MWorks experiment XML file in your selection"];
        return;
    }
    
    NSURL *xmlURL = urls[xmlIndexes.firstIndex];
    [APP_DELEGATE openExperiment:xmlURL.path completionHandler:^(BOOL success) {
        if (!success) {
            [self presentExperimentChooserAlertWithTitle:@"Experiment loading failed"
                                                 message:(@"Please include a valid MWorks experiment XML file and all required"
                                                          " supporting files (images, sounds, etc.) in your selection")];
        }
    }];
}


- (void)presentExperimentChooserAlertWithTitle:(NSString *)title message:(NSString *)message {
    UIAlertController *alert = [UIAlertController alertControllerWithTitle:title
                                                                   message:message
                                                            preferredStyle:UIAlertControllerStyleAlert];
    [alert addAction:[UIAlertAction actionWithTitle:@"OK"
                                              style:UIAlertActionStyleDefault
                                            handler:^(UIAlertAction *action) {
                                                [self dismissViewControllerAnimated:YES completion:nil];
                                            }]];
    [self presentViewController:alert animated:YES completion:nil];
}


@end



























