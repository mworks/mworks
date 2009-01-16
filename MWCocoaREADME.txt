MonkeyWorksCocoa README created March 30 2005

Q)WHAT GOES IN THIS PROJECT?

A) Anything that communicates with the MonkeyWorksCore is a good candidate to be
placed in this project as well as any graphical elements that are going to
be used in both the client and server projects.

Q) HOW DO I USE THE PREFERENCES CONTROLLER?

A) Preferences are added to the MWPreferenceWindowController via a 
plist type dictionary.  The format of the dictionary is listed below.
	Key			Value
BundleName		bundle where nib is located (nil = main bundle)
NibName			nib file to load for component
Identifier		a name for the toolbar button that is going to be made
ItemImage		the image to be used for the toolbar item
                    the type of the image must be one supported by NSImage
NibFileOwner    The name of the class that owns the NIB file with the custom
                    view.

    The dictionaries are stored in an array.  The order of the array is the
order in which toolbar items will appear on the toolbar.  This is all taken
care of by the Preference Window Controller.
--FIX ME-- NIBs should all be  owned by objects that are subclasses of
MWPreferenceViewController.
    The graphical part of the preference object should be a nib file where
file owner is a subclass of NSObject, the graphic is a custom NSView object.
By custom I do not mean that you have to subclass  NSView just that the
component name in Interface Builder (IB) is  a custom view widget. 
Add your components and outlets and  write code to control it in your
file owner class.  Compile it.  All this should go into your custom
bundle.  That is the bundle name the you will be required to pass as
BundleName.  The NibName will be the name of the nib file in your bundle.
The ControllerClass is the name of your subclass of NSObject.  Identifier
is any name that you want.  Be aware that it will be placed into a toolbar so
it should not be that long.  ViewSelector is a string that will be used as
a selector of a method that your ControllerClass implements which returns an
NSView object to be displayed in the preferences pane. Item
image should be the filename (no extension needs to be supplied) of an image
in your bundle that will be used as the image on the toolbar.  You must supply
an image or an error will be thrown.  Also the image must be one supported 
by the NSImage class (tiff is the easiest). Size of the view should be 458x332
if you don't want it to be stretched.

Q) HOW DOES THE MESSAGES CONTROLLER WORK?

A) You write a GUI on the user end that has a spot for a custom view object.
You load the code for the StandardMessagesView and use the MWCustomViewProtocol
methods to get at the premade view and insert it into your view.  Using
the MWMessagesController that you created when loading the nib you must
give it a MWEventDispatchManager that will send it message events.  This is
all you have to do if you want to use the default behavior.  The default 
behavior  will place the messages it receives in the view and use different
colors for error messages.  If this behavior is not good enough for your needs
then you need to create your own NIB file with a custom controller and insert
that whereever you need a message view.   The custom controller must conform
to the MWMessagesProtocol.
--------------------------- NOTE 8/11/05 ------------------------
The message controller has been replaced by the console window.  At this date
the server is still using the old window but that is getting hacked out soon
and a version of the console window will probably be in its place.
-------------------------- END NOTE -----------------------------

--------------------------- NOTE 9/8/05 ------------------------
Added notifications to the system event dispatcher, one for each system
event.  The dispatcher will now generate a more specific notification depending
on the payload type of the event instead of issuing a generic system event
notification.  This should reduce all the if/else statements that are strewn about
the client and server code.  It also should make it easier to change event structure.
All the ifs are essentially replaced by a big case statement in one object. 
-------------------------- END NOTE -----------------------------