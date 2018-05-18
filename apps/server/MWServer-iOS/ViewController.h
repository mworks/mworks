//
//  ViewController.h
//  MWServer-iOS
//
//  Created by Christopher Stawarz on 2/17/17.
//
//

#import <UIKit/UIKit.h>


@interface ViewController : UIViewController <UIDocumentPickerDelegate>

@property (weak, nonatomic) IBOutlet UILabel *listeningAddress;
@property (weak, nonatomic) IBOutlet UILabel *listeningPort;

- (IBAction)openExperimentChooser:(id)sender;

@end

