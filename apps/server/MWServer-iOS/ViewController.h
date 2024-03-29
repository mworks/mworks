//
//  ViewController.h
//  MWServer-iOS
//
//  Created by Christopher Stawarz on 2/17/17.
//
//

@import UIKit;


@class AppDelegate;


@interface ViewController : UIViewController <UIDocumentPickerDelegate>

@property (weak, nonatomic) AppDelegate *appDelegate;

@property (weak, nonatomic) IBOutlet UILabel *listeningAddress;
@property (weak, nonatomic) IBOutlet UILabel *listeningPort;
@property (weak, nonatomic) IBOutlet UIButton *chooseExperiment;

- (IBAction)openExperimentChooser:(id)sender;

@end
