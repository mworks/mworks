/*!
 * @header MWWindowController
 *
 * @discussion A subclass of NSWindowController, this class takes care of
 * some basic functionality that all window controllers should have.  This
 * object will save window position and frame sizes to the  standard user
 * defaults area when closing.  It also overrides the window should close
 * to always return YES but subclasses can override this behavior to perform
 * custom actions.  It also turns off window cascading.
 *
 * History:<BR></BR>
 * Paul Jankunas on 8/18/05 - Created.<BR></BR>
 *
 * @copyright MIT
 * @updated 8/18/05
 * @version 1.0.0
 */

#import <Cocoa/Cocoa.h>

@interface MWWindowController : NSWindowController {
	id delegate;
}

- (id)delegate;
- (void)setDelegate:(id)_delegate;



/*!
 * @function monkeyWorksFrameAutosaveName
 * @discussion Returns a unique name for a window frame.  The base class
 * version returns nil.
 * @result A NSString that is the name put in the preferences for the window
 */
- (NSString *)monkeyWorksFrameAutosaveName;

/*!
 * @function popWindow
 * @discussion Makes a window visible if it is not and vice versa.
 */
- (void)popWindow;

@end
