//
//  ViewController.h
//  MWServer-iOS
//
//  Created by Christopher Stawarz on 2/17/17.
//
//

@import UIKit;


@class SceneDelegate;


@interface ViewController : UIViewController <UIDocumentPickerDelegate>

@property (weak, nonatomic) SceneDelegate *sceneDelegate;

@property (weak, nonatomic) IBOutlet UILabel *listeningAddress;
@property (weak, nonatomic) IBOutlet UILabel *listeningPort;
@property (weak, nonatomic) IBOutlet UIButton *chooseExperiment;

- (IBAction)openExperimentChooser:(id)sender;

@end
